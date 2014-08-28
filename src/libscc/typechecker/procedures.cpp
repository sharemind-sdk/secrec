/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "typechecker.h"

#include "log.h"
#include "ModuleInfo.h"
#include "symbol.h"
#include "symboltable.h"
#include "templates.h"
#include "treenode.h"

#include <boost/foreach.hpp>
#include <boost/range.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple.hpp>
#include <sstream>
#include <vector>

namespace SecreC {

namespace /* anonymous */ {

std::string mangleProcedure (const std::string& name, TypeProc* dt)
{
    std::ostringstream os;
    os << name << dt->mangle ();
    return os.str ();
}

SymbolProcedure* appendProcedure (SymbolTable* st, const TreeNodeProcDef& procdef)
{
    TypeProc* dt = procdef.procedureType();
    const std::string actualName = mangleProcedure (procdef.procedureName ().str(), dt);
    BOOST_FOREACH (Symbol* _proc, st->findAll (SYM_PROCEDURE, actualName)) {
        SymbolProcedure* proc = static_cast<SymbolProcedure*>(_proc);
        if (proc->secrecType () == dt)
            return proc;
    }

    SymbolProcedure * ns = new SymbolUserProcedure (actualName, &procdef);
    st->appendSymbol (ns);
    return ns;
}

SymbolProcedure*
findProcedure (SymbolTable* st, StringRef name, TypeProc* dt)
{
    return st->find<SYM_PROCEDURE>(mangleProcedure (name.str(), dt));
}

std::vector<SymbolProcedure*>
findProcedures (SymbolTable* st, StringRef name, TypeProc* dt)
{
    std::vector<SymbolProcedure* > out;
    const std::string actualName = mangleProcedure (name.str(), dt);
    BOOST_FOREACH (Symbol* _procSym, st->findAll (SYM_PROCEDURE, actualName)) {
        assert (dynamic_cast<SymbolProcedure*>(_procSym) != NULL);
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
        assert (dynamic_cast<SymbolTemplate*>(_symTempl) != NULL);
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

bool providesExpectedTypeContext (SymbolTemplate* sym, const TypeContext& tyCxt) {
    if (sym->expectsSecType () && !tyCxt.haveContextSecType ())
        return false;

    if (sym->expectsDataType () && !tyCxt.haveContextDataType ())
        return false;

    if (sym->expectsDimType () && !tyCxt.haveContextDimType ())
        return false;

    return true;
}

} // anonymous namespace

/// Return symbol for the main procedure (if exists).
SymbolProcedure* TypeChecker::mainProcedure ()
{
    TypeProc* ty = TypeProc::get (getContext (), std::vector<TypeBasic*>());
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

TypeChecker::Status TypeChecker::populateParamTypes(std::vector<TypeBasic *> & params,
                                                    TreeNodeProcDef * proc)
{
    params.clear ();
    params.reserve (proc->params ().size ());
    BOOST_FOREACH (TreeNodeStmtDecl& decl, proc->params ()) {
        TCGUARD (visit (&decl));
        assert (dynamic_cast<TypeBasic*>(decl.resultType()) != NULL);
        params.push_back (static_cast<TypeBasic*>(decl.resultType()));
    }

    return OK;
}


/// Procedure definitions.
TypeChecker::Status TypeChecker::visit(TreeNodeProcDef * proc,
                                       SymbolTable * localScope)
{
    if (proc->m_cachedType != NULL) {
        return OK;
    }

    std::swap (m_st, localScope);
    TreeNodeType* rt = proc->returnType ();
    TCGUARD (visit(rt));

    if (proc->procedureName() == "main" && rt->isNonVoid()) {
        m_log.fatal() << "Invalid return type procedure 'main' at " << proc->location() << '.';
        return E_TYPE;
    }

    if (proc->procedureName() == "main" && proc->params ().size () > 0) {
        m_log.fatal() << "Invalid parameters for procedure 'main' at " << proc->location() << '.';
        return E_TYPE;
    }

    std::vector<TypeBasic*> params;
    TCGUARD (populateParamTypes(params, proc));
    proc->m_cachedType = TypeProc::get (getContext (), params, rt->secrecType ());

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

    m_st->appendSymbol(new SymbolUserProcedure(shortName, proc));

    return OK;
}

/**
 * Procedure calls. This includes both calls to templates and calls to
 * regular procedures.
 */

TypeChecker::Status TreeNodeExprProcCall::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::checkProcCall(TreeNodeIdentifier * name,
                                               const TreeNodeExprProcCall & tyCxt,
                                               const TreeNodeSeqView<TreeNodeExpr>& arguments,
                                               SecreC::Type *& resultType,
                                               SymbolProcedure *& symProc)
{
    std::vector<TypeBasic*> argumentDataTypes;

    BOOST_FOREACH (TreeNodeExpr& arg, arguments) {
        TCGUARD (visitExpr(&arg));
        if (checkAndLogIfVoid(&arg))
            return E_TYPE;
        arg.instantiateDataType (getContext ());
        assert(arg.resultType ()->kind () == Type::BASIC);
        argumentDataTypes.push_back (static_cast<TypeBasic*>(arg.resultType ()));
    }

    TypeProc* argTypes = TypeProc::get (getContext (), argumentDataTypes);
    TCGUARD (findBestMatchingProc(symProc, name->value(), tyCxt, argTypes, &tyCxt));

    if (symProc == NULL) {
        m_log.fatalInProc(&tyCxt) << "No matching procedure definitions for:";
        m_log.fatal () << '\t' << name->value() << argTypes->paramsToNormalString();
        m_log.fatal () << "In context " << TypeContext::PrettyPrint (tyCxt) << " at " << tyCxt.location() << '.';

        bool haveCandidatesLabel = false;
        std::vector<Symbol *> cs = m_st->findPrefixed(SYM_PROCEDURE, name->value());
        if (!cs.empty()) {
            std::vector<SymbolProcedure *> cps;
            do {
                assert(dynamic_cast<SymbolProcedure *>(cs.back()) != NULL);
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
                assert(dynamic_cast<SymbolTemplate *>(c) != NULL);
                if (c->location()) {
                    m_log.info() << '\t' << *c << " at " << *(c->location());
                } else {
                    m_log.info() << '\t' << *c;
                }
            }
        }

        return E_TYPE;
    }

    TypeProc* ft = symProc->decl()->procedureType();

    // Check security types of parameters:
    assert(ft->paramTypes().size() == arguments.size ());
    for (unsigned i = 0; i < ft->paramTypes().size(); i++) {
        TypeBasic* need = ft->paramTypes()[i];
        TypeBasic* have = argTypes->paramTypes()[i];

        if (need->secrecSecType ()->isPublic () && have->secrecSecType ()->isPrivate ()) {
            m_log.fatalInProc(&tyCxt) << "Argument " << (i + 1) << " to procedure "
                << name->value() << " at " << arguments[i].location()
                << " is expected to be of public type instead of private!";
            return E_TYPE;
        }

        if (need->secrecDimType () != have->secrecDimType ()) {
            m_log.fatalInProc(&tyCxt) << "Argument " << (i + 1) << " to procedure "
                << name->value() << " at " << arguments[i].location()
                << " has mismatching dimensionality.";
            return E_TYPE;
        }
    }

    // Set result type:
    resultType = ft->returnType ();
    return OK;
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprProcCall * root) {
    if (root->haveResultType())
        return OK;

    Type* resultType = NULL;
    SymbolProcedure* symProc = NULL;
    TreeNodeIdentifier *id = root->procName ();
    TCGUARD (checkProcCall(id, *root, root->params (), resultType, symProc));
    root->setProcedure (symProc);
    root->setResultType (resultType);
    return OK;
}

TypeChecker::Status TypeChecker::findBestMatchingProc(SymbolProcedure *& symProc,
                                                      StringRef name,
                                                      const TypeContext & tyCxt,
                                                      TypeProc* argTypes,
                                                      const TreeNode * errorCxt)
{
    assert(errorCxt);

    // Look for regular procedures:
    assert (argTypes != NULL);
    SymbolProcedure* procTempSymbol = NULL;
    BOOST_FOREACH (SymbolProcedure* s, findProcedures (m_st, name, argTypes)) {
        SecreC::Type* _ty = s->decl ()->returnType ()->secrecType ();
        if (! _ty->isVoid ()) { // and procedure is non-void...
            assert (dynamic_cast<TypeNonVoid*>(_ty) != NULL);
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

        if (procTempSymbol != NULL) {
            m_log.fatalInProc(errorCxt) << "Multiple matching procedures at "
                                        << errorCxt->location() << '.';
            return E_TYPE;
        }

        procTempSymbol = s;
    }

    if (procTempSymbol != NULL) {
        symProc = procTempSymbol;
        return OK;
    }

    // Look for templates:
    SymbolTemplate::Weight best;
    std::vector<Instantiation> bestMatches;
    BOOST_FOREACH (SymbolTemplate* s, findTemplates (m_st, name)) {
        assert (s->decl ()->containingModule () != NULL);
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
                         TypeProc* argTypes) const
{
    SymbolTemplate* sym = inst.getTemplate ();
    std::vector<TemplateParameter>& params = inst.getParams ();
    const TreeNodeTemplate* t = sym->decl ();
    TemplateVarMap varMap;

    params.clear ();

    if (! providesExpectedTypeContext (sym, tyCxt))
        return false;

    if (t->body ()->params ().size () != argTypes->paramTypes ().size ())
        return false;

    unsigned i = 0;
    BOOST_FOREACH (TreeNodeStmtDecl& decl, t->body ()->params ()) {
        TreeNodeType* argNodeTy = decl.varType ();
        TypeBasic* expectedTy = argTypes->paramTypes ().at (i ++);

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
            SymbolDataType* symDataType = m_st->find<SYM_TYPE> (styId);
            if (symDataType != NULL) {
                if (expectedTy->secrecDataType () != symDataType->dataType ())
                    return false;
            }
            else {
                if (! mapVariable (varMap, styId, expectedTy->secrecDataType ()))
                    return false;
            }
        }
        else {
            TreeNodeDataTypeConstF* argDataType = static_cast<TreeNodeDataTypeConstF*>(argNodeTy->dataType ());
            SecrecDataType secrecDataType = argDataType->secrecDataType ();
            if (! sameDataTypes (expectedTy->secrecDataType (), secrecDataType)) {
                return false;
            }
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
                TreeNodeDataTypeConstF* argDataType = static_cast<TreeNodeDataTypeConstF*>(dataType);
                SecrecDataType secrecDataType = argDataType->secrecDataType ();
                if (! sameDataTypes (secrecDataType, tyCxt.contextDataType ()))
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
            if (domain->kind () != NULL) {
                if (param.secType ()->isPublic ())
                    return false;
                else {
                    SymbolKind* sym = m_st->find<SYM_KIND>(domain->kind ()->value ());
                    PrivateSecType* privArgTy = static_cast<PrivateSecType*>(param.secType ());
                    if (sym != privArgTy->securityKind ()) {
                        return false;
                    }
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
    TCGUARD (visit(body, localST));

    proc = findProcedure (m_st, body->procedureName (), body->procedureType ());
    if (proc == NULL) {
        proc = appendProcedure (m_st, *body);
    }

    body->setSymbol (proc);
    std::swap (m_st, moduleST);
    return OK;
}

} // namespace SecreC
