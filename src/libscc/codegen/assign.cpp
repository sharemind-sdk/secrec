#include <stack>
#include <vector>

#include "treenode.h"
#include "symboltable.h"
#include "codegen.h"
#include "constant.h"

/**
 * Code generation for assignment expression
 */

namespace SecreC {

/*******************************************************************************
  TreeNodeExprAssign
*******************************************************************************/

CGResult TreeNodeExprAssign::codeGenWith (CodeGen &cg) {
    return cg.cgExprAssign (this);
}

CGResult CodeGen::cgExprAssign (TreeNodeExprAssign *e) {
    typedef SubscriptInfo::SPV SPV; // symbol pair vector


    // Type check:
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGStmtResult (s);
    }

    CGResult result;

    // Generate code for child expressions:
    TreeNode *lval = e->children ().at (0);
    assert (lval != 0);
    assert (dynamic_cast<TreeNodeIdentifier*>(lval->children ().at (0)) != 0);
    TreeNodeIdentifier *eArg1 = static_cast<TreeNodeIdentifier*>(lval->children ().at (0));
    SymbolSymbol *destSym = 0;
    {
        Symbol *t = m_st->find (eArg1->value ());
        assert (t->symbolType () == Symbol::SYMBOL);
        assert (dynamic_cast<SymbolSymbol*> (t) != 0);
        destSym = static_cast<SymbolSymbol*> (t);
    }
    result.setResult (destSym);

    // Generate code for righthand side:
    TreeNodeExpr *eArg2 = e->rightHandSide ();
    const CGResult& arg2Result = codeGen (eArg2);
    append (result, arg2Result);
    if (result.isNotOk ()) {
        return result;
    }

    TypeNonVoid* pubIntTy = TypeNonVoid::getIndexType (getContext ());

