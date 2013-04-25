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
#include "symbol.h"
#include "symboltable.h"
#include "templates.h"
#include "treenode.h"

namespace SecreC {

namespace /* anonymous */ {

std::string mangleProcedure (const std::string& name, DataTypeProcedureVoid* dt)
{
    std::ostringstream os;
    os << name << dt->mangle ();
    return os.str ();
}

SymbolProcedure* appendProcedure (SymbolTable* st, const TreeNodeProcDef& procdef)
{
    typedef DataTypeProcedureVoid DTPV;
    assert(procdef.procedureType()->kind() == TypeNonVoid::PROCEDURE
           || procdef.procedureType()->kind() == TypeNonVoid::PROCEDUREVOID);
    assert(dynamic_cast<DTPV*>(procdef.procedureType()->dataType()) != 0);
    DTPV* dt = static_cast<DTPV*>(procdef.procedureType()->dataType());
    SymbolProcedure * ns = new SymbolProcedure(mangleProcedure(procdef.procedureName().str(), dt),
                                               &procdef);
    st->appendSymbol (ns);
    return ns;
}

SymbolProcedure*
findProcedure (SymbolTable* st, StringRef name, DataTypeProcedureVoid* dt)
{
    return st->find<SYM_PROCEDURE>(mangleProcedure (name.str(), dt));
}

std::vector<SymbolProcedure*>
findProcedures (SymbolTable* st, StringRef name, DataTypeProcedureVoid* dt)
{
    std::vector<SymbolProcedure* > out;
    const std::string actualName = mangleProcedure (name.str(), dt);
    BOOST_FOREACH (Symbol* _procSym, st->findAll (SYM_PROCEDURE, actualName)) {
        assert (dynamic_cast<SymbolProcedure*>(_procSym) != 0);
        SymbolProcedure* procSym = static_cast<SymbolProcedure*>(_procSym);
        out.push_back (procSym);
    }

    return out;
}

std::vector<SymbolTemplate*>
findTemplates (SymbolTable* st, StringRef name)
{
    std::vector<SymbolTemplate*> out;
    const std::string actualName = name.str();
    BOOST_FOREACH (Symbol* _symTempl, st->findAll (SYM_TEMPLATE, actualName)) {
        assert (dynamic_cast<SymbolTemplate*>(_symTempl) != 0);
        SymbolTemplate* symTempl = static_cast<SymbolTemplate*>(_symTempl);
        out.push_back (symTempl);
    }

    return out;
}

bool mapVariable (TemplateVarMap& varMap, StringRef id, const TemplateParameter& param)
{
    TemplateVarMap::iterator it = varMap.find (id);
    if (it != varMap.end () && it->second != param)
        return false;

    varMap.insert (it, std::make_pair (id, param));
    return true;
}

} // anonymous namespace

/// Return symbol for the main procedure (if exists).
SymbolProcedure* TypeChecker::mainProcedure ()
{
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
    params.clear ();
    params.reserve (proc->params ().size ());
    BOOST_FOREACH (TreeNodeStmtDecl& decl, proc->params ()) {
        Status s = visit (&decl);
        if (s != OK)
            return s;

        TypeNonVoid* pt = decl.resultType();
        assert(dynamic_cast<DataTypeVar*>(pt->dataType()) != 0);
        params.push_back (static_cast<DataTypeVar*>(pt->dataType())->dataType());
    }

    return OK;
}


/// Procedure definitions.
TypeChecker::Status TypeChecker::visit(TreeNodeProcDef * proc,
                                       SymbolTable * localScope)
{
    typedef TypeNonVoid TNV;
    if (proc->m_cachedType != 0) {
        return OK;
    }

    std::swap (m_st, localScope);
    TreeNodeType* rt = proc->returnType ();
    Status s = visit(rt);
    if (s != OK)
        return s;

    if (proc->procedureName() == "main" && rt->isNonVoid()) {
        m_log.fatal() << "Invalid return type procedure 'main' at " << proc->location() << '.';
        return E_TYPE;
    }

    if (proc->procedureName() == "main" && proc->params ().size () > 0) {
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

    const std::string& shortName = proc->identifier ()->value ().str();
    BOOST_FOREACH (Symbol* sym, m_st->findAll (SYM_PROCEDURE, shortName)) {
        if (sym->symbolType () == SYM_PROCEDURE) {
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

    return OK;
}

/**
 * Procedure calls. This includes both calls to templates and calls to
 * regular procedures.
 */

TypeChecker::Status TreeNodeExprProcCall::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
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
                                               const TreeNodeChildren<TreeNodeExpr>& arguments,
                                               SecreC::Type *& resultType,
                                               SymbolProcedure *& symProc)
{
    typedef DataTypeProcedureVoid DTFV;
    typedef DataTypeProcedure DTF;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    std::vector<DataType*> argumentDataTypes;

    BOOST_FOREACH (TreeNodeExpr& arg, arguments) {
        Status status = visitExpr(&arg);
        if (status != OK)
            return status;
        if (checkAndLogIfVoid(&arg))
            return E_TYPE;
        arg.instantiateDataType (getContext ());
        assert(dynamic_cast<TypeNonVoid*>(arg.resultType()) != 0);
        TypeNonVoid* t = static_cast<TypeNonVoid*>(arg.resultType());
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
        m_log.fatal () << "In context " << TypeContext::PrettyPrint (tyCxt) << " at " << tyCxt.location() << '.';

        bool haveCandidatesLabel = false;
        std::vector<Symbol *> cs = m_st->findPrefixed(SYM_PROCEDURE, name->value());
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
                    if (c->location()) {
                        m_log.info() << '\t' << *c << " at " << *(c->location());
                    } else {
                        m_log.info() << '\t' << *c;
                    }
                }
            }
        }
        cs = m_st->findPrefixed(SYM_TEMPLATE, name->value());
        if (!cs.empty()) {
            if (!haveCandidatesLabel)
                m_log.info() << "Candidates are:";
            BOOST_REVERSE_FOREACH(Symbol * c, cs) {
                assert(dynamic_cast<SymbolTemplate *>(c) != 0);
                if (c->location()) {
                    m_log.info() << '\t' << *c << " at " << *(c->location());
                } else {
                    m_log.info() << '\t' << *c;
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

        if (need->secType()->isPublic () && have->secType()->isPrivate ()) {
            m_log.fatalInProc(&tyCxt) << "Argument " << (i + 1) << " to procedure "
                << name->value() << " at " << arguments[i].location()
                << " is expected to be of public type instead of private!";
            return E_TYPE;
        }

        if (need->dimType() != have->dimType()) {
            m_log.fatalInProc(&tyCxt) << "Argument " << (i + 1) << " to procedure "
                << name->value() << " at " << arguments[i].location()
                << " has mismatching dimensionality.";
            return E_TYPE;
        }
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
    TreeNodeIdentifier *id = root->procName ();
    Status s = checkProcCall(id, *root, root->params (), resultType, symProc);
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
    SymbolTemplate::Weight best;
    std::vector<Instantiation> bestMatches;
    BOOST_FOREACH (SymbolTemplate* s, findTemplates (m_st, name)) {
        assert (s->decl ()->containingModule () != 0);
        Instantiation inst (s);
        if (unify (inst, tyCxt, argTypes)) {
            const SymbolTemplate::Weight& w = s->weight ();
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

//
// Following is little strange as the type AST nodes have not
// yet been visited by the type checker and thus don't have the
// cached type. There probably is much nicer solution to what im doing
// but ill stick to what works for now.
//
bool TypeChecker::unify (Instantiation& inst,
                         const TypeContext& tyCxt,
                         DataTypeProcedureVoid* argTypes) const
{
    SymbolTemplate* sym = inst.getTemplate ();
    std::vector<TemplateParameter>& params = inst.getParams ();
    const TreeNodeTemplate* t = sym->decl ();
    TemplateVarMap varMap;

    params.clear ();

    if (sym->expectsSecType () && !tyCxt.haveContextSecType ())
        return false;

    if (sym->expectsDimType () && !tyCxt.haveContextDimType ())
        return false;

    if (t->body ()->params ().size () != argTypes->paramTypes ().size ())
        return false;

    unsigned i = 0;
    BOOST_FOREACH (TreeNodeStmtDecl& decl, t->body ()->params ()) {
        TreeNodeType* argNodeTy = decl.varType ();
        DataType* expectedTy = argTypes->paramTypes ().at (i ++);

        // Verify security type:
        if (argNodeTy->secType ()->isPublic ()) {
            if (! expectedTy->secrecSecType ()->isPublic ())
                return false;
        }
        else {
            StringRef styId = argNodeTy->secType ()->identifier ()->value ();
            if (! mapVariable (varMap, styId, expectedTy->secrecSecType ()))
                return false;
        }

        // Verify data type:
        if (argNodeTy->dataType ()->isVariable ()) {
            TreeNodeDataTypeVarF* dataVar = static_cast<TreeNodeDataTypeVarF*>(argNodeTy->dataType ());
            StringRef styId = dataVar->identifier ()->value ();
            if (! mapVariable (varMap, styId, expectedTy->secrecDataType ()))
                return false;
        }
        else
        if (expectedTy->secrecDataType () != argNodeTy->dataType ()->cachedType ()) {
            return false;
        }

        // Verify dimensionality type:
        if (argNodeTy->dimType ()->isVariable ()) {
            TreeNodeDimTypeVarF* dimVar = static_cast<TreeNodeDimTypeVarF*>(argNodeTy->dimType ());
            StringRef styId = dimVar->identifier ()->value ();
            if (! mapVariable (varMap, styId, expectedTy->secrecDimType ()))
                return false;
        }
        else
        if (expectedTy->secrecDimType () != argNodeTy->dimType ()->cachedType ()) {
            return false;
        }
    }

    TreeNodeType* retNodeTy = t->body ()->returnType ();
    if (retNodeTy->isNonVoid ()) {

        // Verify security type:
        if (tyCxt.haveContextSecType ()) {
            if (retNodeTy->secType ()->isPublic ()) {
                if (! tyCxt.contextSecType ()->isPublic ())
                    return false;
            }
            else {
                StringRef styId = retNodeTy->secType ()->identifier ()->value ();
                if (! mapVariable (varMap, styId, tyCxt.contextSecType ()))
                    return false;
            }
        }

        // Verify data type:
        if (tyCxt.haveContextDataType ()) {
            TreeNodeDataTypeF* dataType = retNodeTy->dataType ();
            if (dataType->isVariable ()) {
                StringRef styId = static_cast<TreeNodeDataTypeVarF*>(dataType)->identifier ()->value ();
                if (! mapVariable (varMap, styId, tyCxt.contextDataType ()))
                    return false;
            }
            else {
                if (dataType->cachedType () != tyCxt.contextDataType ())
                    return false;
            }
        }

        // Verify dimensionality type:
        if (tyCxt.haveContextDimType ()) {
            TreeNodeDimTypeF* dimType = retNodeTy->dimType ();
            if (dimType->isVariable ()) {
                StringRef styId = static_cast<TreeNodeDimTypeVarF*>(dimType)->identifier ()->value ();
                if (! mapVariable (varMap, styId, tyCxt.contextDimType ()))
                    return false;
            }
            else {
                if (dimType->cachedType () != tyCxt.contextDimType ())
                    return false;
            }
        }
    }
    else {
        // this is not very pretty either...
        if (tyCxt.haveContextDataType ()) {
            return false;
        }
    }

    BOOST_FOREACH (TreeNodeQuantifier& quant, t->quantifiers ()) {
        StringRef typeVar = quant.typeVariable ()->value ();
        const TemplateParameter& param = varMap.find (typeVar)->second;
        if (quant.type () == NODE_TEMPLATE_DOMAIN_QUANT) {
            TreeNodeDomainQuantifier* domain = static_cast<TreeNodeDomainQuantifier*>(&quant);
            if (domain->kind () != 0) {
                if (param.secType ()->isPublic ())
                    return false;
                SymbolKind* sym = m_st->find<SYM_KIND>(domain->kind ()->value ());
                PrivateSecType* privArgTy = static_cast<PrivateSecType*>(param.secType ());
                if (sym != privArgTy->securityKind ()) {
                    return false;
                }
            }
        }

        params.push_back (param);
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

    proc = findProcedure (m_st, body->procedureName (),
        static_cast<DataTypeProcedureVoid*>(body->procedureType ()->dataType ()));
    if (proc == 0) {
        proc = appendProcedure (m_st, *body);
    }

    body->setSymbol (proc);
    std::swap (m_st, moduleST);
    return OK;
}

} // namespace SecreC
