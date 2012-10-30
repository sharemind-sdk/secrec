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

#include "log.h"
#include "ModuleInfo.h"
#include "symboltable.h"
#include "templates.h"
#include "treenode.h"

namespace SecreC {

namespace /* anonymous */ {

void mangleTemplateParameters (std::ostream& os,
                               const TemplateParams& targs)
{
    if (! targs.empty ()) {
        os << '(';
        bool first = true;
        BOOST_FOREACH (SecurityType* ty, targs) {
            if (! first) os << ',';
            os << *ty;
            first = false;
        }

        os << ')';
    }
}

std::string mangleProcedure (const std::string& name,
                             DataTypeProcedureVoid* dt,
                             const TemplateParams& targs = TemplateParams ())
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
    SymbolProcedure * ns = new SymbolProcedure(mangleProcedure(procdef.procedureName().str(),
                                                               dt,
                                                               targs),
                                               &procdef);
    st->appendSymbol (ns);
    return ns;
}

SymbolProcedure*
findProcedure (SymbolTable* st,
               const StringRef& name,
               DataTypeProcedureVoid* dt,
               const TemplateParams& targs = TemplateParams ())
{
    const std::string actualName = mangleProcedure (name.str(), dt, targs);
    SymbolProcedure* procSym = 0;
    Symbol* _procSym = st->find (actualName);
    if (_procSym != 0) {
        assert (dynamic_cast<SymbolProcedure*>(_procSym) != 0);
        procSym = static_cast<SymbolProcedure*>(_procSym);
    }

    return procSym;
}

std::vector<SymbolProcedure*>
findProcedures (SymbolTable* st,
                const StringRef& name,
                DataTypeProcedureVoid* dt)
{
    std::vector<SymbolProcedure* > out;
    const std::string actualName = mangleProcedure (name.str(), dt);
    BOOST_FOREACH (Symbol* _procSym, st->findAll (actualName)) {
        assert (dynamic_cast<SymbolProcedure*>(_procSym) != 0);
        SymbolProcedure* procSym = static_cast<SymbolProcedure*>(_procSym);
        out.push_back (procSym);
    }

    return out;
}

std::vector<SymbolTemplate*>
findTemplates (SymbolTable* st, const StringRef& name)
{
    std::vector<SymbolTemplate*> out;
    BOOST_FOREACH (Symbol* _symTempl, st->findAll ("{templ}" + name.str())) {
        assert (dynamic_cast<SymbolTemplate*>(_symTempl) != 0);
        SymbolTemplate* symTempl = static_cast<SymbolTemplate*>(_symTempl);
        out.push_back (symTempl);
    }

    return out;
}

} // anonymous namespace

/// Return symbol for the main procedure (if exists).
SymbolProcedure* TypeChecker::mainProcedure () {
    DataTypeProcedureVoid* ty = DataTypeProcedureVoid::get (getContext ());
    std::vector<SymbolProcedure *> ms = findProcedures (m_st, "main", ty);
    if (ms.size() > 1u) {
        m_log.fatal() << "Multiple definitions of main found!";
        return NULL;
    } else if (ms.empty()) {
        m_log.fatal() << "No procedure \"void main()\" found!";
        return NULL;
    }
    return ms.at(0u);
}

TypeChecker::Status TypeChecker::populateParamTypes(std::vector<DataType *> & params,
                                                    TreeNodeProcDef * proc)
{
    typedef DataTypeVar DTV;
    params.clear ();
    params.reserve (boost::distance (proc->paramRange ()));
    BOOST_FOREACH (TreeNode* n, proc->paramRange ()) {
        assert(n->type() == NODE_DECL);
        assert(dynamic_cast<TreeNodeStmtDecl*>(n) != 0);
        TreeNodeStmtDecl *d = static_cast<TreeNodeStmtDecl*>(n);
        Status s = visit(d);
        if (s != OK)
            return s;
        TypeNonVoid* pt = d->resultType();
        assert(pt->dataType()->kind() == DataType::VAR);
        assert(dynamic_cast<DTV*>(pt->dataType()) != 0);
        params.push_back (static_cast<DTV*>(pt->dataType())->dataType());
    }

    return OK;
}


