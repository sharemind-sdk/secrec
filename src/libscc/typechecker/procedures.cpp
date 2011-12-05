/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "typechecker.h"

#include <vector>
#include <sstream>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/foreach.hpp>

#include "templates.h"

namespace {

using namespace SecreC;

void mangleTemplateParameters (std::ostream& os,
                               const TemplateParams& targs)
{
    if (! targs.empty ()) {
        os << '(';
        bool first = true;
        BOOST_FOREACH (SecurityType* ty, targs) {
            if (! first) os << ',';
            os << ty->toString ();
            first = false;
        }

        os << ')';
    }
}

std::string mangleProcedure (const std::string& name,
                             DataTypeProcedureVoid* dt,
                             const TemplateParams& targs)
{
    std::ostringstream os;
    os << "{proc}" << name << dt->mangle ();
    mangleTemplateParameters (os, targs);
    return os.str ();
}

SymbolProcedure* appendProcedure (SymbolTable* st,
                                  const TreeNodeProcDef& procdef,
                                  const TemplateParams& targs = TemplateParams ())
{
    typedef DataTypeProcedureVoid DTPV;
    assert(procdef.procedureType()->kind() == TypeNonVoid::PROCEDURE
           || procdef.procedureType()->kind() == TypeNonVoid::PROCEDUREVOID);
    assert(dynamic_cast<DTPV*>(procdef.procedureType()->dataType()) != 0);
    DTPV* dt = static_cast<DTPV*>(procdef.procedureType()->dataType());
    const std::string name = mangleProcedure (procdef.procedureName(), dt, targs);

    SymbolProcedure* ns = static_cast<SymbolProcedure*>(st->find (name));
    if (ns == 0) {
        ns = new SymbolProcedure(&procdef);
        ns->setName (name);
        st->appendSymbol (ns);
    }

    return ns;
}

SymbolProcedure*
findProcedure (SymbolTable* st,
               const std::string& name,
               DataTypeProcedureVoid* dt,
               const TemplateParams& targs = TemplateParams ())
{
    const std::string actualName = mangleProcedure (name, dt, targs);
    SymbolProcedure* procSym = 0;
    Symbol* _procSym = st->find (actualName);
    if (_procSym != 0) {
        assert (dynamic_cast<SymbolProcedure*>(_procSym) != 0);
        procSym = static_cast<SymbolProcedure*>(_procSym);
    }

    return procSym;
}

std::list<SymbolProcedure*>
findProcedures (SymbolTable* st,
                const std::string& name,
                DataTypeProcedureVoid* dt,
                const TemplateParams& targs = TemplateParams ())
{
    std::list<SymbolProcedure* > out;
    const std::string actualName = mangleProcedure (name, dt, targs);
    BOOST_FOREACH (Symbol* _procSym, st->findAll (actualName)) {
        assert (dynamic_cast<SymbolProcedure*>(_procSym) != 0);
        SymbolProcedure* procSym = static_cast<SymbolProcedure*>(_procSym);
        out.push_back (procSym);
    }

    return out;
}

std::list<SymbolTemplate*>
findTemplates (SymbolTable* st, const std::string& name)
{
    std::list<SymbolTemplate*> out;
    BOOST_FOREACH (Symbol* _symTempl, st->findAll ("{templ}" + name)) {
        assert (dynamic_cast<SymbolTemplate*>(_symTempl) != 0);
        SymbolTemplate* symTempl = static_cast<SymbolTemplate*>(_symTempl);
        out.push_back (symTempl);
    }

    return out;
}

} // anonymous namespace

