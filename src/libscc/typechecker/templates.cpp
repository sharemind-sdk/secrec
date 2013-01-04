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

/*******************************************************************************
  TreeNodeTemplate
*******************************************************************************/

/**
 * Type check template definitions.
 * We do not perform two phase name lookup like C++ so at the pass over
 * template declarations we perform no type checking of the template body --
 * it only needs to parse.
 *
 * TODO: refactor this.
 */
TypeChecker::Status TypeChecker::visit(TreeNodeTemplate * templ) {
    TreeNodeProcDef* body = templ->body ();
    TreeNodeIdentifier* id = body->identifier ();

    if (m_st->find (id->value ()) != 0) {
        m_log.fatal ()
                << "Redeclaration of template \'" << id->value () << "\'"
                << " at " << id->location () << '.';
        return E_TYPE;
    }

    // Check that quantifiers are saneley defined
    typedef std::map<StringRef, TreeNodeIdentifier*, StringRef::FastCmp> TypeVariableMap;
    typedef std::set<StringRef, StringRef::FastCmp> TypeVariableSet;

    TypeVariableSet domVariables;
    TypeVariableSet dimVariables;
    TypeVariableMap freeTypeVariables;

    BOOST_FOREACH (TreeNodeQuantifier& quant, templ->quantifiers ()) {
        StringRef typeVariable = quant.typeVariable ()->value ();
        TypeVariableMap::iterator it = freeTypeVariables.find (typeVariable);
        if (it != freeTypeVariables.end ()) {
            m_log.fatal ()
                    << "Redeclaration of a type variable \'" << typeVariable << '\''
                    << " at " << id->location () << '.';
            return E_TYPE;
        }

        freeTypeVariables.insert (it, std::make_pair (typeVariable, quant.typeVariable ()));
        switch (quant.type ()) {
        case NODE_TEMPLATE_DOMAIN_QUANT: domVariables.insert (typeVariable); break;
        case NODE_TEMPLATE_DIM_QUANT: dimVariables.insert (typeVariable); break;
        default: break;
        }

        Status status = visit (&quant);
        if (status != OK)
            return status;
    }

    // Check return type.
    TreeNodeIdentifier* retSecTyIdent = 0;
    TreeNodeIdentifier* retDimTyIdent = 0;
    bool expectsSecType = false;
    bool expectsDimType = false;

    if (body->returnType ()->isNonVoid ()) {
        TreeNodeType* t = body->returnType ();
        if (! t->secType ()->isPublic ()) {
            retSecTyIdent = t->secType ()->identifier ();
            StringRef id = retSecTyIdent->value ();
            expectsSecType = true;

            if (dimVariables.count (id) > 0) {
                m_log.fatal () << "Unexpected dimensionality type variable \'" << id
                               << "\' at " << retSecTyIdent->location () << ". "
                               << "Expecting security domain.";
                return E_TYPE;
            }

            if (domVariables.count (id) == 0) {
                if (findIdentifier(retSecTyIdent) == 0)
                    return E_TYPE;
            }

            freeTypeVariables.erase (id);
        }

        if (t->dimType ()->type () == NODE_DIMTYPE_VAR_F) {
            TreeNodeDimTypeVarF* dimType = static_cast<TreeNodeDimTypeVarF*>(t->dimType ());
            retDimTyIdent = dimType->identifier ();
            StringRef id = retDimTyIdent->value ();
            expectsDimType = true;

            if (domVariables.count (id) > 0) {
                m_log.fatal () << "Unexpected domain type variable \'" << id
                               << "\' at " << retDimTyIdent->location () << ". "
                               << "Expecting dimensionality type.";
                return E_TYPE;
            }

            if (dimVariables.count (retDimTyIdent->value ()) == 0) {
                m_log.fatal () << "Dimensionality variable \'" << retDimTyIdent->value ()
                               <<  "\' at " << retDimTyIdent->location () << " not declared.";
                return E_TYPE;
            }

            freeTypeVariables.erase (retDimTyIdent->value ());
        }
    }

    // Check that security types of parameters are either quantified or defined.
    BOOST_FOREACH (TreeNodeStmtDecl& decl, body->params ()) {
        TreeNodeType* t = decl.varType ();
        if (! t->secType ()->isPublic ()) {
            TreeNodeSecTypeF* secType = t->secType ();
            StringRef id = secType->identifier ()->value ();
            if (retSecTyIdent != 0) {
                if (id == retSecTyIdent->value ()) {
                    expectsSecType = false;
                }
            }

            if (dimVariables.count (id) > 0) {
                m_log.fatal () << "Unexpected dimensionality type variable \'" << id
                               << "\' at " << retSecTyIdent->location () << ". "
                               << "Expecting security domain.";
                return E_TYPE;
            }

            if (domVariables.count (id) == 0) {
                if (findIdentifier(secType->identifier ()) == 0)
                    return E_TYPE;
            }

            freeTypeVariables.erase (id);
        }

        if (t->dimType ()->type () == NODE_DIMTYPE_VAR_F) {
            TreeNodeDimTypeVarF* dimType = static_cast<TreeNodeDimTypeVarF*>(t->dimType ());
            StringRef id = dimType->identifier ()->value ();
            if (retDimTyIdent != 0) {
                if (id == retDimTyIdent->value ()) {
                    expectsDimType = false;
                }
            }

            if (domVariables.count (id) > 0) {
                m_log.fatal () << "Unexpected domain type variable \'" << id
                               << "\' at " << retDimTyIdent->location () << ". "
                               << "Expecting dimensionality type.";
                return E_TYPE;
            }

            if (dimVariables.count (id) == 0) {
                m_log.fatal () << "Dimensionality variable \'" << id
                               <<  "\' at " << dimType->identifier ()->location () << " not declared.";
                return E_TYPE;
            }

            freeTypeVariables.erase (id);
        }
    }

    if (!freeTypeVariables.empty()) {
        std::stringstream ss;
        BOOST_FOREACH(const TypeVariableMap::value_type& v, freeTypeVariables) {
            ss << " " << v.second->location();
        }

        m_log.fatal() << "Template definition has free type variables at" << ss.str () << '.';
        return E_TYPE;
    }

    SymbolTemplate* s = new SymbolTemplate (templ, expectsSecType, expectsDimType);
    s->setName ("{templ}" + id->value ().str());
    m_st->appendSymbol (s);
    return OK;
}

/*******************************************************************************
  TemplateParameter
*******************************************************************************/

Symbol* TemplateParameter::bind (StringRef name) const {
    switch (m_tag) {
    case SEC: return new SymbolDomain (name, secType ());
    case DIM: return new SymbolDimensionality (name, dimType ());
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
            assert (info.m_generatedBody != 0);
            assert (info.m_moduleInfo != 0);
            assert (info.m_localScope != 0);
            m_generated.insert (i);
            return true;
        }
    }

    return false;
}

} // namespace SecreC