/// Procedure definitions.
TypeChecker::Status TypeChecker::visit(TreeNodeProcDef * proc,
                                       SymbolTable * localScope)
{
    typedef TypeNonVoid TNV;
    if (proc->m_cachedType == 0) {
        std::swap (m_st, localScope);
        TreeNodeType* rt = proc->returnType ();
        Status s = visit(rt);
        if (s != OK)
            return s;

        if (proc->procedureName() == "main" && rt->isNonVoid()) {
            m_log.fatal() << "Invalid return type procedure 'main' at " << proc->location() << '.';
            return E_TYPE;
        }

        if (proc->procedureName() == "main" && proc->paramBegin() != proc->paramEnd()) {
            m_log.fatal() << "Invalid parameters for procedure 'main' at " << proc->location() << '.';
            return E_TYPE;
        }
        std::vector<DataType*> params;
        if ((s = populateParamTypes(params, proc)) != OK)
            return s;

        DataTypeProcedureVoid* voidProcType =
                DataTypeProcedureVoid::get (getContext (), params);

        if (rt->secrecType()->isVoid()) {
            proc->m_cachedType = TypeNonVoid::get (getContext (), voidProcType);
        }
        else {
            TNV* tt = static_cast<TNV*>(rt->secrecType());
            assert (tt->dataType()->kind() == DataType::BASIC);
            proc->m_cachedType = TypeNonVoid::get (getContext (),
                DataTypeProcedure::get (getContext (), voidProcType, tt->dataType ()));
        }

        std::swap (m_st, localScope);

        SymbolProcedure* procSym = appendProcedure (m_st, *proc);
        proc->setSymbol (procSym);

        const std::string& shortName = "{proc}" + proc->identifier ()->value ().str();
        BOOST_FOREACH (Symbol* sym, m_st->findAll (shortName)) {
            if (sym->symbolType () == Symbol::PROCEDURE) {
                SymbolProcedure* t = static_cast<SymbolProcedure*>(sym);
                if (t->decl ()->m_cachedType == proc->m_cachedType) {
                    m_log.fatal () << "Redefinition of procedure '"
                                   << proc->identifier()->value()
                                   << "' at "
                                   << proc->location () << '.'
                                   << " Conflicting with procedure '"
                                   << t->decl()->printableSignature()
                                   << "' declared at "
                                   << t->decl ()->location () << '.';
                    return E_TYPE;
                }
                if (proc->identifier()->value() == "main" && t->decl()->identifier()->value() == "main") {
                    m_log.fatal() << "Redefinition of procedure 'main' at "
                                  << proc->location () << " not allowed!";
                    m_log.fatal() << "Procedure 'main' already defined at "
                                   << t->decl ()->location () << '.';
                    return E_TYPE;
                }
            }
        }

        m_st->appendSymbol(new SymbolProcedure(shortName, proc));
    }

    return OK;
}

