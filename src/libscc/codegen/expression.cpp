#include "treenode.h"
#include "symboltable.h"
#include "misc.h"

#include <stack>

/**
 * Code generation for expressions.
 */

namespace SecreC {
  
/******************************************************************
  TreeNodeExprIndex
******************************************************************/

ICode::Status TreeNodeExprIndex::generateCode(ICodeList &code,
                                              SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    typedef TreeNode::ChildrenListConstIterator CLCI; // children list const iterator
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV; // symbol pair vector

    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;
    bool isScalar = resultType().isScalar();


    // 0. Generate temporary for the result of the binary expression, if needed:
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    // 1. evaluate subexpressions

    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;
    Symbol* x = e->result();

    TreeNodeExpr::SubscriptInfo subInfo = codegenSubscript (e->result (), children ().at (1), code, st, log);
    if (subInfo.status != ICode::OK) {
        return subInfo.status;
    }

    SPV& spv= subInfo.spv;
    std::vector<unsigned >& slices = subInfo.slices;

    // 5. compute resulting shape
    {
        code.push_comment("Computing shape:");
        std::vector<unsigned>::const_iterator
                it = slices.begin(),
                it_end = slices.end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            int k = *it;
            Symbol* sym = result()->getDim(count);
            assert (sym != 0);
            Imop* i = new Imop(this, Imop::SUB, sym, spv[k].second, spv[k].first);
            code.push_imop(i);
            patchNextList(i, st);
            prevPatchNextList(i, st);
        }

        s = computeSize(code, st);
        if (s != ICode::OK) return s;
    }

    // r = FILL def size
    if (!isScalar) {
        Symbol* def = st.defaultConstant (resultType ().secrecDataType ());
        Imop* i = new Imop(this, Imop::FILL, result(), def, result()->getSizeSym());
        code.push_imop(i);
    }


    // 4. initialze required temporary symbols
    std::vector<Symbol* > indices;
    for (SPV::iterator it(spv.begin()); it != spv.end(); ++ it) {
        Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
        indices.push_back(sym);
    }

    Symbol* offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
    Symbol* tmp_result = st.appendTemporary(TypeNonVoid(e->resultType().secrecSecType(), e->resultType().secrecDataType(), 0));
    Symbol* tmp_result2 = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));

    // 3. initialize strides
    std::vector<Symbol* > strides [2];
    strides[0] = codegenStride (x, code, st);
    strides[1] = codegenStride (result (), code, st);

    // 7. start
    std::stack<Imop*> jump_stack;
    {
        SPV::iterator spv_it = spv.begin();
        SPV::iterator spv_it_end = spv.end();
        std::vector<Symbol* >::iterator it_it = indices.begin();
        code.push_comment("Head of indexing loop:");

        for (; spv_it != spv_it_end; ++ spv_it, ++ it_it) {
            Symbol* i_lo = spv_it->first;
            Symbol* i_hi = spv_it->second;
            Symbol* idx  = *it_it;

            // i = i_lo;
            Imop* i = new Imop(this, Imop::ASSIGN, idx, i_lo);
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
        code.push_comment("Compute offset:");
        Imop* i = new Imop(this, Imop::ASSIGN, offset, st.constantInt(0));
        code.push_imop(i);


        std::vector<Symbol* >::iterator stride_it = strides[0].begin();
        std::vector<Symbol* >::iterator it_end = indices.end();
        std::vector<Symbol* >::iterator it_it = indices.begin();
        for (; it_it != it_end; ++ stride_it, ++ it_it) {
            // tmp_result2 = s[k] * idx[k]
            i = new Imop(this, Imop::MUL, tmp_result2, *stride_it, *it_it);
            code.push_imop(i);

            // old_offset = old_offset + tmp_result2
            i = new Imop(this, Imop::ADD, offset, offset, tmp_result2);
            code.push_imop(i);
        }
    }

    // 9. load and store
    {
        code.push_comment("Load and store:");

        // tmp = x[old_offset] or r = x[old_offset] if scalar
        Imop* i = new Imop(this, Imop::LOAD, (isScalar ? result() : tmp_result), x, offset);
        code.push_imop(i);

        //code.push_imop(new Imop(this, Imop::PRINT, (Symbol*) 0, st.constantString(">")));

        // r[offset] = tmp is not scalar
        if (!isScalar) {
            i = new Imop(this, Imop::ASSIGN, offset, st.constantInt(0));
            code.push_imop(i);

            unsigned count = 0;
            for (std::vector<unsigned >::iterator it(slices.begin()); it != slices.end(); ++ it, ++ count) {
                unsigned k = *it;
                Symbol* idx = indices.at(k);

                i = new Imop(this, Imop::SUB, tmp_result2, idx, spv[k].first);
                code.push_imop(i);

                i = new Imop(this, Imop::MUL, tmp_result2, tmp_result2, strides[1].at(count));
                code.push_imop(i);

                i = new Imop(this, Imop::ADD, offset, offset, tmp_result2);
                code.push_imop(i);
            }

            i = new Imop(this, Imop::STORE, result(), offset, tmp_result);
            code.push_imop(i);
        }
    }

    // 9. loop exit
    {
        code.push_comment("Tail of indexing loop:");
        std::vector<Symbol* >::reverse_iterator
                rit = indices.rbegin(),
                rit_end = indices.rend();
        Imop* prevJump = 0;
        for (; rit != rit_end; ++ rit) {
            Symbol* idx = *rit;

            // i = i + 1
            Imop* i = new Imop(this, Imop::ADD, idx, idx, st.constantInt(1));
            code.push_imop(i);
            if (prevJump != 0) {
                prevJump->setJumpDest(st.label(i));
            }

            // GOTO L1;
            i = new Imop(this, Imop::JUMP, (Symbol*) 0);
            code.push_imop(i);
            i->setJumpDest(st.label(jump_stack.top()));
            prevJump = jump_stack.top();
            jump_stack.pop();

            // O1:
        }

        if (prevJump != 0) addToNextList(prevJump);
    }

    return ICode::OK;
}

