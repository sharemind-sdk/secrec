/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "TypeChecker.h"

#include "Log.h"
#include "ModuleInfo.h"
#include "SecurityType.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "Templates.h"
#include "TreeNode.h"
#include "TypeUnifier.h"
#include "Types.h"

#include <boost/range.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <sstream>
#include <vector>

namespace SecreC {

namespace /* anonymous */ {

std::string mangleProcedure (const std::string& name, const TypeProc* dt)
{
    std::ostringstream os;
    os << name << dt->mangle ();
    return os.str ();
}

SymbolProcedure* appendProcedure (SymbolTable* st, const TreeNodeProcDef& procdef)
{
    const TypeProc* dt = procdef.procedureType();
    const std::string actualName = mangleProcedure (procdef.procedureName ().str(), dt);
    for (Symbol* _proc : st->findAll (SYM_PROCEDURE, actualName)) {
        SymbolProcedure* proc = static_cast<SymbolProcedure*>(_proc);
        if (proc->secrecType () == dt)
            return proc;
    }

    SymbolProcedure * ns = new SymbolUserProcedure (actualName, &procdef);
    st->appendSymbol (ns);
    return ns;
}

SymbolProcedure*
findProcedure (SymbolTable* st, StringRef name, const TypeProc* dt)
{
    return st->find<SYM_PROCEDURE>(mangleProcedure (name.str(), dt));
}

std::vector<SymbolProcedure*>
findProcedures (SymbolTable* st, StringRef name, const TypeProc* dt)
{
    std::vector<SymbolProcedure* > out;
    const std::string actualName = mangleProcedure (name.str(), dt);
    for (Symbol* _procSym : st->findAll (SYM_PROCEDURE, actualName)) {
        assert (dynamic_cast<SymbolProcedure*>(_procSym) != nullptr);
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
    for (Symbol* _symTempl : st->findAll (SYM_TEMPLATE, actualName)) {
        assert (dynamic_cast<SymbolTemplate*>(_symTempl) != nullptr);
        SymbolTemplate* symTempl = static_cast<SymbolTemplate*>(_symTempl);
        out.push_back (symTempl);
    }

    return out;
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
    const TypeProc* ty = TypeProc::get (std::vector<const TypeBasic*>());
    std::vector<SymbolProcedure *> ms = findProcedures (m_st, "main", ty);
    if (ms.size() > 1u) {
        m_log.fatal() << "Multiple definitions of main found!";
        return nullptr;
    } else if (ms.empty()) {
        m_log.fatal() << "No procedure \"void main()\" found!";
        return nullptr;
    }
    return ms.at(0u);
}

TypeChecker::Status TypeChecker::populateParamTypes(std::vector<const TypeBasic *> & params,
                                                    TreeNodeProcDef * proc)
{
    params.clear ();
    params.reserve (proc->params ().size ());
    for (TreeNodeStmtDecl& decl : proc->params ()) {
        TCGUARD (visitStmtDecl (&decl));
        assert (dynamic_cast<const TypeBasic*>(decl.resultType()) != nullptr);
        params.push_back (static_cast<const TypeBasic*>(decl.resultType()));
    }

    return OK;
}


/// Procedure definitions.
TypeChecker::Status TypeChecker::visitProcDef (TreeNodeProcDef * proc,
                                               SymbolTable * localScope)
{
    if (proc->m_cachedType != nullptr) {
        return OK;
    }

    std::swap (m_st, localScope);
    TreeNodeType* rt = proc->returnType ();
    TCGUARD (visitType (rt));

    if (proc->procedureName() == "main" && rt->isNonVoid()) {
        m_log.fatal() << "Invalid return type procedure 'main' at " << proc->location() << '.';
        return E_TYPE;
    }

    if (proc->procedureName() == "main" && proc->params ().size () > 0) {
        m_log.fatal() << "Invalid parameters for procedure 'main' at " << proc->location() << '.';
        return E_TYPE;
    }

    std::vector<const TypeBasic*> params;
    TCGUARD (populateParamTypes(params, proc));
    proc->m_cachedType = TypeProc::get (params, rt->secrecType ());

    std::swap (m_st, localScope);

    SymbolProcedure* procSym = appendProcedure (m_st, *proc);
    proc->setSymbol (procSym);

    const std::string& shortName = proc->identifier ()->value ().str();
    for (Symbol* sym : m_st->findAll (SYM_PROCEDURE, shortName)) {
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

TypeChecker::Status TypeChecker::checkProcCall(TreeNodeIdentifier * name,
                                               const TreeNodeExprProcCall & tyCxt,
                                               const TreeNodeSeqView<TreeNodeExpr>& arguments,
                                               const SecreC::Type *& resultType,
                                               SymbolProcedure *& symProc)
{
    std::vector<const TypeBasic*> argumentDataTypes;

    for (TreeNodeExpr& arg : arguments) {
        TCGUARD (visitExpr(&arg));
        if (checkAndLogIfVoid(&arg))
            return E_TYPE;
        arg.instantiateDataType ();
        assert(arg.resultType ()->kind () == Type::BASIC);
        argumentDataTypes.push_back (static_cast<const TypeBasic*>(arg.resultType ()));
    }

    const TypeProc* argTypes = TypeProc::get (argumentDataTypes);
    TCGUARD (findBestMatchingProc(symProc, name->value(), tyCxt, argTypes, &tyCxt));

    if (symProc == nullptr) {
        m_log.fatalInProc(&tyCxt) << "No matching procedure definitions for:";
        m_log.fatal () << '\t' << name->value() << argTypes->paramsToNormalString();
        m_log.fatal () << "In context " << TypeContext::PrettyPrint (tyCxt) << " at " << tyCxt.location() << '.';

        bool haveCandidatesLabel = false;
        std::vector<Symbol *> cs = m_st->findPrefixed(SYM_PROCEDURE, name->value());
        if (!cs.empty()) {
            std::vector<SymbolProcedure *> cps;
            do {
                assert(dynamic_cast<SymbolProcedure *>(cs.back()) != nullptr);
                SymbolProcedure * const p = static_cast<SymbolProcedure *>(cs.back());
                if (p->shortOf() == nullptr)
                    cps.push_back(p);
                cs.pop_back();
            } while (!cs.empty());
            if (!cps.empty()) {
                m_log.info() << "Candidates are:";
                haveCandidatesLabel = true;
                for (SymbolProcedure * c : cps) {
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
            for (Symbol * c : boost::adaptors::reverse (cs)) {
                assert(dynamic_cast<SymbolTemplate *>(c) != nullptr);
                if (c->location()) {
                    m_log.info() << '\t' << *c << " at " << *(c->location());
                } else {
                    m_log.info() << '\t' << *c;
                }
            }
        }

        return E_TYPE;
    }

    const TypeProc* ft = symProc->decl()->procedureType();

    // Check security types of parameters:
    assert(ft->paramTypes().size() == arguments.size ());
    for (unsigned i = 0; i < ft->paramTypes().size(); i++) {
        const TypeBasic* need = ft->paramTypes()[i];
        const TypeBasic* have = argTypes->paramTypes()[i];

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

TypeChecker::Status TypeChecker::visitExprProcCall (TreeNodeExprProcCall * root) {
    if (root->haveResultType())
        return OK;

    const Type* resultType = nullptr;
    SymbolProcedure* symProc = nullptr;
    TreeNodeIdentifier *id = root->procName ();
    TCGUARD (checkProcCall(id, *root, root->params (), resultType, symProc));
    root->setProcedure (symProc);
    root->setResultType (resultType);
    return OK;
}

TypeChecker::Status TypeChecker::findBestMatchingProc(SymbolProcedure *& symProc,
                                                      StringRef name,
                                                      const TypeContext & tyCxt,
                                                      const TypeProc* argTypes,
                                                      const TreeNode * errorCxt)
{
    assert(errorCxt);

    // Look for regular procedures:
    assert (argTypes != nullptr);
    SymbolProcedure* procTempSymbol = nullptr;
    for (SymbolProcedure* s : findProcedures (m_st, name, argTypes)) {
        const SecreC::Type* _ty = s->decl ()->returnType ()->secrecType ();
        if (! _ty->isVoid ()) { // and procedure is non-void...
            assert (dynamic_cast<const TypeNonVoid*>(_ty) != nullptr);
            const auto ty = static_cast<const TypeNonVoid*>(_ty);
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

        if (procTempSymbol != nullptr) {
            m_log.fatalInProc(errorCxt) << "Multiple matching procedures at "
                                        << errorCxt->location() << '.';
            return E_TYPE;
        }

        procTempSymbol = s;
    }

    if (procTempSymbol != nullptr) {
        symProc = procTempSymbol;
        return OK;
    }

    // Look for templates:
    SymbolTemplate::Weight best;
    std::vector<Instantiation> bestMatches;
    for (SymbolTemplate* s : findTemplates (m_st, name)) {
        assert (s->decl ()->containingModule () != nullptr);
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
        for (const Instantiation& i : bestMatches) {
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
                         const TypeProc* argTypes) const
{
    SymbolTemplate* sym = inst.getTemplate ();
    std::vector<TypeArgument>& params = inst.getParams ();
    const TreeNodeTemplate* t = sym->decl ();

    params.clear ();

    if (! providesExpectedTypeContext (sym, tyCxt))
        return false;

    if (t->body ()->params ().size () != argTypes->paramTypes ().size ())
        return false;

    TypeUnifier typeUnifier {m_st, t};

    unsigned i = 0;
    for (TreeNodeStmtDecl& decl : t->body ()->params ()) {
        TreeNodeType* argNodeTy = decl.varType ();
        const TypeBasic* expectedTy = argTypes->paramTypes ().at (i ++);
        if (! typeUnifier.visitType (argNodeTy, expectedTy)) {
            return false;
        }
    }

    TreeNodeType* retNodeTy = t->body ()->returnType ();
    if (retNodeTy->isNonVoid ()) {
        if (tyCxt.haveContextSecType ()) {
            const auto secType = retNodeTy->secType ();
            if (! typeUnifier.visitSecTypeF (secType, tyCxt.contextSecType ()))
                return false;
        }

        if (tyCxt.haveContextDataType ()) {
            const auto dataType = retNodeTy->dataType ();
            if (! typeUnifier.visitDataTypeF (dataType, tyCxt.contextDataType ()))
                return false;
        }

        // Verify dimensionality type:
        if (tyCxt.haveContextDimType ()) {
            const auto dimType = retNodeTy->dimType ();
            if (! typeUnifier.visitDimTypeF (dimType, tyCxt.contextDimType ()))
                return false;
        }
    }
    else {
        // this is not very pretty either...
        if (tyCxt.haveContextDataType ()) {
            return false;
        }
    }


    const auto& varMap = typeUnifier.typeVars ();
    for (TreeNodeQuantifier& quant : t->quantifiers ()) {
        StringRef typeVar = quant.typeVariable ()->value ();
        assert (varMap.find (typeVar) != varMap.end ());
        const TypeArgument& param = varMap.find (typeVar)->second;
        if (quant.type () == NODE_TEMPLATE_QUANTIFIER_DOMAIN) {
            TreeNodeQuantifierDomain* domain = static_cast<TreeNodeQuantifierDomain*>(&quant);
            if (domain->kind () != nullptr) {
                if (param.secType ()->isPublic ())
                    return false;
                else {
                    SymbolKind* sym = m_st->find<SYM_KIND>(domain->kind ()->value ());
                    const auto privArgTy = static_cast<const PrivateSecType*>(param.secType ());
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
    TCGUARD (visitProcDef (body, localST));

    proc = findProcedure (m_st, body->procedureName (), body->procedureType ());
    if (proc == nullptr) {
        proc = appendProcedure (m_st, *body);
    }

    body->setSymbol (proc);
    std::swap (m_st, moduleST);
    return OK;
}

} // namespace SecreC