namespace SecreC {

/// Return symbol for the main procedure (if exists).
SymbolProcedure* TypeChecker::mainProcedure () {
    DataTypeProcedureVoid* ty = DataTypeProcedureVoid::get (getContext ());
    SymbolProcedure* out = 0;
    BOOST_FOREACH (SymbolProcedure* p, findProcedures (m_st, "main", ty)) {
        if (out != 0) {
            assert (false && "ICE: multiple definitions of main should not be possible.");
            return 0;
        }

        out = p;
    }

    return out;
}

ICode::Status TypeChecker::populateParamTypes (std::vector<DataType*>& params,
                                               TreeNodeProcDef* proc) {
    typedef DataTypeVar DTV;
    params.clear ();
    params.reserve (std::distance (proc->paramBegin (), proc->paramEnd ()));
    BOOST_FOREACH (TreeNode* n, std::make_pair (proc->paramBegin (),
                                                proc->paramEnd ()))
    {
        assert(n->type() == NODE_DECL);
        assert(dynamic_cast<TreeNodeStmtDecl*>(n) != 0);
        TreeNodeStmtDecl *d = static_cast<TreeNodeStmtDecl*>(n);
        ICode::Status s = visit (d);
        if (s != ICode::OK) return s;
        TypeNonVoid* pt = d->resultType();
        assert(pt->dataType()->kind() == DataType::VAR);
        assert(dynamic_cast<DTV*>(pt->dataType()) != 0);
        params.push_back (static_cast<DTV*>(pt->dataType())->dataType());
    }

    return ICode::OK;
}

/// Procedure definitions.
ICode::Status TypeChecker::visit (TreeNodeProcDef* proc) {
    typedef TypeNonVoid TNV;
    if (proc->m_cachedType == 0) {
        TreeNodeType* rt = proc->returnType ();
        ICode::Status s = visit (rt);
        if (s != ICode::OK) return s;
        std::vector<DataType*> params;
        if ((s = populateParamTypes (params, proc)) != ICode::OK) {
            return s;
        }

        DataTypeProcedureVoid* voidProcType =
                DataTypeProcedureVoid::get (getContext (), params);

        if (rt->secrecType()->isVoid()) {
            proc->m_cachedType = TypeNonVoid::get (m_context, voidProcType);
        }
        else {
            TNV* tt = static_cast<TNV*>(rt->secrecType());
            assert (tt->dataType()->kind() == DataType::BASIC);
            proc->m_cachedType = TypeNonVoid::get (m_context,
                DataTypeProcedure::get (m_context, voidProcType, tt->dataType ()));
        }

        SymbolProcedure* procSym = appendProcedure (m_st->globalScope (), *proc);
        proc->setSymbol (procSym);

        const std::string& shortName = "{proc}" + proc->identifier ()->value ();
        BOOST_FOREACH (Symbol* sym, m_st->findAll (shortName)) {
            if (sym->symbolType () == Symbol::PROCEDURE) {
                SymbolProcedure* t = static_cast<SymbolProcedure*>(sym);
                if (t->decl ()->m_cachedType == proc->m_cachedType) {
                    m_log.fatal () << "Redefinition of procedure at "
                                   << proc->location () << "."
                                   << " Conflicting with procedure declared at "
                                   << t->decl ()->location () << ".";
                    return ICode::E_TYPE;
                }
            }
        }

        procSym = new SymbolProcedure (proc);
        procSym->setName (shortName);
        m_st->appendSymbol (procSym);
    }

    return ICode::OK;
}

/// Template definitions.
ICode::Status TypeChecker::visit (TreeNodeTemplate* templ) {
    TreeNodeProcDef* body = templ->body ();
    TreeNodeIdentifier* id = body->identifier ();

    if (m_st->find (id->value ()) != 0) {
        m_log.fatal ()
                << "Redeclaration of template \"" << id->value () << "\""
                << " at " << id->location () << ".";
        return ICode::E_TYPE;
    }

    // Check that quantifiers are saneley defined
    std::set<std::string > quantifiedDomains;
    BOOST_FOREACH (TreeNode* _quant, templ->quantifiers ()) {
        TreeNodeQuantifier* quant = static_cast<TreeNodeQuantifier*>(_quant);
        quantifiedDomains.insert (quant->domain ()->value ());
        if (quant->kind ()) {
            Symbol* kindSym = findIdentifier (quant->kind ());
            if (kindSym == 0) return ICode::E_TYPE;
            if (kindSym->symbolType () != Symbol::PKIND) {
                m_log.fatal () << "Identifier at " << quant->location ()
                               << " is not a security domain kind.";
                return ICode::E_TYPE;
            }
        }
    }

    // Check that security types of parameters are either quantified or defined.
    BOOST_FOREACH (TreeNode* _d, std::make_pair (body->paramBegin (), body->paramEnd ())) {
        assert (dynamic_cast<TreeNodeStmtDecl*>(_d) != 0);
        TreeNodeStmtDecl* d = static_cast<TreeNodeStmtDecl*>(_d);
        TreeNodeType* t = d->varType ();
        TreeNodeIdentifier* id = t->secType ()->identifier ();
        if (quantifiedDomains.find (id->value ()) == quantifiedDomains.end ()) {
            if (! t->secType ()->isPublic () && findIdentifier (id) == 0) {
                return ICode::E_TYPE;
            }
        }
    }

    // Check return type.
    if (body->returnType ()->type () == NODE_TYPETYPE) {
        TreeNodeType* t = body->returnType ();
        if (! t->secType ()->isPublic ()) {
            TreeNodeIdentifier* id = t->secType ()->identifier ();
            if (quantifiedDomains.find (id->value ()) == quantifiedDomains.end ()) {
                if (findIdentifier (id) == 0) {
                    return ICode::E_TYPE;
                }
            }
        }
    }

    SymbolTemplate* s = new SymbolTemplate (templ);
    s->setName ("{templ}" + id->value ());
    m_st->appendSymbol (s);
    return ICode::OK;
}

/**
 * Procedure calls. This includes both calls to templates and calls to
 * regular procedures.
 */

ICode::Status TreeNodeExprProcCall::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprProcCall* root) {
    typedef DataTypeProcedureVoid DTFV;
    typedef DataTypeProcedure DTF;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    if (root->haveResultType ()) {
        return ICode::OK;
    }

    std::vector<TreeNodeExpr*> arguments;
    std::vector<DataType*> argumentDataTypes;

    BOOST_FOREACH (TreeNode* node,
        std::make_pair (++ root->children ().begin (),
                           root->children ().end ()))
    {
        assert(dynamic_cast<TreeNodeExpr*>(node) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(node);
        ICode::Status status = visitExpr (e);
        if (status != ICode::OK) return status;
        if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
        assert(dynamic_cast<TypeNonVoid*>(e->resultType()) != 0);
        TypeNonVoid* t = static_cast<TypeNonVoid*>(e->resultType());
        argumentDataTypes.push_back (t->dataType());
        arguments.push_back (e);
    }

    DataTypeProcedureVoid* argTypes =
            DataTypeProcedureVoid::get (getContext (), argumentDataTypes);
    SymbolProcedure* symProc = 0;
    TreeNodeIdentifier *id = root->procName ();
    ICode::Status status = findBestMatchingProc (symProc, id->value (),
                                                 root->contextSecType (),
                                                 argTypes);
    if (status != ICode::OK) {
        m_log.fatal () << "Error at " << root->location () << ".";
        return status;
    }

    assert (symProc != 0);
    root->setProcedure (symProc);
    TypeNonVoid* ft = symProc->decl()->procedureType();
    assert(ft->kind() == TypeNonVoid::PROCEDURE
           || ft->kind() == TypeNonVoid::PROCEDUREVOID);
    assert(dynamic_cast<DTFV*>(ft->dataType()) != 0);
    DTFV* rstv = static_cast<DTFV*>(ft->dataType());

    // Check security types of parameters:
    assert(rstv->paramTypes().size() == root->children().size() - 1);
    for (unsigned i = 0; i < rstv->paramTypes().size(); i++) {
        assert(rstv->paramTypes().at(i)->kind() == DataType::BASIC);
        assert(dynamic_cast<DataTypeBasic*>(rstv->paramTypes().at(i)) != 0);
        DataTypeBasic *need = static_cast<DataTypeBasic*>(rstv->paramTypes()[i]);

        assert(argTypes->paramTypes().at(i)->kind() == DataType::BASIC);
        assert(dynamic_cast<DataTypeBasic*>(argTypes->paramTypes().at(i)) != 0);
        DataTypeBasic *have = static_cast<DataTypeBasic*>(argTypes->paramTypes()[i]);

        if (need->secType()->isPublic () && have->secType()->isPrivate ())
        {
            m_log.fatal() << "Argument " << (i + 1) << " to function "
                << id->value() << " at " << arguments[i]->location()
                << " is expected to be of public type instead of private!";
            return ICode::E_TYPE;
        }

        if (need->dimType() != have->dimType()) {
            m_log.fatal() << "Argument " << (i + 1) << " to function "
                << id->value() << " at " << arguments[i]->location()
                << " has mismatching dimensionality.";
            return ICode::E_TYPE;
        }

        // Add implicit classify node if needed:
        classifyIfNeeded (static_cast<TreeNodeExpr*>(root->children ().at (i + 1)));
    }

    // Set result type:
    if (ft->kind() == TypeNonVoid::PROCEDURE) {
        assert(dynamic_cast<DTF*>(ft->dataType()) != 0);
        const DataTypeProcedure &rdt = *static_cast<DTF*>(ft->dataType());
        root->setResultType(TypeNonVoid::get (getContext (), rdt.returnType()));
    } else {
        root->setResultType(TypeVoid::get (getContext ()));
    }

    return ICode::OK;
}

ICode::Status TypeChecker::findBestMatchingProc (SymbolProcedure*& symProc,
                                                 const std::string& name,
                                                 SecurityType* contextTy,
                                                 DataTypeProcedureVoid* argTypes)
{
    typedef boost::tuple<unsigned, unsigned, unsigned > Weight;

    assert (argTypes != 0);
    SymbolProcedure* procTempSymbol = 0;
    BOOST_FOREACH (SymbolProcedure* s, findProcedures (m_st, name, argTypes)) {
        if (procTempSymbol != 0) {
            assert (false);
            return ICode::E_TYPE;
        }

        procTempSymbol = s;
    }

    if (procTempSymbol != 0) {
        symProc = procTempSymbol;
        return ICode::OK;
    }

    Weight best (argTypes->paramTypes ().size () + 1, 0, 0);
    std::vector<Instantiation> bestMatches;
    BOOST_FOREACH (SymbolTemplate* s, findTemplates (m_st, name)) {
        Instantiation inst (s);
        if (unify (inst, argTypes)) {
            Weight w (inst.templateParamCount (),
                      inst.unrestrictedTemplateParamCount (),
                      inst.quantifiedDomainOccurrenceCount ());
            if (w > best) continue;
            if (w < best) {
                bestMatches.clear ();
                best = w;
            }

            bestMatches.push_back (inst);
        }
    }

    if (bestMatches.empty ()) {
        m_log.fatal () << "No matching procedure or template.";
        return ICode::E_TYPE;
    }

    if (bestMatches.size () > 1) {
        std::ostringstream os;
        os << "Multiple matching templates:";
        BOOST_FOREACH (Instantiation i, bestMatches) {
            os << i.getTemplate ()->decl ()->location ();
        }

        m_log.fatal () << os.str ();
        return ICode::E_TYPE;
    }

    return getInstance (symProc, bestMatches.front ());
}

bool TypeChecker::unify (Instantiation& inst, DataTypeProcedureVoid* argTypes) const {
    typedef std::map<std::string, SecurityType* > DomainMap;
    const TreeNodeTemplate* t = inst.getTemplate ()->decl ();
    DomainMap argDomains;

    unsigned i = 0;
    BOOST_FOREACH (TreeNode* _d,
                   std::make_pair (t->body ()->paramBegin (),
                                   t->body ()->paramEnd ()))
    {
        assert(dynamic_cast<TreeNodeStmtDecl*>(_d) != 0);
        TreeNodeStmtDecl *d = static_cast<TreeNodeStmtDecl*>(_d);
        TreeNodeIdentifier* styId = d->varType ()->secType ()->identifier ();
        SecurityType*& ty = argDomains[styId->value ()];
        DataType* expectedTy = argTypes->paramTypes ().at (i ++);

        if (ty != 0 && ty != expectedTy->secrecSecType ())
            return false;

        if (expectedTy->secrecDataType () != d->varType ()->dataType ()->dataType ())
            return false;

        if (expectedTy->secrecDimType () != d->varType ()->dimType ()->dimType ())
            return false;

        ty = expectedTy->secrecSecType ();
    }

    std::list<SecurityType*> tmp;

    BOOST_FOREACH (TreeNode* _quant, t->quantifiers ()) {
        TreeNodeQuantifier* quant = static_cast<TreeNodeQuantifier*>(_quant);
        SecurityType* argTy = argDomains[quant->domain ()->value ()];
        assert (argTy != 0);
        if (quant->kind () != 0) {
            if (argTy->isPublic ()) return false;
            SymbolKind* sym = static_cast<SymbolKind*>(m_st->find (quant->kind ()->value ()));
            PrivateSecType* privArgTy = static_cast<PrivateSecType*>(argTy);
            if (sym != privArgTy->securityKind ()) {
                return false;
            }
        }

        tmp.push_back (argTy);
    }

    BOOST_FOREACH (SecurityType* ty, tmp) {
        inst.addParam (ty);
    }

    return true;
}

ICode::Status TypeChecker::getInstance (SymbolProcedure*& proc, const Instantiation& inst) {
    TreeNodeProcDef* body = m_instantiator->add (inst, m_st);
    SymbolTable* localST = m_instantiator->getLocalST (inst);
    std::swap (localST, m_st);
    ICode::Status status = visit (body);
    std::swap (localST, m_st);
    if (status != ICode::OK) {
        return status;
    }

    const std::vector<SecurityType*> targs (inst.begin (), inst.end ());
    proc = findProcedure (m_st,
        body->procedureName (),
        static_cast<DataTypeProcedureVoid*>(body->procedureType ()->dataType ()),
        targs);
    if (proc == 0) {
        proc = appendProcedure (m_st, *body, targs);
    }

    body->setSymbol (proc);
    return ICode::OK;
}

} // namespace SecreC
