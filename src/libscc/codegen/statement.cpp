#include "treenode.h"

#include <boost/foreach.hpp>

#include "codegen.h"
#include "constant.h"
#include "log.h"
#include "misc.h"
#include "symboltable.h"
#include "typechecker.h"

/**
 * Code generation for statements.
 */

namespace SecreC {

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
        assert(dynamic_cast<TreeNodeStmt *>(c_) != 0);
        TreeNodeStmt * c = static_cast<TreeNodeStmt *>(c_);
        const CGStmtResult & cResult = codeGenStmt(c);
        append(result, cResult);
        if (result.isFatal()) break;
        if (result.isNotOk()) continue;

        if (cResult.firstImop() == 0) {
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
    if (loopST() == 0) {
        m_log.fatalInProc(s) << "Break statement not embedded in loop at " << s->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    CGStmtResult result;
    assert(loopST() != 0);
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
    if (loopST() == 0) {
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

CGStmtResult CodeGen::cgVarInit (TypeNonVoid* ty, TreeNodeVarInit* varInit,
                                 bool isGlobal, bool isProcParam)
{
    if (m_tyChecker->checkVarInit(ty, varInit) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    // Initialize the new symbol (for initializer target)
    CGStmtResult result;
    SymbolSymbol::ScopeType scopeType = isGlobal ? SymbolSymbol::GLOBAL : SymbolSymbol::LOCAL;
    const bool isScalar = ty->isScalar();
    const bool isString = ty->secrecDataType() == DATATYPE_STRING;
    const bool isPrivate = ty->secrecSecType()->isPrivate();

    TypeNonVoid * const pubBoolTy = TypeNonVoid::getPublicBoolType(getContext());

    SymbolSymbol * ns = new SymbolSymbol(varInit->variableName(), ty);
    ns->setScopeType(scopeType);
    m_st->appendSymbol(ns);

    SecrecDimType n = 0;
    assert((isScalar || !isString) && "ICE: string arrays should be forbidden by the type checker!");

    // Initialize shape:
    TypeNonVoid * dimType = TypeNonVoid::getIndexType(getContext());
    for (SecrecDimType i = 0; i < ty->secrecDimType(); ++ i) {
        std::stringstream ss;
        ss << varInit->variableName() << "{d" << i << "}";
        SymbolSymbol * sym = new SymbolSymbol(ss.str(), dimType);
        sym->setScopeType(scopeType);
        m_st->appendSymbol(sym);
        ns->setDim(i, sym);
    }

    if (!isScalar) { // set size symbol
        std::stringstream ss;
        ss << varInit->variableName() << "{size}";
        SymbolSymbol * sizeSym = new SymbolSymbol(ss.str(), dimType);
        sizeSym->setScopeType(scopeType);
        m_st->appendSymbol(sizeSym);
        ns->setSizeSym(sizeSym);
    }

    // evaluate shape if given, also compute size
    if (! varInit->shape()->children().empty()) {
        if (!isScalar) {
            Imop * i = new Imop(varInit, Imop::ASSIGN, ns->getSizeSym(), indexConstant(1));
            pushImopAfter(result, i);
        }

        BOOST_FOREACH (TreeNode * _e, varInit->shape()->children()) {
            assert(dynamic_cast<TreeNodeExpr *>(_e) != 0);
            TreeNodeExpr * e = static_cast<TreeNodeExpr *>(_e);
            const CGResult & eResult = codeGen(e);
            append(result, eResult);
            if (result.isNotOk()) {
                return result;
            }

            Imop * i = new Imop(varInit, Imop::ASSIGN, ns->getDim(n), eResult.symbol());
            pushImopAfter(result, i);

            i = new Imop(varInit, Imop::MUL, ns->getSizeSym(), ns->getSizeSym(),
                    eResult.symbol());
            push_imop(i);
            ++ n;
        }
    }
    else {
        if (!isScalar) {
            Imop * i = new Imop(varInit, Imop::ASSIGN, ns->getSizeSym(), indexConstant(0));
            pushImopAfter(result, i);
        }

        for (SecrecDimType it = 0; it < ty->secrecDimType(); ++ it) {
            Imop * i = new Imop(varInit, Imop::ASSIGN, ns->getDim(it), indexConstant(0));
            push_imop(i);
        }
    }

    if (isProcParam) {
        Imop * i = 0;
        SymbolSymbol * tns = m_st->appendTemporary(
                static_cast<TypeNonVoid *>(ns->secrecType()));

        if (isScalar) {
            i = new Imop(varInit, Imop::PARAM, ns);
            pushImopAfter(result, i);
        }
        else {
            i = new Imop(varInit, Imop::PARAM, tns);
            pushImopAfter(result, i);

            for (dim_iterator di = dim_begin(ns), de = dim_end(ns); di != de; ++ di) {
                i = new Imop(varInit, Imop::PARAM, *di);
                push_imop(i);
            }

            i = new Imop(varInit, Imop::ASSIGN, ns->getSizeSym(), indexConstant(1));
            push_imop(i);

            for (dim_iterator di = dim_begin(ns), de = dim_end(ns); di != de; ++ di) {
                i = new Imop(varInit, Imop::MUL, ns->getSizeSym(), ns->getSizeSym(), *di);
                push_imop(i);
            }

            i = new Imop(varInit, Imop::COPY, ns, tns, ns->getSizeSym());
            push_imop(i);
        }
    }
    else // Regular declaration, right hand side is defined:
        if (varInit->rightHandSide() != 0) {

            // evaluate rhs
            TreeNodeExpr * e = varInit->rightHandSide();
            const CGResult & eResult = codeGen(e);
            append(result, eResult);
            if (result.isNotOk()) {
                return result;
            }

            // type x = foo;
            if (ty->secrecDimType() > 0 && n == 0) {
                if (ty->secrecDimType() > e->resultType()->secrecDimType()) {
                    Imop * i = new Imop(varInit, Imop::ALLOC, ns, eResult.symbol(), ns->getSizeSym());
                    pushImopAfter(result, i);
                    releaseTemporary(result, eResult.symbol());
                }
                else {
                    assert(ty->secrecDimType() == e->resultType()->secrecDimType());

                    SymbolSymbol * eResultSymbol = static_cast<SymbolSymbol *>(eResult.symbol());
                    dim_iterator srcIter = dim_begin(eResultSymbol);
                    BOOST_FOREACH (Symbol * destSym, dim_range(ns)) {
                        assert(srcIter != dim_end(eResultSymbol));
                        Imop * i = new Imop(varInit, Imop::ASSIGN, destSym, *srcIter);
                        pushImopAfter(result, i);
                        ++ srcIter;
                    }

                    Imop * i = newAssign(varInit,  ns->getSizeSym(), eResultSymbol->getSizeSym());
                    pushImopAfter(result, i);

                    i = new Imop(varInit, Imop::COPY, ns, eResultSymbol, ns->getSizeSym());
                    pushImopAfter(result, i);
                    releaseTemporary(result, eResult.symbol());
                }
            }

            // arr x[e1,...,en] = foo;
            if (n > 0 && ty->secrecDimType() == n) {
                if (ty->secrecDimType() > e->resultType()->secrecDimType()) {
                    // fill lhs with constant value
                    Imop * i = new Imop(varInit, Imop::ALLOC, ns, eResult.symbol(), ns->getSizeSym());
                    pushImopAfter(result, i);
                    releaseTemporary(result, eResult.symbol());
                }
                else {
                    // check that shapes match and assign
                    std::stringstream ss;
                    ss << "Shape mismatch at " << varInit->location();
                    Imop * err = newError(varInit, ConstantString::get(getContext(), ss.str()));
                    SymbolLabel * errLabel = m_st->label(err);
                    SymbolSymbol * eResultSymbol = static_cast<SymbolSymbol *>(eResult.symbol());
                    dim_iterator lhsDimIter = dim_begin(ns);
                    BOOST_FOREACH (Symbol * rhsDim, dim_range(eResultSymbol)) {
                        SymbolTemporary * temp_bool = m_st->appendTemporary(pubBoolTy);

                        Imop * i = new Imop(varInit, Imop::NE, temp_bool, rhsDim, *lhsDimIter);
                        pushImopAfter(result, i);

                        i = new Imop(varInit, Imop::JT, errLabel, temp_bool);
                        push_imop(i);

                        ++ lhsDimIter;
                    }

                    Imop * jmp = new Imop(varInit, Imop::JUMP, (Symbol *) 0);
                    Imop * i = new Imop(varInit, Imop::COPY, ns, eResultSymbol, ns->getSizeSym());
                    jmp->setJumpDest(m_st->label(i));
                    pushImopAfter(result, jmp);
                    push_imop(err);
                    push_imop(i);
                    releaseTemporary(result, eResultSymbol);
                }
            }

            // scalar_type x = scalar;
            if (ty->isScalar()) {
                Imop * i = new Imop(varInit, Imop::ASSIGN, ns, eResult.symbol());
                pushImopAfter(result, i);
            }
        } // Regular declaration, right hand side is missing:
        else {
            if (!isScalar && n == 0) {
                Imop * i = new Imop(varInit, Imop::ASSIGN, ns->getSizeSym(), indexConstant(0));
                pushImopAfter(result, i);

                for (SecrecDimType it = 0; it < ty->secrecDimType(); ++ it) {
                    Imop * i = new Imop(varInit, Imop::ASSIGN, ns->getDim(it), indexConstant(0));
                    push_imop(i);
                }
            }

            Symbol * def = defaultConstant(getContext(),  ty->secrecDataType());
            Imop * i = 0;
            if (isScalar) {
                Imop::Type iType = isPrivate ? Imop::CLASSIFY : Imop::ASSIGN;
                i = new Imop(varInit, iType, ns, def);
                pushImopAfter(result, i);
            }
            else {
                i = new Imop(varInit, Imop::ALLOC, ns, def, getSizeOr(ns, 0));
                pushImopAfter(result, i);
            }
        }

    return result;
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

    BOOST_FOREACH (TreeNode * _varInit, s->initializers()) {
        assert(dynamic_cast<TreeNodeVarInit *>(_varInit) != 0);
        TreeNodeVarInit * varInit = static_cast<TreeNodeVarInit *>(_varInit);
        append(result, cgVarInit(s->resultType(), varInit, isGlobal, isProcParam));
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
    Symbol * temp = 0;

    // Initialization expression:
    if (s->initializer() != 0) {
        if (dynamic_cast<TreeNodeExpr *>(s->initializer()) != 0) {
            TreeNodeExpr * initE = static_cast<TreeNodeExpr *>(s->initializer());
            const CGResult & initResult = codeGen(initE);
            append(result, initResult);
            temp = initResult.symbol();
        }
        else if (dynamic_cast<TreeNodeStmtDecl *>(s->initializer()) != 0) {
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
    if (s->conditional() != 0) {
        TreeNodeExpr * e1 = s->conditional();
        if (m_tyChecker->checkPublicBooleanScalar(e1) != TypeChecker::OK) {
            m_log.fatalInProc(s) << "Conditional expression in if statement "
                                    "must be of type public bool at "
                                 << e1->location() << '.';
            result.setStatus(CGResult::ERROR_FATAL);
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
    CGStmtResult bodyResult(codeGenStmt(body));
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
    j->setJumpDest(m_st->label(condResult.firstImop()));    // after iteration jump to contitional
    condResult.patchTrueList(firstBodyDest);  // if conditional is true jump to body
    bodyResult.patchNextList(nextIterDest);  // next jumps to iteration
    bodyResult.patchContinueList(nextIterDest);  // continue jumps to iteration

    if (createdScope) {
        releaseScopeVariables(result);
        popScope();
    }
    else {
        if (temp != 0)
            releaseTemporary(result, temp);
    }

    // Static checking:
    assert(bodyResult.flags() != 0x0);
    if ((bodyResult.flags()
                & (CGStmtResult::FALLTHRU | CGStmtResult::CONTINUE)) == 0x0)
    {
        m_log.fatalInProc(s) << "For loop at " << s->location() << " wont loop!";
        result.setStatus(CGResult::ERROR_FATAL);
        return result;
    }
    if (condResult.firstImop() == 0 && ((bodyResult.flags()
                    & (CGStmtResult::BREAK | CGStmtResult::RETURN)) == 0x0))
    {
        m_log.fatalInProc(s) << "For loop at " << s->location() << " is clearly infinite!";
        result.setStatus(CGResult::ERROR_FATAL);
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
    CGBranchResult eResult(codeGenBranch(e));
    append(result, eResult);
    if (result.isNotOk()) {
        return result;
    }


    assert(result.firstImop() != 0);

    // Generate code for first branch:
    newScope();
    TreeNodeStmt * s1 = s->trueBranch();
    CGStmtResult trueResult(codeGenStmt(s1));
    if (trueResult.isNotOk()) {
        result.setStatus(trueResult.status());
        return result;
    }

    popScope();

    if (trueResult.firstImop() != 0) {
        eResult.patchTrueList(m_st->label(trueResult.firstImop()));
        result.addToNextList(trueResult.nextList());
        result.addToBreakList(trueResult.breakList());
        result.addToContinueList(trueResult.continueList());
        assert(trueResult.flags() != 0x0);
    } else {
        result.addToNextList(eResult.trueList());
    }

    if (s->falseBranch() == 0) {
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

        if (falseResult.firstImop() == 0) {
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
    if (s->expression() == 0) {
        releaseProcVariables(result);

        Imop * i = newReturn(s);
        i->setReturnDestFirstImop(m_st->label(s->containingProcedure()->symbol()->target()));
        pushImopAfter(result, i);
    } else {
        TreeNodeExpr * e = s->expression();
        assert(e->haveResultType());

        const CGResult & eResult(codeGen(e));
        append(result, eResult);
        releaseProcVariables(result, eResult.symbol());
        if (result.isNotOk()) {
            return result;
        }

        std::vector<Symbol * > rets(dim_begin(eResult.symbol()), dim_end(eResult.symbol()));
        rets.push_back(eResult.symbol());
        Imop * i = newReturn(s, rets.begin(), rets.end());
        i->setReturnDestFirstImop(m_st->label(s->containingProcedure()->symbol()->target()));
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
    assert(result.firstImop() != 0);
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
    i->setJumpDest(jumpDest);

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

    TypeNonVoid * strTy = TypeNonVoid::get(getContext(), DATATYPE_STRING);

    CGStmtResult result;
    Symbol * accum = 0;
    BOOST_FOREACH (TreeNode * node, s->expressions()) {
        TreeNodeExpr * e = static_cast<TreeNodeExpr *>(node);
        const CGResult & eResult = codeGen(e);
        append(result, eResult);
        if (result.isNotOk()) {
            return result;
        }

        Symbol * str = eResult.symbol();
        if (e->resultType()->secrecDataType() != DATATYPE_STRING) {
            str = m_st->appendTemporary(strTy);
            Imop * i = new Imop(s, Imop::TOSTRING, str, eResult.symbol());
            pushImopAfter(result, i);
        }

        if (accum == 0) {
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

    pushImopAfter(result, new Imop(s, Imop::PRINT, (Symbol *) 0, accum));
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
    typedef std::pair<TreeNode *, Symbol *> NodeSymbolPair;
    std::vector<NodeSymbolPair> results;
    BOOST_FOREACH (TreeNode * arg, s->paramRange()) {
        TreeNodeExpr * e = static_cast<TreeNodeExpr *>(arg->children().at(0));
        const CGResult & eResult = codeGen(e);
        append(result, eResult);
        if (result.isNotOk()) {
            return result;
        }

        results.push_back(std::make_pair(arg, eResult.symbol()));
    }

    const CGResult & nameResult = codeGen(s->name());
    append(result, nameResult);
    if (result.isNotOk()) {
        return result;
    }

    BOOST_FOREACH (const NodeSymbolPair & ts, results) {
        Imop::Type iType;
        switch (ts.first->type()) {
        case NODE_PUSH:     iType = Imop::PUSH;     break;
        case NODE_PUSHREF:  iType = Imop::PUSHREF;  break;
        case NODE_PUSHCREF: iType = Imop::PUSHCREF; break;
        default:
            assert(false && "ICE!");
            result.setStatus(CGResult::ERROR_FATAL);
            return result;
        }

        Imop * i = new Imop(ts.first, iType, (Symbol *) 0, ts.second);
        pushImopAfter(result, i);
    }

    Imop * i = new Imop(s, Imop::SYSCALL, 0, nameResult.symbol());
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
    if (result.firstImop() == 0) {
        m_log.fatalInProc(s) << "Empty loop body at " << body->location() << ".";
        result.setStatus(CGResult::ERROR_FATAL);
        return result;
    }

    assert(result.flags() != 0x0);

    if ((result.flags()
                & (CGStmtResult::FALLTHRU | CGStmtResult::CONTINUE)) == 0x0)
    {
        m_log.fatalInProc(s) << "Do-while loop at " << s->location() << " wont loop!";
        result.setStatus(CGResult::ERROR_FATAL);
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

    assert(eResult.firstImop() != 0);

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
    CGStmtResult result = codeGen(s->expression());
    if (result.symbol() != 0) {
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
    assert(eResult.firstImop() != 0);
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
