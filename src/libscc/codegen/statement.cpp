#include "treenode.h"

#include "symboltable.h"
#include "misc.h"
#include "typechecker.h"
#include "codegen.h"
#include "constant.h"

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

CGStmtResult CodeGen::cgStmtCompound (TreeNodeStmtCompound* s) {
    typedef TreeNode::ChildrenListConstIterator CLCI;

    CGStmtResult result;

    newScope ();

    for (CLCI it (s->children ().begin ()); it != s->children ().end (); ++ it) {
        assert(dynamic_cast<TreeNodeStmt*> (*it) != 0);
        TreeNodeStmt *c = static_cast<TreeNodeStmt*> (*it);
        const CGStmtResult& cResult = codeGenStmt (c);
        append (result, cResult);
        if (result.isNotOk ()) {
            break;
        }

        if (cResult.firstImop () == 0) {
            if (c->type () != NODE_DECL) {
                log.fatal () << "Statement with no effect at " << c->location ();
                result.setStatus (ICode::E_OTHER);
                break;
            }
        }

        result.addToBreakList (cResult.breakList ());
        result.addToContinueList (cResult.continueList ());

        // Static checking:
        if ((result.flags () & CGStmtResult::FALLTHRU) == 0x0) {
            log.fatal () << "Unreachable statement at " << c->location ();
            result.setStatus (ICode::E_OTHER);
            break;
        } else {
            result.setFlags((result.flags () & ~CGStmtResult::FALLTHRU)
                           | cResult.flags ());
        }
    }

    popScope ();

    return result;
}

/*******************************************************************************
  TreeNodeStmtBreak
*******************************************************************************/

CGStmtResult TreeNodeStmtBreak::codeGenWith (CodeGen& cg) {
    return cg.cgStmtBreak (this);
}

CGStmtResult CodeGen::cgStmtBreak (TreeNodeStmtBreak* s) {
    CGStmtResult result;
    Imop *i = new Imop (s, Imop::JUMP, 0);
    code.push_imop (i);
    result.setFirstImop (i);
    result.addToBreakList (i);
    result.setFlags(CGStmtResult::BREAK);
    return result;
}

/*******************************************************************************
  TreeNodeStmtContinue
*******************************************************************************/

CGStmtResult TreeNodeStmtContinue::codeGenWith (CodeGen& cg) {
    return cg.cgStmtContinue (this);
}

CGStmtResult CodeGen::cgStmtContinue (TreeNodeStmtContinue* s) {
    CGStmtResult result;
    Imop *i = new Imop (s, Imop::JUMP, 0);
    code.push_imop (i);
    result.setFirstImop (i);
    result.addToContinueList (i);
    result.setFlags(CGStmtResult::CONTINUE);
    return result;
}

/*******************************************************************************
  TreeNodeStmtDecl
*******************************************************************************/

CGStmtResult TreeNodeStmtDecl::codeGenWith (CodeGen& cg) {
    assert (children ().size () > 0 && children ().size () <= 4);
    return cg.cgStmtDecl (this);
}