/// \todo generate specialized code here
ICode::Status TreeNodeExprIndex::generateBoolCode(ICodeList &code,
                                                  SymbolTable &st,
                                                  CompileLog &log)
{
    assert(havePublicBoolType());

    ICode::Status s = generateCode(code, st, log);
    if (s != ICode::OK) return s;

    Imop *i = new Imop(this, Imop::JT, 0, result());
    code.push_imop(i);
    patchFirstImop(i);
    patchNextList(i, st);
    addToTrueList(i);

    i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    addToFalseList(i);

    return ICode::OK;
}

/******************************************************************
  TreeNodeExprSize
******************************************************************/


ICode::Status TreeNodeExprSize::generateCode(ICodeList &code,
                                             SymbolTable &st,
                                             CompileLog &log,
                                             Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    Symbol* size = e->resultType().isScalar() ? st.constantInt(1) : e->result()->getSizeSym();

    if (r != 0) {
        assert (r->secrecType().canAssign(resultType()));
        Imop* i = new Imop(this, Imop::ASSIGN, r, size);
        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i, st);
        setResult(r);
    }
    else {
        addToNextList(e->nextList());
        setResult(size);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprSize::generateBoolCode(ICodeList &,
                                                  SymbolTable &,
                                                  CompileLog &)
{
    assert (false && "TreeNodeExprSize::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
}

/******************************************************************
  TreeNodeExprShape
******************************************************************/

ICode::Status TreeNodeExprShape::generateCode(ICodeList &code,
                                              SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;
    Symbol* n = st.constantInt(e->resultType().secrecDimType());
    Imop* i = 0;

    if (e->resultType().isScalar())
        i = new Imop(this, Imop::ASSIGN, result(), st.constantInt(0));
    else
        i = new Imop(this, Imop::FILL, result(), st.constantInt(0), n);
    code.push_imop(i);
    prevPatchNextList(i, st);

    i = new Imop(this, Imop::ASSIGN, result()->getDim(0), n);
    code.push_imop(i);
    patchFirstImop(i);

    Symbol::dim_iterator
            dti = e->result()->dim_begin(),
            dte = e->result()->dim_end();
    for (unsigned count = 0; dti != dte; ++ dti, ++ count) {
        Imop* i = new Imop(this, Imop::STORE, result(), st.constantInt(count), *dti);
        code.push_imop(i);
        patchFirstImop(i);
    }

    computeSize(code, st);

    return ICode::OK;
}

ICode::Status TreeNodeExprShape::generateBoolCode(ICodeList &,
                                                  SymbolTable &,
                                                  CompileLog &)
{
    assert (false && "TreeNodeExprShape::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
}

/******************************************************************
  TreeNodeExprCat
******************************************************************/

ICode::Status TreeNodeExprCat::generateCode(ICodeList &code,
                                            SymbolTable &st,
                                            CompileLog &log,
                                            Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary result if needed:
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    TreeNodeExpr* e1 = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e1, code, st, log);
    if (s != ICode::OK) return s;

    TreeNodeExpr* e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = generateSubexprCode(e2, code, st, log);
    if (s != ICode::OK) return s;

    unsigned k = static_cast<TreeNodeExprInt*>(children().at(2))->value();
    unsigned n = resultType().secrecDimType();

    // Compute resulting shape and perform sanity check:
    std::stringstream ss;
    ss << "Different sized dimensions in concat at " << location() << ".";
    Imop* err = newError (this, st.constantString (ss.str ()));
    SymbolLabel* errLabel = st.label(err);
    for (unsigned it = 0; it < resultType().secrecDimType(); ++ it) {
        Symbol* s1 = e1->result()->getDim(it);
        Symbol* s2 = e2->result()->getDim(it);
        if (it == k) {
            Imop* i = new Imop(this, Imop::ADD, result()->getDim(it), s1, s2);
            code.push_imop(i);
            patchFirstImop(i);
            prevPatchNextList(i, st);
        }
        else {
            Imop* i = new Imop(this, Imop::JNE, (Symbol*) 0, s1, s2);
            code.push_imop(i);
            patchFirstImop(i);
            i->setJumpDest(errLabel);
            prevPatchNextList(i, st);

            i = new Imop(this, Imop::ASSIGN, result()->getDim(it), s1);
            code.push_imop(i);
        }
    }

    Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
    code.push_imop(jmp);
    patchFirstImop(jmp);
    prevPatchNextList(jmp, st);
    addToNextList(jmp);

    code.push_imop(err);

    // Initialize strides:
    std::vector<Symbol* > strides [3];
    strides[0] = codegenStride (e1->result(), code, st);
    strides[1] = codegenStride (e2->result(), code, st);
    strides[2] = codegenStride (    result(), code, st);

    // Symbols for running indices:
    std::vector<Symbol* > indices;
    for (unsigned it = 0; it < n; ++ it) {
        Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
        indices.push_back(sym);
    }

    // Compute size and allocate resulting array:

    s = computeSize(code, st);
    if (s != ICode::OK) return s;

    Symbol* rhs = st.defaultConstant (resultType ().secrecDataType ());
    Imop* i = new Imop(this, Imop::FILL, result(), rhs, result()->getSizeSym());
    code.push_imop(i);

    // Loop:
    std::stack<Imop*> imop_stack;
    {
        std::vector<Symbol*>::iterator
                it = indices.begin(),
                it_end = indices.end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            Symbol* idx = *it;

            // i = 0
            i = new Imop(this, Imop::ASSIGN, idx, st.constantInt(0));
            code.push_imop(i);

            // L1: IF (i >= i_hi) GOTO O1;
            Imop* l1 = new Imop(this, Imop::JGE, (Symbol*) 0, idx, result()->getDim(count));
            code.push_imop(l1);
            imop_stack.push(l1);
        }
    }

    Symbol* offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT));
    Symbol* tmp_int = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT));

    // j = 0 (right hand side index)
    i = new Imop(this, Imop::ASSIGN, offset, st.constantInt(0));
    code.push_imop(i);

    // IF (i_k >= d_k) GOTO T1;
    i = new Imop(this, Imop::JGE, (Symbol*) 0, indices[k], e1->result()->getDim(k));
    code.push_imop(i);
    addToNextList(i);

    { // compute j if i < d (for e1)
        code.push_comment("START LHS offset:");
        std::vector<Symbol* >::iterator
                it = strides[0].begin(),
                it_end = strides[0].end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            Imop* i = new Imop(this, Imop::MUL, tmp_int, *it, indices[count]);
            code.push_imop(i);

            i = new Imop(this, Imop::ADD, offset, offset, tmp_int);
            code.push_imop(i);
        }

        code.push_comment("END LHS offset:");
    }

    // t = x[j]
    Symbol* tmp_elem = st.appendTemporary(TypeNonVoid(resultType().secrecSecType(), resultType().secrecDataType()));
    i = new Imop(this, Imop::LOAD, tmp_elem, e1->result(), offset);
    code.push_imop(i);

    // jump out
    Imop* jump_out = new Imop(this, Imop::JUMP, (Symbol*) 0);
    code.push_imop(jump_out);

    { // compute j if i >= d (for e2)
        code.push_comment("START RHS offset:");
        std::vector<Symbol* >::iterator
                it = strides[1].begin(),
                it_end = strides[1].end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            if (count == k) {
                i = new Imop(this, Imop::SUB, tmp_int, indices[count], e1->result()->getDim(k));
                code.push_imop(i);
                patchNextList(i, st);

                i = new Imop(this, Imop::MUL, tmp_int, *it, tmp_int);
                code.push_imop(i);
            }
            else {
                i = new Imop(this, Imop::MUL, tmp_int, *it, indices[count]);
                code.push_imop(i);
                patchNextList(i, st);
            }

            i = new Imop(this, Imop::ADD, offset, offset, tmp_int);
            code.push_imop(i);
        }
        code.push_comment("END RHS offset.");
    }

    // t = y[j]
    i = new Imop(this, Imop::LOAD, tmp_elem, e2->result(), offset);
    code.push_imop(i);
    patchNextList(i, st);

    // out: r[i] = t
    i = new Imop(this, Imop::ASSIGN, offset, st.constantInt(0));
    code.push_imop(i);
    jump_out->setJumpDest(st.label(i));

    { // compute j if i < d (for e1)
        code.push_comment("START LHS offset:");
        std::vector<Symbol* >::iterator
                it = strides[2].begin(),
                it_end = strides[2].end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            Imop* i = new Imop(this, Imop::MUL, tmp_int, *it, indices[count]);
            code.push_imop(i);

            i = new Imop(this, Imop::ADD, offset, offset, tmp_int);
            code.push_imop(i);
        }

        code.push_comment("END LHS offset:");
    }

    i = new Imop(this, Imop::STORE, result(), offset, tmp_elem);
    code.push_imop(i);


    // i = i + 1
    //i = new Imop(this, Imop::ADD, offset, offset, st.constantUInt(1));
    //code.push_imop(i);

    // loop exit
    {
        code.push_comment("Tail of indexing loop:");
        std::vector<Symbol* >::reverse_iterator
                rit = indices.rbegin(),
                rit_end = indices.rend();
        Imop* prevJump = 0;
        for (; rit != rit_end; ++ rit) {
            Symbol* idx = *rit;

            // i = i + 1
            Imop* i = new Imop(this, Imop::ADD, idx, idx, st.constantInt(1));
            code.push_imop(i);
            if (prevJump != 0) prevJump->setJumpDest(st.label(i));

            // GOTO L1;
            i = new Imop(this, Imop::JUMP, (Symbol*) 0);
            code.push_imop(i);
            i->setJumpDest(st.label(imop_stack.top()));
            prevJump = imop_stack.top();
            imop_stack.pop();

            // O1:
        }

        if (prevJump != 0) addToNextList(prevJump);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprCat::generateBoolCode(ICodeList &,
                                                SymbolTable &,
                                                CompileLog &)
{
    assert (false && "TreeNodeExprCat::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
}

/******************************************************************
  TreeNodeExprReshape
******************************************************************/

ICode::Status TreeNodeExprReshape::generateCode(ICodeList &code,
                                                SymbolTable &st,
                                                CompileLog &log,
                                                Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary result if needed:
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    // Evaluate subexpressions:
    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    {
        TreeNode::ChildrenListConstIterator it = children().begin() + 1;
        Symbol::dim_iterator dim_it = result()->dim_begin();
        for (; it != children().end(); ++ it, ++ dim_it) {
            TreeNodeExpr* ei = static_cast<TreeNodeExpr*>(*it);
            s = generateSubexprCode(ei, code, st, log, *dim_it);
            if (s != ICode::OK) return s;
        }
    }

    // Compute new size:
    Symbol* s_new = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC,DATATYPE_INT,0));
    Imop* i = new Imop(this, Imop::ASSIGN, s_new, st.constantInt(1));
    code.push_imop(i);
    prevPatchNextList(i, st);
    patchFirstImop(i);
    Symbol::dim_iterator
            dim_it = result()->dim_begin(),
            dim_end = result()->dim_end();
    for (; dim_it != dim_end; ++ dim_it) {
        Imop* i = new Imop(this, Imop::MUL, s_new, s_new, *dim_it);
        code.push_imop(i);
    }

    Symbol* rhs = e->result();

    if (!e->resultType().isScalar()) {
        // Check that new and old sizes are equal:
        Imop* jmp = new Imop (this, Imop::JE, (Symbol*) 0, rhs->getSizeSym(), s_new);
        code.push_imop(jmp);
        addToNextList(jmp);
        std::stringstream ss;
        ss << "ERROR: Mismatching sizes in reshape at " << location();
        Imop* err = newError (this, st.constantString (ss.str ()));
        //new Imop (this, Imop::ERROR, (Symbol*) 0, (Symbol*) new std::string(ss.str()));
        code.push_imop(err);

    }
    else {
        // Convert scalar to constant array:
        rhs = st.appendTemporary(TypeNonVoid(e->resultType().secrecSecType(),
                                             e->resultType().secrecDataType(),
                                             resultType().secrecDimType()));
        Imop* i = new Imop(this, Imop::FILL, rhs, e->result(), s_new);
        code.push_imop(i);
    }

    // Copy result:
    i = new Imop(this, Imop::ASSIGN, result(), rhs, s_new);
    code.push_imop(i);
    patchNextList(i, st);

    i = new Imop(this, Imop::ASSIGN, result()->getSizeSym(), s_new);
    code.push_imop(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprReshape::generateBoolCode(ICodeList &,
                                                    SymbolTable &,
                                                    CompileLog &)
{
    assert (false && "TreeNodeExprReshape::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
}

/*******************************************************************************
  TreeNodeExprFRead
*******************************************************************************/

ICode::Status TreeNodeExprFRead::generateCode(ICodeList &code,
                                              SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate code for child expression
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the classification, if needed:
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    Imop* i = 0;

    i = new Imop(this, Imop::FREAD, (Symbol*) 0, e->result());
    code.push_imop(i);
    patchFirstImop(i);
    prevPatchNextList(i, st);

    i = new Imop(this, Imop::POP, result()->getDim(1));
    code.push_imop(i);

    i = new Imop(this, Imop::POP, result()->getDim(0));
    code.push_imop(i);

    computeSize(code, st);

    i = new Imop(this, Imop::FILL, result(), st.constantInt(0), result()->getSizeSym());
    code.push_imop(i);

    i = new Imop(this, Imop::POP, result(), result()->getSizeSym());
    code.push_imop(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprFRead::generateBoolCode(ICodeList &,
                                                    SymbolTable &,
                                                    CompileLog &)
{
    assert (false && "TreeNodeExprFRead::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
}

/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/


ICode::Status TreeNodeExprBinary::generateCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log,
                                               Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));

    /*
      If first sub-expression is public, then generate short-circuit code for
      logical && and logical ||.
    */
    if (e1->resultType().secrecSecType() == SECTYPE_PUBLIC
        && e1->resultType().isScalar()
        && e2->resultType().isScalar()
        && (type() == NODE_EXPR_BINARY_LAND || type() == NODE_EXPR_BINARY_LOR))
    {
        if (r == 0) {
            generateResultSymbol(st);
        } else {
            assert(r->secrecType().canAssign(resultType()));
            setResult(r);
        }

        // Generate code for first child expression:
        /**
          \note The short-circuit code generated here is not the exactly the
                same as in the semantics paper. Namely to optimize some more, we
                immediately assign the result of the first operator instead of
                assigning first to a temporary.
        */
        s = e1->generateCode(code, st, log, result());
        if (s != ICode::OK) return s;
        setFirstImop(e1->firstImop());

        Imop *j = new Imop(this,
                           type() == NODE_EXPR_BINARY_LAND ? Imop::JF : Imop::JT,
                           0,
                           result());
        code.push_imop(j);
        addToNextList(j);
        e1->patchNextList(j, st);

        // Generate code for second child expression:
        s = e2->generateCode(code, st, log, result());
        if (s != ICode::OK) return s;
        patchFirstImop(e2->firstImop());

        return ICode::OK;
    }

    // Generate code for first child expression:
    s = generateSubexprCode(e1, code, st, log);
    if (s != ICode::OK) return s;
    Symbol* e1result = e1->result();

    // Generate code for second child expression:
    s = generateSubexprCode(e2, code, st, log);
    if (s != ICode::OK) return s;
    Symbol* e2result = e2->result();

    // Implicitly convert scalar to array if needed:
    Imop* jmp = 0;
    if (e1->resultType().secrecDimType() > e2->resultType().secrecDimType()) {
        e2result = st.appendTemporary(static_cast<TypeNonVoid const&>(e1->resultType()));
        e2result->inheritShape(e1result);
        Imop* i = new Imop(this, Imop::FILL, e2result, e2->result(), e1result->getSizeSym());
        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i, st);
    }
    else
    if (e2->resultType().secrecDimType() > e1->resultType().secrecDimType()) {
        e1result = st.appendTemporary(static_cast<TypeNonVoid const&>(e2->resultType()));
        e1result->inheritShape(e2result);
        Imop* i = new Imop(this, Imop::FILL, e1result, e1->result(), e2result->getSizeSym());
        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i, st);
    }
    else {
        std::stringstream ss;
        ss << "Mismaching shapes in addition at " << location();
        Imop* err = newError (this, st.constantString (ss.str ()));
        SymbolLabel* errLabel = st.label(err);
        Symbol::dim_iterator
                di = e1result->dim_begin(),
                dj = e2result->dim_begin(),
                de = e1result->dim_end();
        for (; di != de; ++ di, ++ dj) {
            Imop* i = new Imop(this, Imop::JNE, (Symbol*) 0, *di, *dj);
            i->setJumpDest(errLabel);
            patchFirstImop(i);
            prevPatchNextList(i, st);
            code.push_imop(i);
        }

        jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
        code.push_imop(jmp);
        patchFirstImop(jmp);
        prevPatchNextList(jmp, st);
        addToNextList(jmp);

        code.push_imop(err);
    }

    // Generate temporary for the result of the binary expression, if needed:
    if (r == 0) {
        generateResultSymbol(st);
        result()->inheritShape(e1result);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(e1result, code, st);
    }

    // Generate code for binary expression:
    Imop *i;
    Imop::Type cType;
    switch (type()) {
        case NODE_EXPR_BINARY_ADD:  cType = Imop::ADD;  break;
        case NODE_EXPR_BINARY_SUB:  cType = Imop::SUB;  break;
        case NODE_EXPR_BINARY_MUL:  cType = Imop::MUL;  break;
        case NODE_EXPR_BINARY_DIV:  cType = Imop::DIV;  break;
        case NODE_EXPR_BINARY_MOD:  cType = Imop::MOD;  break;
        case NODE_EXPR_BINARY_EQ:   cType = Imop::EQ;   break;
        case NODE_EXPR_BINARY_GE:   cType = Imop::GE;   break;
        case NODE_EXPR_BINARY_GT:   cType = Imop::GT;   break;
        case NODE_EXPR_BINARY_LE:   cType = Imop::LE;   break;
        case NODE_EXPR_BINARY_LT:   cType = Imop::LT;   break;
        case NODE_EXPR_BINARY_NE:   cType = Imop::NE;   break;
        case NODE_EXPR_BINARY_LAND: cType = Imop::LAND; break;
        case NODE_EXPR_BINARY_LOR:  cType = Imop::LOR;  break;
        default:
            log.fatal() << "Binary " << operatorString()
                        << " not yet implemented. At " << location();
            return ICode::E_NOT_IMPLEMENTED;
    }

    i = newBinary (this, cType, result (), e1result, e2result);
    code.push_imop(i);
    patchFirstImop(i);
    patchNextList(i, st);
    prevPatchNextList(i, st);

    return ICode::OK;
}

ICode::Status TreeNodeExprBinary::generateBoolCode(ICodeList &code,
                                                   SymbolTable &st,
                                                   CompileLog &log)
{
    typedef TypeNonVoid TNV;

    assert(havePublicBoolType());

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));

    switch (type()) {
        case NODE_EXPR_BINARY_LAND: // fall through
        case NODE_EXPR_BINARY_LOR:
            assert(!e1->resultType().isVoid());
            assert(dynamic_cast<const TNV*>(&e1->resultType()) != 0);

            /*
              If first sub-expression is public, then generate short-circuit
              code for logical && and logical ||.
            */
            if (static_cast<const TNV&>(e1->resultType()).secrecSecType()
                    == SECTYPE_PUBLIC)
            {
                // Generate code for first child expression:
                ICode::Status s = e1->generateBoolCode(code, st, log);
                if (s != ICode::OK) return s;
                setFirstImop(e1->firstImop());

                // Generate code for second child expression:
                s = e2->generateBoolCode(code, st, log);
                if (s != ICode::OK) return s;
                patchFirstImop(e2->firstImop());

                // Short circuit the code:
                if (type() == NODE_EXPR_BINARY_LAND) {
                    e1->patchTrueList(
                                st.label(e2->firstImop()));
                    setFalseList(e1->falseList());

                    setTrueList(e2->trueList());
                    addToFalseList(e2->falseList());
                } else {
                    assert(type() == NODE_EXPR_BINARY_LOR);

                    e1->patchFalseList(
                                st.label(e2->firstImop()));
                    setTrueList(e1->trueList());

                    setFalseList(e2->falseList());
                    addToTrueList(e2->trueList());
                }
            } else {
                ICode::Status s = generateCode(code, st, log);
                if (s != ICode::OK) return s;

                Imop *j1, *j2;
                if (type() == NODE_EXPR_BINARY_LAND) {
                    j1 = new Imop(this, Imop::JF, 0);
                    patchFirstImop(j1);
                    addToFalseList(j1);

                    j2 = new Imop(this, Imop::JUMP, 0);
                    addToTrueList(j2);
                } else {
                    assert(type() == NODE_EXPR_BINARY_LOR);

                    j1 = new Imop(this, Imop::JT, 0);
                    patchFirstImop(j1);
                    addToTrueList(j1);

                    j2 = new Imop(this, Imop::JUMP, 0);
                    addToFalseList(j2);
                }
                j1->setArg1(result());
                code.push_imop(j1);
                code.push_imop(j2);
                patchNextList(j1, st);
            }
            break;
        case NODE_EXPR_BINARY_EQ:   // fall through
        case NODE_EXPR_BINARY_GE:   // fall through
        case NODE_EXPR_BINARY_GT:   // fall through
        case NODE_EXPR_BINARY_LE:   // fall through
        case NODE_EXPR_BINARY_LT:   // fall through
        case NODE_EXPR_BINARY_NE:   // fall through
        {
            // Generate code for first child expression:
            ICode::Status s = e1->generateCode(code, st, log);
            if (s != ICode::OK) return s;
            setFirstImop(e1->firstImop());

            // Generate code for second child expression:
            s = e2->generateCode(code, st, log);
            if (s != ICode::OK) return s;
            patchFirstImop(e2->firstImop());

            Imop::Type cType;
            switch (type()) {
                case NODE_EXPR_BINARY_EQ: cType = Imop::JE; break;
                case NODE_EXPR_BINARY_GE: cType = Imop::JGE; break;
                case NODE_EXPR_BINARY_GT: cType = Imop::JGT; break;
                case NODE_EXPR_BINARY_LE: cType = Imop::JLE; break;
                case NODE_EXPR_BINARY_LT: cType = Imop::JLT; break;
                case NODE_EXPR_BINARY_NE: cType = Imop::JNE; break;
                default:
                    assert(false); // Shouldn't happen.
            }

            Imop *tj = new Imop(this, cType, (Symbol*) 0,
                                e1->result(), e2->result());
            code.push_imop(tj);
            addToTrueList(tj);
            patchFirstImop(tj);

            if (e2->firstImop()) {
                e1->patchNextList(e2->firstImop(), st);
                e2->patchNextList(tj, st);
            }
            else {
                e1->patchNextList(tj, st);
            }

            Imop *fj = new Imop(this, Imop::JUMP, 0);
            addToFalseList(fj);
            code.push_imop(fj);
            break;
        }
        default:
            assert(false);
            break;
    }
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprProcCall
*******************************************************************************/


