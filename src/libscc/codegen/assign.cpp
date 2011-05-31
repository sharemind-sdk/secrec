#include "treenode.h"
#include "symboltable.h"

#include <stack>
#include <vector>

/**
 * Code generation for assignment expression
 */

namespace SecreC {

/*******************************************************************************
  TreeNodeExprAssign
*******************************************************************************/

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

    // x[e1,...,ek] = e
    if (lval->children().size() == 2) {
        SPV spv;
        std::vector<unsigned > slices;

        { // evaluate indices
            TreeNode::ChildrenListConstIterator
                    it = lval->children().at(1)->children().begin(),
                    it_end = lval->children().at(1)->children().end();
            for (unsigned count = 0; it != it_end; ++ it, ++ count) {
                TreeNode* t = *it;
                Symbol* r_lo = 0;
                Symbol* r_hi = 0;

                // lower bound
                TreeNode* t_lo = t->children().at(0);
                if (t_lo->type() == NODE_EXPR_NONE) {
                    r_lo = st.constantInt(0);
                }
                else {
                    TreeNodeExpr* e_lo = static_cast<TreeNodeExpr*>(t_lo);
                    s = generateSubexprCode(e_lo, code, st, log);
                    if (s != ICode::OK) return s;
                    r_lo = e_lo->result();
                }

                // upper bound
                if (t->type() == NODE_INDEX_SLICE) {
                    TreeNode* t_hi = t->children().at(1);
                    if (t_hi->type() == NODE_EXPR_NONE) {
                        r_hi = destSym->getDim(count);
                    }
                    else {
                        TreeNodeExpr* e_hi = static_cast<TreeNodeExpr*>(t_hi);
                        s = generateSubexprCode(e_hi, code, st, log);
                        if (s != ICode::OK) return s;
                        r_hi = e_hi->result();
                    }

                    slices.push_back(count);
                }
                else {
                    // if there is no upper bound then make one up
                    r_hi = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
                    Imop* i = new Imop(this, Imop::ADD, r_hi, r_lo, st.constantInt(1));
                    code.push_imop(i);
                    patchFirstImop(i);
                    prevPatchNextList(i, st);
                }

                spv.push_back(std::make_pair(r_lo, r_hi));
            }
        }

        // 2. check that indices are legal
        {
            std::stringstream ss;
            ss << "Index out of bounds at " << location() << ".";
            Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
            Imop* err = newError (this, st.constantString (ss.str ()));
            SymbolLabel* errLabel = st.label (err);

            // check lower indices
            Symbol::dim_iterator di = destSym->dim_begin();
            for (SPV::const_iterator it(spv.begin()); it != spv.end(); ++ it, ++ di) {
                code.push_comment("checking s_lo");
                Symbol* s_lo = it->first;
                Imop* i = new Imop(this, Imop::JGE, (Symbol*) 0, s_lo, *di);
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i, st);
                i->setJumpDest(errLabel);
            }

            // check upper indices
            for (std::vector<unsigned>::const_iterator it(slices.begin()); it != slices.end(); ++ it) {
                code.push_comment("checking s_hi");
                unsigned k = *it;
                Symbol* s_hi = spv[k].second;
                Symbol* d = destSym->getDim(k);
                Imop* i = new Imop(this, Imop::JGT, (Symbol*) 0, s_hi, d);
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i, st);
                i->setJumpDest(errLabel);

                Symbol* s_lo = spv[k].first;
                i = new Imop(this, Imop::JGE, (Symbol*) 0, s_lo, s_hi);
                code.push_imop(i);
                i->setJumpDest(errLabel);
            }

            // check that rhs has correct dimensions
            if (!e2->resultType().isScalar()) {
                assert (e2->resultType().secrecDimType() == slices.size());
                for (unsigned k = 0; k < e2->resultType().secrecDimType(); ++ k) {
                    Symbol* tsym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
                    Imop* i = new Imop(this, Imop::SUB, tsym, spv[slices[k]].second, spv[slices[k]].first);
                    code.push_imop(i);
                    patchFirstImop(i);
                    prevPatchNextList(i, st);

                    i = new Imop(this, Imop::JNE, (Symbol*) 0, tsym, e2->result()->getDim(k));
                    code.push_imop(i);
                    i->setJumpDest(errLabel);
                }
            }

            code.push_imop(jmp);
            patchFirstImop(jmp);
            prevPatchNextList(jmp, st);
            addToNextList(jmp); /// \todo not sure about that...

            code.push_imop(err);
        }

        // 3. initialize stride: s[0] = 1; ...; s[n+1] = s[n] * d[n];
        // - Note that after this point next lists and first imop are patched!
        // - size of 'x' is stored as last element of the stride
        std::vector<Symbol* > stride;
        {
            code.push_comment("Computing stride:");
            Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
            Imop* i = new Imop(this, Imop::ASSIGN, sym, st.constantInt(1));
            stride.push_back(sym);
            code.push_imop(i);
            patchFirstImop(i);
            patchNextList(i, st);
            prevPatchNextList(i, st);

            Symbol* prev = sym;
            for (Symbol::dim_iterator it(destSym->dim_begin()); it != destSym->dim_end(); ++ it) {
                sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
                i = new Imop(this, Imop::MUL, sym, prev, *it);
                stride.push_back(sym);
                code.push_imop(i);
                prev = sym;
            }
        }

        // 4. initialze running indices
        std::vector<Symbol* > indices;
        for (SPV::iterator it(spv.begin()); it != spv.end(); ++ it) {
            Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
            indices.push_back(sym);
        }

        // 6. initialze symbols for offsets and temporary results
        Symbol* offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
        Symbol* old_offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
        Symbol* tmp_result2 = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));

        // 7. start
        std::stack<Imop*> jump_stack;
        {
            SPV::iterator
                    spv_it = spv.begin(),
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
