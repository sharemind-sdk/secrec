/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "typechecker/templates.h"

#include <boost/foreach.hpp>

#include "log.h"
#include "ModuleInfo.h"
#include "symbol.h"
#include "symboltable.h"
#include "treenode.h"

namespace SecreC {

namespace /* anonymous */ {

struct TemplateTypeVariable {
    TreeNodeIdentifier*  id;
    TypeVariableKind     kind;
    bool                 bound;
    bool                 ret;

    TemplateTypeVariable (TreeNodeIdentifier* id, TypeVariableKind kind)
        : id (id)
        , kind (kind)
        , bound (false)
        , ret (false)
    { }
};

typedef std::map<StringRef, TemplateTypeVariable, StringRef::FastCmp> TypeVariableMap;

TypeVariableKind typeFragmentKind (const TreeNodeTypeF& ty) {
    switch (ty.type ()) {
    case NODE_DATATYPE_VAR_F:
    case NODE_DATATYPE_CONST_F:
        return TV_DATA;
    case NODE_DIMTYPE_VAR_F:
    case NODE_DIMTYPE_CONST_F:
        return TV_DIM;
    case NODE_SECTYPE_PRIVATE_F:
    case NODE_SECTYPE_PUBLIC_F:
        return TV_SEC;
    default:
        assert (false && "Invalid tree node.");
        return TV_UNDEF;
    }
}

SymbolCategory symbolCategory (TypeVariableKind kind) {
    switch (kind) {
    case TV_SEC: return SYM_DOMAIN;
    case TV_DATA: return SYM_TYPE;
    case TV_DIM: return SYM_DIM;
    case TV_UNDEF: return SYM_UNDEFINED;
    }
}

TypeVariableKind quantifierKind (const TreeNodeQuantifier& quant) {
    switch (quant.type ()) {
    case NODE_TEMPLATE_DOMAIN_QUANT: return TV_SEC;
    case NODE_TEMPLATE_DATA_QUANT: return TV_DATA;
    case NODE_TEMPLATE_DIM_QUANT: return TV_DIM;
    default: return TV_UNDEF;
    }
}

const char* kindAsString (TypeVariableKind kind) {
    switch (kind) {
    case TV_SEC: return "domain";
    case TV_DATA: return "data";
    case TV_DIM: return "dimensionality";
    case TV_UNDEF:
        assert (false && "Invalid type variable kind.");
        return "undefined";
    }
}

TypeChecker::Status checkTypeVariable (TypeVariableMap& map, SymbolTable* st, CompileLog& log, const TreeNodeTypeF& t, bool isReturn) {
    if (! t.isVariable ())
        return TypeChecker::OK;

    const StringRef name = t.identifier ()->value ();
    const TypeVariableKind kind = typeFragmentKind (t);
    const TypeVariableMap::iterator it = map.find (name);
    if (it != map.end ()) {
        TemplateTypeVariable& tv = it->second;
        if (tv.kind != kind) {
            log.fatal () << "Unexpected " << kindAsString (kind) << " type variable \'" << name
                         << "\' at " << t.identifier ()->location () << ". "
                         << "Expecting " <<  kindAsString (tv.kind) << " type variable.";
            return TypeChecker::E_TYPE;
        }

        tv.bound = true;
        tv.ret = isReturn;
    }
    else {
        Symbol* sym = st->find (symbolCategory (kind), name);
        if (sym == NULL) {
            log.fatal () << "Unable to find " << kindAsString (kind) << " type variable \'" << name
                         << "\' at " << t.identifier ()->location () << ". ";
            return TypeChecker::E_TYPE;
        }
    }

    return TypeChecker::OK;
}

} // namespace anonymous

/*******************************************************************************
  TreeNodeTemplate
*******************************************************************************/

/**
 * Type check template definitions.
 * We do not perform two phase name lookup like C++ so at the pass over
 * template declarations we perform no type checking of the template body --
 * it only needs to parse.
 */
TypeChecker::Status TypeChecker::visit(TreeNodeTemplate * templ) {
    TreeNodeProcDef* body = templ->body ();
    TreeNodeIdentifier* id = body->identifier ();

    // Collect quantifiers:
    TypeVariableMap typeVariables;
    BOOST_FOREACH (TreeNodeQuantifier& quant, templ->quantifiers ()) {
        const StringRef name = quant.typeVariable ()->value ();
        TypeVariableMap::iterator it = typeVariables.find (name);
        if (it != typeVariables.end ()) {
            m_log.fatal ()
                    << "Redeclaration of a type variable \'" << name << '\''
                    << " at " << id->location () << '.';
            return E_TYPE;
        }

        typeVariables.insert (it, std::make_pair (name,
            TemplateTypeVariable (quant.typeVariable (), quantifierKind (quant))));

        TCGUARD (visit (&quant));
    }

    if (body->returnType ()->isNonVoid ()) {
        BOOST_FOREACH (TreeNodeTypeF& t, body->returnType ()->types ()) {
            TCGUARD (checkTypeVariable (typeVariables, m_st, m_log, t, true));
        }
    }

    BOOST_FOREACH (TreeNodeStmtDecl& decl, body->params ()) {
        BOOST_FOREACH (TreeNodeTypeF& t, decl.varType ()->types ()) {
            TCGUARD (checkTypeVariable (typeVariables, m_st, m_log, t, false));
        }
    }

    bool expectsSecType = false;
    bool expectsDataType = false;
    bool expectsDimType = false;

    std::vector<TreeNodeIdentifier*> unboundTV;
    BOOST_FOREACH (const TypeVariableMap::value_type& v, typeVariables) {
        const TemplateTypeVariable& tv = v.second;
        if (! tv.bound) {
            unboundTV.push_back (tv.id);
            continue;
        }

        if (! tv.ret) {
            continue;
        }

        if (tv.kind == TV_SEC) expectsSecType = true;
        if (tv.kind == TV_DATA) expectsDataType = true;
        if (tv.kind == TV_DIM) expectsDimType = true;
    }

    if (! unboundTV.empty()) {
        bool first = true;
        std::stringstream ss;
        BOOST_FOREACH (TreeNodeIdentifier* id, unboundTV) {
            if (! first)
                ss << ",";
            ss << " \'" << id->value () << "\' at " << id->location();
            first = false;
        }

        m_log.fatal() << "Template definition has free type variables:" << ss.str () << '.';
        return E_TYPE;
    }

    SymbolTemplate* s = new SymbolTemplate (templ, expectsSecType, expectsDataType, expectsDimType);
    s->setName (id->value ());
    m_st->appendSymbol (s);
    return OK;
}

/*******************************************************************************
  TemplateParameter
*******************************************************************************/

SymbolTypeVariable* TemplateParameter::bind (StringRef name) const {
    switch (m_kind) {
    case TV_SEC:   return new SymbolDomain (name, secType ());
    case TV_DIM:   return new SymbolDimensionality (name, dimType ());
    case TV_DATA:  return new SymbolDataType (name, dataType ());
    case TV_UNDEF: return 0;
    }
}

/*******************************************************************************
  TemplateInstantiator
*******************************************************************************/

/// \todo figure out how to delay copying of procedures even more (or completeley avoid
/// if possible).
const InstanceInfo& TemplateInstantiator::add (const Instantiation& i, ModuleInfo& mod) {
    InstanceInfoMap::iterator it = m_instanceInfo.find (i);
    if (it == m_instanceInfo.end ()) {
        TreeNodeProcDef* cloned = static_cast<TreeNodeProcDef*>(i.getTemplate ()->decl ()->body ()->clone (0));
        mod.body()->addGeneratedInstance(cloned);
        InstanceInfo info;
        SymbolTable* local = mod.codeGenState ().st ()->newScope ();
        local->setName ("Template " + i.getTemplate ()->name ());
        m_workList.push_back (i);
        info.m_generatedBody = cloned;
        info.m_moduleInfo = &mod;
        info.m_localScope = local;
        it = m_instanceInfo.insert (it, std::make_pair (i, info));

        std::vector<TemplateParameter>::const_iterator it = i.getParams ().begin ();
        BOOST_FOREACH (TreeNodeQuantifier& quant, i.getTemplate ()->decl ()->quantifiers ()) {
            StringRef qname = quant.typeVariable ()->value ();
            local->appendSymbol (it->bind (qname));
            ++ it;
        }
    }

    return it->second;
}

bool TemplateInstantiator::getForInstantiation (InstanceInfo& info) {
    while (! m_workList.empty ()) {
        Instantiation i = m_workList.front ();
        m_workList.pop_front ();
        if (! isInstantiated (i)) {
            info = m_instanceInfo[i];
            assert (info.m_generatedBody != NULL);
            assert (info.m_moduleInfo != NULL);
            assert (info.m_localScope != NULL);
            m_generated.insert (i);
            return true;
        }
    }

    return false;
}

} // namespace SecreC
