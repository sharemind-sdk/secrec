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

#include "../CastTypeUnifier.h"
#include "../DataType.h"
#include "../Log.h"
#include "../ModuleInfo.h"
#include "../OperatorTable.h"
#include "../OperatorTypeUnifier.h"
#include "../SecurityType.h"
#include "../SecurityType.h"
#include "../Symbol.h"
#include "../SymbolTable.h"
#include "../TreeNode.h"
#include "../typechecker/Templates.h"
#include "../TypeChecker.h"
#include "../TypeUnifier.h"
#include "../Types.h"

#include <algorithm>
#include <boost/range.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <sstream>
#include <vector>


using boost::adaptors::reverse;


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

// Both operators and casts use this
SymbolProcedure* appendOperator (OperatorTable* ops, const TreeNodeProcDef& procdef) {
    const TypeProc* dt = procdef.procedureType();
    const std::string actualName = mangleProcedure (procdef.procedureName ().str (), dt);
    for (SymbolProcedure* op : ops->findOperators (actualName)) {
        if (op->secrecType () == dt)
            return op;
    }

    SymbolProcedure* op = new SymbolUserProcedure (actualName, &procdef);
    ops->appendOperator (op);
    return op;
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

template <SymbolCategory symbolType>
std::vector<typename SymbolTraits<symbolType>::Type*>
findTemplates (SymbolTable* st, StringRef name)
{
    using T = typename SymbolTraits<symbolType>::Type;
    std::vector<T*> out;
    const std::string actualName = name.str();
    for (Symbol* _symTempl : st->findAll (symbolType, actualName)) {
        assert (dynamic_cast<T*>(_symTempl) != nullptr);
        T* symTempl = static_cast<T*>(_symTempl);
        out.push_back (symTempl);
    }

    return out;
}

bool providesExpectedTypeContext (SymbolProcedureTemplate* sym, const TypeContext& tyCxt) {
    if (sym->expectsSecType () && !tyCxt.haveContextSecType ())
        return false;

    if (sym->expectsDataType () && !tyCxt.haveContextDataType ())
        return false;

    if (sym->expectsDimType () && !tyCxt.haveContextDimType ())
        return false;

    return true;
}

bool opSupportedType (const TypeBasic* ty) {
    if (ty->secrecDataType ()->isComposite ())
        return false;

    if (ty->secrecDataType ()->isUserPrimitive ())
        return true;

    assert (dynamic_cast<const DataTypeBuiltinPrimitive*> (ty->secrecDataType ()));
    const DataTypeBuiltinPrimitive* dataTy = static_cast<const DataTypeBuiltinPrimitive*> (ty->secrecDataType ());

    if (dataTy->secrecDataType () == DATATYPE_STRING)
        return false;

    SecrecDimType dim = ty->secrecDimType ();
    if (!(dim == 0u || dim == 1u))
        return false;

    return true;
}

unsigned calculateOpScore (const TypeProc* callTypeProc,
                           const TreeNodeProcDef* proc)
{
    unsigned score = 0;

    assert (callTypeProc->paramTypes ().size () == proc->params ().size ());

    unsigned i = 0;
    for (TreeNodeStmtDecl const & param : proc->params ()) {
        const TypeBasic* argTy = callTypeProc->paramTypes ()[i++];
        const TreeNodeType* paramTy = param.varType ();

        if (! paramTy->secType ()->isPublic () &&
            argTy->secrecSecType ()->isPublic ())
            ++score;

        if (paramTy->dimType ()->cachedType () !=
            argTy->secrecDimType ())
            ++score;
    }

    return score;
}

// Tuple of: has kind constraint?, number of quantifier occurences in
// parameters, number of implicit classifies and reshapes
using OpWeight = std::tuple<unsigned, unsigned, unsigned>;

OpWeight calculateOpWeight (Instantiation& inst,
                            const TypeProc* callTypeProc)
{
    SymbolTemplate* t_ = inst.getTemplate ();
    assert (dynamic_cast<SymbolOperatorTemplate*> (t_) != nullptr);
    SymbolOperatorTemplate* t = static_cast<SymbolOperatorTemplate*> (t_);

    unsigned quantParamCount = t->quantifiedParamCount ();
    // Kind constraint is good, so we give lower weight (0) when it's
    // present
    unsigned domainWeight = t->domainWeight ();
    unsigned score = calculateOpScore (callTypeProc, t->decl ()->body ());

    return std::make_tuple (domainWeight, quantParamCount, score);
}

bool isRelational (SecrecOperator op) {
    switch (op) {
        case SCOP_BIN_EQ:
        case SCOP_BIN_GE:
        case SCOP_BIN_GT:
        case SCOP_BIN_LE:
        case SCOP_BIN_LT:
        case SCOP_BIN_NE:
            return true;
        default:
            return false;
    }
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

TypeChecker::Status TypeChecker::checkRedefinitions (const TreeNodeProcDef& proc)
{
    const std::string& shortName = proc.identifier ()->value ().str();
    for (Symbol* sym : m_st->findAll (SYM_PROCEDURE, shortName)) {
        if (sym->symbolType () == SYM_PROCEDURE) {
            SymbolProcedure* t = static_cast<SymbolProcedure*>(sym);
            if (t->decl ()->m_cachedType == proc.m_cachedType) {
                m_log.fatal () << "Redefinition of procedure '"
                               << proc.identifier()->value()
                               << "' at "
                               << proc.location () << '.'
                               << " Conflicting with procedure '"
                               << t->decl()->printableSignature()
                               << "' declared at "
                               << t->decl ()->location () << '.';
                return E_TYPE;
            }
            if (proc.identifier()->value() == "main" && t->decl()->identifier()->value() == "main") {
                m_log.fatal() << "Redefinition of procedure 'main' at "
                              << proc.location () << " not allowed!";
                m_log.fatal() << "Procedure 'main' already defined at "
                              << t->decl ()->location () << '.';
                return E_TYPE;
            }
        }
    }

    return OK;
}

/// Procedure definitions.
TypeChecker::Status TypeChecker::visitProcDef (TreeNodeProcDef * proc,
                                               SymbolTable * localScope,
                                               bool append)
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

    if (append) {
        SymbolProcedure* procSym = appendProcedure (m_st, *proc);
        proc->setSymbol (procSym);
    }

    TCGUARD (checkRedefinitions (*proc));

    return OK;
}

TypeChecker::Status TypeChecker::visitOpDef (TreeNodeOpDef* def,
                                             SymbolTable* localScope,
                                             bool append)
{
    if (def->m_cachedType != nullptr) {
        return OK;
    }

    std::swap (m_st, localScope);
    TreeNodeType* rtNode = def->returnType ();
    TCGUARD (visitType (rtNode));

    std::vector<const TypeBasic*> params;
    TCGUARD (populateParamTypes(params, def));
    const TypeProc* opType = TypeProc::get (params, rtNode->secrecType ());

    std::swap (m_st, localScope);

    const TypeBasic* lub = nullptr;
    if (params.size () == 2u) {
        lub = upperTypeBasic (params[0u], params[1u]);
    }
    else if (params.size () == 1u) {
        lub = params[0u];
    }
    else {
        assert (false);
    }

    SecrecOperator op = def->getOperator ();
    if (lub != nullptr && lub->secrecSecType ()->isPrivate () &&
        (op == SCOP_BIN_LAND || op == SCOP_BIN_LOR))
    {
        m_log.fatal () << "Short-circuited logical and/or defined on private values at"
                       << def->location() << ".";
        return E_TYPE;
    }

    const Type* returnType = rtNode->m_cachedType;

    if (returnType->isVoid ()) {
        m_log.fatal () << "Operator definition at " << def->location ()
                       << " does not return a value.";
        return E_TYPE;
    }

    const TypeBasic* rt = static_cast<const TypeBasic*> (returnType);

    if (! opSupportedType (rt)) {
        m_log.fatal () << "Operator definition at " << def->location ()
                       << " returns a value that is not a scalar or vector.";
        return E_TYPE;
    }

    for (unsigned i = 0; i < params.size (); ++i) {
        const TypeBasic* ty = params[i];

        if (! opSupportedType (ty)) {
            m_log.fatal () << "Operator definition at " << def->location ()
                           << " has an operand which is not a scalar or vector.";
            return E_TYPE;
        }
    }

    if (isRelational (op)) {
        if (lub == nullptr) {
            m_log.fatal () << "Operator definition at " << def->location ()
                           << " has operand types which have no least upper bound.";
            return E_TYPE;
        }

        if (lub->secrecSecType () != rt->secrecSecType ()) {
            m_log.fatal () << "Domain of return type of operator definition at "
                           << def->location ()
                           << " is not the least upper bound of domains of operand types.";
            return E_TYPE;
        }
    }
    else if (rt != lub) {
        m_log.fatal () << "Return type of operator definition at " << def->location ()
                       << " is not the least upper bound of the operand types.";
        return E_TYPE;
    }

    def->m_cachedType = opType;

    if (append) {
        SymbolProcedure* procSym = appendOperator (m_operators, *def);
        def->setSymbol (procSym);
    }

    TCGUARD (checkRedefinitions (*def));

    return OK;
}

TypeChecker::Status TypeChecker::visitCastDef (TreeNodeCastDef* def,
                                               SymbolTable* localScope,
                                               bool append)
{
    if (def->m_cachedType != nullptr) {
        return OK;
    }

    std::swap (m_st, localScope);

    TreeNodeType* rtNode = def->returnType ();
    TCGUARD (visitType (rtNode));

    std::vector<const TypeBasic*> params;
    TCGUARD (populateParamTypes (params, def));
    const TypeProc* procType = TypeProc::get (params, rtNode->secrecType ());

    std::swap (m_st, localScope);

    // Check return type
    if (rtNode->secrecType ()->isVoid ()) {
        m_log.fatalInProc (def)
            << "Return type of cast definition at "
            << def->location () << " is void.";
        return E_TYPE;
    }

    assert (dynamic_cast<const TypeBasic*> (rtNode->secrecType ()) != nullptr);
    const TypeBasic* rt = static_cast<const TypeBasic*> (rtNode->secrecType ());

    // Check security types
    assert (params.size() == 1);
    const TypeBasic* param = params[0];
    if (param->secrecSecType () != rt->secrecSecType ()) {
        m_log.fatalInProc (def)
            << "Security type of argument of cast definition at "
            << def->location () << " does not match security type of returned value.";
        return E_TYPE;
    }

    // Check data types
    if (rt->secrecDataType ()->isComposite () ||
        rt->secrecDataType ()->isString () ||
        param->secrecDataType ()->isComposite () ||
        param->secrecDataType ()->isString ())
    {
        m_log.fatalInProc (def)
            << "Cast defined on non-primitive or string type at "
            << def->location () << ".";
        return E_TYPE;
    }

    // Check dimensions
    if (param->secrecDimType () != rt->secrecDimType ()) {
        m_log.fatalInProc (def)
            << "Dimensionality of argument of cast definition at "
            << def->location () << " does not match dimensionality of returned value.";
        return E_TYPE;
    }

    if (rt->secrecDimType () != 1) {
        m_log.fatalInProc (def)
            << "Cast definition at " << def->location () << " is not defined on vectors.";
        return E_TYPE;
    }

    // Check that the definition is private
    if (! rtNode->secrecType ()->secrecSecType ()->isPrivate ()) {
        m_log.fatalInProc (def)
            << "Cast definition on public types at " << def->location () << ".";
        return E_TYPE;
    }

    def->m_cachedType = procType;

    if (append) {
        SymbolProcedure* procSym = appendOperator (m_operators, *def);
        def->setSymbol (procSym);
    }

    TCGUARD (checkRedefinitions (*def));

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
        cs = m_st->findPrefixed(SYM_PROCEDURE_TEMPLATE, name->value());
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

TypeChecker::Status TypeChecker::findRegularProc(SymbolProcedure *& symProc,
                                                 StringRef name,
                                                 const TypeContext & tyCxt,
                                                 const TypeProc * argTypes,
                                                 const TreeNode * errorCxt)
{
    assert (argTypes != nullptr);
    symProc = nullptr;

    for (SymbolProcedure* s : findProcedures (m_st, name, argTypes)) {
        const SecreC::Type* _ty = s->decl ()->returnType ()->secrecType ();
        if (! _ty->isVoid ()) { // and procedure is non-void...
            assert (dynamic_cast<const TypeNonVoid*>(_ty) != nullptr);
            const TypeNonVoid* ty = static_cast<const TypeNonVoid*>(_ty);
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

        if (symProc != nullptr) {
            symProc = nullptr;
            m_log.fatalInProc(errorCxt) << "Multiple matching procedures at "
                                        << errorCxt->location() << '.';
            return E_TYPE;
        }

        symProc = s;
    }

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
    SymbolProcedure* procTempSymbol = nullptr;
    TCGUARD (findRegularProc (procTempSymbol, name, tyCxt, argTypes, errorCxt));
    if (procTempSymbol != nullptr) {
        symProc = procTempSymbol;
        return OK;
    }

    // Look for templates:
    SymbolProcedureTemplate::Weight best;
    std::vector<Instantiation> bestMatches;
    for (SymbolProcedureTemplate* s : findTemplates<SYM_PROCEDURE_TEMPLATE> (m_st, name)) {
        assert (s->decl ()->containingModule () != nullptr);
        Instantiation inst (s);
        if (unify (inst, tyCxt, argTypes)) {
            const SymbolProcedureTemplate::Weight& w = s->weight ();
            if (w > best) continue;
            if (w < best) {
                bestMatches.clear ();
                best = w;
            }

            bestMatches.push_back (inst);
        }
    }

    if (bestMatches.empty ())
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

// This is latticeLEQ which does not check dimensions
bool latticeLeqOp(const TypeNonVoid* a, const TypeNonVoid* b) {
    if (a->kind () != b->kind ())
        return false;

    const DataType* dataType = b->secrecDataType ();
    if (b->secrecSecType ()->isPrivate () && a->secrecSecType ()->isPublic ()) {
        dataType = dtypeDeclassify (b->secrecSecType (), dataType);
    }

    return latticeSecTypeLEQ (a->secrecSecType (), b->secrecSecType ())
        && latticeDataTypeLEQ (a->secrecDataType (), dataType);
}

TypeChecker::Status TypeChecker::findRegularOpDef(SymbolProcedure *& symProc,
                                                  StringRef name,
                                                  const TypeProc * callTypeProc,
                                                  const TreeNode * errorCxt)
{
    assert (callTypeProc != nullptr);
    symProc = nullptr;

    std::vector<SymbolProcedure*> ops = m_operators->findOperators (name);
    std::vector<SymbolProcedure*> matching;
    unsigned best = ~0u;

    for (SymbolProcedure* op : ops) {
        const TypeProc* ty = op->decl ()->procedureType ();
        const std::vector<const TypeBasic*>& paramTypes = ty->paramTypes ();
        const std::vector<const TypeBasic*>& argTypes = callTypeProc->paramTypes ();

        if (paramTypes.size () != argTypes.size ())
            continue;

        bool bad = false;

        // Check if parameters and arguments match
        for (unsigned i = 0; i < argTypes.size (); ++i) {
            const TypeBasic* paramTy = paramTypes[i];
            const TypeBasic* argTy = argTypes[i];

            // Check if the combination of security and data type are leq parameter type
            if (! latticeLeqOp (argTy, paramTy)) {
                bad = true;
                break;
            }

            // If the call has non-scalar but procedure expects a scalar
            if (argTy->secrecDimType () > paramTy->secrecDimType () &&
                paramTy->isScalar())
            {
                bad = true;
                break;
            }
        }

        // Check return type
        const Type* retTy = ty->returnType ();
        assert (dynamic_cast<const TypeNonVoid*> (retTy) != nullptr);

        // This check is necessary to avoid using a private definition
        // when we can use a public operation. It basically checks if
        // sec(lub(call arguments)) == sec(return type)
        if (argTypes.size () == 1u) {
            if (argTypes[0u]->secrecSecType () != paramTypes[0u]->secrecSecType())
                bad = true;
        } else {
            const SecurityType* a = upperSecType (argTypes[0u]->secrecSecType (),
                                                  argTypes[1u]->secrecSecType ());
            const SecurityType* b = retTy->secrecSecType();

            if (a != b)
                bad = true;
        }

        if (! bad) {
            unsigned score = calculateOpScore (callTypeProc, op->decl ());

            if (score > best) continue;
            if (score < best) {
                matching.clear ();
                best = score;
            }
            matching.push_back (op);
        }
    }

    if (matching.size () > 1u) {
        std::ostringstream os;
        os << "Multiple matching operator definitions: ";
        for (const SymbolProcedure* i : matching) {
            os << i->decl ()->location () << ' ';
        }

        m_log.fatalInProc (errorCxt) << os.str () << "at "
                                     << errorCxt->location () << '.';
        return E_TYPE;
    }

    if (matching.size () == 1u)
        symProc = matching[0u];

    return OK;
}

TypeChecker::Status TypeChecker::findBestMatchingOpDef(SymbolProcedure *& symProc,
                                                       StringRef name,
                                                       const TypeContext & tyCxt,
                                                       const TypeProc * callTypeProc,
                                                       const TreeNode * errorCxt)
{
    assert (errorCxt);

    // Look for non-templated operator definitions:
    TCGUARD (findRegularOpDef (symProc, name, callTypeProc, errorCxt));
    if (symProc != nullptr)
        return OK;

    // Look for templates:
    unsigned maxi = ~((unsigned) 0);
    OpWeight best = std::make_tuple (maxi, maxi, maxi);
    std::vector<Instantiation> bestMatches;

    for (SymbolOperatorTemplate* s : m_operators->findOperatorTemplates (name)) {
        assert (s->decl ()->containingModule () != nullptr);
        Instantiation inst (s);

        if (unifyOperator (inst, tyCxt, callTypeProc)) {
            OpWeight w = calculateOpWeight (inst, callTypeProc);
            if (w > best)
                continue;
            if (w < best) {
                bestMatches.clear ();
                best = w;
            }

            bestMatches.push_back (inst);
        }
    }

    if (bestMatches.empty ())
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

TypeChecker::Status TypeChecker::findBestMatchingCastDef(SymbolProcedure *& symProc,
                                                         const TypeBasic * arg,
                                                         const TypeBasic * want,
                                                         const TreeNode * errorCxt)
{
    assert (errorCxt);

    symProc = nullptr;

    // Look for non-templated cast definitions:
    {
        std::vector<SymbolProcedure*> defs = m_operators->findOperators ("__cast");
        std::vector<SymbolProcedure*> matching;
        unsigned best = ~0u;

        for (SymbolProcedure* def : defs) {
            const TypeProc* ty = def->decl ()->procedureType ();

            assert (ty->paramTypes ().size () == 1u);
            assert (dynamic_cast<const TypeBasic*> (ty->paramTypes ()[0u]) != nullptr);
            const TypeBasic* param = static_cast<const TypeBasic*> (ty->paramTypes ()[0u]);

            // Check arg
            if (arg->secrecSecType () != param->secrecSecType () ||
                arg->secrecDataType () != param->secrecDataType ())
            {
                continue;
            }

            // Check return type
            assert (dynamic_cast<const TypeBasic*> (ty->returnType ()) != nullptr);
            const TypeBasic* retTy = static_cast<const TypeBasic*> (ty->returnType ());
            if (retTy->secrecDataType () != want->secrecDataType ()) {
                continue;
            }

            unsigned score = 0u;
            if (arg->secrecDimType () != param->secrecDimType ())
                ++score;

            if (score > best) continue;
            if (score < best) {
                matching.clear ();
                best = score;
            }
            matching.push_back (def);
        }

        if (matching.size () > 1u) {
            std::ostringstream os;
            os << "Multiple matching cast definitions: ";
            for (const SymbolProcedure* i : matching) {
                os << i->decl ()->location () << ' ';
            }

            m_log.fatalInProc (errorCxt) << os.str () << "at "
                                         << errorCxt->location () << '.';
            return E_TYPE;
        }

        if (matching.size () == 1u) {
            symProc = matching[0u];
            return OK;
        }
    }

    // Look for templates:
    unsigned maxi = ~((unsigned) 0);
    OpWeight best = std::make_tuple (maxi, maxi, maxi);
    std::vector<Instantiation> bestMatches;
    std::vector<const TypeBasic*> args { arg };
    const TypeProc* callTypeProc = TypeProc::get (args);

    for (SymbolOperatorTemplate* s : m_operators->findOperatorTemplates ("__cast")) {
        assert (s->decl ()->containingModule () != nullptr);
        Instantiation inst (s);

        if (unifyCast (inst, arg, want)) {
            OpWeight w = calculateOpWeight (inst, callTypeProc);
            if (w > best)
                continue;
            if (w < best) {
                bestMatches.clear ();
                best = w;
            }

            bestMatches.push_back (inst);
        }
    }

    if (bestMatches.empty ())
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
    SymbolTemplate* sym_ = inst.getTemplate ();
    assert (dynamic_cast<SymbolProcedureTemplate*> (sym_));
    SymbolProcedureTemplate* sym = static_cast<SymbolProcedureTemplate*> (sym_);
    std::vector<TypeArgument>& params = inst.getParams ();
    const TreeNodeTemplate* t = sym->decl ();

    params.clear ();

    if (! providesExpectedTypeContext (sym, tyCxt))
        return false;

    if (t->body ()->params ().size () != argTypes->paramTypes ().size ())
        return false;

    TypeUnifier typeUnifier {m_st, sym};

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

bool TypeChecker::unifyOperator (Instantiation& inst,
                                 const TypeContext& tyCxt,
                                 const TypeProc* argTypes) const
{
    SymbolTemplate* sym_ = inst.getTemplate ();
    assert (dynamic_cast<SymbolOperatorTemplate*> (sym_) != nullptr);
    SymbolOperatorTemplate* sym = static_cast<SymbolOperatorTemplate*> (sym_);
    std::vector<TypeArgument>& params = inst.getParams ();
    const TreeNodeTemplate* t = sym->decl ();

    params.clear ();

    if (t->body ()->params ().size () != argTypes->paramTypes ().size ())
        return false;

    if (sym->expectsDataType () && ! tyCxt.haveContextDataType ())
        return false;

    OperatorTypeUnifier typeUnifier {argTypes->paramTypes (), m_st, sym};

    if (! typeUnifier.checkDomainQuantifier ())
        return false;

    unsigned i = 0;
    for (TreeNodeStmtDecl& decl : t->body ()->params ()) {
        TreeNodeType* argNodeTy = decl.varType ();
        const TypeBasic* expectedTy = argTypes->paramTypes ().at (i ++);
        if (! typeUnifier.visitType (argNodeTy, expectedTy))
            return false;
    }

    TreeNodeType* retNodeTy = t->body ()->returnType ();
    assert (retNodeTy->isNonVoid ());

    if (tyCxt.haveContextDataType () &&
        ! typeUnifier.checkReturnDataType (retNodeTy, tyCxt.contextDataType ()))
    {
        return false;
    }

    if (! typeUnifier.checkSecLUB ())
        return false;

    typeUnifier.getTypeArguments (params);

    return true;
}

bool TypeChecker::unifyCast (Instantiation& inst,
                             const TypeBasic* arg,
                             const TypeBasic* want) const
{
    SymbolTemplate* sym = inst.getTemplate ();
    std::vector<TypeArgument>& params = inst.getParams ();
    const TreeNodeTemplate* t = sym->decl ();

    params.clear ();

    CastTypeUnifier typeUnifier {arg, m_st, sym};

    if (! typeUnifier.checkKind ()) {
        return false;
    }

    assert (t->body ()->params ().size () == 1);
    TreeNodeStmtDecl& decl = t->body ()->params ()[0];
    TreeNodeType* argNodeTy = decl.varType ();
    if (! typeUnifier.visitType (argNodeTy, arg)) {
        return false;
    }

    TreeNodeType* retTy = t->body ()->returnType ();
    assert (retTy->isNonVoid ());
    if (! typeUnifier.visitDataTypeF (retTy, want)) {
        return false;
    }

    typeUnifier.getTypeArguments (params);

    return true;
}

TypeChecker::Status TypeChecker::getInstance (SymbolProcedure *& proc,
                                              const Instantiation & inst)
{
    ModuleInfo* mod = inst.getTemplate ()->decl ()->containingModule ();
    InstanceInfo info = m_instantiator->add (inst, *mod);
    TreeNodeProcDef* body = info.m_generatedBody;
    SymbolTable* moduleST = info.m_moduleInfo->codeGenState ().st ();
    SymbolTable* localST = info.m_localScope;

    SymbolProcedure* tmp = m_instantiator->getProcedure (inst);
    if (tmp != nullptr) {
        proc = tmp;
        return OK;
    }

    assert (localST->parent () == moduleST);
    std::swap (m_st, moduleST);
    if (body->isOperator ())
        TCGUARD (visitOpDef (static_cast<TreeNodeOpDef*> (body), localST, false));
    else if (body->isCast ())
        TCGUARD (visitCastDef (static_cast<TreeNodeCastDef*> (body), localST, false));
    else
        TCGUARD (visitProcDef (body, localST, false));
    std::swap (m_st, moduleST);

    const TypeProc* dt = body->procedureType ();
    const std::string actualName = mangleProcedure (body->procedureName ().str (), dt);
    proc = new SymbolUserProcedure (actualName, body);
    body->setSymbol (proc);
    moduleST->appendOtherSymbol (proc);
    m_instantiator->addProcedure (inst, proc);
    m_instantiator->addToWorkList (inst);

    return OK;
}

} // namespace SecreC
