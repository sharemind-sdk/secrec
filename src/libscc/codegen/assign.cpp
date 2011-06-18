#include "treenode.h"
#include "symboltable.h"
#include "codegen.h"

#include <stack>
#include <vector>

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
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV; // symbol pair vector

    // Type check:
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGStmtResult (s);
    }

    CGResult result;

    // Generate code for child expressions:
    TreeNode *lval = e->children ().at (0);
    assert (lval != 0);
    assert (dynamic_cast<TreeNodeIdentifier*>(lval->children ().at (0)) != 0);
    TreeNodeIdentifier *eArg1 = static_cast<TreeNodeIdentifier*>(lval->children ().at (0));
    Symbol *destSym = st.find (eArg1->value ());
    assert (destSym->symbolType () == Symbol::SYMBOL);
    assert (dynamic_cast<SymbolSymbol*> (destSym) != 0);
    SymbolSymbol *destSymSym = static_cast<SymbolSymbol*> (destSym);
    result.setResult (destSym);

    // Generate code for righthand side:
    assert (dynamic_cast<TreeNodeExpr*> (e->children().at (1)) != 0);
    TreeNodeExpr *eArg2 = static_cast<TreeNodeExpr*>(e->children ().at (1));
    const CGResult& arg2Result = codeGen (eArg2);
    append (result, arg2Result);
    if (result.isNotOk ()) {
        return result;
    }

    const TypeNonVoid& pubIntTy (TypeNonVoid (SECTYPE_PUBLIC, DATATYPE_INT, 0));

    // x[e1,...,ek] = e
    if (lval->children().size() == 2) {

        // 1. evaluate subscript
        CodeGenSubscript cgSub (*this);
        append (result, cgSub.codeGenSubscript (destSym, lval->children ().at (1)));
        if (result.isNotOk ()) {
            return result;
        }

        const SPV& spv = cgSub.spv ();
        const std::vector<unsigned>& slices = cgSub.slices ();

        // 2. check that rhs has correct dimensions
        if (!eArg2->resultType().isScalar()) {
            assert (eArg2->resultType().secrecDimType() == slices.size ());

            std::stringstream ss;
            ss << "Shape of RHS doesnt match shape of LHS in assignment at " << e->location() << ".";
            Imop* jmp = new Imop (e, Imop::JUMP, (Symbol*) 0);
            Imop* err = newError (e, st.constantString (ss.str ()));
            SymbolLabel* errLabel = st.label (err);

            for (unsigned k = 0; k < eArg2->resultType().secrecDimType(); ++ k) {
                Symbol* tsym = st.appendTemporary (pubIntTy);
                Imop* i = new Imop (e, Imop::SUB, tsym, spv[slices[k]].second, spv[slices[k]].first);
                pushImopAfter (result, i);

                i = new Imop (e, Imop::JNE, (Symbol*) 0, tsym, arg2Result.symbol ()->getDim(k));
                code.push_imop (i);
                i->setJumpDest (errLabel);
            }

            code.push_imop (jmp);
            result.addToNextList (jmp);
            code.push_imop (err);
        }

        // 3. initialize stride
        CodeGenStride stride (*this);
        append (result, stride.codeGenStride (destSym));
        if (result.isNotOk ()) {
            return result;
        }

        // 4. initialze running indices
        std::vector<Symbol* > indices;
        for (SPV::const_iterator it (spv.begin ()); it != spv.end (); ++ it) {
            Symbol* sym = st.appendTemporary (pubIntTy);
            indices.push_back (sym);
        }

        // 6. initialze symbols for offsets and temporary results
        Symbol* offset = st.appendTemporary(pubIntTy);
        Symbol* old_offset = st.appendTemporary(pubIntTy);
        Symbol* tmp_result2 = st.appendTemporary(pubIntTy);

        CodeGenLoop loop (*this);

        // offset = 0
        Imop* i = new Imop (e, Imop::ASSIGN, offset, st.constantInt(0));
        pushImopAfter (result, i);

        // 7. start
        append (result, loop.enterLoop (spv, indices));
        if (result.isNotOk ()) {
            return result;
        }

        // 8. compute offset for RHS
        {
            // old_ffset = 0
            Imop* i = new Imop (e, Imop::ASSIGN, old_offset, st.constantInt (0));
            pushImopAfter (result, i);

            unsigned count = 0;
            std::vector<Symbol* >::iterator itIt = indices.begin();
            std::vector<Symbol* >::iterator itEnd = indices.end();
            for (; itIt != itEnd; ++ count, ++ itIt) {
                // tmp_result2 = s[k] * idx[k]
                i = new Imop (e, Imop::MUL, tmp_result2, stride.at (count), *itIt);
                code.push_imop (i);

                // old_offset = old_offset + tmp_result2
                i = new Imop (e, Imop::ADD, old_offset, old_offset, tmp_result2);
                code.push_imop (i);
            }
        }

        // 9. load and store
        {
            const TypeNonVoid ty (e->resultType().secrecSecType(), e->resultType().secrecDataType(), 0);
            if (e->type () == NODE_EXPR_ASSIGN) {
                if (!eArg2->resultType ().isScalar ()) {
                    Symbol* t1 = st.appendTemporary (ty);

                    Imop* i = new Imop (e, Imop::LOAD, t1, arg2Result.symbol (), offset);
                    code.push_imop(i);

                    i = new Imop (e, Imop::STORE, destSymSym, old_offset, t1);
                    code.push_imop(i);
                }
                else {
                    Imop* i = new Imop (e, Imop::STORE, destSymSym, old_offset, arg2Result.symbol ());
                    code.push_imop(i);
                }
            }
            else {
                Symbol* t1 = st.appendTemporary (ty);
                Symbol* t2 = st.appendTemporary (ty);
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

                Imop* i = new Imop(e, Imop::LOAD, t1, destSymSym, old_offset);
                code.push_imop(i);

                if (!eArg2->resultType().isScalar()) {
                    i = new Imop(e, Imop::LOAD, t2, arg2Result.symbol (), offset);
                    code.push_imop(i);

                    i = new Imop(e, iType, t1, t1, t2);
                    code.push_imop(i);

                    i = new Imop(e, Imop::STORE, destSymSym, old_offset, t1);
                    code.push_imop(i);
                }
                else {
                    i = new Imop(e, iType, t1, t1, arg2Result.symbol ());
                    code.push_imop(i);

                    i = new Imop(e, Imop::STORE, destSymSym, old_offset, t1);
                    code.push_imop(i);
                }
            }

            // offset = offset + 1
            Imop* i = new Imop (e, Imop::ADD, offset, offset, st.constantInt (1));
            code.push_imop (i);
       }

        // 9. loop exit
        append (result, loop.exitLoop (indices));
        if (result.isNotOk ()) {
            return result;
        }

        return result;
    }

    // Generate code for regular x = e assignment
    if (e->type () == NODE_EXPR_ASSIGN) {
        if (!eArg2->resultType().isScalar()) {
            copyShapeFrom (result, arg2Result.symbol ());
            if (result.isNotOk ()) {
                return result;
            }
        }

        Imop *i = 0;
        if (e->resultType ().isScalar())
            i = new Imop (e, Imop::ASSIGN, destSym, arg2Result.symbol ());
        else {
            Imop::Type iType = eArg2->resultType ().isScalar () ? Imop::FILL : Imop::ASSIGN;
            i = new Imop (e, iType, destSym, arg2Result.symbol (), destSym->getSizeSym ());
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
        if (e->resultType().isScalar()) {
            i = new Imop (e, iType, destSymSym, destSymSym, arg2Result.symbol ());
        }
        else {
            Symbol* tmp = st.appendTemporary (static_cast<TypeNonVoid const&> (e->resultType ()));
            i = new Imop (e, Imop::FILL, tmp, arg2Result.symbol (), destSym->getSizeSym ());
            pushImopAfter (result, i);

            i = new Imop (e, iType, destSymSym, destSymSym, tmp, destSym->getSizeSym());
        }

        pushImopAfter (result, i);
    }

    return result;
}