CGStmtResult CodeGen::cgStmtDecl (TreeNodeStmtDecl* s) {
    CGStmtResult result;
    ICode::Status status = m_tyChecker.visit (s);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    // Initialize the new symbol (for initializer target)
    SymbolSymbol *ns = new SymbolSymbol (s->resultType ());
    SymbolSymbol::ScopeType scopeType = s->global () ? SymbolSymbol::GLOBAL : SymbolSymbol::LOCAL;
    ns->setScopeType (scopeType);
    ns->setName (s->variableName ());
    st->appendSymbol (ns);

    bool isScalar = s->resultType ()->isScalar ();
    unsigned n = 0;

    // Initialize shape:
    TypeNonVoid* dimType =
        TypeNonVoid::get (getContext (),
            DataTypeVar::get (getContext (),
                DataTypeBasic::get (getContext (), DATATYPE_INT)));
    for (unsigned i = 0; i < s->resultType ()->secrecDimType(); ++ i) {
        SymbolSymbol* sym = new SymbolSymbol (dimType);
        sym->setScopeType (scopeType);
        std::stringstream ss;
        ss << s->variableName () << "{d" << i << "}";
        sym->setName (ss.str ());
        st->appendSymbol (sym);
        ns->setDim (i, sym);
    }

    if (!isScalar) { // set size symbol
        SymbolSymbol* sizeSym = new SymbolSymbol (dimType);
        sizeSym->setScopeType (scopeType);
        std::stringstream ss;
        ss << s->variableName () << "{size}";
        sizeSym->setName (ss.str ());
        st->appendSymbol (sizeSym);
        ns->setSizeSym (sizeSym);
    }

    // evaluate shape if given, also compute size
    if (s->children ().size () > 2) {
        if (!isScalar) {
            Imop* i = new Imop (s, Imop::ASSIGN, ns->getSizeSym (),
                                ConstantInt::get (getContext (), 1));
            pushImopAfter (result, i);
        }

        TreeNode::ChildrenListConstIterator
                ei = s->children ().at (2)->children ().begin (),
                ee = s->children ().at (2)->children ().end ();
        for (; ei != ee; ++ ei, ++ n) {
            assert (dynamic_cast<TreeNodeExpr*> (*ei) != 0);
            TreeNodeExpr* e = static_cast<TreeNodeExpr*> (*ei);
            const CGResult& eResult = codeGen (e);
            append (result, eResult);
            if (result.isNotOk ()) {
                return result;
            }

            Imop* i = new Imop (s, Imop::ASSIGN, ns->getDim (n), eResult.symbol ());
            pushImopAfter (result, i);

            i = new Imop (s, Imop::MUL, ns->getSizeSym (), ns->getSizeSym (),
                          eResult.symbol ());
            code.push_imop (i);
        }
    }
    else {
        if (!isScalar) {
            Imop* i = new Imop (s, Imop::ASSIGN, ns->getSizeSym (),
                                ConstantInt::get (getContext (), 0));
            pushImopAfter (result, i);
        }

        for (unsigned it = 0; it < s->resultType ()->secrecDimType (); ++ it) {
            Imop* i = new Imop( s, Imop::ASSIGN, ns->getDim (it),
                                ConstantInt::get (getContext (), 0));
            code.push_imop(i);
        }
    }

    if (s->procParam ()) {
        Imop *i = 0;

        if (isScalar) {
            i = new Imop (s, Imop::PARAM, ns);
            pushImopAfter (result, i);
        }
        else {

            SymbolSymbol* tns = st->appendTemporary(
                        static_cast<TypeNonVoid*> (ns->secrecType ()));

            i = new Imop (s, Imop::PARAM, tns);
            pushImopAfter (result, i);

            for (dim_iterator di = dim_begin (ns), de = dim_end (ns); di != de; ++ di) {
                i = new Imop (s, Imop::PARAM, *di);
                code.push_imop(i);
            }

            i = new Imop (s, Imop::ASSIGN, ns->getSizeSym (),
                          ConstantInt::get (getContext (), 1));
            pushImopAfter (result, i);

            for (dim_iterator di = dim_begin (ns), de = dim_end (ns); di != de; ++ di) {
                i = new Imop (s, Imop::MUL, ns->getSizeSym (), ns->getSizeSym (), *di);
                code.push_imop(i);
            }

            i = new Imop (s, Imop::ALLOC, ns,
                defaultConstant (getContext (), ns->secrecType ()->secrecDataType ()),
                ns->getSizeSym ());
            code.push_imop (i);

            i = new Imop (s, Imop::ASSIGN, ns, tns, ns->getSizeSym());
            code.push_imop (i);
        }
    }
    else
    if (s->children ().size () > 3) {

        // evaluate rhs
        TreeNode *t = s->children ().at (3);
        assert ((t->type () & NODE_EXPR_MASK) != 0x0);
        assert (dynamic_cast<TreeNodeExpr*> (t) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        const CGResult& eResult = codeGen (e);
        append (result, eResult);
        if (result.isNotOk ()) {
            return result;
        }

        // type x = foo;
        if (s->resultType ()->secrecDimType () > 0 && n == 0) {
            if (s->resultType ()->secrecDimType () > e->resultType ()->secrecDimType ()) {
                Imop* i = new Imop (s, Imop::ALLOC, ns, eResult.symbol (), ns->getSizeSym());
                pushImopAfter (result, i);
            }
            else {
                assert (s->resultType ()->secrecDimType() == e->resultType()->secrecDimType());

                SymbolSymbol* eResultSymbol = static_cast<SymbolSymbol*>(eResult.symbol ());
                dim_iterator
                    di = dim_begin (ns),
                    dj = dim_begin (eResultSymbol),
                    de = dim_end (ns);
                for (; di != de; ++ di, ++ dj) {
                    Imop* i = new Imop (s, Imop::ASSIGN, *di, *dj);
                    pushImopAfter (result, i);
                }

                if (!isScalar) {
                    Imop* i = newAssign (s,  ns->getSizeSym (), eResultSymbol->getSizeSym ());
                    code.push_imop(i);
                }

                Imop* i = new Imop (s, Imop::ALLOC, ns, eResultSymbol, ns->getSizeSym());
                pushImopAfter (result, i);

                i = newAssign (s, ns, eResultSymbol);
                code.push_imop (i);
            }
        }

        // arr x[e1,...,en] = foo;
        if (n > 0 && s->resultType ()->secrecDimType () == n) {
            if (s->resultType ()->secrecDimType() > e->resultType ()->secrecDimType ()) {
                // fill lhs with constant value
                Imop* i = new Imop (s, Imop::ALLOC, ns, eResult.symbol (), ns->getSizeSym ());
                pushImopAfter (result, i);
            }
            else {
                // check that shapes match and assign
                std::stringstream ss;
                ss << "Shape mismatch at " << s->location ();
                Imop* err = newError (s, ConstantString::get (getContext (), ss.str ()));
                SymbolLabel* errLabel = st->label (err);
                SymbolSymbol* eResultSymbol = static_cast<SymbolSymbol*>(eResult.symbol ());
                dim_iterator
                        di = dim_begin (eResultSymbol),
                        dj = dim_begin (ns),
                        de = dim_end (eResultSymbol);
                for (; di != de; ++ di, ++ dj) {
                    Imop* i = new Imop (s, Imop::JNE, (Symbol*) 0, *di, *dj);
                    i->setJumpDest (errLabel);
                    pushImopAfter (result, i);
                }

                Imop* jmp = new Imop (s, Imop::JUMP, (Symbol*) 0);
                Imop* i = new Imop (s, Imop::ASSIGN, ns, eResultSymbol, ns->getSizeSym ());
                jmp->setJumpDest (st->label(i));
                pushImopAfter (result, jmp);
                code.push_imop(err);
                code.push_imop(i);
            }
        }

        // scalar_type x = scalar;
        if (s->resultType ()->isScalar()) {
            Imop* i = new Imop (s, Imop::ASSIGN, ns, eResult.symbol ());
            pushImopAfter (result, i);
        }
    }
    else {
        if (!isScalar && n == 0) {
            Imop* i = new Imop (s, Imop::ASSIGN, ns->getSizeSym (),
                                ConstantInt::get (getContext (), 0));
            pushImopAfter (result, i);

            for (unsigned it = 0; it < s->resultType ()->secrecDimType (); ++ it) {
                Imop* i = new Imop (s, Imop::ASSIGN, ns->getDim (it),
                                    ConstantInt::get (getContext (), 0));
                code.push_imop (i);
            }
        }

        Imop *i = 0;
        if (isScalar) {
            i = new Imop (s, Imop::ASSIGN, ns, (Symbol*) 0);
        }
        else {
            i = new Imop (s, Imop::ALLOC, ns, (Symbol*) 0, (Symbol*) 0);
            if (n == 0) {
                i->setArg2 (ConstantInt::get (getContext (), 0));
            }
            else {
                i->setArg2 (ns->getSizeSym());
            }
        }

        pushImopAfter (result, i);

        typedef DataTypeBasic DTB;
        typedef DataTypeVar DTV;
        assert (s->resultType ()->dataType ()->kind () == DataType::VAR);
        assert (dynamic_cast<DTV*> (s->resultType ()->dataType ()) != 0);
        const DTV &dtv (*static_cast<DTV*> (s->resultType ()->dataType ()));
        assert (dtv.dataType()->kind() == DataType::BASIC);
        assert (dynamic_cast<DTB*> (dtv.dataType ()) != 0);
        const DTB &dtb (*static_cast<DTB*> (dtv.dataType ()));

        Symbol *def = defaultConstant (getContext (), dtb.dataType ());
        i->setArg1(def);
    }

    return result;
}

/*******************************************************************************
  TreeNodeStmtFor
*******************************************************************************/

CGStmtResult TreeNodeStmtFor::codeGenWith (CodeGen& cg) {
    return cg.cgStmtFor (this);
}

CGStmtResult CodeGen::cgStmtFor (TreeNodeStmtFor* s) {
    CGStmtResult result;
    bool createdScope = false;

    // Initialization expression:
    if (s->initializer () != 0) {
        if (dynamic_cast<TreeNodeExpr*> (s->initializer ()) != 0) {
            append (result, codeGen (static_cast<TreeNodeExpr*> (s->initializer ())));
        }
        else
        if (dynamic_cast<TreeNodeStmtDecl*>(s->initializer ()) != 0) {
            newScope ();
            createdScope = true;
            append (result, codeGenStmt (static_cast<TreeNodeStmtDecl*>(s->initializer ())));
        }
    }

    if (result.isNotOk ()) {
        return result;
    }

    // Conditional expression:
    CGBranchResult condResult;
    if (s->conditional () != 0) {
        TreeNodeExpr *e1 = s->conditional ();
        e1->setContextType (PublicSecType::get (getContext ()));
        ICode::Status status = e1->accept (m_tyChecker);
        if (status != ICode::OK) {
            result.setStatus (status);
            return result;
        }

        if (!e1->havePublicBoolType ()) {
            log.fatal () << "Conditional expression in if statement must be of "
                            "type public bool at " << e1->location () << ".";
            result.setStatus (ICode::E_TYPE);
            return result;
        }

        condResult = codeGenBranch (e1);
        append (result, condResult);
        if (result.isNotOk ()) {
            return result;
        }
    }

    // Body of for loop:
    TreeNodeStmt *body = s->body ();
    CGStmtResult bodyResult (codeGenStmt (body));
    if (bodyResult.isNotOk ()) {
        result.setStatus (bodyResult.status ());
        return result;
    }

    // Iteration expression:
    CGResult iterResult;
    if (s->iteratorExpr ()) {
        iterResult = codeGen (s->iteratorExpr ());
        if (iterResult.isNotOk ()) {
            result.setStatus (iterResult.status ());
            return result;
        }
    }

    // Next iteration jump:
    Imop *j = new Imop (s, Imop::JUMP, 0);
    pushImopAfter (iterResult, j);

    bodyResult.patchFirstImop (iterResult.firstImop ());
    condResult.patchFirstImop (bodyResult.firstImop ());
    SymbolLabel* nextIterDest = st->label (iterResult.firstImop ());
    SymbolLabel* firstBodyDest = st->label (bodyResult.firstImop ());

    // i hope the following is not too unreadable:
    result.patchFirstImop (condResult.firstImop ());
    result.addToNextList (condResult.falseList ()); // if condition if false jump out of for loop
    result.addToNextList (bodyResult.breakList ()); // if break is reach jump out of for loop
    j->setJumpDest (st->label (condResult.firstImop ())); // after iteration jump to contitional
    condResult.patchTrueList (firstBodyDest); // if conditional is true jump to body
    bodyResult.patchNextList (nextIterDest); // next jumps to iteration
    bodyResult.patchContinueList (nextIterDest); // continue jumps to iteration

    if (createdScope) {
        popScope ();
    }

    // Static checking:
    assert (bodyResult.flags () != 0x0);
    if ((bodyResult.flags ()
        & (CGStmtResult::FALLTHRU | CGStmtResult::CONTINUE)) == 0x0)
    {
        log.fatal() << "For loop at " << s->location () << " wont loop!";
        result.setStatus (ICode::E_OTHER);
        return result;
    }
    if (condResult.firstImop () == 0 && ((bodyResult.flags ()
        & (CGStmtResult::BREAK | CGStmtResult::RETURN)) == 0x0))
    {
        log.fatal () << "For loop at " << s->location () << " is clearly infinite!";
        result.setStatus (ICode::E_OTHER);
        return result;
    }

    result.setFlags ((bodyResult.flags ()
                    & ~(CGStmtResult::BREAK | CGStmtResult::CONTINUE))
                    | CGStmtResult::FALLTHRU);

    return result;
}

/*******************************************************************************
  TreeNodeStmtIf
*******************************************************************************/

CGStmtResult TreeNodeStmtIf::codeGenWith (CodeGen& cg) {
    return cg.cgStmtIf (this);
}

CGStmtResult CodeGen::cgStmtIf (TreeNodeStmtIf* s) {
    CGStmtResult result;
    TreeNodeExpr *e = s->conditional ();
    e->setContextType (PublicSecType::get (getContext ()));
    ICode::Status status = e->accept (m_tyChecker);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    if (!e->havePublicBoolType ()) {
        log.fatal () << "Conditional expression in if statement must be of "
                        "type public bool in " << e->location ();
        result.setStatus (ICode::E_TYPE);
        return result;
    }

    // Generate code for conditional expression:
    CGBranchResult eResult (codeGenBranch (e));
    append (result, eResult);
    if (result.isNotOk ()) {
        return result;
    }

    assert (result.firstImop () != 0);

    // Generate code for first branch:
    newScope ();
    TreeNodeStmt *s1 = s->trueBranch ();
    CGStmtResult trueResult (codeGenStmt (s1));
    if (trueResult.isNotOk ()) {
        result.setStatus (trueResult.status ());
        return result;
    }

    popScope ();

    if (trueResult.firstImop () != 0) {
        eResult.patchTrueList (st->label (trueResult.firstImop ()));
        result.addToNextList (trueResult.nextList ());
        result.addToBreakList (trueResult.breakList ());
        result.addToContinueList (trueResult.continueList ());
        assert (trueResult.flags () != 0x0);
    } else {
        result.addToNextList (eResult.trueList ());
    }

    if (s->falseBranch () == 0) {
        result.addToNextList (eResult.falseList ());
        result.setFlags (trueResult.flags () | CGStmtResult::FALLTHRU);
    } else {
        // Generate jump out of first branch, if needed:
        if ((trueResult.flags () & CGStmtResult::FALLTHRU) != 0x0) {
            Imop *i = new Imop (s, Imop::JUMP, 0);
            code.push_imop (i);
            result.addToNextList (i);
        }

        // Generate code for second branch:
        newScope ();
        TreeNodeStmt *s2 = s->falseBranch ();
        CGStmtResult falseResult (codeGenStmt (s2));
        if (falseResult.isNotOk ()) {
            result.setStatus (falseResult.status ());
            return result;
        }

        popScope ();

        eResult.patchFalseList(st->label(falseResult.firstImop ()));
        result.addToNextList (falseResult.nextList ());
        result.addToBreakList (falseResult.breakList ());
        result.addToContinueList (falseResult.continueList ());
        assert (falseResult.flags() != 0x0);
        result.setFlags (trueResult.flags () | falseResult.flags ());
    }

    return result;
}

/*******************************************************************************
  TreeNodeStmtReturn
*******************************************************************************/

CGStmtResult TreeNodeStmtReturn::codeGenWith (CodeGen& cg) {
    return cg.cgStmtReturn (this);
}

CGStmtResult CodeGen::cgStmtReturn (TreeNodeStmtReturn* s) {
    CGStmtResult result;
    ICode::Status status = m_tyChecker.visit (s);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    if (s->expression () == 0) {
        Imop *i = new Imop (s, Imop::RETURNVOID, 0);
        i->setReturnDestFirstImop (st->label (s->containingProcedure ()->symbol ()->target ()));
        pushImopAfter (result, i);
    } else {
        TreeNodeExpr *e = s->expression ();
        assert (e->haveResultType ());

        const CGResult& eResult (codeGen (e));
        append (result, eResult);
        if (result.isNotOk ()) {
            return result;
        }

        std::list<Symbol* > rets;
        rets.insert (rets.end (), dim_begin (eResult.symbol ()), dim_end (eResult.symbol ()));
        rets.push_back (eResult.symbol ());
        Imop* i = newReturn (s, rets.begin (), rets.end ());
        i->setReturnDestFirstImop (st->label (s->containingProcedure ()->symbol ()->target ()));
        pushImopAfter (result, i);
    }

    result.setFlags (CGStmtResult::RETURN);
    return result;
}

/*******************************************************************************
  TreeNodeStmtWhile
*******************************************************************************/

CGStmtResult TreeNodeStmtWhile::codeGenWith (CodeGen& cg) {
    return cg.cgStmtWhile (this);
}

CGStmtResult CodeGen::cgStmtWhile (TreeNodeStmtWhile* s) {

    // Conditional expression:
    CGStmtResult result;
    TreeNodeExpr *e = s->conditional ();
    e->setContextType (PublicSecType::get (getContext ()));
    ICode::Status status = e->accept (m_tyChecker);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in while statement must be of "
                       "type public bool in " << e->location();
        result.setStatus (ICode::E_TYPE);
        return result;
    }

    CGBranchResult eResult (codeGenBranch (e));
    append (result, eResult);
    if (result.isNotOk ()) {
        return result;
    }

    assert (result.firstImop () != 0);
    assert (result.nextList ().empty ());

    SymbolLabel* jumpDest = st->label (result.firstImop ());

    // Loop body:
    newScope ();

    TreeNodeStmt *body = s->body ();
    CGStmtResult bodyResult (codeGenStmt (body));
    if (bodyResult.isNotOk ()) {
        result.setStatus (bodyResult.status ());
        return result;
    }

    assert (bodyResult.flags () != 0x0);
    if ((bodyResult.flags ()
        & (CGStmtResult::FALLTHRU | CGStmtResult::CONTINUE)) == 0x0)
    {
        log.fatal () << "While loop at " << s->location () << " wont loop!";
        result.setStatus (ICode::E_OTHER);
        return result;
    }

    result.setFlags ((bodyResult.flags ()
                      & ~(CGStmtResult::BREAK | CGStmtResult::CONTINUE))
                      | CGStmtResult::FALLTHRU);

    Imop* i = new Imop (s, Imop::JUMP, 0);
    pushImopAfter (bodyResult, i);
    i->setJumpDest (jumpDest);

    // Patch jump lists:
    eResult.patchTrueList (st->label (bodyResult.firstImop ()));
    result.setNextList (eResult.falseList ());
    result.addToNextList (bodyResult.breakList ());
    bodyResult.patchContinueList (jumpDest);

    popScope ();

    return result;
}

