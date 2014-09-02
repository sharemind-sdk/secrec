#include "codegen.h"
#include "constant.h"
#include "log.h"
#include "misc.h"
#include "StringTable.h"
#include "symboltable.h"
#include "treenode.h"
#include "typechecker.h"
#include "types.h"

#include <boost/foreach.hpp>

/**
 * Code generation for statements.
 */

namespace SecreC {

namespace /* anonymous */ {

// Initializes associated size and shape symbols. Does not assign values.
void initShapeSymbols (Context& cxt, SymbolTable* st, SymbolSymbol* sym) {
    TypeBasic * const dimType = TypeBasic::getIndexType(cxt);
    TypeNonVoid* ty = sym->secrecType ();
    for (SecrecDimType i = 0; i < ty->secrecDimType(); ++ i) {
        sym->setDim(i, st->appendTemporary (dimType));
    }

    if (sym->isArray ()) { // set size symbol
        sym->setSizeSym(st->appendTemporary (dimType));
    }
}

// Initializes structure fields (and also associated size and shape symbols of the fields).
// Note that this does not assign values to those symbols.
void initFieldSymbols (Context& cxt, SymbolTable* st, SymbolSymbol* sym, TypeBasic* ty) {
    DataTypeStruct* structType = static_cast<DataTypeStruct*>(ty->secrecDataType ());
    typedef DataTypeStruct::Field Field;
    BOOST_FOREACH (const Field& field, structType->fields ()) {
        TypeBasic* fieldType = field.type;
        SymbolSymbol* fieldSymbol = st->appendTemporary (fieldType);
        initShapeSymbols (cxt, st, fieldSymbol);
        if (fieldType->secrecDataType ()->isComposite ()) {
            initFieldSymbols (cxt, st, fieldSymbol, fieldType);
        }

        sym->appendField (fieldSymbol);
    }
}

// Mark the given symbol, and associated temporaries, as global symbols.
void setSymbolGlobalScope (SymbolSymbol* sym) {
    TypeNonVoid* ty = sym->secrecType ();
    sym->setScopeType (SymbolSymbol::GLOBAL);
    for (SecrecDimType i = 0; i < ty->secrecDimType(); ++ i) {
        setSymbolGlobalScope (sym->getDim (i));
    }

    if (!ty->isScalar ()) {
        setSymbolGlobalScope (sym->getSizeSym ());
    }

    if (ty->secrecDataType ()->isComposite ()) {
        BOOST_FOREACH (SymbolSymbol* field, sym->fields ()) {
            setSymbolGlobalScope (field);
        }
    }
}

} // namespace anonymous


/*******************************************************************************
  TreeNodeStmtCompound
*******************************************************************************/

CGStmtResult TreeNodeStmtCompound::codeGenWith (CodeGen& cg) {
    return cg.cgStmtCompound (this);
}

CGStmtResult CodeGen::cgStmtCompound(TreeNodeStmtCompound * s) {
    CGStmtResult result;

    newScope();

    BOOST_FOREACH (TreeNode * c_, s->children()) {
        assert(dynamic_cast<TreeNodeStmt *>(c_) != NULL);
        TreeNodeStmt * c = static_cast<TreeNodeStmt *>(c_);
        const CGStmtResult & cResult = codeGenStmt(c);
        append(result, cResult);
        if (result.isFatal()) break;
        if (result.isNotOk()) continue;

        if (cResult.firstImop() == NULL) {
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
    if (loopST() == NULL) {
        m_log.fatalInProc(s) << "Break statement not embedded in loop at " << s->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    CGStmtResult result;
    assert(loopST() != NULL);
    BOOST_FOREACH (SymbolSymbol * var, m_st->variablesUpTo(loopST())) {
        releaseResource(result, var);
    }

    Imop * i = new Imop(s, Imop::JUMP, 0);
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
    if (loopST() == NULL) {
        m_log.fatalInProc(s) << "Continue statement not embedded in loop at " << s->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    CGStmtResult result;
    BOOST_FOREACH (SymbolSymbol * var, m_st->variablesUpTo(loopST())) {
        releaseResource(result, var);
    }

    Imop * i = new Imop(s, Imop::JUMP, 0);
    pushImopAfter(result, i);
    result.setFirstImop(i);
    result.addToContinueList(i);
    result.setFlags(CGStmtResult::CONTINUE);
    return result;
}

/*******************************************************************************
  TreeNodeStmtDecl
*******************************************************************************/

CGStmtResult CodeGen::cgGlobalVarInit (TypeNonVoid* ty, TreeNodeVarInit* varInit)
{
    if (m_tyChecker->checkVarInit(ty, varInit) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGStmtResult result;

    Imop* i = NULL;

    // JUMP SKIP;
    i = pushComment ("Global variable initialization:");
    result.setFirstImop (i);
    Imop* skip = new Imop (varInit, Imop::JUMP, (Symbol*) 0);
    push_imop (skip);

    SymbolSymbol * const ns = new SymbolSymbol(varInit->variableName(), ty);
    if (true) {
        initShapeSymbols (getContext (), m_st, ns);
        setSymbolGlobalScope (ns);
        m_st->appendSymbol (ns);
    }
    else {
        // Initialize the result symbol:
        TypeBasic * const dimType = TypeBasic::getIndexType(getContext());
        for (SecrecDimType i = 0; i < ty->secrecDimType(); ++ i) {
            std::stringstream ss;
            ss << varInit->variableName() << "{d" << i << "}";
            SymbolSymbol * sym = new SymbolSymbol(ss.str(), dimType);
            sym->setScopeType(SymbolSymbol::GLOBAL);
            m_st->appendSymbol(sym);
            ns->setDim(i, sym);
        }

        if (!ty->isScalar ()) { // set size symbol
            std::stringstream ss;
            ss << varInit->variableName() << "{size}";
            SymbolSymbol * const sizeSym = new SymbolSymbol(ss.str(), dimType);
            sizeSym->setScopeType(SymbolSymbol::GLOBAL);
            m_st->appendSymbol(sizeSym);
            ns->setSizeSym(sizeSym);
        }

        m_st->appendSymbol (ns);
    }

    // <initialization function>
    SymbolProcedure* procSym = NULL;
    {
        ScopedScope localScope (*this);
        std::stringstream ss;
        ss << "__global_init_" << varInit->variableName ();

        const StringRef procName = *getStringTable ().addString (ss.str ());
        TypeProc* procType = TypeProc::get (getContext (), std::vector<TypeBasic*>(), ty);
        procSym = new SymbolProcedure (procName, procType);

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
        releaseProcVariables (result, localResultSym);
        if (result.isNotOk ())
            return result;

        std::vector<Symbol*> rets (dim_begin(localResultSym), dim_end(localResultSym));
        rets.push_back (localResultSym);
        Imop * i = newReturn(varInit, rets.begin(), rets.end());
        i->setDest(m_st->label(funcStart));
        pushImopAfter(result, i);
    }

    std::vector<Symbol*> retList, argList;
    retList.insert (retList.end (), dim_begin (ns), dim_end (ns));
    retList.push_back (ns);

    Imop* callImop = newCall (varInit, retList.begin (), retList.end (), argList.begin (), argList.end ());
    Imop* cleanImop = new Imop (varInit, Imop::RETCLEAN, NULL, NULL, NULL);
    cleanImop->setArg2 (m_st->label (callImop));
    skip->setDest (m_st->label (callImop));
    callImop->setDest (procSym);
    push_imop (callImop);
    push_imop (cleanImop);
    return result;
}

CGStmtResult CodeGen::cgLocalVarInit (TypeNonVoid* ty, TreeNodeVarInit* varInit)
{
    return cgVarInit (ty, varInit, false);
}

CGStmtResult CodeGen::cgProcParamInit (TypeNonVoid* ty, TreeNodeVarInit* varInit)
{
    return cgVarInit (ty, varInit, true);
}

CGStmtResult CodeGen::cgVarInit (TypeNonVoid* ty,
                                 TreeNodeVarInit* varInit,
                                 bool isProcParam)
{
    if (m_tyChecker->checkVarInit(ty, varInit) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    const bool isScalar = ty->isScalar();
    const bool isString = ty->secrecDataType()->isString ();
    const bool isPrivate = ty->secrecSecType()->isPrivate();
    const bool isStruct = ty->secrecDataType ()->isComposite ();


    TypeBasic * const pubBoolTy = TypeBasic::getPublicBoolType(getContext());

    SymbolSymbol * const ns = new SymbolSymbol(varInit->variableName(), ty);
    m_st->appendSymbol(ns);

    SecrecDimType shapeExpressions = 0;
    assert((isScalar || !isString)
           && "ICE: string arrays should be forbidden by the type checker!");

    // Initialize shape:
    initShapeSymbols (getContext (), m_st, ns);

    if (isStruct) {
        initFieldSymbols (getContext (), m_st, ns, static_cast<TypeBasic*>(ty));
    }

    CGStmtResult result;
    result.setResult (ns);

    // evaluate shape if given, also compute size
    if (! varInit->shape().empty()) {
        if (!isScalar)
            pushImopAfter(result, new Imop(varInit, Imop::ASSIGN,
                                           ns->getSizeSym(), indexConstant(1)));

        BOOST_FOREACH (TreeNodeExpr& e, varInit->shape()) {

            // Evaluate shape expression:
            const CGResult & eResult = codeGen(&e);
            append(result, eResult);
            if (result.isNotOk())
                return result;

            pushImopAfter(result, new Imop(varInit, Imop::ASSIGN,
                                           ns->getDim(shapeExpressions),
                                           eResult.symbol()));

            push_imop(new Imop(varInit, Imop::MUL, ns->getSizeSym(),
                               ns->getSizeSym(), eResult.symbol()));
            ++shapeExpressions;
        }

        // TypeChecker::checkVarInit() should ensure this:
        assert(shapeExpressions == ty->secrecDimType());
    } else {
        if (!isScalar)
            pushImopAfter(result, new Imop(varInit, Imop::ASSIGN, ns->getSizeSym(), indexConstant(0)));

        for (SecrecDimType it = 0; it < ty->secrecDimType(); ++it)
            push_imop(new Imop(varInit, Imop::ASSIGN, ns->getDim(it), indexConstant(0)));
    }

    // TypeChecker::checkVarInit() should ensure this:
    assert(shapeExpressions == 0 || shapeExpressions == ty->secrecDimType());

    if (isProcParam) { // This is a procedure parameter definition
        append (result, cgProcParam (ns));
        return result;
    }

    if (varInit->rightHandSide() != NULL) { // This is a regular definition with an initializer expression

        // Evaluate rhs:
        const CGResult & eResult = codeGen(varInit->rightHandSide());
        append(result, eResult);
        if (result.isNotOk())
            return result;

        Symbol * const eResultSymbol = eResult.symbol();

        if (shapeExpressions != 0) { // type[[n>0]] x(i_1,...,i_n) = foo;
            if (ty->secrecDimType() > eResultSymbol->secrecType()->secrecDimType()) {
                // fill lhs with constant value
                pushImopAfter(result, new Imop(varInit, Imop::ALLOC, ns, eResultSymbol, ns->getSizeSym()));
                releaseTemporary(result, eResultSymbol);
            }
            else {
                // check that shapes match and assign
                std::stringstream ss;
                ss << "Shape mismatch at " << varInit->location();
                Imop * err = newError(varInit, ConstantString::get(getContext(), ss.str()));
                SymbolLabel * const errLabel = m_st->label(err);
                dim_iterator lhsDimIter = dim_begin(ns);
                BOOST_FOREACH (Symbol * rhsDim, dim_range(eResultSymbol)) {
                    SymbolTemporary * const temp_bool = m_st->appendTemporary(pubBoolTy);
                    pushImopAfter(result, new Imop(varInit, Imop::NE, temp_bool, rhsDim, *lhsDimIter));
                    push_imop(new Imop(varInit, Imop::JT, errLabel, temp_bool));

                    ++lhsDimIter;
                }

                Imop * const jmp = new Imop(varInit, Imop::JUMP, NULL);
                Imop * const i = new Imop(varInit, Imop::COPY, ns, eResultSymbol, ns->getSizeSym());
                jmp->setDest(m_st->label(i));
                pushImopAfter(result, jmp);
                push_imop(err);
                push_imop(i);
                releaseTemporary(result, eResultSymbol);
            }
        } else {
            if (ty->secrecDimType() > 0) { // type[[>0]] x = foo;
                if (ty->secrecDimType() > eResultSymbol->secrecType()->secrecDimType()) {
                    pushImopAfter(result, new Imop(varInit, Imop::ALLOC, ns, eResultSymbol, ns->getSizeSym()));
                    releaseTemporary(result, eResultSymbol);
                } else {
                    assert(ty->secrecDimType() == eResultSymbol->secrecType()->secrecDimType());

                    dim_iterator srcIter = dim_begin(eResultSymbol);
                    BOOST_FOREACH (Symbol * const destSym, dim_range(ns)) {
                        assert(srcIter != dim_end(eResultSymbol));
                        pushImopAfter(result, new Imop(varInit, Imop::ASSIGN, destSym, *srcIter));
                        ++srcIter;
                    }

                    // The type checker should ensure that the symbol is a symbol (and not a constant):
                    assert(eResultSymbol->symbolType() == SYM_SYMBOL);
                    SymbolSymbol * const s = static_cast<SymbolSymbol *>(eResultSymbol);
                    pushImopAfter(result, newAssign(varInit,  ns->getSizeSym(), s->getSizeSym()));
                    pushImopAfter(result, new Imop(varInit, Imop::COPY, ns, s, ns->getSizeSym()));
                    releaseTemporary(result, eResultSymbol);
                }
            } else { // scalar_type x = scalar;
                pushImopAfter(result, new Imop(varInit, Imop::ASSIGN, ns, eResultSymbol));
                releaseTemporary(result, eResultSymbol);
            }
        }

        return result;
    }
    else {

        if (true) {
            append (result, cgInitalizeToDefaultValue (ns, shapeExpressions > 0));
        }
        else {
            // This is a regular definition without an initializer expression:
            if (!isScalar && shapeExpressions == 0) {
                pushImopAfter(result, new Imop(varInit, Imop::ASSIGN, ns->getSizeSym(), indexConstant(0)));

                for (SecrecDimType it = 0; it < ty->secrecDimType(); ++it)
                    push_imop(new Imop(varInit, Imop::ASSIGN, ns->getDim(it), indexConstant(0)));
            }

            Symbol * const def = defaultConstant(getContext(),  ty->secrecDataType());
            if (isScalar) {
                Imop::Type iType = isPrivate ? Imop::CLASSIFY : Imop::ASSIGN;
                pushImopAfter(result, new Imop(varInit, iType, ns, def));
            } else {
                pushImopAfter(result, new Imop(varInit, Imop::ALLOC, ns, def, getSizeOr(ns, 0)));
            }
        }

        return result;
    }
}

CGStmtResult TreeNodeStmtDecl::codeGenWith(CodeGen & cg) {
    return cg.cgStmtDecl(this);
}

CGStmtResult CodeGen::cgStmtDecl(TreeNodeStmtDecl * s) {
    // Type check:
    if (m_tyChecker->visit(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGStmtResult result;
    const bool isGlobal = s->global();
    const bool isProcParam = s->procParam();
    assert (! (isGlobal && isProcParam));

    typedef CGStmtResult (CodeGen::*varInitFunc)(TypeNonVoid*, TreeNodeVarInit*);
    const varInitFunc cgInit =
        isProcParam ? &CodeGen::cgProcParamInit :
        isGlobal    ? &CodeGen::cgGlobalVarInit :
                      &CodeGen::cgLocalVarInit;

    BOOST_FOREACH (TreeNodeVarInit& varInit, s->initializers()) {
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
    Symbol * temp = NULL;

    // Initialization expression:
    if (s->initializer() != NULL) {
        if (dynamic_cast<TreeNodeExpr *>(s->initializer()) != NULL) {
            TreeNodeExpr * initE = static_cast<TreeNodeExpr *>(s->initializer());
            const CGResult & initResult = codeGen(initE);
            append(result, initResult);
            temp = initResult.symbol();
        }
        else
        if (dynamic_cast<TreeNodeStmtDecl *>(s->initializer()) != NULL) {
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
    if (s->conditional() != NULL) {
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
    Imop * j = new Imop(s, Imop::JUMP, 0);
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
        if (temp != NULL)
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
    if (condResult.firstImop() == NULL && ((bodyResult.flags()
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

    if (m_tyChecker->visit(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGStmtResult result;

    // Generate code for conditional expression:
    TreeNodeExpr * e = s->conditional();
    CGBranchResult eResult = codeGenBranch(e);
    append(result, eResult);
    if (result.isNotOk()) {
        return result;
    }


    assert(result.firstImop() != NULL);

    // Generate code for first branch:
    newScope();
    TreeNodeStmt * s1 = s->trueBranch();
    CGStmtResult trueResult = codeGenStmt(s1);
    if (trueResult.isNotOk()) {
        result.setStatus(trueResult.status());
        return result;
    }

    popScope();

    if (trueResult.firstImop() != NULL) {
        eResult.patchTrueList(m_st->label(trueResult.firstImop()));
        result.addToNextList(trueResult.nextList());
        result.addToBreakList(trueResult.breakList());
        result.addToContinueList(trueResult.continueList());
        assert(trueResult.flags() != 0x0);
    } else {
        result.addToNextList(eResult.trueList());
    }

    if (s->falseBranch() == NULL) {
        result.addToNextList(eResult.falseList());
        result.setFlags(trueResult.flags() | CGStmtResult::FALLTHRU);
    } else {
        // Generate jump out of first branch, if needed:
        if ((trueResult.flags() & CGStmtResult::FALLTHRU) != 0x0) {
            Imop * i = new Imop(s, Imop::JUMP, 0);
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

        if (falseResult.firstImop() == NULL) {
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
    if (m_tyChecker->visit(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGStmtResult result;
    if (s->expression() == NULL) {
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
        if (resultSym->isConstant () && resultSym->secrecType ()->secrecDataType ()->isString ())
        {
            Symbol* copy = m_st->appendTemporary (resultSym->secrecType ());
            Imop* i = new Imop (s, Imop::ASSIGN, copy, resultSym);
            pushImopAfter (result, i);
            resultSym = copy;
            eResult.setResult (resultSym);
        }

        releaseProcVariables(result, resultSym);
        if (result.isNotOk()) {
            return result;
        }

        std::vector<Symbol * > rets(dim_begin(eResult.symbol()), dim_end(eResult.symbol()));
        rets.push_back(eResult.symbol());
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
    if (m_tyChecker->visit(s) == TypeChecker::OK) {
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
    assert(result.firstImop() != NULL);
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

    Imop * i = new Imop(s, Imop::JUMP, 0);
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
    if (m_tyChecker->visit(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    TypeBasic * strTy = TypeBasic::get(getContext(), DATATYPE_STRING);

    CGStmtResult result;
    Symbol * accum = NULL;
    BOOST_FOREACH (TreeNodeExpr& e, s->expressions()) {
        const CGResult & eResult = codeGen(&e);
        append(result, eResult);
        if (result.isNotOk()) {
            return result;
        }

        Symbol * str = eResult.symbol();
        if (! e.resultType()->secrecDataType()->isString ()) {
            str = m_st->appendTemporary(strTy);
            Imop * i = new Imop(s, Imop::TOSTRING, str, eResult.symbol());
            pushImopAfter(result, i);
        }

        if (accum == NULL) {
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

    pushImopAfter(result, new Imop(s, Imop::PRINT, NULL, accum));
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
    if (m_tyChecker->visit(s) != TypeChecker::OK) {
        return CGResult::ERROR_CONTINUE;
    }

    CGStmtResult result;
    typedef std::pair<TreeNodeSyscallParam *, Symbol *> NodeSymbolPair;
    std::vector<NodeSymbolPair> results;
    BOOST_FOREACH (TreeNodeSyscallParam& param, s->params()) {
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

    Symbol* ret = NULL;

    // Release stuff that is returned by the syscall.
    BOOST_FOREACH (const NodeSymbolPair & ts, results) {
        switch (ts.first->type()) {
        case NODE_SYSCALL_RETURN:
            releaseResource (result, ts.second);
        default:
            continue;
        }
    }

    BOOST_FOREACH (const NodeSymbolPair & ts, results) {
        Imop::Type iType;
        switch (ts.first->type()) {
        case NODE_SYSCALL_RETURN: ret = ts.second;  continue;
        case NODE_PUSH:     iType = Imop::PUSH;     break;
        case NODE_PUSHREF:  iType = Imop::PUSHREF;  break;
        case NODE_PUSHCREF: iType = Imop::PUSHCREF; break;
        default:
            assert(false && "ICE!");
            result.setStatus(CGResult::ERROR_FATAL);
            return result;
        }

        Imop * i = new Imop(ts.first, iType, NULL, ts.second);
        pushImopAfter(result, i);
    }

    assert (dynamic_cast<ConstantString*>(nameResult.symbol ()) != NULL);
    ConstantString* syscallName = static_cast<ConstantString*>(nameResult.symbol ());

    Imop * i = new Imop(s, Imop::SYSCALL, ret, syscallName);
    pushImopAfter(result, newComment(syscallName->value()));
    pushImopAfter(result, i);

    BOOST_FOREACH (const NodeSymbolPair & ts, results) {
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

    if (m_tyChecker->visit(s) != TypeChecker::OK)
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
    if (result.firstImop() == NULL) {
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

    assert(eResult.firstImop() != NULL);

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

    s->expression ()->instantiateDataType (getContext ());
    CGStmtResult result = codeGen(s->expression());
    if (result.symbol() != NULL) {
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
    if (m_tyChecker->visit(s) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    TreeNodeExpr * e = s->expression();
    CGBranchResult eResult(codeGenBranch(e));
    assert(eResult.firstImop() != NULL);
    if (eResult.isNotOk()) {
        return eResult;
    }

    CGStmtResult result;
    releaseAllVariables(result);

    std::ostringstream ss;
    ss << "assert failed at " << s->location();
    Imop * i = newError(s, ConstantString::get(getContext(), ss.str()));
    pushImopAfter(result, i);

    eResult.patchFalseList(m_st->label(result.firstImop()));
    result.addToNextList(eResult.trueList());
    result.setFirstImop(eResult.firstImop());
    return result;
}

} // namespace SecreC
