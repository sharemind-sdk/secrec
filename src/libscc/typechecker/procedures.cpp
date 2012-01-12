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
#include <boost/range.hpp>

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
    SymbolProcedure* ns = new SymbolProcedure(&procdef);
    ns->setName (name);
    st->appendSymbol (ns);
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
                DataTypeProcedureVoid* dt)
{
    std::list<SymbolProcedure* > out;
    const std::string actualName = mangleProcedure (name, dt, TemplateParams ());
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
    params.reserve (boost::distance (proc->paramRange ()));
    BOOST_FOREACH (TreeNode* n, proc->paramRange ()) {
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

        SymbolProcedure* procSym = appendProcedure (m_st, *proc);
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

    templ->setContextDependance (false);

    // Check return type.
    TreeNodeIdentifier* retSecTyIdent = 0;
    if (body->returnType ()->type () == NODE_TYPETYPE) {
        TreeNodeType* t = body->returnType ();
        if (! t->secType ()->isPublic ()) {
            retSecTyIdent = t->secType ()->identifier ();
            templ->setContextDependance (true); // may depend on context!
            if (quantifiedDomains.find (retSecTyIdent->value ()) == quantifiedDomains.end ()) {
                if (findIdentifier (retSecTyIdent) == 0) {
                    return ICode::E_TYPE;
                }
            }
        }
    }

    // Check that security types of parameters are either quantified or defined.
    BOOST_FOREACH (TreeNode* _d, body->paramRange ()) {
        assert (dynamic_cast<TreeNodeStmtDecl*>(_d) != 0);
        TreeNodeStmtDecl* d = static_cast<TreeNodeStmtDecl*>(_d);
        TreeNodeType* t = d->varType ();
        if (! t->secType ()->isPublic ()) {
            std::cerr << t->toString () << std::endl;
            TreeNodeIdentifier* id = t->secType ()->identifier ();
            if (retSecTyIdent != 0) {
                if (id->value () == retSecTyIdent->value ()) {
                    templ->setContextDependance (false); // nope, false alert
                }
            }

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

ICode::Status TypeChecker::checkParams (const std::vector<TreeNodeExpr*>& arguments,
                                        DataTypeProcedureVoid*& argTypes)
{
    std::vector<DataType*> argumentDataTypes;

    BOOST_FOREACH (TreeNode* _arg, arguments) {
        assert(dynamic_cast<TreeNodeExpr*>(_arg) != 0);
        TreeNodeExpr *arg = static_cast<TreeNodeExpr*>(_arg);
        ICode::Status status = visitExpr (arg);
        if (status != ICode::OK) return status;
        if (checkAndLogIfVoid (arg)) return ICode::E_TYPE;
        assert(dynamic_cast<TypeNonVoid*>(arg->resultType()) != 0);
        TypeNonVoid* t = static_cast<TypeNonVoid*>(arg->resultType());
        argumentDataTypes.push_back (t->dataType());
    }

    argTypes = DataTypeProcedureVoid::get (getContext (), argumentDataTypes);
    return ICode::OK;
}

ICode::Status TypeChecker::checkProcCall (SymbolProcedure* symProc,
                                          DataTypeProcedureVoid* argTypes,
                                          SecreC::Type*& resultType)
{
    typedef DataTypeProcedureVoid DTFV;
    typedef DataTypeProcedure DTF;

    TypeNonVoid* ft = symProc->decl()->procedureType();
    assert(   ft->kind() == TypeNonVoid::PROCEDURE
           || ft->kind() == TypeNonVoid::PROCEDUREVOID);
    assert(dynamic_cast<DTFV*>(ft->dataType()) != 0);
    DTFV* rstv = static_cast<DTFV*>(ft->dataType());

    // Check security types of parameters:
    assert (rstv->paramTypes().size() == argTypes->paramTypes ().size ());
    for (unsigned i = 0; i < rstv->paramTypes().size(); i++) {
        assert(rstv->paramTypes().at(i)->kind() == DataType::BASIC);
        assert(dynamic_cast<DataTypeBasic*>(rstv->paramTypes().at(i)) != 0);
        DataTypeBasic *need = static_cast<DataTypeBasic*>(rstv->paramTypes()[i]);

        assert(argTypes->paramTypes().at(i)->kind() == DataType::BASIC);
        assert(dynamic_cast<DataTypeBasic*>(argTypes->paramTypes().at(i)) != 0);
        DataTypeBasic *have = static_cast<DataTypeBasic*>(argTypes->paramTypes()[i]);

        assert (have == need);
    }

    // Set result type:
    if (ft->kind() == TypeNonVoid::PROCEDURE) {
        assert(dynamic_cast<DTF*>(ft->dataType()) != 0);
        DataTypeProcedure* rdt = static_cast<DTF*>(ft->dataType());
        resultType = TypeNonVoid::get (getContext (), rdt->returnType());
    } else {
        resultType = TypeVoid::get (getContext ());
    }

    return ICode::OK;
}

ICode::Status TypeChecker::checkProcCall (TreeNodeIdentifier* name,
                                          SecurityType* contextSecType,
                                          const std::vector<TreeNodeExpr*>& arguments,
                                          SecreC::Type*& resultType,
                                          SymbolProcedure*& symProc)
{
    typedef DataTypeProcedureVoid DTFV;
    typedef DataTypeProcedure DTF;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    std::vector<DataType*> argumentDataTypes;

    BOOST_FOREACH (TreeNode* _arg, arguments) {
        assert(dynamic_cast<TreeNodeExpr*>(_arg) != 0);
        TreeNodeExpr *arg = static_cast<TreeNodeExpr*>(_arg);
        ICode::Status status = visitExpr (arg);
        if (status != ICode::OK) return status;
        if (checkAndLogIfVoid (arg)) return ICode::E_TYPE;
        assert(dynamic_cast<TypeNonVoid*>(arg->resultType()) != 0);
        TypeNonVoid* t = static_cast<TypeNonVoid*>(arg->resultType());
        argumentDataTypes.push_back (t->dataType());
    }

    DataTypeProcedureVoid* argTypes =
            DataTypeProcedureVoid::get (getContext (), argumentDataTypes);
    ICode::Status status = findBestMatchingProc (symProc, name->value (),
                                                 contextSecType,
                                                 argTypes);
    if (status != ICode::OK) {
        return status;
    }

    if (symProc == 0) {
        m_log.fatal () << "No matching procedure definitions.";
        return ICode::E_TYPE;
    }

    TypeNonVoid* ft = symProc->decl()->procedureType();
    assert(   ft->kind() == TypeNonVoid::PROCEDURE
           || ft->kind() == TypeNonVoid::PROCEDUREVOID);
    assert(dynamic_cast<DTFV*>(ft->dataType()) != 0);
    DTFV* rstv = static_cast<DTFV*>(ft->dataType());

    // Check security types of parameters:
    assert(rstv->paramTypes().size() == arguments.size ());
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
                << name->value() << " at " << arguments[i]->location()
                << " is expected to be of public type instead of private!";
            return ICode::E_TYPE;
        }

        if (need->dimType() != have->dimType()) {
            m_log.fatal() << "Argument " << (i + 1) << " to function "
                << name->value() << " at " << arguments[i]->location()
                << " has mismatching dimensionality.";
            return ICode::E_TYPE;
        }

        // Add implicit classify node if needed:
        classifyIfNeeded (arguments[i]);
    }

    // Set result type:
    if (ft->kind() == TypeNonVoid::PROCEDURE) {
        assert(dynamic_cast<DTF*>(ft->dataType()) != 0);
        const DataTypeProcedure &rdt = *static_cast<DTF*>(ft->dataType());
        resultType = TypeNonVoid::get (getContext (), rdt.returnType());
    } else {
        resultType = TypeVoid::get (getContext ());
    }

    return ICode::OK;
}

ICode::Status TypeChecker::visit (TreeNodeExprProcCall* root) {
    typedef DataTypeProcedureVoid DTFV;
    typedef DataTypeProcedure DTF;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    if (root->haveResultType ()) {
        return ICode::OK;
    }

    Type* resultType = 0;
    SymbolProcedure* symProc = 0;
    std::vector<TreeNodeExpr*> arguments;
    TreeNodeIdentifier *id = root->procName ();

    BOOST_FOREACH (TreeNode* _arg, root->paramRange ()) {
        assert(dynamic_cast<TreeNodeExpr*>(_arg) != 0);
        TreeNodeExpr *arg = static_cast<TreeNodeExpr*>(_arg);
        arguments.push_back (arg);
    }

    ICode::Status s = checkProcCall (id, root->contextSecType (),
                                     arguments, resultType, symProc);
    if (s != ICode::OK) {
        m_log.fatal () << "Error at " << root->location () << ".";
        return s;
    }

    root->setProcedure (symProc);
    root->setResultType (resultType);
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
        if (contextTy != 0) { // if we have context...
            SecreC::Type* _ty = s->decl ()->returnType ()->secrecType ();
            if (! _ty->isVoid ()) { // and procedure is non-void...
                TypeNonVoid* ty = static_cast<TypeNonVoid*>(_ty);
                if (ty->secrecSecType () != contextTy) // skip unmatching definitions.
                    continue;
            }
        }

        if (procTempSymbol != 0) {
            m_log.fatal () << "Multiple matching procedures!";
            return ICode::E_TYPE;
        }

        procTempSymbol = s;
    }

    if (procTempSymbol != 0) {
        symProc = procTempSymbol;
        return ICode::OK;
    }

    Weight best (argTypes->paramTypes ().size () + 2, 0, 0);
    std::vector<Instantiation> bestMatches;
    BOOST_FOREACH (SymbolTemplate* s, findTemplates (m_st, name)) {
        Instantiation inst (s);
        if (unify (inst, contextTy, argTypes)) {
            const Weight w (inst.templateParamCount (),
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
        return ICode::OK;
    }

    if (bestMatches.size () > 1) {
        std::ostringstream os;
        os << "Multiple matching templates: ";
        BOOST_FOREACH (const Instantiation& i, bestMatches) {
            os << i.getTemplate ()->decl ()->location () << ' ';
        }

        m_log.fatal () << os.str ();
        return ICode::E_TYPE;
    }

    return getInstance (symProc, bestMatches.front ());
}

bool TypeChecker::unify (Instantiation& inst,
                         SecurityType* contextTy,
                         DataTypeProcedureVoid* argTypes) const {
    typedef std::map<std::string, SecurityType* > DomainMap;
    const TreeNodeTemplate* t = inst.getTemplate ()->decl ();
    DomainMap argDomains;

    if (inst.getTemplate ()->decl ()->isContextDependent ()) {
        if (contextTy == 0) {
            return false;
        }
    }

    unsigned i = 0;
    BOOST_FOREACH (TreeNode* _d, t->body ()->paramRange ()) {
        assert(dynamic_cast<TreeNodeStmtDecl*>(_d) != 0);
        TreeNodeStmtDecl *d = static_cast<TreeNodeStmtDecl*>(_d);
        TreeNodeType* argNodeTy = d->varType ();
        DataType* expectedTy = argTypes->paramTypes ().at (i ++);
        if (argNodeTy->secType ()->isPublic ()) {
            if (! expectedTy->secrecSecType ()->isPublic ())
                return false;
        }
        else {
            TreeNodeIdentifier* styId = argNodeTy->secType ()->identifier ();
            SecurityType*& ty = argDomains[styId->value ()];
            if (ty != 0 && ty != expectedTy->secrecSecType ())
                return false;

            ty = expectedTy->secrecSecType ();
        }

        if (expectedTy->secrecDataType () != argNodeTy->dataType ()->dataType ())
            return false;

        if (expectedTy->secrecDimType () != argNodeTy->dimType ()->dimType ())
            return false;
    }

    TreeNodeType* retNodeTy = t->body ()->returnType ();
    if (retNodeTy->isNonVoid () && contextTy != 0) {
        if (retNodeTy->secType ()->isPublic ()) {
            if (! contextTy->isPublic ())
                return false;
        }
        else {
            TreeNodeIdentifier* styId = retNodeTy->secType ()->identifier ();
            SecurityType*& ty = argDomains[styId->value ()];
            if (ty != 0 && ty != contextTy) {
                return false;
            }

            ty = contextTy;
        }
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
    proc = findProcedure (m_st->globalScope (),
        body->procedureName (),
        static_cast<DataTypeProcedureVoid*>(body->procedureType ()->dataType ()),
        targs);
    if (proc == 0) {
        proc = appendProcedure (m_st->globalScope (), *body, targs);
    }

    body->setSymbol (proc);
    return ICode::OK;
}

} // namespace SecreC