/*******************************************************************************
  TreeNodeStmtPrint
*******************************************************************************/

CGStmtResult TreeNodeStmtPrint::codeGenWith (CodeGen& cg) {
    return cg.cgStmtPrint (this);
}

CGStmtResult CodeGen::cgStmtPrint (TreeNodeStmtPrint* s) {
    TreeNodeExpr *e = s->expression ();
    // Type check:
    CGStmtResult result;
    result.setStatus (m_tyChecker.visit (s));
    if (result.isNotOk ()) {
        return result;
    }

    // Generate code:
    const CGResult& eResult = codeGen (e);
    append (result, eResult);
    if (result.isNotOk ()) {
        return result;
    }

    Imop* i = new Imop (s, Imop::PRINT, (Symbol*) 0, eResult.symbol ());
    pushImopAfter (result, i);
    return result;
}

/*******************************************************************************
  TreeNodeStmtSyscall
*******************************************************************************/

CGStmtResult TreeNodeStmtSyscall::codeGenWith (CodeGen& cg) {
    return cg.cgStmtSyscall (this);
}

CGStmtResult CodeGen::cgStmtSyscall (TreeNodeStmtSyscall* s) {
    TreeNodeExprString *e = s->expression ();

    // Generate code:
    CGStmtResult result;
    const CGResult& eResult = codeGen (e);
    append (result, eResult);
    if (result.isNotOk ()) {
        return result;
    }

    Imop* i = new Imop (s, Imop::SYSCALL, (Symbol*) 0, eResult.symbol ());
    pushImopAfter (result, i);
    return result;
}