ICode::Status TreeNodeExprProcCall::generateCode(ICodeList &code,
                                                 SymbolTable &st,
                                                 CompileLog &log,
                                                 Symbol *r)
{
    typedef ChildrenListConstIterator CLCI;

    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // generate temporary result, if needed
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        setResult(r);
    }

    std::stack<TreeNodeExpr*> resultList;

    // Initialize arguments
    for (CLCI it(children().begin() + 1); it != children().end(); it++) {
        assert(((*it)->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(*it) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(*it);
        s = generateSubexprCode(e, code, st, log);
        if (s != ICode::OK) return s;

        assert(e->result() != 0);
        resultList.push(e);
    }

    // Add them as arguments in a backward manner:
    while (!resultList.empty()) {
        TreeNodeExpr* e = resultList.top();
        Symbol* sym = e->result();
        Imop *i = 0;
        if (!e->resultType().isScalar()) {
            i = new Imop(this, Imop::PUSH, 0, sym, sym->getSizeSym());
        }
        else {
            i = new Imop(this, Imop::PUSH, 0, sym);
        }

        code.push_imop(i);     
        patchFirstImop(i);
        prevPatchNextList(i, st);

        // push shape in reverse order
        Symbol::dim_reverese_iterator
                di = sym->dim_rbegin(),
                de = sym->dim_rend();
        for (; di != de; ++ di) {
            Imop* i = new Imop(this, Imop::PUSH, (Symbol*) 0, *di);
            code.push_imop(i);
        }

        resultList.pop();
    }

    // Do function call
    Imop *i = new Imop(this, Imop::CALL, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
    Imop *c = new Imop(this, Imop::RETCLEAN, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
    i->setCallDest(m_procedure, st.label(c));
    c->setArg2(st.label(i));
    code.push_imop(i);
    code.push_imop(c);
    patchFirstImop(i);

    // pop shape, recompute size, and pop data
    if (!resultType().isVoid()) {
        Symbol::dim_iterator
                di = result()->dim_begin(),
                de = result()->dim_end();
        for (; di != de; ++ di) {
            Imop* i = new Imop(this, Imop::POP, *di);
            code.push_imop(i);
        }

        computeSize(code, st);
        i = newNullary (this, Imop::POP, result ());
        code.push_imop(i);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprProcCall::generateBoolCode(ICodeList &code,
                                                     SymbolTable &st,
                                                     CompileLog &log)
{
    assert(havePublicBoolType());

    ICode::Status s = generateCode(code, st, log);
    if (s != ICode::OK) return s;

    Imop *i = new Imop(this, Imop::JT, 0, result());
    code.push_imop(i);
    addToTrueList(i);

    i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    addToFalseList(i);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprRVariable
*******************************************************************************/


ICode::Status TreeNodeExprRVariable::generateCode(ICodeList &code,
                                                  SymbolTable &st,
                                                  CompileLog &log,
                                                  Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));
    Symbol* src = id->getSymbol(st, log);

    if (r == 0) {
        setResult(src);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(src, code, st);
        Imop *i = newAssign (this, r, src);
        code.push_imop(i);
        patchFirstImop(i);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprRVariable::generateBoolCode(ICodeList &code,
                                                      SymbolTable &st,
                                                      CompileLog &log)
{
    assert(havePublicBoolType());

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));

    Imop *i = new Imop(this, Imop::JT, 0, id->getSymbol(st, log));
    code.push_imop(i);
    setFirstImop(i);
    addToTrueList(i);

    i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    addToFalseList(i);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprString
*******************************************************************************/

ICode::Status TreeNodeExprString::generateCode(ICodeList &code, SymbolTable &st,
                                               CompileLog &log,
                                               Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    ConstantString *sym = st.constantString(m_value);
    if (r != 0) {
        Imop *i = new Imop(this, Imop::ASSIGN, r, sym);
        setFirstImop(i);
        code.push_imop(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprString::generateBoolCode(ICodeList &, SymbolTable &,
                                                   CompileLog &)
{
    assert(false && "TreeNodeExprString::generateBoolCode called."); // This method shouldn't be called.
    return ICode::E_NOT_IMPLEMENTED;
}

/*******************************************************************************
  TreeNodeExprTernary
*******************************************************************************/


ICode::Status TreeNodeExprTernary::generateCode(ICodeList &code,
                                                SymbolTable &st,
                                                CompileLog &log,
                                                Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2));

    if (e1->havePublicBoolType()) {

        // Generate temporary for the result of the ternary expression, if needed:
        if (r == 0) {
            generateResultSymbol(st);
        } else {
            assert(r->secrecType().canAssign(resultType()));
            setResult(r);
        }

        // Generate code for boolean expression:
        s = e1->generateBoolCode(code, st, log);
        if (s != ICode::OK) return s;
        setFirstImop(e1->firstImop());

        // Generate code for first value child expression:
        s = e2->generateCode(code, st, log, result());
        if (s != ICode::OK) return s;

        // Jump out of the ternary construct:
        Imop *j = new Imop(this, Imop::JUMP, 0);
        addToNextList(j);
        code.push_imop(j);

        // Generate code for second value child expression:
        s = e3->generateCode(code, st, log, result());
        if (s != ICode::OK) return s;

        // Link boolean expression code to the rest of the code:
        e1->patchTrueList(st.label(e2->firstImop()));
        e1->patchFalseList(st.label(e3->firstImop()));

        // Handle next lists of value child expressions:
        addToNextList(e2->nextList());
        addToNextList(e3->nextList());
    }
    else {
        // Evaluate subexpressions:
        s = generateSubexprCode(e1, code, st, log);
        if (s != ICode::OK) return s;

        s = generateSubexprCode(e2, code, st, log);
        if (s != ICode::OK) return s;

        s = generateSubexprCode(e3, code, st, log);
        if (s != ICode::OK) return s;

        // Generate temporary for the result of the ternary expression, if needed:
        if (r == 0) {
            generateResultSymbol(st);
            result()->inheritShape(e1->result());
        } else {
            assert(r->secrecType().canAssign(resultType()));
            setResult(r);
            copyShapeFrom(e1->result(), code, st);
        }

        // check that shapes match
        std::stringstream ss;
        ss << "Mismatching shapes at " << location();
        Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
        Imop* err = newError (this, st.constantString (ss.str ()));
        SymbolLabel* errLabel = st.label(err);
        Symbol::dim_iterator
                di = e1->result()->dim_begin(),
                dj = e2->result()->dim_begin(),
                dk = e3->result()->dim_begin(),
                de = e1->result()->dim_end();
        for (; di != de; ++ di, ++ dj, ++ dk) {
            Imop* i = new Imop(this, Imop::JNE, (Symbol*) 0, *di, *dj);
            code.push_imop(i);
            i->setJumpDest(errLabel);
            i = new Imop(this, Imop::JNE, (Symbol*) 0, *dj, *dk);
            code.push_imop(i);
            i->setJumpDest(errLabel);
        }

        prevPatchNextList(jmp, st);
        code.push_imop(jmp);
        code.push_imop(err);

        // loop to set all values of resulting array
        Symbol* counter = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
        Symbol* b = st.appendTemporary(static_cast<TypeNonVoid const&>(e1->resultType()));
        Symbol* t = st.appendTemporary(static_cast<TypeNonVoid const&>(resultType()));

        // r = e1
        Imop* i = new Imop(this, Imop::ASSIGN, result(), e2->result(), result()->getSizeSym());
        code.push_imop(i);
        jmp->setJumpDest(st.label(i));

        // counter = 0
        i = new Imop(this, Imop::ASSIGN, counter, st.constantInt(0));
        code.push_imop(i);

        // L0: if (counter >= size) goto next;
        Imop* jge = new Imop(this, Imop::JGE, (Symbol*) 0, counter, result()->getSizeSym());
        code.push_imop(jge);
        addToNextList(jge);

        // b = e1[counter]
        i = new Imop(this, Imop::LOAD, b, e1->result(), counter);
        code.push_imop(i);

        Imop* t0 = new Imop(this, Imop::ADD, counter, counter, st.constantInt(1));
        Imop* t1 = new Imop(this, Imop::STORE, result(), counter, t);

        // if b goto T0
        i = new Imop(this, Imop::JT, (Symbol*) 0, b);
        code.push_imop(i);
        i->setJumpDest(st.label(t0));

        // t = e3[counter]
        // T1: result[counter] = t
        i = new Imop(this, Imop::LOAD, t, e3->result(), counter);
        code.push_imop(i);
        code.push_imop(t1);

        // T0: counter = counter + 1
        i = new Imop(this, Imop::ADD, counter, counter, st.constantInt(1));
        code.push_imop(t0);

        // goto L0
        i = new Imop(this, Imop::JUMP, (Symbol*) 0);
        code.push_imop(i);
        i->setJumpDest(st.label(jge));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprTernary::generateBoolCode(ICodeList &code,
                                                    SymbolTable &st,
                                                    CompileLog &log)
{
    assert(havePublicBoolType());

    // Generate code for boolean expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e1->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;

    // Generate code for first value child expression:
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;

    // Generate code for second value child expression:
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2));
    s = e3->generateCode(code, st, log);
    if (s != ICode::OK) return s;

    // Link conditional expression code to the rest of the code:
    e1->patchTrueList(st.label(e2->firstImop()));
    e1->patchFalseList(st.label(e3->firstImop()));

    addToTrueList(e2->trueList());
    addToTrueList(e3->trueList());
    addToFalseList(e2->falseList());
    addToFalseList(e3->falseList());

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprInt
*******************************************************************************/

ICode::Status TreeNodeExprInt::generateCode(ICodeList &code, SymbolTable &st,
                                            CompileLog &log, Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    ConstantInt *sym = st.constantInt(m_value);
    if (r != 0) {
        Imop *i = new Imop(this, Imop::ASSIGN, r, sym);
        setFirstImop(i);
        code.push_imop(i);
        setResult(r);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprInt::generateBoolCode(ICodeList &, SymbolTable &,
                                                CompileLog &)
{
    assert(false &&
           "ICE: TreeNodeExprInt::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
}

/*******************************************************************************
  TreeNodeExprUInt
*******************************************************************************/


ICode::Status TreeNodeExprUInt::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log,
                                             Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    ConstantUInt *sym = st.constantUInt(m_value);
    if (r != 0) {
        Imop *i = new Imop(this, Imop::ASSIGN, r, sym);
        setFirstImop(i);
        code.push_imop(i);
        setResult(r);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprUInt::generateBoolCode(ICodeList &, SymbolTable &,
                                                 CompileLog &)
{
    assert(false
           && "ICE: TreeNodeExprUInt::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
}

/*******************************************************************************
  TreeNodeExprBool
*******************************************************************************/

ICode::Status TreeNodeExprBool::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log,
                                             Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    ConstantBool *sym = st.constantBool(m_value);
    if (r != 0) {
        Imop *i = new Imop(this, Imop::ASSIGN, r, sym);
        code.push_imop(i);
        setFirstImop(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprBool::generateBoolCode(ICodeList &code,
                                                 SymbolTable &,
                                                 CompileLog &)
{
    assert(havePublicBoolType());

    Imop *i = new Imop(this, Imop::JUMP, 0);
    setFirstImop(i);
    if (m_value) {
        addToTrueList(i);
    } else {
        addToFalseList(i);
    }
    code.push_imop(i);
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprClassify
*******************************************************************************/

ICode::Status TreeNodeExprClassify::generateCode(ICodeList &code,
                                                 SymbolTable &st,
                                                 CompileLog &log,
                                                 Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate code for child expression
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the classification, if needed:
    if (r == 0) {
        generateResultSymbol(st);
        result()->inheritShape(e->result());
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(e->result(), code, st);
    }

    Imop *i = newUnary (this, Imop::CLASSIFY, result (), e->result ());
    code.push_imop(i);
    patchFirstImop(i);
    prevPatchNextList(i, st);

    return ICode::OK;
}

ICode::Status TreeNodeExprClassify::generateBoolCode(ICodeList &, SymbolTable &,
                                                     CompileLog &)
{
    assert(false &&
           "ICE: TreeNodeExprClassify::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
}

/*******************************************************************************
  TreeNodeExprDeclassify
*******************************************************************************/

ICode::Status TreeNodeExprDeclassify::generateCode(ICodeList &code,
                                                   SymbolTable &st,
                                                   CompileLog &log,
                                                   Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the declassification, if needed:
    if (r == 0) {
        generateResultSymbol(st);
        result()->inheritShape(e->result());
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(e->result(), code, st);
    }

    Imop *i = newUnary (this, Imop::DECLASSIFY, result (), e->result ());
    code.push_imop(i);
    patchFirstImop(i);
    prevPatchNextList(i, st);

    return ICode::OK;
}

ICode::Status TreeNodeExprDeclassify::generateBoolCode(ICodeList &code,
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

/*******************************************************************************
  TreeNodeExprUnary
*******************************************************************************/

ICode::Status TreeNodeExprUnary::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        generateResultSymbol(st);
        result()->inheritShape(e->result());
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(e->result(), code, st);
    }

    // Generate code for unary expression:
    Imop *i = newUnary (this,
                        type() == NODE_EXPR_UNEG ? Imop::UNEG : Imop::UMINUS,
                        result (), e->result ());
    code.push_imop(i);
    patchFirstImop(i);
    prevPatchNextList(i, st);

    return ICode::OK;
}

ICode::Status TreeNodeExprUnary::generateBoolCode(ICodeList &code,
                                                  SymbolTable &st,
                                                  CompileLog &log)
{
    assert(havePublicBoolType());
    assert(type() == NODE_EXPR_UNEG);

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;

    addToFalseList(e->trueList());
    addToTrueList(e->falseList());
    setFirstImop(e->firstImop());
    return ICode::OK;
}

/******************************************************************
  TreeNodeExprPrefix
******************************************************************/

/// \todo only works for identifiers
ICode::Status TreeNodeExprPrefix::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate code for child expression:
    TreeNode *lval = children().at(0);
    assert(lval != 0);
    assert(dynamic_cast<TreeNodeIdentifier*>(lval->children().at(0)) != 0);
    TreeNodeIdentifier *e1 = static_cast<TreeNodeIdentifier*>(lval->children().at(0));
    Symbol *destSym = st.find(e1->value());
    assert(destSym->symbolType() == Symbol::SYMBOL);
    assert(dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol *destSymSym = static_cast<SymbolSymbol*>(destSym);


    // either use ADD or SUB
    Imop::Type cType;
    switch (type ()) {
    case NODE_EXPR_PREFIX_INC: cType = Imop::ADD; break;
    case NODE_EXPR_PREFIX_DEC: cType = Imop::SUB; break;
    default: assert (false && "impossible");
    }

    // ++ x[e1,..,ek]
    if (lval->children().size() == 2) {
        assert (false&& "TODO");
        return ICode::E_NOT_IMPLEMENTED;
    }

    Symbol* one = st.constantInt (1);
    if (!resultType ().isScalar ()) {
        one = st.appendTemporary (static_cast<const TypeNonVoid&>(resultType ()));
        Imop* i = new Imop (this, Imop::FILL, one, st.constantInt (1), destSymSym->getSizeSym ());
        code.push_imop (i);
        patchFirstImop (i);
        prevPatchNextList (i, st);
    }

    // x = x + 1
    Imop* i = newBinary (this, cType, destSymSym, destSymSym, one);
    code.push_imop (i);
    patchFirstImop (i);
    prevPatchNextList (i, st);

    // r = x
    if (r != 0) {
        assert (r->secrecType().canAssign(resultType()));
        Imop* i = newAssign (this, result (), destSymSym);
        code.push_imop (i);
        copyShapeFrom (destSymSym, code, st);
    }
    else {
        setResult (destSymSym);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprPrefix::generateBoolCode(ICodeList&, SymbolTable&, CompileLog&)
{
    assert (false && "TODO");
    return ICode::E_NOT_IMPLEMENTED;
}

/******************************************************************
  TreeNodeExprPostfix
******************************************************************/

ICode::Status TreeNodeExprPostfix::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate code for child expression:
    TreeNode *lval = children().at(0);
    assert(lval != 0);
    assert(dynamic_cast<TreeNodeIdentifier*>(lval->children().at(0)) != 0);
    TreeNodeIdentifier *e1 = static_cast<TreeNodeIdentifier*>(lval->children().at(0));
    Symbol *destSym = st.find(e1->value());
    assert(destSym->symbolType() == Symbol::SYMBOL);
    assert(dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol *destSymSym = static_cast<SymbolSymbol*>(destSym);

    // either use ADD or SUB
    Imop::Type cType;
    switch (type ()) {
    case NODE_EXPR_POSTFIX_INC: cType = Imop::ADD; break;
    case NODE_EXPR_POSTFIX_DEC: cType = Imop::SUB; break;
    default: assert (false && "impossible");
    }

    // ++ x[e1,..,ek]
    if (lval->children().size() == 2) {
        assert (false && "TODO");
        return ICode::E_NOT_IMPLEMENTED;
    }

    Symbol* one = st.constantInt (1);
    if (!resultType ().isScalar ()) {
        one = st.appendTemporary (static_cast<const TypeNonVoid&>(resultType ()));
        Imop* i = new Imop (this, Imop::FILL, one, st.constantInt (1), destSymSym->getSizeSym ());
        code.push_imop (i);
        patchFirstImop (i);
        prevPatchNextList (i, st);
    }

    if (r != 0) {
        assert (r->secrecType().canAssign(resultType()));
        setResult (r);
    }
    else {
        generateResultSymbol (st);
    }

    Imop* i = 0;

    // r = x
    i = newAssign (this, result (), destSymSym);
    code.push_imop (i);
    copyShapeFrom (destSymSym, code, st);

    // x = x + 1
    i = newBinary (this, cType, destSymSym, destSymSym, one);
    code.push_imop (i);
    patchFirstImop (i);
    prevPatchNextList (i, st);

    return ICode::OK;
}

ICode::Status TreeNodeExprPostfix::generateBoolCode(ICodeList&, SymbolTable&, CompileLog&)
{
     assert (false && "TODO");
     return ICode::E_NOT_IMPLEMENTED;
 }

} // namespace SecreC