/// Template definitions.
TypeChecker::Status TypeChecker::visit(TreeNodeTemplate * templ) {
    TreeNodeProcDef* body = templ->body ();
    TreeNodeIdentifier* id = body->identifier ();

    if (m_st->find (id->value ()) != 0) {
        m_log.fatal ()
                << "Redeclaration of template \"" << id->value () << "\""
                << " at " << id->location () << '.';
        return E_TYPE;
    }

    // Check that quantifiers are saneley defined
    typedef std::map<StringRef, TreeNodeIdentifier*, StringRef::FastCmp> TypeVariableMap;
    typedef std::set<StringRef, StringRef::FastCmp> TypeVariableSet;
    TypeVariableSet quantifiedDomains;
    TypeVariableMap freeTypeVariables;
    BOOST_FOREACH (TreeNode* _quant, templ->quantifiers ()) {
        TreeNodeQuantifier* quant = static_cast<TreeNodeQuantifier*>(_quant);
        quantifiedDomains.insert (quant->domain ()->value ());
        freeTypeVariables[quant->domain ()->value ()] = quant->domain ();
        if (quant->kind ()) {
            Symbol* kindSym = findIdentifier (quant->kind ());
            if (kindSym == 0)
                return E_TYPE;
            if (kindSym->symbolType () != Symbol::PKIND) {
                m_log.fatal () << "Identifier at " << quant->location ()
                               << " is not a security domain kind.";
                return E_TYPE;
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
            freeTypeVariables.erase(retSecTyIdent->value ());
            if (quantifiedDomains.find (retSecTyIdent->value ()) == quantifiedDomains.end ()) {
                if (findIdentifier(retSecTyIdent) == 0)
                    return E_TYPE;
            }
        }
    }

    // Check that security types of parameters are either quantified or defined.
    BOOST_FOREACH (TreeNode* _d, body->paramRange ()) {
        assert (dynamic_cast<TreeNodeStmtDecl*>(_d) != 0);
        TreeNodeStmtDecl* d = static_cast<TreeNodeStmtDecl*>(_d);
        TreeNodeType* t = d->varType ();
        if (! t->secType ()->isPublic ()) {
            TreeNodeIdentifier* id = t->secType ()->identifier ();
            if (retSecTyIdent != 0) {
                if (id->value () == retSecTyIdent->value ()) {
                    templ->setContextDependance (false); // nope, false alert
                }
            }

            if (quantifiedDomains.find (id->value ()) == quantifiedDomains.end ()) {
                if (findIdentifier(id) == 0)
                    return E_TYPE;
            }

            freeTypeVariables.erase(id->value ());
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

    SymbolTemplate* s = new SymbolTemplate (templ);
    s->setName ("{templ}" + id->value ().str());
    m_st->appendSymbol (s);
    return OK;
}

/**
 * Procedure calls. This includes both calls to templates and calls to
 * regular procedures.
 */

TypeChecker::Status TreeNodeExprProcCall::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::checkParams(const std::vector<TreeNodeExpr *> & arguments,
                                             DataTypeProcedureVoid *& argTypes)
{
    std::vector<DataType*> argumentDataTypes;

    BOOST_FOREACH (TreeNode* _arg, arguments) {
        assert(dynamic_cast<TreeNodeExpr*>(_arg) != 0);
        TreeNodeExpr *arg = static_cast<TreeNodeExpr*>(_arg);
        Status status = visitExpr(arg);
        if (status != OK)
            return status;
        if (checkAndLogIfVoid(arg))
            return E_TYPE;
        arg->instantiateDataType (getContext ());
        assert(dynamic_cast<TypeNonVoid*>(arg->resultType()) != 0);
        TypeNonVoid* t = static_cast<TypeNonVoid*>(arg->resultType());
        argumentDataTypes.push_back (t->dataType());
    }

    argTypes = DataTypeProcedureVoid::get (getContext (), argumentDataTypes);
    return OK;
}

TypeChecker::Status TypeChecker::checkProcCall(SymbolProcedure * symProc,
                                               DataTypeProcedureVoid * argTypes,
                                               SecreC::Type *& resultType)
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

    return OK;
}

TypeChecker::Status TypeChecker::checkProcCall(TreeNodeIdentifier * name,
                                               const TreeNodeExprProcCall & tyCxt,
                                               const std::vector<TreeNodeExpr *> & arguments,
                                               SecreC::Type *& resultType,
                                               SymbolProcedure *& symProc)
{
    typedef DataTypeProcedureVoid DTFV;
    typedef DataTypeProcedure DTF;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    std::vector<DataType*> argumentDataTypes;

    BOOST_FOREACH (TreeNode* _arg, arguments) {
        assert(dynamic_cast<TreeNodeExpr*>(_arg) != 0);
        TreeNodeExpr *arg = static_cast<TreeNodeExpr*>(_arg);
        Status status = visitExpr(arg);
        if (status != OK)
            return status;
        if (checkAndLogIfVoid(arg))
            return E_TYPE;
        arg->instantiateDataType (getContext ());
        assert(dynamic_cast<TypeNonVoid*>(arg->resultType()) != 0);
        TypeNonVoid* t = static_cast<TypeNonVoid*>(arg->resultType());
        argumentDataTypes.push_back (t->dataType());
    }

    DataTypeProcedureVoid* argTypes =
            DataTypeProcedureVoid::get (getContext (), argumentDataTypes);
    Status status = findBestMatchingProc(symProc, name->value(), tyCxt, argTypes, &tyCxt);
    if (status != OK)
        return status;

    if (symProc == 0) {
        m_log.fatalInProc(&tyCxt) << "No matching procedure definitions for:";
        m_log.fatal () << '\t' << name->value() << argTypes->paramsToNormalString();
        m_log.fatal () << "In context " << tyCxt.TypeContext::toString() << " at " << tyCxt.location() << '.';

        bool haveCandidatesLabel = false;
        std::vector<Symbol *> cs = m_st->findPrefixed("{proc}" + name->value().str());
        if (!cs.empty()) {
            std::vector<SymbolProcedure *> cps;
            do {
                assert(dynamic_cast<SymbolProcedure *>(cs.back()) != 0);
                SymbolProcedure * const p = static_cast<SymbolProcedure *>(cs.back());
                if (p->shortOf() == NULL)
                    cps.push_back(p);
                cs.pop_back();
            } while (!cs.empty());
            if (!cps.empty()) {
                m_log.info() << "Candidates are:";
                haveCandidatesLabel = true;
                BOOST_FOREACH(SymbolProcedure * c, cps) {
                    std::ostringstream oss;
                    c->print(oss);
                    if (c->location()) {
                        m_log.info() << '\t' << oss.str() << " at " << *(c->location());
                    } else {
                        m_log.info() << '\t' << oss.str();
                    }
                }
            }
        }
        cs = m_st->findPrefixed("{templ}" + name->value().str());
        if (!cs.empty()) {
            if (!haveCandidatesLabel)
                m_log.info() << "Candidates are:";
            BOOST_REVERSE_FOREACH(Symbol * c, cs) {
                assert(dynamic_cast<SymbolTemplate *>(c) != 0);
                std::ostringstream oss;
                c->print(oss);
                if (c->location()) {
                    m_log.info() << '\t' << oss.str() << " at " << *(c->location());
                } else {
                    m_log.info() << '\t' << oss.str();
                }
            }
        }

        return E_TYPE;
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
            m_log.fatalInProc(&tyCxt) << "Argument " << (i + 1) << " to procedure "
                << name->value() << " at " << arguments[i]->location()
                << " is expected to be of public type instead of private!";
            return E_TYPE;
        }

        if (need->dimType() != have->dimType()) {
            m_log.fatalInProc(&tyCxt) << "Argument " << (i + 1) << " to procedure "
                << name->value() << " at " << arguments[i]->location()
                << " has mismatching dimensionality.";
            return E_TYPE;
        }

        // Add implicit classify node if needed (NOT NEEDED):
        // classifyIfNeeded (arguments[i]);
    }

    // Set result type:
    if (ft->kind() == TypeNonVoid::PROCEDURE) {
        assert(dynamic_cast<DTF*>(ft->dataType()) != 0);
        const DataTypeProcedure &rdt = *static_cast<DTF*>(ft->dataType());
        resultType = TypeNonVoid::get (getContext (), rdt.returnType());
    } else {
        resultType = TypeVoid::get (getContext ());
    }

    return OK;
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprProcCall * root) {
    if (root->haveResultType())
        return OK;

    Type* resultType = 0;
    SymbolProcedure* symProc = 0;
    std::vector<TreeNodeExpr*> arguments;
    TreeNodeIdentifier *id = root->procName ();

    BOOST_FOREACH (TreeNode* _arg, root->paramRange ()) {
        assert(dynamic_cast<TreeNodeExpr*>(_arg) != 0);
        TreeNodeExpr *arg = static_cast<TreeNodeExpr*>(_arg);
        arguments.push_back (arg);
    }

    Status s = checkProcCall(id, *root, arguments, resultType, symProc);
    if (s != OK)
        return s;

    root->setProcedure (symProc);
    root->setResultType (resultType);
    return OK;
}

TypeChecker::Status TypeChecker::findBestMatchingProc(SymbolProcedure *& symProc,
                                                      StringRef name,
                                                      const TypeContext & tyCxt,
                                                      DataTypeProcedureVoid * argTypes,
                                                      const TreeNode * errorCxt)
{
    assert(errorCxt);
    typedef boost::tuple<unsigned, unsigned, unsigned > Weight;

    // Look for regular procedures:
    assert (argTypes != 0);
    SymbolProcedure* procTempSymbol = 0;
    BOOST_FOREACH (SymbolProcedure* s, findProcedures (m_st, name, argTypes)) {
        SecreC::Type* _ty = s->decl ()->returnType ()->secrecType ();
        if (! _ty->isVoid ()) { // and procedure is non-void...
            assert (dynamic_cast<TypeNonVoid*>(_ty) != 0);
            TypeNonVoid* ty = static_cast<TypeNonVoid*>(_ty);
            if (! tyCxt.matchSecType (ty->secrecSecType ()))   continue;
            if (! tyCxt.matchDataType (ty->secrecDataType ())) continue;
            if (! tyCxt.matchDimType (ty->secrecDimType ()))   continue;
        }
        else {
            // if the procedure is void, and context expects non-void then skip
            if (tyCxt.haveContextSecType ())  continue;
            if (tyCxt.haveContextDataType ()) continue;
            if (tyCxt.haveContextDimType ())  continue;
        }

        if (procTempSymbol != 0) {
            m_log.fatalInProc(errorCxt) << "Multiple matching procedures at "
                                        << errorCxt->location() << '.';
            return E_TYPE;
        }

        procTempSymbol = s;
    }

    if (procTempSymbol != 0) {
        symProc = procTempSymbol;
        return OK;
    }

    // Look for templates:
    Weight best (argTypes->paramTypes ().size () + 2, 0, 0);
    std::vector<Instantiation> bestMatches;
    BOOST_FOREACH (SymbolTemplate* s, findTemplates (m_st, name)) {
        Instantiation inst (s);
        assert (s->decl ()->containingModule () != 0);
        if (unify (inst, tyCxt, argTypes)) {
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

    if (bestMatches.empty())
        return OK;

    if (bestMatches.size () > 1) {
        std::ostringstream os;
        os << "Multiple matching templates: ";
        BOOST_FOREACH (const Instantiation& i, bestMatches) {
            os << i.getTemplate ()->decl ()->location () << ' ';
        }

        m_log.fatalInProc(errorCxt) << os.str() << "at "
                                    << errorCxt->location() << '.';
        return E_TYPE;
    }

    return getInstance (symProc, bestMatches.front ());
}

bool TypeChecker::unify (Instantiation& inst,
                         const TypeContext& tyCxt,
                         DataTypeProcedureVoid* argTypes) const {
    typedef std::map<StringRef, SecurityType*, StringRef::FastCmp > DomainMap;
    const TreeNodeTemplate* t = inst.getTemplate ()->decl ();
    DomainMap argDomains;

    if (inst.getTemplate ()->decl ()->isContextDependent ()) {
        if (! tyCxt.haveContextSecType ()) {
            return false;
        }
    }

    //
    // Following is little strange as the type AST nodes have not
    // yet been visited by the type checker and thus don't have the
    // cached type. There probably is much nicer solution to what im doing
    // but ill stick to what works for now.
    //

    if (boost::size (t->body ()->paramRange ()) != argTypes->paramTypes ().size ()) {
        return false;
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
    if (retNodeTy->isNonVoid ()) {
        if (tyCxt.haveContextSecType ()) {
            if (retNodeTy->secType ()->isPublic ()) {
                if (! tyCxt.contextSecType ()->isPublic ())
                    return false;
            }
            else {
                TreeNodeIdentifier* styId = retNodeTy->secType ()->identifier ();
                SecurityType*& ty = argDomains[styId->value ()];
                if (ty != 0 && ty != tyCxt.contextSecType ()) {
                    return false;
                }

                ty = tyCxt.contextSecType ();
            }
        }

        if (! tyCxt.matchDataType (retNodeTy->dataType ()->dataType ()))
            return false;

        if (! tyCxt.matchDimType (retNodeTy->dimType ()->dimType ())) {
            return false;
        }
    }
    else {
        // this is not very pretty either...
        if (tyCxt.haveContextDataType ()) {
            return false;
        }
    }

    std::vector<SecurityType*> tmp;
    tmp.reserve (t->quantifiers ().size ());

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

TypeChecker::Status TypeChecker::getInstance(SymbolProcedure *& proc,
                                             const Instantiation & inst)
{
    ModuleInfo* mod = inst.getTemplate ()->decl ()->containingModule ();
    InstanceInfo info = m_instantiator->add (inst, *mod);
    TreeNodeProcDef* body = info.m_generatedBody;
    SymbolTable* moduleST = info.m_moduleInfo->codeGenState ().st ();
    SymbolTable* localST = info.m_localScope;
    assert (localST->parent () == moduleST);
    std::swap (m_st, moduleST);
    Status status = visit(body, localST);
    if (status != OK)
        return status;

    const std::vector<SecurityType*> targs (inst.begin (), inst.end ());
    proc = findProcedure (m_st,
        body->procedureName (),
        static_cast<DataTypeProcedureVoid*>(body->procedureType ()->dataType ()),
        targs);
    if (proc == 0) {
        proc = appendProcedure (m_st, *body, targs);
    }

    body->setSymbol (proc);
    std::swap (m_st, moduleST);
    return OK;
}

} // namespace SecreC
