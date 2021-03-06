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

#include "../CodeGen.h"
#include "../CodeGenResult.h"
#include "../Constant.h"
#include "../DataType.h"
#include "../Log.h"
#include "../Misc.h"
#include "../StringTable.h"
#include "../SymbolTable.h"
#include "../TreeNode.h"
#include "../TypeChecker.h"
#include "../Types.h"

/**
 * Code generation for statements.
 */

namespace SecreC {

namespace /* anonymous */ {

// Initializes structure fields (and also associated size and shape symbols of the fields).
// Note that this does not assign values to those symbols.
void initFieldSymbols (StringTable& strTab, SymbolTable* st, SymbolSymbol* sym, const TypeBasic* ty) {
    const auto structType = static_cast<const DataTypeStruct*>(ty->secrecDataType ());
    for (const auto& field : structType->fields ()) {
        // The following is safe as a "." can not normally occur in a name
        const auto name = sym->name () + "." + field.name.str ();
        const TypeBasic* fieldType = field.type;
        SymbolSymbol* fieldSymbol = new SymbolSymbol (*strTab.addString (name), fieldType);
        initShapeSymbols (st, fieldSymbol);
        if (fieldType->secrecDataType ()->isComposite ()) {
            initFieldSymbols (strTab, st, fieldSymbol, fieldType);
        }

        sym->appendField (fieldSymbol);
    }
}

// Mark the given symbol, and associated temporaries, as global symbols.
void setSymbolGlobalScope (SymbolSymbol* sym) {
    const TypeNonVoid* ty = sym->secrecType ();
    sym->setScopeType (SymbolSymbol::GLOBAL);
    for (SecrecDimType i = 0; i < ty->secrecDimType(); ++ i) {
        setSymbolGlobalScope (sym->getDim (i));
    }

    if (!ty->isScalar ()) {
        setSymbolGlobalScope (sym->getSizeSym ());
    }

    if (ty->secrecDataType ()->isComposite ()) {
        for (SymbolSymbol* field : sym->fields ()) {
            setSymbolGlobalScope (field);
        }
    }
}

} // namespace anonymous

// Initializes associated size and shape symbols. Does not assign values.
void initShapeSymbols (SymbolTable* st, SymbolSymbol* sym) {
    const TypeBasic * const dimType = TypeBasic::getIndexType();
    const TypeNonVoid* ty = sym->secrecType ();
    for (SecrecDimType i = 0; i < ty->secrecDimType(); ++ i) {
        sym->setDim(i, st->appendTemporary (dimType));
    }

    if (sym->isArray ()) { // set size symbol
        sym->setSizeSym(st->appendTemporary (dimType));
    }
}

/*******************************************************************************
  TreeNodeStmtCompound
*******************************************************************************/

CGStmtResult TreeNodeStmtCompound::codeGenWith (CodeGen& cg) {
    return cg.cgStmtCompound (this);
}

CGStmtResult CodeGen::cgStmtCompound(TreeNodeStmtCompound * s) {
    CGStmtResult result;

    newScope();

    for (TreeNode * c_ : s->children()) {
        assert(dynamic_cast<TreeNodeStmt *>(c_) != nullptr);
        TreeNodeStmt * c = static_cast<TreeNodeStmt *>(c_);
        const CGStmtResult & cResult = codeGenStmt(c);
        append(result, cResult);
        if (result.isFatal()) break;
        if (result.isNotOk()) continue;

        if (cResult.firstImop() == nullptr) {
            if (c->type() != NODE_DECL) {
                m_log.fatalInProc(s) << "Statement with no effect at " << c->location() << '.';
                result |= CGResult::ERROR_CONTINUE;
                continue;
            }
        }

        result.addToBreakList(cResult.breakList());
        result.addToContinueList(cResult.continueList());

        // Static checking:
        if ((result.flags() & CGStmtResult::FALLTHRU) == 0x0) {
            m_log.fatalInProc(s) << "Unreachable statement at " << c->location() << '.';
            result |= CGResult::ERROR_CONTINUE;
            continue;
        } else {
            result.setFlags((result.flags() & ~CGStmtResult::FALLTHRU)
                    | cResult.flags());
        }
    }

    if (result.mayFallThrough()) {
        releaseScopeVariables(result);
    }

    popScope();

    return result;
}

/*******************************************************************************
  TreeNodeStmtBreak
*******************************************************************************/

CGStmtResult TreeNodeStmtBreak::codeGenWith(CodeGen & cg) {
    return cg.cgStmtBreak(this);
}

CGStmtResult CodeGen::cgStmtBreak(TreeNodeStmtBreak * s) {
    if (loopST() == nullptr) {
        m_log.fatalInProc(s) << "Break statement not embedded in loop at " << s->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    CGStmtResult result;
    assert(loopST() != nullptr);
    for (SymbolSymbol * var : m_st->variablesUpTo(loopST())) {
        releaseResource(result, var);
    }

    auto i = new Imop(s, Imop::JUMP, nullptr);
    pushImopAfter(result, i);
    result.setFirstImop(i);
    result.addToBreakList(i);
    result.setFlags(CGStmtResult::BREAK);
    return result;
}

/*******************************************************************************
  TreeNodeStmtContinue
*******************************************************************************/

CGStmtResult TreeNodeStmtContinue::codeGenWith(CodeGen & cg) {
    return cg.cgStmtContinue(this);
}

CGStmtResult CodeGen::cgStmtContinue(TreeNodeStmtContinue * s) {
    if (loopST() == nullptr) {
        m_log.fatalInProc(s) << "Continue statement not embedded in loop at " << s->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    CGStmtResult result;
    for (SymbolSymbol * var : m_st->variablesUpTo(loopST())) {
        releaseResource(result, var);
    }

    auto i = new Imop(s, Imop::JUMP, nullptr);
    pushImopAfter(result, i);
    result.setFirstImop(i);
    result.addToContinueList(i);
    result.setFlags(CGStmtResult::CONTINUE);
    return result;
}

/*******************************************************************************
  TreeNodeStmtDecl
*******************************************************************************/

CGStmtResult CodeGen::cgGlobalVarInit (const TypeNonVoid* ty, TreeNodeVarInit* varInit)
{
    if (m_tyChecker->checkVarInit(ty, varInit) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGStmtResult result;

    // JUMP SKIP;
    result.setFirstImop(pushComment ("Global variable initialization:"));
    auto skip = new Imop(varInit, Imop::JUMP, static_cast<Symbol *>(nullptr));
    push_imop (skip);

    const auto ns = new SymbolSymbol(varInit->variableName(), ty);
    initShapeSymbols (m_st, ns);
    m_st->appendSymbol (ns);

    if (ty->secrecDataType ()->isComposite()) {
        initFieldSymbols (getStringTable (), m_st, ns, static_cast<const TypeBasic*> (ty));
    }

    setSymbolGlobalScope (ns);

    // <initialization function>
    SymbolProcedure* procSym = nullptr;
    {
        ScopedScope localScope (*this);
        std::stringstream ss;
        ss << "__global_init_" << varInit->variableName ();

        const StringRef procName = *getStringTable ().addString (ss.str ());
        const TypeProc* procType = TypeProc::get (std::vector<const TypeBasic*>(), ty);
        procSym = new SymbolProcedure (procName, procType);
        m_st->appendSymbol (procSym);

        // header
        Imop* funcStart = pushComment ("Global variable initialization function:");
        procSym->setTarget (funcStart);

        // body
        CGStmtResult varInitResult = cgLocalVarInit (ty, varInit);
        append (result, varInitResult);
        if (result.isNotOk ())
            return result;

        // footer
        Symbol* localResultSym = varInitResult.symbol ();

        std::vector<Symbol*> rets;
        for (Symbol* sym : flattenSymbol (localResultSym)) {
            rets.push_back (copyNonTemporary (result, sym));
        }

        releaseProcVariables (result, localResultSym);
        if (result.isNotOk ())
            return result;

        Imop * i = newReturn(varInit, rets.begin(), rets.end());
        i->setDest(m_st->label(funcStart));
        pushImopAfter(result, i);
    }

    std::vector<Symbol*> retList, argList;
    retList = flattenSymbol (ns);

    Imop* callImop = newCall (varInit, retList.begin (), retList.end (), argList.begin (), argList.end ());
    auto cleanImop = new Imop (varInit, Imop::RETCLEAN, nullptr, nullptr, nullptr);
    m_callsTo[procSym].insert(callImop);
    cleanImop->setArg2 (m_st->label (callImop));
    skip->setDest (m_st->label (callImop));
    callImop->setDest (procSym);
    push_imop (callImop);
    push_imop (cleanImop);
    result.setResult(ns);
    codeGenSize(result);
    return result;
}

CGStmtResult CodeGen::cgLocalVarInit (const TypeNonVoid* ty, TreeNodeVarInit* varInit)
{
    return cgVarInit (ty, varInit, false);
}

CGStmtResult CodeGen::cgProcParamInit (const TypeNonVoid* ty, TreeNodeVarInit* varInit)
{
    return cgVarInit (ty, varInit, true);
}

CGStmtResult CodeGen::cgVarInit (const TypeNonVoid* ty,
                                 TreeNodeVarInit* varInit,
                                 bool isProcParam)
{
    if (m_tyChecker->checkVarInit(ty, varInit) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    ScopedSetNode scopedSetNode (*this, varInit);

    const bool isScalar = ty->isScalar();
    #ifndef NDEBUG
    const bool isString = ty->secrecDataType()->isString ();
    #endif
    const bool isStruct = ty->secrecDataType ()->isComposite ();

    const auto ns = new SymbolSymbol(varInit->variableName(), ty);
    m_st->appendSymbol(ns);

    SecrecDimType shapeExpressions = 0;
    assert((isScalar || !isString)
           && "ICE: string arrays should be forbidden by the type checker!");

    // Initialize shape:
    initShapeSymbols (m_st, ns);

    if (isStruct) {
        initFieldSymbols (getStringTable (), m_st, ns, static_cast<const TypeBasic*>(ty));
    }

    CGStmtResult result;
    result.setResult (ns);

    // evaluate shape if given, also compute size
    if (! varInit->shape().empty()) {
        if (!isScalar)
            emplaceImopAfter(result, varInit, Imop::ASSIGN,
                             ns->getSizeSym(), indexConstant(1));

        for (TreeNodeExpr& e : varInit->shape()) {

            // Evaluate shape expression:
            const CGResult & eResult = codeGen(&e);
            append(result, eResult);
            if (result.isNotOk())
                return result;

            emplaceImopAfter(result, varInit, Imop::ASSIGN,
                             ns->getDim(shapeExpressions), eResult.symbol());

            emplaceImop(varInit, Imop::MUL, ns->getSizeSym(),
                        ns->getSizeSym(), eResult.symbol());

            ++shapeExpressions;
        }

        // TypeChecker::checkVarInit() should ensure this:
        assert(shapeExpressions == ty->secrecDimType());
    } else if (! isProcParam) {
        if (!isScalar)
            emplaceImopAfter(result, varInit, Imop::ASSIGN, ns->getSizeSym(), indexConstant(0));

        for (SecrecDimType it = 0; it < ty->secrecDimType(); ++it)
            emplaceImop(varInit, Imop::ASSIGN, ns->getDim(it), indexConstant(0));
    }

    // TypeChecker::checkVarInit() should ensure this:
    assert(shapeExpressions == 0 || shapeExpressions == ty->secrecDimType());

    if (isProcParam) { // This is a procedure parameter definition
        append (result, cgProcParam (ns));
        return result;
    }

    if (varInit->rightHandSide() != nullptr) { // This is a regular definition with an initializer expression

        // Evaluate rhs:
        const CGResult & eResult = codeGen(varInit->rightHandSide());
        append(result, eResult);
        if (result.isNotOk())
            return result;

        append (result, cgInitializeToSymbol (ns, eResult.symbol (), shapeExpressions != 0));
        return result;
    }
    else {
        append (result, cgInitializeToDefaultValue (ns, shapeExpressions > 0));
        return result;
    }
}

CGStmtResult TreeNodeStmtDecl::codeGenWith(CodeGen & cg) {
    return cg.cgStmtDecl(this);
}

CGStmtResult CodeGen::cgStmtDecl(TreeNodeStmtDecl * s) {
    // Type check:
    if (m_tyChecker->visitStmtDecl(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGStmtResult result;
    const bool isGlobal = s->global();
    const bool isProcParam = s->procParam();
    assert (! (isGlobal && isProcParam));

    const auto cgInit =
        isProcParam ? &CodeGen::cgProcParamInit :
        isGlobal    ? &CodeGen::cgGlobalVarInit :
                      &CodeGen::cgLocalVarInit;

    for (TreeNodeVarInit& varInit : s->initializers()) {
        append(result, (this->*cgInit) (s->resultType(), &varInit));
        if (result.isNotOk()) {
            return result;
        }
    }

    return result;
}

/*******************************************************************************
  TreeNodeStmtFor
*******************************************************************************/

CGStmtResult TreeNodeStmtFor::codeGenWith (CodeGen& cg) {
    return cg.cgStmtFor (this);
}

CGStmtResult CodeGen::cgStmtFor(TreeNodeStmtFor * s) {
    CGStmtResult result;
    bool createdScope = false;
    Symbol * temp = nullptr;

    // Initialization expression:
    if (s->initializer() != nullptr) {
        if (dynamic_cast<TreeNodeExpr *>(s->initializer()) != nullptr) {
            TreeNodeExpr * initE = static_cast<TreeNodeExpr *>(s->initializer());
            const CGResult & initResult = codeGen(initE);
            append(result, initResult);
            temp = initResult.symbol();
        }
        else
        if (dynamic_cast<TreeNodeStmtDecl *>(s->initializer()) != nullptr) {
            newScope();
            createdScope = true;
            append(result, codeGenStmt(static_cast<TreeNodeStmtDecl *>(s->initializer())));
        }
    }

    if (result.isNotOk()) {
        return result;
    }

    // Conditional expression:
    CGBranchResult condResult;
    if (s->conditional() != nullptr) {
        TreeNodeExpr * e1 = s->conditional();
        if (m_tyChecker->checkPublicBooleanScalar(e1) != TypeChecker::OK) {
            m_log.fatalInProc(s) << "Conditional expression in if statement "
                                    "must be of type public bool at "
                                 << e1->location() << '.';
            result.setStatus(CGResult::ERROR_CONTINUE);
            return result;
        }

        condResult = codeGenBranch(e1);
        append(result, condResult);
        if (result.isNotOk()) {
            return result;
        }
    }

    // Body of for loop:
    TreeNodeStmt * body = s->body();
    startLoop();
    CGStmtResult bodyResult = codeGenStmt(body);
    endLoop();
    if (bodyResult.isNotOk()) {
        result.setStatus(bodyResult.status());
        return result;
    }

    // Iteration expression:
    CGResult iterResult;
    if (s->iteratorExpr()) {
        iterResult = codeGen(s->iteratorExpr());
        if (iterResult.isNotOk()) {
            result.setStatus(iterResult.status());
            return result;
        }
    }

    // Next iteration jump:
    auto j = new Imop(s, Imop::JUMP, nullptr);
    pushImopAfter(iterResult, j);

    bodyResult.patchFirstImop(iterResult.firstImop());
    condResult.patchFirstImop(bodyResult.firstImop());
    SymbolLabel * nextIterDest = m_st->label(iterResult.firstImop());
    SymbolLabel * firstBodyDest = m_st->label(bodyResult.firstImop());

    // i hope the following is not too unreadable:
    result.patchFirstImop(condResult.firstImop());
    result.addToNextList(condResult.falseList());   // if condition if false jump out of for loop
    result.addToNextList(bodyResult.breakList());   // if break is reach jump out of for loop
    j->setDest(m_st->label(condResult.firstImop()));    // after iteration jump to contitional
    condResult.patchTrueList(firstBodyDest);  // if conditional is true jump to body
    bodyResult.patchNextList(nextIterDest);  // next jumps to iteration
    bodyResult.patchContinueList(nextIterDest);  // continue jumps to iteration

    if (createdScope) {
        releaseScopeVariables(result);
        popScope();
    }
    else {
        if (temp != nullptr)
            releaseTemporary(result, temp);
    }

    // Static checking:
    assert(bodyResult.flags() != 0x0);
    if ((bodyResult.flags()
                & (CGStmtResult::FALLTHRU | CGStmtResult::CONTINUE)) == 0x0)
    {
        m_log.fatalInProc(s) << "For loop at " << s->location() << " wont loop!";
        result.setStatus(CGResult::ERROR_CONTINUE);
        return result;
    }
    if (condResult.firstImop() == nullptr && ((bodyResult.flags()
                    & (CGStmtResult::BREAK | CGStmtResult::RETURN)) == 0x0))
    {
        m_log.fatalInProc(s) << "For loop at " << s->location() << " is clearly infinite!";
        result.setStatus(CGResult::ERROR_CONTINUE);
        return result;
    }

    result.setFlags((bodyResult.flags()
                & ~(CGStmtResult::BREAK | CGStmtResult::CONTINUE))
            | CGStmtResult::FALLTHRU);

    return result;
}

/*******************************************************************************
  TreeNodeStmtIf
*******************************************************************************/

CGStmtResult TreeNodeStmtIf::codeGenWith(CodeGen & cg) {
    return cg.cgStmtIf(this);
}

CGStmtResult CodeGen::cgStmtIf(TreeNodeStmtIf * s) {

    if (m_tyChecker->visitStmtIf(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGStmtResult result;

    // Generate code for conditional expression:
    TreeNodeExpr * e = s->conditional();
    CGBranchResult eResult = codeGenBranch(e);
    append(result, eResult);
    if (result.isNotOk()) {
        return result;
    }


    assert(result.firstImop() != nullptr);

    // Generate code for first branch:
    newScope();
    TreeNodeStmt * s1 = s->trueBranch();
    CGStmtResult trueResult = codeGenStmt(s1);
    if (trueResult.isNotOk()) {
        result.setStatus(trueResult.status());
        return result;
    }

    popScope();

    if (trueResult.firstImop() != nullptr) {
        eResult.patchTrueList(m_st->label(trueResult.firstImop()));
        result.addToNextList(trueResult.nextList());
        result.addToBreakList(trueResult.breakList());
        result.addToContinueList(trueResult.continueList());
        assert(trueResult.flags() != 0x0);
    } else {
        result.addToNextList(eResult.trueList());
    }

    if (s->falseBranch() == nullptr) {
        result.addToNextList(eResult.falseList());
        result.setFlags(trueResult.flags() | CGStmtResult::FALLTHRU);
    } else {
        // Generate jump out of first branch, if needed:
        if ((trueResult.flags() & CGStmtResult::FALLTHRU) != 0x0) {
            auto i = new Imop(s, Imop::JUMP, nullptr);
            push_imop(i);
            result.addToNextList(i);
        }

        // Generate code for second branch:
        newScope();
        TreeNodeStmt * s2 = s->falseBranch();
        CGStmtResult falseResult(codeGenStmt(s2));
        if (falseResult.isNotOk()) {
            result.setStatus(falseResult.status());
            return result;
        }

        popScope();

        if (falseResult.firstImop() == nullptr) {
            result.addToNextList(eResult.falseList());
            result.setFlags(trueResult.flags() | CGStmtResult::FALLTHRU);
        }
        else {
            eResult.patchFalseList(m_st->label(falseResult.firstImop()));
            result.addToNextList(falseResult.nextList());
            result.addToBreakList(falseResult.breakList());
            result.addToContinueList(falseResult.continueList());
            assert(falseResult.flags() != 0x0);
            result.setFlags(trueResult.flags() | falseResult.flags());
        }
    }

    return result;
}

/*******************************************************************************
  TreeNodeStmtReturn
*******************************************************************************/

CGStmtResult TreeNodeStmtReturn::codeGenWith(CodeGen & cg) {
    return cg.cgStmtReturn(this);
}

CGStmtResult CodeGen::cgStmtReturn(TreeNodeStmtReturn * s) {
    // Type check:
    if (m_tyChecker->visitStmtReturn(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGStmtResult result;
    if (s->expression() == nullptr) {
        releaseProcVariables(result);

        Imop * i = newReturn(s);
        i->setDest(m_st->label(s->containingProcedure()->symbol()->target()));
        pushImopAfter(result, i);
    } else {
        TreeNodeExpr * e = s->expression();
        assert(e->haveResultType());

        CGResult eResult = codeGen(e);
        append(result, eResult);
        Symbol* resultSym = eResult.symbol ();

        // TODO: this is somewhat of a hack.
        if (resultSym->isConstant () && resultSym->secrecType ()->secrecDataType ()->isString ())
        {
            Symbol* copy = m_st->appendTemporary (resultSym->secrecType ());
            auto i = new Imop (s, Imop::ASSIGN, copy, resultSym);
            pushImopAfter (result, i);
            resultSym = copy;
            eResult.setResult (resultSym);
        }

        std::vector<Symbol*> rets;
        for (Symbol* sym : flattenSymbol (eResult.symbol ())) {
            rets.push_back (copyNonTemporary (result, sym));
        }

        releaseProcVariables(result);
        if (result.isNotOk()) {
            return result;
        }

        Imop * i = newReturn(s, rets.begin(), rets.end());
        i->setDest(m_st->label(s->containingProcedure()->symbol()->target()));
        pushImopAfter(result, i);
    }

    result.setFlags(CGStmtResult::RETURN);
    return result;
}

/*******************************************************************************
  TreeNodeStmtWhile
*******************************************************************************/

CGStmtResult TreeNodeStmtWhile::codeGenWith(CodeGen & cg) {
    return cg.cgStmtWhile(this);
}

CGStmtResult CodeGen::cgStmtWhile(TreeNodeStmtWhile * s) {

    CGStmtResult result;

    // Generate the conditional expression:
    CGBranchResult eResult = CGResult::ERROR_CONTINUE;
    if (m_tyChecker->visitStmtWhile(s) == TypeChecker::OK) {
        eResult = codeGenBranch(s->conditional());
    }

    // Generate the loop body:
    CGStmtResult bodyResult;
    {
        ScopedScope scope(*this);
        ScopedLoop loop(*this);
        bodyResult = codeGenStmt(s->body());
    }

    // Halt if something went wrong:
    append(result, eResult);
    if (result.isNotOk()) {
        return result;
    }

    if (bodyResult.isNotOk()) {
        result |= bodyResult.status();
        return result;
    }

    // Link the conditional and body together:
    assert(result.firstImop() != nullptr);
    assert(result.nextList().empty());

    SymbolLabel * jumpDest = m_st->label(result.firstImop());

    assert(bodyResult.flags() != 0x0);
    if ((bodyResult.flags()
                & (CGStmtResult::FALLTHRU | CGStmtResult::CONTINUE)) == 0x0)
    {
        m_log.fatalInProc(s) << "While loop at " << s->location() << " wont loop!";
        result |= CGResult::ERROR_CONTINUE;
        return result;
    }

    result.setFlags((bodyResult.flags()
                & ~(CGStmtResult::BREAK | CGStmtResult::CONTINUE))
            | CGStmtResult::FALLTHRU);

    auto i = new Imop(s, Imop::JUMP, nullptr);
    pushImopAfter(bodyResult, i);
    i->setDest(jumpDest);

    // Patch jump lists:
    eResult.patchTrueList(m_st->label(bodyResult.firstImop()));
    result.setNextList(eResult.falseList());
    result.addToNextList(bodyResult.breakList());
    bodyResult.patchContinueList(jumpDest);

    return result;
}

/*******************************************************************************
  TreeNodeStmtPrint
*******************************************************************************/

CGStmtResult TreeNodeStmtPrint::codeGenWith(CodeGen & cg) {
    return cg.cgStmtPrint(this);
}

CGStmtResult CodeGen::cgStmtPrint(TreeNodeStmtPrint * s) {
    // Type check:
    if (m_tyChecker->visitStmtPrint(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    const TypeBasic * strTy = TypeBasic::get(DATATYPE_STRING);

    CGStmtResult result;
    Symbol * accum = nullptr;
    for (TreeNodeExpr& e : s->expressions()) {
        const CGResult & eResult = codeGen(&e);
        append(result, eResult);
        if (result.isNotOk()) {
            return result;
        }

        Symbol * str = eResult.symbol();
        if (! e.resultType()->secrecDataType()->isString ()) {
            str = m_st->appendTemporary(strTy);
            auto i = new Imop(s, Imop::TOSTRING, str, eResult.symbol());
            pushImopAfter(result, i);
        }

        if (accum == nullptr) {
            accum = str;
        }
        else {
            Symbol * temp = m_st->appendTemporary(strTy);
            pushImopAfter(result, new Imop(s, Imop::ADD, temp, accum, str));
            releaseTemporary(result, str);
            releaseTemporary(result, accum);
            accum = temp;
        }
    }

    pushImopAfter(result, new Imop(s, Imop::PRINT, nullptr, accum));
    releaseTemporary(result, accum);
    return result;
}

/*******************************************************************************
  TreeNodeStmtSyscall
*******************************************************************************/

CGStmtResult TreeNodeStmtSyscall::codeGenWith(CodeGen & cg) {
    return cg.cgStmtSyscall(this);
}

CGStmtResult CodeGen::cgStmtSyscall(TreeNodeStmtSyscall * s) {
    if (m_tyChecker->visitStmtSyscall(s) != TypeChecker::OK) {
        return CGResult::ERROR_CONTINUE;
    }

    CGStmtResult result;
    using NodeSymbolPair = std::pair<TreeNodeSyscallParam*, Symbol*>;
    std::vector<NodeSymbolPair> results;
    for (TreeNodeSyscallParam& param : s->params()) {
        TreeNodeExpr * e = param.expression ();
        const CGResult & eResult = codeGen(e);
        append(result, eResult);
        if (result.isNotOk()) {
            return result;
        }

        results.push_back(std::make_pair(&param, eResult.symbol()));
    }

    const CGResult & nameResult = codeGen(s->name());
    append(result, nameResult);
    if (result.isNotOk()) {
        return result;
    }

    // Release stuff that is returned by the syscall.
    for (const NodeSymbolPair & ts : results) {
        switch (ts.first->type()) {
        case NODE_SYSCALL_RETURN:
            releaseResource (result, ts.second);
        default:
            continue;
        }
    }

    SyscallOperands operands;
    operands.reserve(results.size());
    for (const NodeSymbolPair & ts : results) {
        switch (ts.first->type()) {
        case NODE_SYSCALL_RETURN:
            operands.emplace_back(ts.second, Return);
            break;
        case NODE_READONLY:
            operands.emplace_back(ts.second, Push, ReadOnly);
            break;
        case NODE_PUSH:
            operands.emplace_back(ts.second, Push);
            break;
        case NODE_PUSHREF:
            operands.emplace_back(ts.second, PushRef);
            break;
        case NODE_PUSHCREF:
            operands.emplace_back(ts.second, PushCRef);
            break;
        default:
            assert(false && "ICE!");
            result.setStatus(CGResult::ERROR_FATAL);
            return result;
        }
    }

    assert (dynamic_cast<ConstantString*>(nameResult.symbol ()) != nullptr);
    ConstantString* syscallName = static_cast<ConstantString*>(nameResult.symbol ());

    pushImopAfter(result, newComment(syscallName->value()));
    pushImopAfter(result, new Imop(s, syscallName, operands));

    for (const NodeSymbolPair & ts : results) {
        releaseTemporary(result, ts.second);
    }

    return result;
}

/*******************************************************************************
  TreeNodeStmtDoWhile
*******************************************************************************/

CGStmtResult TreeNodeStmtDoWhile::codeGenWith (CodeGen& cg) {
    return cg.cgStmtDoWhile (this);
}

CGStmtResult CodeGen::cgStmtDoWhile(TreeNodeStmtDoWhile * s) {

    if (m_tyChecker->visitStmtDoWhile(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    // Loop body:
    newScope();

    TreeNodeStmt * body = s->body();
    startLoop();
    CGStmtResult result(codeGenStmt(body));
    endLoop();
    if (result.isNotOk()) {
        return result;
    }

    popScope();  // end of loop body

    // Static checking:
    if (result.firstImop() == nullptr) {
        m_log.fatalInProc(s) << "Empty loop body at " << body->location() << '.';
        result.setStatus(CGResult::ERROR_CONTINUE);
        return result;
    }

    assert(result.flags() != 0x0);

    if ((result.flags()
                & (CGStmtResult::FALLTHRU | CGStmtResult::CONTINUE)) == 0x0)
    {
        m_log.fatalInProc(s) << "Do-while loop at " << s->location() << " wont loop!";
        result.setStatus(CGResult::ERROR_CONTINUE);
        return result;
    }

    result.setFlags((result.flags()
                & ~(CGStmtResult::BREAK | CGStmtResult::CONTINUE))
            | CGStmtResult::FALLTHRU);

    // Conditional expression:
    TreeNodeExpr * e = s->conditional();
    CGBranchResult eResult(codeGenBranch(e));
    append(result, eResult);
    if (result.isNotOk()) {
        return result;
    }

    assert(eResult.firstImop() != nullptr);

    // Patch jump lists:

    eResult.patchTrueList(m_st->label(result.firstImop()));
    result.patchContinueList(m_st->label(eResult.firstImop()));
    result.setNextList(result.breakList());
    result.addToNextList(eResult.falseList());
    result.clearBreakList();

    return result;
}

/*******************************************************************************
  TreeNodeStmtExpr
*******************************************************************************/

CGStmtResult TreeNodeStmtExpr::codeGenWith(CodeGen & cg) {
    return cg.cgStmtExpr(this);
}

CGStmtResult CodeGen::cgStmtExpr(TreeNodeStmtExpr * s) {
    if (m_tyChecker->visitExpr (s->expression ()) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    s->expression ()->instantiateDataType ();
    CGStmtResult result = codeGen(s->expression());
    if (result.symbol() != nullptr) {
        releaseTemporary(result, result.symbol());
    }

    return result;
}

/*******************************************************************************
  TreeNodeStmtAssert
*******************************************************************************/

CGStmtResult TreeNodeStmtAssert::codeGenWith(CodeGen & cg) {
    return cg.cgStmtAssert(this);
}

CGStmtResult CodeGen::cgStmtAssert(TreeNodeStmtAssert * s) {
    if (m_tyChecker->visitStmtAssert(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    TreeNodeExpr * e = s->expression();
    CGBranchResult eResult(codeGenBranch(e));
    assert(eResult.firstImop() != nullptr);
    if (eResult.isNotOk()) {
        return eResult;
    }

    CGStmtResult result;
    releaseAllVariables(result);

    std::ostringstream ss;
    ss << "assert failed at " << s->location().printer(m_pathStyle);
    Imop * i = newError(s, ConstantString::get(getContext(), ss.str()));
    pushImopAfter(result, i);

    eResult.patchFalseList(m_st->label(result.firstImop()));
    result.addToNextList(eResult.trueList());
    result.setFirstImop(eResult.firstImop());
    return result;
}

} // namespace SecreC