    // x[e1,...,ek] = e
    if (lval->children().size() == 2) {

        // 1. evaluate subscript
        SubscriptInfo cgSub;
        append (result, codeGenSubscript (cgSub, destSym, lval->children ().at (1)));
        if (result.isNotOk ()) {
            return result;
        }

        const SPV& spv = cgSub.spv ();
        const SubscriptInfo::SliceIndices& slices = cgSub.slices ();

        // 2. check that rhs has correct dimensions
        if (!eArg2->resultType ()->isScalar()) {
            assert (static_cast<size_t>(eArg2->resultType ()->secrecDimType ()) == slices.size ());

            std::stringstream ss;
            ss << "Shape of RHS doesnt match shape of LHS in assignment at " << e->location() << ".";
            Imop* jmp = new Imop (e, Imop::JUMP, (Symbol*) 0);
            Imop* err = newError (e, ConstantString::get (getContext (), ss.str ()));
            SymbolLabel* errLabel = m_st->label (err);
            SymbolSymbol* arg2ResultSymbol = static_cast<SymbolSymbol*>(arg2Result.symbol ());

            for (SecrecDimType k = 0; k < eArg2->resultType ()->secrecDimType(); ++ k) {
                Symbol* tsym = m_st->appendTemporary (pubIntTy);
                Imop* i = new Imop (e, Imop::SUB, tsym, spv[slices[k]].second, spv[slices[k]].first);
                pushImopAfter (result, i);

                i = new Imop (e, Imop::JNE, (Symbol*) 0, tsym, arg2ResultSymbol->getDim(k));
                push_imop (i);
                i->setJumpDest (errLabel);
            }

            push_imop (jmp);
            result.addToNextList (jmp);
            push_imop (err);
        }

        // 3. initialize stride
        ArrayStrideInfo stride (destSym);
        append (result, codeGenStride (stride));
        if (result.isNotOk ()) {
            return result;
        }

        // 4. initialze running indices
        LoopInfo loopInfo;
        for (SPV::const_iterator it (spv.begin ()); it != spv.end (); ++ it) {
            Symbol* sym = m_st->appendTemporary (pubIntTy);
            loopInfo.push_index (sym);
        }

        // 6. initialze symbols for offsets and temporary results
        Symbol* offset = m_st->appendTemporary(pubIntTy);
        Symbol* old_offset = m_st->appendTemporary(pubIntTy);
        Symbol* tmp_result2 = m_st->appendTemporary(pubIntTy);

        // offset = 0
        Imop* i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (), 0));
        pushImopAfter (result, i);

        // 7. start
        append (result, enterLoop (loopInfo, spv));
        if (result.isNotOk ()) {
            return result;
        }

        // 8. compute offset for RHS
        {
            // old_ffset = 0
            Imop* i = new Imop (e, Imop::ASSIGN, old_offset, ConstantInt::get (getContext (), 0));
            pushImopAfter (result, i);

            unsigned count = 0;
            LoopInfo::iterator itIt = loopInfo.begin();
            LoopInfo::iterator itEnd = loopInfo.end();
            for (; itIt != itEnd; ++ count, ++ itIt) {
                // tmp_result2 = s[k] * idx[k]
                i = new Imop (e, Imop::MUL, tmp_result2, stride.at (count), *itIt);
                push_imop (i);

                // old_offset = old_offset + tmp_result2
                i = new Imop (e, Imop::ADD, old_offset, old_offset, tmp_result2);
                push_imop (i);
            }
        }

        // 9. load and store
        {
            TypeNonVoid* ty = TypeNonVoid::get (getContext (),
                e->resultType ()->secrecSecType(), e->resultType ()->secrecDataType());
            if (e->type () == NODE_EXPR_ASSIGN) {
                if (!eArg2->resultType ()->isScalar ()) {
                    Symbol* t1 = m_st->appendTemporary (ty);

                    Imop* i = new Imop (e, Imop::LOAD, t1, arg2Result.symbol (), offset);
                    push_imop(i);

                    i = new Imop (e, Imop::STORE, destSym, old_offset, t1);
                    push_imop(i);
                }
                else {
                    Imop* i = new Imop (e, Imop::STORE, destSym, old_offset, arg2Result.symbol ());
                    push_imop(i);
                }
            }
            else {
                Symbol* t1 = m_st->appendTemporary (ty);
                Symbol* t2 = m_st->appendTemporary (ty);
                Imop::Type iType;

                switch (e->type ()) {
                    case NODE_EXPR_ASSIGN_MUL: iType = Imop::MUL; break;
                    case NODE_EXPR_ASSIGN_DIV: iType = Imop::DIV; break;
                    case NODE_EXPR_ASSIGN_MOD: iType = Imop::MOD; break;
                    case NODE_EXPR_ASSIGN_ADD: iType = Imop::ADD; break;
                    case NODE_EXPR_ASSIGN_SUB: iType = Imop::SUB; break;
                    default:
                        assert (false); // shouldn't happen
                        result.setStatus (ICode::E_OTHER);
                        return result;
                }

                Imop* i = new Imop(e, Imop::LOAD, t1, destSym, old_offset);
                push_imop(i);

                if (!eArg2->resultType ()->isScalar()) {
                    i = new Imop(e, Imop::LOAD, t2, arg2Result.symbol (), offset);
                    push_imop(i);

                    i = new Imop(e, iType, t1, t1, t2);
                    push_imop(i);

                    i = new Imop(e, Imop::STORE, destSym, old_offset, t1);
                    push_imop(i);
                }
                else {
                    i = new Imop(e, iType, t1, t1, arg2Result.symbol ());
                    push_imop(i);

                    i = new Imop(e, Imop::STORE, destSym, old_offset, t1);
                    push_imop(i);
                }
            }

            // offset = offset + 1
            Imop* i = new Imop (e, Imop::ADD, offset, offset, ConstantInt::get (getContext (), 1));
            push_imop (i);
       }

        // 9. loop exit
        append (result, exitLoop (loopInfo));
        if (result.isNotOk ()) {
            return result;
        }

        return result;
    }

    // Generate code for regular x = e assignment
    if (e->type () == NODE_EXPR_ASSIGN) {
        if (!eArg2->resultType ()->isScalar()) {
            copyShapeFrom (result, arg2Result.symbol ());
            if (result.isNotOk ()) {
                return result;
            }
        }

        Imop *i = 0;
        if (e->resultType ()->isScalar()) {
            if (e->resultType ()->secrecDataType () == DATATYPE_STRING) {
                i = new Imop (e, Imop::RELEASE, (Symbol*) 0, destSym);
                pushImopAfter (result, i);

                i = new Imop (e, Imop::PUSHCREF, arg2Result.symbol ());
                pushImopAfter (result, i);

                i = new Imop (e, Imop::PUSHREF, destSym);
                pushImopAfter (result, i);

                ConstantString* sc_strcpy = ConstantString::get (getContext (), "strcpy");
                i = new Imop (e, Imop::SYSCALL, (Symbol*) 0, sc_strcpy);
            }
            else {
                i = new Imop (e, Imop::ASSIGN, destSym, arg2Result.symbol ());
            }
        } else {
            i = new Imop (e, Imop::RELEASE, 0, destSym);
            pushImopAfter (result, i);

            if (eArg2->resultType ()->isScalar ()) {
                i = new Imop (e, Imop::ALLOC, destSym, arg2Result.symbol (), destSym->getSizeSym ());
            }
            else {
                i = new Imop (e, Imop::COPY, destSym, arg2Result.symbol (), destSym->getSizeSym ());
            }
        }

        pushImopAfter (result, i);
    } else {
        // Arithmetic assignments

        Imop::Type iType;
        switch (e->type ()) {
            case NODE_EXPR_ASSIGN_MUL: iType = Imop::MUL; break;
            case NODE_EXPR_ASSIGN_DIV: iType = Imop::DIV; break;
            case NODE_EXPR_ASSIGN_MOD: iType = Imop::MOD; break;
            case NODE_EXPR_ASSIGN_ADD: iType = Imop::ADD; break;
            case NODE_EXPR_ASSIGN_SUB: iType = Imop::SUB; break;
            default:
                assert (false); // shouldn't happen
                result.setStatus (ICode::E_OTHER);
                return result;
        }

        Imop *i = 0;
        if (e->resultType ()->isScalar()) {
            i = new Imop (e, iType, destSym, destSym, arg2Result.symbol ());
            pushImopAfter (result, i);
        }
        else {
            ScopedAllocations allocs (*this, result);
            Symbol* rhsSym = arg2Result.symbol ();
            if (eArg2->resultType ()->isScalar ()) {
                rhsSym = m_st->appendTemporary (static_cast<TypeNonVoid*> (e->resultType ()));
                allocs.allocTemporary (rhsSym, arg2Result.symbol (), destSym->getSizeSym ());
            }
            else {
                std::stringstream ss;
                ss << "Shape of RHS doesn't match shape of LHS in assignment at " << e->location() << ".";
                Imop* jmp = new Imop (e, Imop::JUMP, (Symbol*) 0);
                Imop* err = newError (e, ConstantString::get (getContext (), ss.str ()));
                SymbolLabel* errLabel = m_st->label (err);
                SymbolSymbol* arg2ResultSymbol = static_cast<SymbolSymbol*>(arg2Result.symbol ());

                dim_iterator
                        di = dim_begin (arg2ResultSymbol),
                        dj = dim_begin (destSym),
                        de = dim_end (arg2ResultSymbol);
                for (; di != de; ++ di, ++ dj) {
                    i = new Imop (e, Imop::JNE, errLabel, *di, *dj);
                    push_imop (i);
                }

                push_imop (jmp);
                result.addToNextList (jmp);
                push_imop (err);
            }

            i = new Imop (e, iType, destSym, destSym, rhsSym, destSym->getSizeSym());
            pushImopAfter (result, i);
        }
    }

    return result;
}

CGBranchResult TreeNodeExprAssign::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprAssign (this);
}

CGBranchResult CodeGen::cgBoolExprAssign (TreeNodeExprAssign *e) {
    CGBranchResult result = codeGen (e);
    if (result.isNotOk ()) {
        return result;
    }

    Imop *i = new Imop (e, Imop::JT, 0, result.symbol ());
    pushImopAfter (result, i);
    result.addToTrueList (i);

    i = new Imop (e, Imop::JUMP, 0);
    push_imop (i);
    result.addToFalseList(i);

    return result;
}

} // namespace SecreC