/*******************************************************************************
  TreeNodeStmtPush
*******************************************************************************/

CGStmtResult TreeNodeStmtPush::codeGenWith (CodeGen& cg) {
    return cg.cgStmtPush (this);
}

CGStmtResult CodeGen::cgStmtPush (TreeNodeStmtPush* s) {
    TreeNodeExpr *e = s->expression ();

    // Generate code:
    CGStmtResult result;
    const CGResult& eResult = codeGen (e);
    append (result, eResult);
    if (result.isNotOk ()) {
        return result;
    }

    Imop* i = new Imop (s, Imop::PUSH, (Symbol*) 0, eResult.symbol ());
    pushImopAfter (result, i);
    return result;
}

/*******************************************************************************
  TreeNodeStmtDoWhile
*******************************************************************************/

CGStmtResult TreeNodeStmtDoWhile::codeGenWith (CodeGen& cg) {
    return cg.cgStmtDoWhile (this);
}

CGStmtResult CodeGen::cgStmtDoWhile (TreeNodeStmtDoWhile* s) {

    // Loop body:
    newScope ();

    TreeNodeStmt* body = s->body ();
    CGStmtResult result (codeGenStmt (body));
    if (result.isNotOk ()) {
        return result;
    }

    // Static checking:
    if (result.firstImop () == 0) {
        log.fatal () << "Empty loop body at " << body->location () << ".";
        result.setStatus (ICode::E_OTHER);
        return result;
    }

    assert (result.flags () != 0x0);

    if ((result.flags ()
         & (CGStmtResult::FALLTHRU | CGStmtResult::CONTINUE)) == 0x0)
    {
        log.fatal () << "Do-while loop at " << s->location () << " wont loop!";
        result.setStatus (ICode::E_OTHER);
        return result;
    }

    result.setFlags ((result.flags ()
                    & ~(CGStmtResult::BREAK | CGStmtResult::CONTINUE))
                   | CGStmtResult::FALLTHRU);

    popScope (); // end of loop body

    // Conditional expression:

    TreeNodeExpr *e = s->conditional ();
    e->setContextType (PublicSecType::get (getContext ()));
    ICode::Status status = e->accept (m_tyChecker);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    if (!e->havePublicBoolType()) {
        log.fatal () << "Conditional expression in if statement must be of "
                       "type public bool in " << e->location();
        result.setStatus (ICode::E_TYPE);
        return result;
    }

    CGBranchResult eResult (codeGenBranch (e));
    append (result, eResult);
    if (result.isNotOk ()) {
        return result;
    }

    assert (eResult.firstImop() != 0);

    // Patch jump lists:

    eResult.patchTrueList (st->label (result.firstImop ()));
    result.patchContinueList (st->label (eResult.firstImop ()));
    result.setNextList (result.breakList ());
    result.addToNextList (eResult.falseList ());
    result.clearBreakList ();

    return result;
}