/// \todo this and TreeNodeExprIndex have a lot of duplicate code, figure out how to fix that
ICode::Status TreeNodeExprAssign::generateCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log,
                                               Symbol *r)
{
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV; // symbol pair vector

    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate code for child expressions:
    TreeNode *lval = children().at(0);
    assert (lval != 0);
    assert(dynamic_cast<TreeNodeIdentifier*>(lval->children().at(0)) != 0);
    TreeNodeIdentifier *e1 = static_cast<TreeNodeIdentifier*>(lval->children().at(0));
    Symbol *destSym = st.find(e1->value());
    assert(destSym->symbolType() == Symbol::SYMBOL);
    assert(dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol *destSymSym = static_cast<SymbolSymbol*>(destSym);

    // Generate code for righthand side:
    assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = generateSubexprCode(e2, code, st, log);
    if (s != ICode::OK) return s;

    const TypeNonVoid& pubIntTy (TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));

    // x[e1,...,ek] = e
    if (lval->children().size() == 2) {

        // 1. evaluate subscript
        TreeNodeExpr::SubscriptInfo subInfo = codegenSubscript (destSym, lval->children ().at (1), code, st, log);
        if (subInfo.status != ICode::OK) {
            return subInfo.status;
        }

        SPV& spv = subInfo.spv;
        std::vector<unsigned>& slices = subInfo.slices;

        // 2. check that rhs has correct dimensions
        if (!e2->resultType().isScalar()) {
            assert (e2->resultType().secrecDimType() == slices.size());

            std::stringstream ss;
            ss << "Shape of RHS doesn shape of LHS in assignment at " << location() << ".";
            Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
            Imop* err = newError (this, st.constantString (ss.str ()));
            SymbolLabel* errLabel = st.label (err);

            for (unsigned k = 0; k < e2->resultType().secrecDimType(); ++ k) {
                Symbol* tsym = st.appendTemporary (pubIntTy);
                Imop* i = new Imop(this, Imop::SUB, tsym, spv[slices[k]].second, spv[slices[k]].first);
                code.push_imop (i);
                patchFirstImop (i);
                patchNextList (i, st);
                prevPatchNextList (i, st);

                i = new Imop(this, Imop::JNE, (Symbol*) 0, tsym, e2->result()->getDim(k));
                code.push_imop (i);
                i->setJumpDest (errLabel);
            }

            code.push_imop (jmp);
            addToNextList (jmp);
            code.push_imop (err);
        }

        // 3. initialize stride
        std::vector<Symbol* > stride = codegenStride (destSym, code, st);

        // 4. initialze running indices
        std::vector<Symbol* > indices;
        for (SPV::iterator it(spv.begin()); it != spv.end(); ++ it) {
            Symbol* sym = st.appendTemporary(pubIntTy);
            indices.push_back(sym);
        }

        // 6. initialze symbols for offsets and temporary results
        Symbol* offset = st.appendTemporary(pubIntTy);
        Symbol* old_offset = st.appendTemporary(pubIntTy);
        Symbol* tmp_result2 = st.appendTemporary(pubIntTy);

        // 7. start
        std::stack<Imop*> jump_stack;
        {
            SPV::iterator spv_it = spv.begin(),
                          spv_it_end = spv.end();
            std::vector<Symbol* >::iterator
                    it_it = indices.begin();

            // offset = 0
            Imop* i = new Imop(this, Imop::ASSIGN, offset, st.constantInt(0));
            code.push_imop(i);
            patchNextList(i, st);

            for (; spv_it != spv_it_end; ++ spv_it, ++ it_it) {
                Symbol* i_lo = spv_it->first;
                Symbol* i_hi = spv_it->second;
                Symbol* idx  = *it_it;

                // i = i_lo;
                i = new Imop(this, Imop::ASSIGN, idx, i_lo);
                code.push_imop(i);

                // L1: IF (i >= i_hi) GOTO O1;
                Imop* l1 = new Imop(this, Imop::JGE, (Symbol*) 0, idx, i_hi);
                code.push_imop(l1);
                jump_stack.push(l1);
            }
        }

        // 8. compute offset for RHS
        {
            // old_ffset = 0
            Imop* i = new Imop(this, Imop::ASSIGN, old_offset, st.constantInt(0));
            code.push_imop(i);

            std::vector<Symbol* >::iterator stride_it = stride.begin();
            std::vector<Symbol* >::iterator it_end = indices.end();
            std::vector<Symbol* >::iterator it_it = indices.begin();
            for (; it_it != it_end; ++ stride_it, ++ it_it) {
                // tmp_result2 = s[k] * idx[k]
                i = new Imop(this, Imop::MUL, tmp_result2, *stride_it, *it_it);
                code.push_imop(i);

                // old_offset = old_offset + tmp_result2
                i = new Imop(this, Imop::ADD, old_offset, old_offset, tmp_result2);
                code.push_imop(i);
            }
        }

        // 9. load and store
        {
            if (type() == NODE_EXPR_ASSIGN) {
                if (!e2->resultType().isScalar()) {
                    Symbol* t1 = st.appendTemporary(TypeNonVoid(resultType().secrecSecType(), resultType().secrecDataType(), 0));

                    Imop* i = new Imop(this, Imop::LOAD, t1, e2->result(), offset);
                    code.push_imop(i);

                    i = new Imop(this, Imop::STORE, destSymSym, old_offset, t1);
                    code.push_imop(i);
                }
                else {
                    Imop* i = new Imop(this, Imop::STORE, destSymSym, old_offset, e2->result());
                    code.push_imop(i);
                }
            }
            else {
                Symbol* t1 = st.appendTemporary(TypeNonVoid(resultType().secrecSecType(), resultType().secrecDataType(), 0));
                Symbol* t2 = st.appendTemporary(TypeNonVoid(resultType().secrecSecType(), resultType().secrecDataType(), 0));
                Imop::Type iType;

                switch (type()) {
                    case NODE_EXPR_ASSIGN_MUL: iType = Imop::MUL; break;
                    case NODE_EXPR_ASSIGN_DIV: iType = Imop::DIV; break;
                    case NODE_EXPR_ASSIGN_MOD: iType = Imop::MOD; break;
                    case NODE_EXPR_ASSIGN_ADD: iType = Imop::ADD; break;
                    case NODE_EXPR_ASSIGN_SUB: iType = Imop::SUB; break;
                    default:
                        assert(false); // shouldn't happen
                        return ICode::E_OTHER;
                }

                Imop* i = new Imop(this, Imop::LOAD, t1, destSymSym, old_offset);
                code.push_imop(i);

                if (!e2->resultType().isScalar()) {
                    i = new Imop(this, Imop::LOAD, t2, e2->result(), offset);
                    code.push_imop(i);

                    i = new Imop(this, iType, t1, t1, t2);
                    code.push_imop(i);

                    i = new Imop(this, Imop::STORE, destSymSym, old_offset, t1);
                    code.push_imop(i);
                }
                else {
                    i = new Imop(this, iType, t1, t1, e2->result());
                    code.push_imop(i);

                    i = new Imop(this, Imop::STORE, destSymSym, old_offset, t1);
                    code.push_imop(i);
                }
            }

            // offset = offset + 1
            Imop* i = new Imop(this, Imop::ADD, offset, offset, st.constantInt(1));
            code.push_imop(i);
       }

       // 9. loop exit
       {
            code.push_comment("Tail of indexing loop:");
            std::vector<Symbol* >::reverse_iterator
                    rit = indices.rbegin(),
                    rit_end = indices.rend();
            Imop* prev_jump = 0;
            for (; rit != rit_end; ++ rit) {
                Symbol* idx = *rit;

                // i = i + 1
                Imop* i = new Imop(this, Imop::ADD, idx, idx, st.constantInt(1));
                code.push_imop(i);
                if (prev_jump != 0) {
                    prev_jump->setJumpDest(st.label(i));
                }

                // GOTO L1;
                i = new Imop(this, Imop::JUMP, (Symbol*) 0);
                code.push_imop(i);
                i->setJumpDest(st.label(jump_stack.top()));
                prev_jump = jump_stack.top();
                jump_stack.pop();

                // O1:
            }

            if (prev_jump != 0) addToNextList(prev_jump);
        }

        if (r != 0) {
            setResult (r);
            assert (false && "TODO"); /// \todo copy from destSymSym to r
        }
        else {
            setResult (destSymSym);
        }

        return ICode::OK;
    }

    // Generate code for regular x = e assignment
    if (type() == NODE_EXPR_ASSIGN) {

        if (r != 0)
            setResult(r);
         else
            setResult(destSymSym);

        if (!e2->resultType().isScalar()) {
            copyShapeFrom(e2->result(), code, st);
        }

        Imop *i = 0;
        if (resultType().isScalar())
            i = new Imop(this, Imop::ASSIGN, result(), e2->result());
        else {
            Imop::Type iType = e2->resultType().isScalar() ? Imop::FILL : Imop::ASSIGN;
            i = new Imop(this, iType, result(), e2->result(), result()->getSizeSym());
        }
        code.push_imop(i);
        patchFirstImop(i);
        e2->patchNextList(i, st);
        setNextList(e2->nextList());
    } else {
        // Arithmetic assignments

        Imop::Type iType;
        switch (type()) {
            case NODE_EXPR_ASSIGN_MUL: iType = Imop::MUL; break;
            case NODE_EXPR_ASSIGN_DIV: iType = Imop::DIV; break;
            case NODE_EXPR_ASSIGN_MOD: iType = Imop::MOD; break;
            case NODE_EXPR_ASSIGN_ADD: iType = Imop::ADD; break;
            case NODE_EXPR_ASSIGN_SUB: iType = Imop::SUB; break;
            default:
                assert(false); // shouldn't happen
        }

        Imop *i = 0;
        if (resultType().isScalar()) {
            i = new Imop(this, iType, destSymSym, destSymSym, e2->result());
        }
        else {
            Symbol* tmp = st.appendTemporary(static_cast<TypeNonVoid const&>(resultType()));
            i = new Imop(this, Imop::FILL, tmp, e2->result(), destSym->getSizeSym());
            code.push_imop(i);
            patchFirstImop(i);
            e2->patchNextList(i, st);

            i = new Imop(this, iType, destSymSym, destSymSym, tmp, destSym->getSizeSym());
        }

        code.push_imop(i);
        patchFirstImop(i);
        e2->patchNextList(i, st);

        if (r != 0) {
            i = 0;
            if (resultType().isScalar()) {
                i = new Imop(this, Imop::ASSIGN, r, destSymSym);
            }
            else {
                i = new Imop(this, Imop::ASSIGN, r, destSymSym, destSym->getSizeSym());
            }
            code.push_imop(i);
            setResult(r);
        } else {
            setResult(destSymSym);
        }
    }

    return ICode::OK;
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
    code.push_imop (i);
    result.addToFalseList(i);

    return result;
}

ICode::Status TreeNodeExprAssign::generateBoolCode(ICodeList &code,
                                                   SymbolTable &st,
                                                   CompileLog &log)
{
    assert(havePublicBoolType());

    ICode::Status s = generateCode(code, st, log);
    if (s != ICode::OK) return s;

    Imop *i = new Imop(this, Imop::JT, 0, result());
    code.push_imop(i);
    patchFirstImop(i);
    addToTrueList(i);

    i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    addToFalseList(i);

    return ICode::OK;
}

}