/*******************************************************************************
  TreeNodeStmtExpr
*******************************************************************************/

CGStmtResult TreeNodeStmtExpr::codeGenWith (CodeGen& cg) {
    return cg.cgStmtExpr (this);
}

CGStmtResult CodeGen::cgStmtExpr (TreeNodeStmtExpr* s) {
    return codeGen (s->expression ());
}

/*******************************************************************************
  TreeNodeStmtAssert
*******************************************************************************/

CGStmtResult TreeNodeStmtAssert::codeGenWith (CodeGen& cg) {
    return cg.cgStmtAssert (this);
}

CGStmtResult CodeGen::cgStmtAssert (TreeNodeStmtAssert* s) {
    CGStmtResult result;

    // Type check the expression
    TreeNodeExpr *e = s->expression ();
    e->setContextType (PublicSecType::get (getContext ()));
    ICode::Status status = e->accept (m_tyChecker);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in assert statement must be of "
                       "type public bool in " << e->location();
        result.setStatus (ICode::E_TYPE);
        return result;
    }

    CGBranchResult eResult (codeGenBranch (e));
    assert (eResult.firstImop () != 0);
    append (result, eResult);
    if (result.isNotOk ()) {
        return result;
    }

    std::ostringstream ss;
    ss << "assert failed at " << s->location ();
    Imop *i = newError (s, ConstantString::get (getContext (), ss.str ()));
    pushImopAfter (result, i);

    eResult.patchFalseList (st->label (i));
    result.addToNextList (eResult.trueList ());
    return result;
}

} // namespace SecreC
