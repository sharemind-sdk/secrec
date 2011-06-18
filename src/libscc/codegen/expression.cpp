#include "treenode.h"
#include "symboltable.h"
#include "misc.h"

#include "codegen.h"

#include <stack>

/**
 * Code generation for expressions.
 */

namespace SecreC {
  
/******************************************************************
  TreeNodeExprIndex
******************************************************************/

CGResult TreeNodeExprIndex::codeGenWith (CodeGen &cg) {
    return cg.cgExprIndex (this);
}

CGResult CodeGen::cgExprIndex (TreeNodeExprIndex *e) {
    typedef TreeNode::ChildrenListConstIterator CLCI; // children list const iterator
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV; // symbol pair vector

    CGResult result;

    // Type check:
    ICode::Status status = e->calculateResultType(st, log);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    bool isScalar = e->resultType ().isScalar ();

    Symbol* resSym = generateResultSymbol (result, e);

    // 1. evaluate subexpressions
    TreeNodeExpr* eArg1 = static_cast<TreeNodeExpr*>(e->children().at(0));
    const CGResult& argResult (codeGen (eArg1));
    append (result, argResult);
    if (result.isNotOk ()) {
        return result;
    }

    Symbol* x = argResult.symbol ();

    CodeGenSubscript subscript (*this);
    append (result, subscript.codeGenSubscript (x, e->children ().at (1)));
    if (result.isNotOk ()) {
        return result;
    }

    const SPV& spv = subscript.spv ();
    const std::vector<unsigned >& slices = subscript.slices ();

    // 5. compute resulting shape
    {
        code.push_comment ("Computing shape:");
        std::vector<unsigned>::const_iterator
                it = slices.begin(),
                itEnd = slices.end();
        for (unsigned count = 0; it != itEnd; ++ it, ++ count) {
            int k = *it;
            Symbol* sym = resSym->getDim (count);
            Imop* i = new Imop (e, Imop::SUB, sym, spv[k].second, spv[k].first);
            pushImopAfter (result, i);
        }

        codeGenSize (result);
    }

    // r = FILL def size
    if (!isScalar) {
        Symbol* def = st.defaultConstant (e->resultType ().secrecDataType ());
        Imop* i = new Imop (e, Imop::FILL, resSym, def, resSym->getSizeSym ());
        pushImopAfter (result, i);
    }

    // 4. initialze required temporary symbols
    std::vector<Symbol* > indices;
    for (SPV::const_iterator it(spv.begin()); it != spv.end(); ++ it) {
        Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
        indices.push_back(sym);
    }

    Symbol* offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
    Symbol* tmp_result = st.appendTemporary(TypeNonVoid(e->resultType().secrecSecType(), e->resultType().secrecDataType(), 0));
    Symbol* tmp_result2 = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));

    // 3. initialize strides
    std::vector<CodeGenStride > strides (2, CodeGenStride (*this));
    append (result, strides[0].codeGenStride (x));
    append (result, strides[1].codeGenStride (resSym));
    if (result.isNotOk ()) {
        return result;
    }

    CodeGenLoop loop (*this);

    append (result, loop.enterLoop (spv, indices));
    if (result.isNotOk ()) {
        return result;
    }

    // 8. compute offset for RHS
    {
        // old_ffset = 0
        code.push_comment("Compute offset:");
        Imop* i = new Imop (e, Imop::ASSIGN, offset, st.constantInt(0));
        code.push_imop(i);

        std::vector<Symbol* >::iterator itIt = indices.begin();
        std::vector<Symbol* >::iterator itEnd = indices.end();
        for (unsigned k = 0; itIt != itEnd; ++ k, ++ itIt) {
            // tmp_result2 = s[k] * idx[k]
            i = new Imop (e, Imop::MUL, tmp_result2, strides[0].at (k), *itIt);
            code.push_imop(i);

            // old_offset = old_offset + tmp_result2
            i = new Imop (e, Imop::ADD, offset, offset, tmp_result2);
            code.push_imop(i);
        }
    }

    // 9. load and store
    {
        code.push_comment("Load and store:");

        // tmp = x[old_offset] or r = x[old_offset] if scalar
        Imop* i = new Imop (e, Imop::LOAD, (isScalar ? resSym : tmp_result), x, offset);
        code.push_imop (i);

        // r[offset] = tmp is not scalar
        if (!isScalar) {
            i = new Imop (e, Imop::ASSIGN, offset, st.constantInt(0));
            code.push_imop (i);

            unsigned count = 0;
            for (std::vector<unsigned >::const_iterator it (slices.begin()); it != slices.end(); ++ it, ++ count) {
                unsigned k = *it;
                Symbol* idx = indices.at (k);

                i = new Imop (e, Imop::SUB, tmp_result2, idx, spv[k].first);
                code.push_imop (i);

                i = new Imop (e, Imop::MUL, tmp_result2, tmp_result2, strides[1].at (count));
                code.push_imop (i);

                i = new Imop (e, Imop::ADD, offset, offset, tmp_result2);
                code.push_imop (i);
            }

            i = new Imop (e, Imop::STORE, resSym, offset, tmp_result);
            code.push_imop(i);
        }
    }

    append (result, loop.exitLoop (indices));
    return result;
}

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

CGBranchResult TreeNodeExprIndex::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprIndex (this);
}

CGBranchResult CodeGen::cgBoolExprIndex (TreeNodeExprIndex *e) {
    CGBranchResult result (codeGen (e));
    if (!result.isOk ()) {
        return result;
    }

    Symbol* resultSym = result.symbol ();
    Imop *i = new Imop (m_node, Imop::JT, 0, resultSym);
    pushImopAfter (result, i);
    result.addToTrueList (i);

    i = new Imop (m_node, Imop::JUMP, 0);
    code.push_imop (i);
    result.addToFalseList (i);

    return result;
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

CGResult TreeNodeExprSize::codeGenWith (CodeGen &cg) {
    return cg.cgExprSize (this);
}

CGResult CodeGen::cgExprSize (TreeNodeExprSize* e) {
    ICode::Status s = e->calculateResultType (st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    TreeNodeExpr* eArg = static_cast<TreeNodeExpr*>(e->children().at(0));
    CGResult result (codeGen (eArg));
    if (!result.isOk ()) {
        return result;
    }

    Symbol* size = st.constantInt (1);
    if (!eArg->resultType().isScalar()) {
        size = result.symbol()->getSizeSym();
    }

    result.setResult (size);
    return result;
}

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

CGResult TreeNodeExprShape::codeGenWith (CodeGen &cg) {
    return cg.cgExprShape (this);
}

CGResult CodeGen::cgExprShape (TreeNodeExprShape *e) {
    // Type check:
    ICode::Status s = e->calculateResultType (st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    generateResultSymbol (result, e);

    TreeNodeExpr* eArg = static_cast<TreeNodeExpr*>(e->children().at(0));
    const CGResult& argResult (codeGen (eArg));
    append (result, argResult);
    if (result.isNotOk ()) {
        return result;
    }

    Symbol* resSym = result.symbol ();
    Symbol* n = st.constantInt (eArg->resultType().secrecDimType());
    Imop* i = 0;

    if (eArg->resultType().isScalar())
        i = new Imop (m_node, Imop::ASSIGN, resSym, st.constantInt (0));
    else
        i = new Imop (m_node, Imop::FILL, resSym, st.constantInt (0), n);
    pushImopAfter (result, i);

    i = new Imop (m_node, Imop::ASSIGN, resSym->getDim (0), n);
    code.push_imop(i);

    Symbol::dim_iterator
            dti = argResult.symbol()->dim_begin(),
            dte = argResult.symbol()->dim_end();
    for (unsigned count = 0; dti != dte; ++ dti, ++ count) {
        Imop* i = new Imop(m_node, Imop::STORE, resSym, st.constantInt(count), *dti);
        code.push_imop(i);
    }

    codeGenSize (result);
    return result;
}

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

CGResult TreeNodeExprCat::codeGenWith (CodeGen &cg) {
    return cg.cgExprCat (this);
}

CGResult CodeGen::cgExprCat (TreeNodeExprCat *e) {

    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    generateResultSymbol (result, e);

    TreeNodeExpr* eArg1 = static_cast<TreeNodeExpr*>(e->children().at(0));
    const CGResult& arg1Result (codeGen (eArg1));
    append (result, arg1Result);
    if (result.isNotOk ()) {
        return result;
    }

    TreeNodeExpr* eArg2 = static_cast<TreeNodeExpr*>(e->children().at(1));
    const CGResult& arg2Result (codeGen (eArg2));
    append (result, arg2Result);
    if (result.isNotOk ()) {
        return result;
    }

    unsigned k = static_cast<TreeNodeExprInt*>(e->children().at(2))->value();
    unsigned n = e->resultType().secrecDimType();
    Symbol* resSym = result.symbol ();

    // Compute resulting shape and perform sanity check:
    std::stringstream ss;
    ss << "Different sized dimensions in concat at " << e->location() << ".";
    Imop* err = newError (m_node, st.constantString (ss.str ()));
    SymbolLabel* errLabel = st.label(err);
    for (unsigned it = 0; it < e->resultType().secrecDimType(); ++ it) {
        Symbol* s1 = arg1Result.symbol ()->getDim(it);
        Symbol* s2 = arg2Result.symbol ()->getDim(it);
        if (it == k) {
            Imop* i = new Imop (m_node, Imop::ADD, resSym->getDim(it), s1, s2);
            pushImopAfter (result, i);
        }
        else {
            Imop* i = new Imop (m_node, Imop::JNE, (Symbol*) 0, s1, s2);
            pushImopAfter (result, i);
            i->setJumpDest(errLabel);

            i = new Imop (m_node, Imop::ASSIGN, resSym->getDim(it), s1);
            code.push_imop(i);
        }
    }

    Imop* jmp = new Imop (m_node, Imop::JUMP, (Symbol*) 0);
    pushImopAfter (result, jmp);
    result.addToNextList (jmp);

    code.push_imop(err);

    // Initialize strides:
    std::vector<CodeGenStride > strides (3, CodeGenStride (*this));
    append (result, strides[0].codeGenStride (arg1Result.symbol ()));
    append (result, strides[1].codeGenStride (arg2Result.symbol ()));
    append (result, strides[2].codeGenStride (resSym));
    if (result.isNotOk ()) {
        return result;
    }

    // Symbols for running indices:
    std::vector<Symbol* > indices;
    for (unsigned it = 0; it < n; ++ it) {
        Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
        indices.push_back(sym);
    }

    // Compute size and allocate resulting array:
    codeGenSize (result);
    if (result.isNotOk ()) {
        return result;
    }

    Symbol* rhs = st.defaultConstant (e->resultType ().secrecDataType ());
    Imop* i = new Imop (m_node, Imop::FILL, resSym, rhs, resSym->getSizeSym());
    code.push_imop(i);

    CodeGenLoop loop (*this);
    append (result, loop.enterLoop (resSym, indices));

    Symbol* offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT));
    Symbol* tmpInt = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT));

    // j = 0 (right hand side index)
    i = new Imop(m_node, Imop::ASSIGN, offset, st.constantInt(0));
    code.push_imop(i);

    // IF (i_k >= d_k) GOTO T1;
    i = new Imop(m_node, Imop::JGE, (Symbol*) 0, indices[k], arg1Result.symbol ()->getDim(k));
    code.push_imop(i);
    result.addToNextList (i);

    // compute j if i < d (for e1)
    for (unsigned count = 0; count < strides[0].size (); ++ count) {
        Imop* i = new Imop(m_node, Imop::MUL, tmpInt, strides[0].at (count), indices[count]);
        code.push_imop(i);

        i = new Imop(m_node, Imop::ADD, offset, offset, tmpInt);
        code.push_imop(i);
    }

    // t = x[j]
    const TypeNonVoid elemType (e->resultType().secrecSecType(), e->resultType().secrecDataType());
    Symbol* tmp_elem = st.appendTemporary(elemType);
    i = new Imop (m_node, Imop::LOAD, tmp_elem, arg1Result.symbol (), offset);
    code.push_imop(i);

    // jump out
    Imop* jump_out = new Imop(m_node, Imop::JUMP, (Symbol*) 0);
    code.push_imop (jump_out);

    // compute j if i >= d (for e2)
    for (unsigned count = 0; count < strides[1].size (); ++ count) {
        if (count == k) {
            i = new Imop (m_node, Imop::SUB, tmpInt, indices[count], arg1Result.symbol ()->getDim(k));
            pushImopAfter (result, i);

            i = new Imop (m_node, Imop::MUL, tmpInt, strides[1].at (count), tmpInt);
            code.push_imop(i);
        }
        else {
            i = new Imop (m_node, Imop::MUL, tmpInt, strides[1].at (count), indices[count]);
            pushImopAfter (result, i);
        }

        i = new Imop (m_node, Imop::ADD, offset, offset, tmpInt);
        code.push_imop(i);
    }

    // t = y[j]
    i = new Imop (m_node, Imop::LOAD, tmp_elem, arg2Result.symbol (), offset);
    pushImopAfter (result, i);

    // out: r[i] = t
    i = new Imop (m_node, Imop::ASSIGN, offset, st.constantInt(0));
    code.push_imop(i);
    jump_out->setJumpDest (st.label (i));

    // compute j if i < d (for e1)
    for (unsigned count = 0; count != strides[2].size (); ++ count) {
        Imop* i = new Imop (m_node, Imop::MUL, tmpInt, strides[2].at (count), indices[count]);
        code.push_imop (i);

        i = new Imop (m_node, Imop::ADD, offset, offset, tmpInt);
        code.push_imop (i);
    }

    i = new Imop (m_node, Imop::STORE, resSym, offset, tmp_elem);
    code.push_imop(i);

    append (result, loop.exitLoop (indices));

    return result;
}

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

CGResult TreeNodeExprReshape::codeGenWith (CodeGen &cg) {
    return cg.cgExprReshape (this);
}

CGResult CodeGen::cgExprReshape (TreeNodeExprReshape *e) {
    // Type check:
    ICode::Status s = e->calculateResultType (st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    // Evaluate subexpression:
    TreeNodeExpr* eArg = static_cast<TreeNodeExpr*>(e->children ().at (0));
    CGResult result (codeGen (eArg));
    if (result.isNotOk ()) {
        return result;
    }

    Symbol* rhs = result.symbol ();
    Symbol* resSym = generateResultSymbol (result, e);

    { // Eval subexpressions and copy dimensionalities:
        TreeNode::ChildrenListConstIterator it = e->children ().begin () + 1;
        Symbol::dim_iterator dimIt = resSym->dim_begin ();
        for (; it != e->children().end(); ++ it, ++ dimIt) {
            TreeNodeExpr* eArgi = static_cast<TreeNodeExpr*>(*it);
            const CGResult& argResult (codeGen (eArgi));
            append (result, argResult);
            if (result.isNotOk ()) {
                return result;
            }

            Imop* i = new Imop (e, Imop::ASSIGN, *dimIt, argResult.symbol ());
            pushImopAfter (result, i);
        }
    }

    // Compute new size:
    codeGenSize (result);

    if (!eArg->resultType().isScalar()) {
        // Check that new and old sizes are equal:
        Imop* jmp = new Imop (e, Imop::JE, (Symbol*) 0, rhs->getSizeSym (), resSym->getSizeSym ());
        pushImopAfter (result, jmp);
        result.addToNextList (jmp);
        std::stringstream ss;
        ss << "ERROR: Mismatching sizes in reshape at " << e->location ();
        Imop* err = newError (e, st.constantString (ss.str ()));
        code.push_imop (err);
    }
    else {
        // Convert scalar to constant array:
        Symbol* tmp = rhs;
        rhs = st.appendTemporary (TypeNonVoid(eArg->resultType ().secrecSecType (),
                                              eArg->resultType ().secrecDataType (),
                                              e->resultType ().secrecDimType ()));
        Imop* i = new Imop (e, Imop::FILL, rhs, tmp, resSym->getSizeSym ());
        code.push_imop (i);
    }

    // Copy result:
    Imop* i = new Imop (e, Imop::ASSIGN, resSym, rhs, resSym->getSizeSym ());
    pushImopAfter (result, i);
    return result;
}

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

CGResult TreeNodeExprFRead::codeGenWith (CodeGen &cg) {
    return cg.cgExprFRead (this);
}

CGResult CodeGen::cgExprFRead (TreeNodeExprFRead *e) {
    // Type check:
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    // Generate code for child expression:
    TreeNodeExpr *eArg = static_cast<TreeNodeExpr*>(e->children ().at (0));
    CGResult result (codeGen (eArg));
    if (result.isNotOk ()) {
        return result;
    }

    Symbol* argSym = result.symbol ();
    Symbol* resSym = generateResultSymbol (result, e);

    Imop* i = 0;

    i = new Imop (e, Imop::FREAD, (Symbol*) 0, argSym);
    pushImopAfter (result, i);

    i = new Imop (e, Imop::POP, resSym->getDim (1));
    code.push_imop(i);

    i = new Imop (e, Imop::POP, resSym->getDim (0));
    code.push_imop(i);

    codeGenSize (result);

    i = new Imop (e, Imop::FILL, resSym, st.constantInt (0), resSym->getSizeSym ());
    code.push_imop(i);

    i = new Imop (e, Imop::POP, resSym, resSym->getSizeSym ());
    code.push_imop(i);

    return result;
}

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

CGResult TreeNodeExprBinary::codeGenWith (CodeGen &cg) {
    return cg.cgExprBinary (this);
}

CGResult CodeGen::cgExprBinary (TreeNodeExprBinary *e) {
    // Type check:
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    TreeNodeExpr* eArg1 = static_cast<TreeNodeExpr*>(e->children().at(0));
    TreeNodeExpr* eArg2 = static_cast<TreeNodeExpr*>(e->children().at(1));

    /*
      If first sub-expression is public, then generate short-circuit code for
      logical && and logical ||.
    */
    if (eArg1->resultType ().secrecSecType () == SECTYPE_PUBLIC
        && eArg1->resultType ().isScalar ()
        && eArg2->resultType ().isScalar ()
        && (e->type () == NODE_EXPR_BINARY_LAND || e->type () == NODE_EXPR_BINARY_LOR))
    {
        // Generate code for first child expression:
        CGResult result (codeGen (eArg1));
        Symbol* oldSym = result.symbol ();
        Symbol* resSym = generateResultSymbol (result, e);
        if (result.isNotOk ()) {
            return result;
        }

        Imop* i = newAssign (e, resSym, oldSym);
        pushImopAfter (result, i);

        const Imop::Type iType = (e->type () == NODE_EXPR_BINARY_LAND) ? Imop::JF : Imop::JT;
        Imop *j = new Imop (e, iType, 0, resSym);
        code.push_imop (j);
        result.addToNextList (j);

        // Generate code for second child expression:
        CGResult arg2Result (codeGen (eArg2));
        Symbol* arg2Sym = arg2Result.symbol ();
        if (arg2Result.isNotOk ()) {
            return result;
        }

        i = new Imop (e, Imop::ASSIGN, resSym, arg2Sym);
        pushImopAfter (arg2Result, i);

        return result;
    }

    CGResult result;

    // Generate code for first child expression:
    const CGResult& arg1Result (codeGen (eArg1));
    append (result, arg1Result);
    if (result.isNotOk ()) {
        return result;
    }

    // Generate code for first child expression:
    const CGResult& arg2Result (codeGen (eArg2));
    append (result, arg2Result);
    if (result.isNotOk ()) {
        return result;
    }

    Symbol* e1result = arg1Result.symbol ();
    Symbol* e2result = arg2Result.symbol ();

    // Implicitly convert scalar to array if needed:
    Imop* jmp = 0;
    if (eArg1->resultType ().secrecDimType () > eArg2->resultType ().secrecDimType ()) {
        e2result = st.appendTemporary (static_cast<TypeNonVoid const&> (eArg1->resultType ()));
        e2result->inheritShape (e1result);
        Imop* i = new Imop (e, Imop::FILL, e2result, arg2Result.symbol (), e1result->getSizeSym ());
        pushImopAfter (result, i);
    }
    else
    if (eArg2->resultType ().secrecDimType () > eArg1->resultType ().secrecDimType ()) {
        e1result = st.appendTemporary (static_cast<TypeNonVoid const&> (eArg2->resultType ()));
        e1result->inheritShape (e2result);
        Imop* i = new Imop (e, Imop::FILL, e1result,arg1Result.symbol (), e2result->getSizeSym ());
        pushImopAfter (result, i);
    }
    else {
        std::stringstream ss;
        ss << "Mismaching shapes in addition at " << e->location();
        Imop* err = newError (e, st.constantString (ss.str ()));
        SymbolLabel* errLabel = st.label (err);
        Symbol::dim_iterator
                di = e1result->dim_begin (),
                dj = e2result->dim_begin (),
                de = e1result->dim_end ();
        for (; di != de; ++ di, ++ dj) {
            Imop* i = new Imop (e, Imop::JNE, (Symbol*) 0, *di, *dj);
            i->setJumpDest (errLabel);
            pushImopAfter (result, i);
        }

        jmp = new Imop(e, Imop::JUMP, (Symbol*) 0);
        pushImopAfter (result, jmp);
        result.addToNextList (jmp);
        code.push_imop (err);
    }

    Symbol* resSym = generateResultSymbol (result, e);
    resSym->inheritShape(e1result);

    // Generate code for binary expression:
    Imop *i;
    Imop::Type iType;
    switch (e->type ()) {
        case NODE_EXPR_BINARY_ADD:  iType = Imop::ADD;  break;
        case NODE_EXPR_BINARY_SUB:  iType = Imop::SUB;  break;
        case NODE_EXPR_BINARY_MUL:  iType = Imop::MUL;  break;
        case NODE_EXPR_BINARY_DIV:  iType = Imop::DIV;  break;
        case NODE_EXPR_BINARY_MOD:  iType = Imop::MOD;  break;
        case NODE_EXPR_BINARY_EQ:   iType = Imop::EQ;   break;
        case NODE_EXPR_BINARY_GE:   iType = Imop::GE;   break;
        case NODE_EXPR_BINARY_GT:   iType = Imop::GT;   break;
        case NODE_EXPR_BINARY_LE:   iType = Imop::LE;   break;
        case NODE_EXPR_BINARY_LT:   iType = Imop::LT;   break;
        case NODE_EXPR_BINARY_NE:   iType = Imop::NE;   break;
        case NODE_EXPR_BINARY_LAND: iType = Imop::LAND; break;
        case NODE_EXPR_BINARY_LOR:  iType = Imop::LOR;  break;
        default:
            log.fatal() << "Binary " << e->operatorString ()
                        << " not yet implemented. At " << e->location ();
            result.setStatus (ICode::E_NOT_IMPLEMENTED);
            return result;
    }

    i = newBinary (e, iType, resSym, e1result, e2result);
    pushImopAfter (result, i);

    return result;
}

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
                    return ICode::E_OTHER;
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

CGBranchResult TreeNodeExprBinary::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprBinary (this);
}

CGBranchResult CodeGen::cgBoolExprBinary (TreeNodeExprBinary *e) {
    typedef TypeNonVoid TNV;


    TreeNodeExpr *eArg1 = static_cast<TreeNodeExpr*>(e->children().at(0));
    TreeNodeExpr *eArg2 = static_cast<TreeNodeExpr*>(e->children().at(1));
    CGBranchResult result;

    switch (e->type()) {
        case NODE_EXPR_BINARY_LAND: // fall through
        case NODE_EXPR_BINARY_LOR:
            assert(!eArg1->resultType().isVoid());
            assert(dynamic_cast<const TNV*>(&eArg1->resultType()) != 0);

            /*
              If first sub-expression is public, then generate short-circuit
              code for logical && and logical ||.
            */
            if (static_cast<const TNV&>(eArg1->resultType()).secrecSecType()
                    == SECTYPE_PUBLIC)
            {
                /// \todo I'm quite sure this is incorrect, we are not handling next lists!

                // Generate code for first child expression:
                result = codeGenBranch (eArg1);
                if (result.isNotOk ()) {
                    return result;
                }

                // Generate code for second child expression:
                const CGBranchResult& arg2Result = codeGenBranch (eArg2);
                result.patchFirstImop (arg2Result.firstImop ());
                if (arg2Result.isNotOk ()) {
                    result.setStatus (arg2Result.status ());
                    return result;
                }

                // Short circuit the code:
                if (e->type() == NODE_EXPR_BINARY_LAND) {
                    result.patchTrueList (st.label(arg2Result.firstImop ()));
                    result.setTrueList (arg2Result.trueList ());
                    result.addToFalseList (arg2Result.falseList ());
                } else {
                    assert (e->type() == NODE_EXPR_BINARY_LOR);
                    result.patchFalseList (st.label (arg2Result.firstImop()));
                    result.setFalseList (arg2Result.falseList());
                    result.addToTrueList (arg2Result.trueList());
                }
            } else {
                result = codeGen (e);
                if (result.isNotOk ()) {
                    return result;
                }

                Imop *j1, *j2;
                if (e->type() == NODE_EXPR_BINARY_LAND) {
                    j1 = new Imop (e, Imop::JF, 0);
                    result.addToFalseList (j1);

                    j2 = new Imop (e, Imop::JUMP, 0);
                    result.addToTrueList (j2);
                } else {
                    assert (e->type() == NODE_EXPR_BINARY_LOR);

                    j1 = new Imop (e, Imop::JT, 0);
                    result.addToTrueList (j1);

                    j2 = new Imop (e, Imop::JUMP, 0);
                    result.addToFalseList (j2);
                }

                j1->setArg1 (result.symbol ());
                code.push_imop(j1);
                pushImopAfter (result, j1);
                code.push_imop(j2);

                return result;
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
            const CGResult& arg1Result = codeGen (eArg1);
            append (result, arg1Result);
            if (result.isNotOk ()) {
                return result;
            }

            // Generate code for second child expression:
            const CGResult& arg2Result = codeGen (eArg2);
            append (result, arg2Result);
            if (result.isNotOk ()) {
                return result;
            }

            Imop::Type iType;
            switch (e->type()) {
                case NODE_EXPR_BINARY_EQ: iType = Imop::JE; break;
                case NODE_EXPR_BINARY_GE: iType = Imop::JGE; break;
                case NODE_EXPR_BINARY_GT: iType = Imop::JGT; break;
                case NODE_EXPR_BINARY_LE: iType = Imop::JLE; break;
                case NODE_EXPR_BINARY_LT: iType = Imop::JLT; break;
                case NODE_EXPR_BINARY_NE: iType = Imop::JNE; break;
                default:
                    assert (false && "Dont know how to handle the node type.");
                    result.setStatus (ICode::E_OTHER);
                    return result;
            }

            Imop *tj = new Imop (e, iType, (Symbol*) 0, arg1Result.symbol (), arg2Result.symbol ());
            pushImopAfter (result, tj);
            result.addToTrueList (tj);

            Imop *fj = new Imop (e, Imop::JUMP, 0);
            result.addToFalseList (fj);
            code.push_imop (fj);
            break;
        }
        default:
            assert (false && "Illegal binary operator");
            result.setStatus (ICode::E_OTHER);
            break;
    }

    return result;
}

/*******************************************************************************
  TreeNodeExprProcCall
*******************************************************************************/

CGResult TreeNodeExprProcCall::codeGenWith (CodeGen &cg) {
    return cg.cgExprProcCall (this);
}

CGResult CodeGen::cgExprProcCall (TreeNodeExprProcCall *e) {
    typedef TreeNode::ChildrenListConstIterator CLCI;

    // Type check:
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    Symbol* r = generateResultSymbol (result, e);
    std::stack<Symbol*> argList;

    // Initialize arguments:
    for (CLCI it (e->children().begin() + 1); it != e->children ().end (); ++ it) {
        assert (((*it)->type() & NODE_EXPR_MASK) != 0x0);
        assert (dynamic_cast<TreeNodeExpr*> (*it) != 0);
        TreeNodeExpr *eArg = static_cast<TreeNodeExpr*> (*it);
        const CGResult& argResult (codeGen (eArg));
        append (result, argResult);
        if (result.isNotOk ()) {
            return result;
        }

        argList.push (argResult.symbol ());
    }

    // Add them as arguments in a backward manner:
    while (!argList.empty ()) {
        Symbol* sym = argList.top ();

        Imop *i = 0;
        if (!sym->secrecType ().isScalar ()) {
            i = new Imop (e, Imop::PUSH, 0, sym, sym->getSizeSym ());
        }
        else {
            i = new Imop (e, Imop::PUSH, 0, sym);
        }

        pushImopAfter (result, i);

        // push shape in reverse order
        Symbol::dim_reverese_iterator
                di = sym->dim_rbegin (),
                de = sym->dim_rend ();
        for (; di != de; ++ di) {
            Imop* i = new Imop (e, Imop::PUSH, (Symbol*) 0, *di);
            code.push_imop (i);
        }

        argList.pop ();
    }

    // Do function call
    Imop *i = new Imop (e, Imop::CALL, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
    Imop *c = new Imop (e, Imop::RETCLEAN, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
    i->setCallDest (e->symbolProcedure (), st.label (c));
    c->setArg2 (st.label (i));
    code.push_imop (i);
    code.push_imop (c);
    result.patchFirstImop (i);

    // pop shape, recompute size, and pop data
    if (!e->resultType ().isVoid ()) {
        Symbol::dim_iterator di = r->dim_begin (), de = r->dim_end ();
        for (; di != de; ++ di) {
            Imop* i = new Imop (e, Imop::POP, *di);
            code.push_imop (i);
        }

        codeGenSize (result);
        i = newNullary (e, Imop::POP, r);
        code.push_imop (i);
    }

    return result;
}

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

CGBranchResult TreeNodeExprProcCall::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprProcCall (this);
}

CGBranchResult CodeGen::cgBoolExprProcCall (TreeNodeExprProcCall *e) {
    CGBranchResult result (codeGen (e));
    if (result.isNotOk ()) {
        return result;
    }

    Imop *i = new Imop (e, Imop::JT, 0, result.symbol ());
    code.push_imop (i);
    result.addToTrueList (i);

    i = new Imop (e, Imop::JUMP, 0);
    code.push_imop (i);
    result.addToFalseList (i);

    return result;
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

CGResult TreeNodeExprRVariable::codeGenWith (CodeGen &cg) {
    assert (children ().size () == 1);
    assert (dynamic_cast<TreeNodeIdentifier*> (children().at (0)) != 0);
    return cg.cgExprRVariable (this);
}

CGResult CodeGen::cgExprRVariable (TreeNodeExprRVariable *e) {
    // Type check:
    ICode::Status s = e->calculateResultType (st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*> (e->children ().at (0));
    CGResult result;
    result.setResult (id->getSymbol (st, log));
    return result;
}

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

CGBranchResult TreeNodeExprRVariable::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    assert (children ().size () == 1);
    assert (dynamic_cast<TreeNodeIdentifier*>(children ().at (0)) != 0);
    return cg.cgBoolExprRVariable (this);
}

CGBranchResult CodeGen::cgBoolExprRVariable (TreeNodeExprRVariable *e) {

    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(e->children().at(0));

    CGBranchResult result;
    Imop *i = new Imop (e, Imop::JT, 0, id->getSymbol (st, log));
    code.push_imop (i);
    result.setFirstImop (i);
    result.addToTrueList (i);

    i = new Imop (e, Imop::JUMP, 0);
    code.push_imop (i);
    result.addToFalseList (i);

    return result;
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

CGResult TreeNodeExprString::codeGenWith (CodeGen &cg) {
    return cg.cgExprString (this);
}

CGResult CodeGen::cgExprString (TreeNodeExprString *e) {
    CGResult result;

    // Type check:
    ICode::Status status = e->calculateResultType (st, log);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    result.setResult (st.constantString (e->value ()));
    return result;
}

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

CGResult TreeNodeExprTernary::codeGenWith (CodeGen &cg) {
    return cg.cgExprTernary (this);
}

CGResult CodeGen::cgExprTernary (TreeNodeExprTernary *e) {
    // Type check:
    ICode::Status s = e->calculateResultType (st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(e->children ().at (0));
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(e->children ().at (1));
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(e->children ().at (2));

    if (e1->havePublicBoolType()) {
        generateResultSymbol (result, e);

        // Generate code for boolean expression:
        CGBranchResult e1Result = codeGenBranch (e1);
        append (result, e1Result);
        if (result.isNotOk ()) {
            return result;
        }

        // Generate code for first value child expression:
        CGResult eTrueResult (codeGen (e2));
        if (eTrueResult.isNotOk ()) {
            result.setStatus (eTrueResult.status ());
            return result;
        }

        if (!e->resultType ().isVoid ()) {
            result.symbol ()->inheritShape (eTrueResult.symbol ());
            Imop* i = newAssign (e, result.symbol (), eTrueResult.symbol ());
            pushImopAfter (eTrueResult, i);
        }

        result.patchFirstImop (eTrueResult.firstImop ());

        // Jump out of the ternary construct:
        Imop* i = new Imop (e, Imop::JUMP, 0);
        result.addToNextList (i);
        result.patchFirstImop (i);
        code.push_imop (i);

        CGResult eFalseResult (codeGen (e3));
        if (eFalseResult.isNotOk ()) {
            result.setStatus (eFalseResult.status ());
            return result;
        }

        if (!e->resultType ().isVoid ()) {
            result.symbol ()->inheritShape (eFalseResult.symbol ());
            Imop* i = newAssign (e, result.symbol (), eFalseResult.symbol ());
            pushImopAfter (eFalseResult, i);
        }

        // Link boolean expression code to the rest of the code:
        e1Result.patchTrueList (st.label (eTrueResult.firstImop ()));
        e1Result.patchFalseList (st.label (eFalseResult.firstImop ()));
    }
    else {
        // Evaluate subexpressions:
        CGResult e1Result (codeGen (e1));
        append (result, e1Result);
        if (result.isNotOk ()) {
            return result;
        }

        CGResult e2Result (codeGen (e2));
        append (result, e2Result);
        if (result.isNotOk ()) {
            return result;
        }

        CGResult e3Result (codeGen (e3));
        append (result, e3Result);
        if (result.isNotOk ()) {
            return result;
        }

        // Generate temporary for the result of the ternary expression, if needed:
        Symbol* resSym = generateResultSymbol (result, e);
        resSym->inheritShape (e1Result.symbol ());

        // check that shapes match
        std::stringstream ss;
        ss << "Mismatching shapes at " << e->location();
        Imop* jmp = new Imop (e, Imop::JUMP, (Symbol*) 0);
        Imop* err = newError (e, st.constantString (ss.str ()));
        SymbolLabel* errLabel = st.label(err);
        Symbol::dim_iterator
                di = e1Result.symbol ()->dim_begin (),
                dj = e2Result.symbol ()->dim_begin (),
                dk = e3Result.symbol ()->dim_begin (),
                de = e1Result.symbol ()->dim_end ();
        for (; di != de; ++ di, ++ dj, ++ dk) {
            Imop* i = new Imop (e, Imop::JNE, (Symbol*) 0, *di, *dj);
            pushImopAfter (result, i);
            i->setJumpDest(errLabel);

            i = new Imop (e, Imop::JNE, (Symbol*) 0, *dj, *dk);
            code.push_imop (i);
            i->setJumpDest (errLabel);
        }

        result.patchNextList (st.label (jmp));
        code.push_imop(jmp);
        code.push_imop(err);

        // loop to set all values of resulting array

        // Set up some temporary scalars:
        Symbol* counter = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0)); // public int
        Symbol* b = st.appendTemporary(TypeNonVoid(e1->resultType ().secrecSecType (), e1->resultType ().secrecDataType ()));
        Symbol* t = st.appendTemporary(TypeNonVoid(e->resultType ().secrecSecType (), e->resultType ().secrecDataType ()));

                // r = e1
        Imop* i = newAssign (e, resSym, e2Result.symbol ());
        code.push_imop (i);
        jmp->setJumpDest (st.label(i));

        // counter = 0
        i = new Imop (e, Imop::ASSIGN, counter, st.constantInt (0));
        code.push_imop (i);

        // L0: if (counter >= size) goto next;
        Imop* jge = new Imop (e, Imop::JGE, (Symbol*) 0, counter, resSym->getSizeSym ());
        code.push_imop (jge);
        result.addToNextList (jge);

        // b = e1[counter]
        i = new Imop (e, Imop::LOAD, b, e1Result.symbol (), counter);
        code.push_imop (i);

        Imop* t0 = new Imop (e, Imop::ADD, counter, counter, st.constantInt (1));
        Imop* t1 = new Imop (e, Imop::STORE, resSym, counter, t);

        // if b goto T0
        i = new Imop (e, Imop::JT, (Symbol*) 0, b);
        code.push_imop(i);
        i->setJumpDest (st.label(t0));

        // t = e3[counter]
        // T1: result[counter] = t
        i = new Imop (e, Imop::LOAD, t, e3Result.symbol (), counter);
        code.push_imop (i);
        code.push_imop (t1);

        // T0: counter = counter + 1
        code.push_imop (t0);

        // goto L0
        i = new Imop (e, Imop::JUMP, (Symbol*) 0);
        code.push_imop (i);
        i->setJumpDest (st.label (jge));
    }

    return result;
}

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

CGBranchResult TreeNodeExprTernary::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprTernary (this);
}

CGBranchResult CodeGen::cgBoolExprTernary (TreeNodeExprTernary *e) {

    // Generate code for boolean expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(e->children().at(0));
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(e->children().at(1));
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(e->children().at(2));

    CGBranchResult result = codeGenBranch (e1);
    if (result.isNotOk ()) {
        return result;
    }

    // Generate code for first value child expression:
    const CGBranchResult& trueResult = codeGenBranch (e2);
    if (trueResult.isNotOk ()) {
        return trueResult;
    }

    // Generate code for second value child expression:
    const CGBranchResult& falseResult = codeGenBranch (e3);
    if (falseResult.isNotOk ()) {
        return falseResult;
    }

    // Link conditional expression code to the rest of the code:
    result.patchTrueList (st.label (trueResult.firstImop ()));
    result.patchFalseList (st.label (falseResult.firstImop ()));

    result.addToTrueList (trueResult.trueList ());
    result.addToTrueList (falseResult.trueList ());
    result.addToFalseList (trueResult.falseList ());
    result.addToFalseList (falseResult.falseList ());

    return result;
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

CGResult TreeNodeExprInt::codeGenWith (CodeGen &cg) {
    return cg.cgExprInt (this);
}

CGResult CodeGen::cgExprInt (TreeNodeExprInt *e) {
    // Type check:
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    result.setResult (st.constantInt (e->value ()));
    return result;
}

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

CGResult TreeNodeExprUInt::codeGenWith (CodeGen &cg) {
    return cg.cgExprUInt (this);
}

CGResult CodeGen::cgExprUInt (TreeNodeExprUInt *e) {
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    result.setResult (st.constantUInt (e->value ()));
    return result;
}

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

CGResult TreeNodeExprBool::codeGenWith (CodeGen &cg) {
    return cg.cgExprBool (this);
}

CGResult CodeGen::cgExprBool (TreeNodeExprBool *e) {
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    result.setResult (st.constantBool (e->value ()));
    return result;
}

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

CGBranchResult TreeNodeExprBool::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprBool (this);
}

CGBranchResult CodeGen::cgBoolExprBool (TreeNodeExprBool *e) {
    CGBranchResult result;
    Imop *i = new Imop(m_node, Imop::JUMP, 0);
    code.push_imop (i);
    result.setFirstImop (i);

    if (e->value ()) {
        result.addToTrueList (i);
    } else {
        result.addToFalseList (i);
    }

    return result;
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

CGResult TreeNodeExprClassify::codeGenWith (CodeGen &cg) {
    return cg.cgExprClassify (this);
}

CGResult CodeGen::cgExprClassify (TreeNodeExprClassify *e) {
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    // Generate code for child expression
    TreeNodeExpr* eArg = static_cast<TreeNodeExpr*>(e->children().at(0));
    CGResult result (codeGen (eArg));
    if (result.isNotOk ()) {
        return result;
    }

    // Generate temporary for the result of the classification, if needed:
    Symbol* argSym = result.symbol ();
    Symbol* resSym = generateResultSymbol (result, e);
    resSym->inheritShape (argSym);

    Imop *i = newUnary (m_node, Imop::CLASSIFY, resSym, argSym);
    pushImopAfter (result, i);
    return result;
}

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

CGResult TreeNodeExprDeclassify::codeGenWith (CodeGen &cg) {
    return cg.cgExprDeclassify (this);
}

CGResult CodeGen::cgExprDeclassify (TreeNodeExprDeclassify *e) {
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    // Generate code for child expression
    TreeNodeExpr* eArg = static_cast<TreeNodeExpr*>(e->children().at(0));
    CGResult result (codeGen (eArg));
    if (result.isNotOk ()) {
        return result;
    }

    // Generate temporary for the result of the classification, if needed:
    Symbol* argSym = result.symbol ();
    Symbol* resSym = generateResultSymbol (result, e);
    resSym->inheritShape (argSym);

    Imop *i = newUnary (e, Imop::DECLASSIFY, resSym, argSym);
    pushImopAfter (result, i);

    return result;
}

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

CGBranchResult TreeNodeExprDeclassify::codeGenBoolWith (CodeGen &cg) {
    assert(havePublicBoolType());
    return cg.cgBoolExprDeclassify (this);
}

CGBranchResult CodeGen::cgBoolExprDeclassify (TreeNodeExprDeclassify *e) {
    CGBranchResult result (codeGen (e));
    if (result.isNotOk ()) {
        return result;
    }

    Imop *i = new Imop (e, Imop::JT, 0, result.symbol ());
    pushImopAfter (result, i);
    result.addToTrueList (i);

    i = new Imop (e, Imop::JUMP, 0);
    code.push_imop (i);
    result.addToFalseList (i);

    return result;
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

CGResult TreeNodeExprUnary::codeGenWith (CodeGen &cg) {
    return cg.cgExprUnary (this);
}

CGResult CodeGen::cgExprUnary (TreeNodeExprUnary *e) {
    // Type check:
    ICode::Status s = e->calculateResultType (st, log);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    // Generate code for child expression:
    TreeNodeExpr *eArg = static_cast<TreeNodeExpr*>(e->children().at(0));
    CGResult result (codeGen (eArg));
    if (!result.isOk ()) {
        return result;
    }

    Symbol* eResult = result.symbol ();
    generateResultSymbol (result, e);
    result.symbol ()->inheritShape (eResult); // no need to copy the symbols

    // Generate code for unary expression:
    Imop::Type iType = (e->type() == NODE_EXPR_UNEG) ? Imop::UNEG : Imop::UMINUS;
    Imop *i = newUnary (m_node, iType, result.symbol (), eResult);
    pushImopAfter (result, i);
    return result;
}

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

CGBranchResult TreeNodeExprUnary::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    assert (type() == NODE_EXPR_UNEG);
    return cg.cgBoolExprUnary (this);
}

CGBranchResult CodeGen::cgBoolExprUnary (TreeNodeExprUnary *e) {
    // Generate code for child expression:
    TreeNodeExpr *eArg = static_cast<TreeNodeExpr*>(e->children().at(0));
    CGBranchResult result = codeGenBranch (eArg);
    if (!result.isOk ()) {
        return result;
    }

    result.swapTrueFalse ();
    return result;
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

CGResult TreeNodeExprPrefix::codeGenWith (CodeGen &cg) {
    assert (children ().size () == 1);
    TreeNode *c0 = children ().at (0);
    assert (c0 != 0);
    assert (1 <= c0->children ().size () && c0->children ().size () <= 2);
    assert (dynamic_cast<TreeNodeIdentifier*>(c0->children ().at (0)) != 0);
    return cg.cgExprPrefix (this);
}

CGResult CodeGen::cgExprPrefix (TreeNodeExprPrefix *e) {
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV;

    // Type check:
    CGResult result;
    ICode::Status status = e->calculateResultType(st, log);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    const TypeNonVoid& pubIntTy (TypeNonVoid (SECTYPE_PUBLIC, DATATYPE_INT, 0));

    // Generate code for child expression:
    TreeNode* lval = e->children ().at (0);
    TreeNodeIdentifier* e1 = static_cast<TreeNodeIdentifier*>(lval->children ().at (0));
    Symbol *destSym = st.find (e1->value ());
    assert (destSym->symbolType() == Symbol::SYMBOL);
    assert (dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol* destSymSym = static_cast<SymbolSymbol*> (destSym);


    // either use ADD or SUB
    Imop::Type iType;
    switch (e->type ()) {
    case NODE_EXPR_PREFIX_INC: iType = Imop::ADD; break;
    case NODE_EXPR_PREFIX_DEC: iType = Imop::SUB; break;
    default:
        assert (false && "ICE: prefix operator on something other than increment or decrement (wut?)!");
        result.setStatus (ICode::E_NOT_IMPLEMENTED);
        return result;
    }

    // ++ x[e1,..,ek]
    if (lval->children().size() == 2) {
        CodeGenSubscript subInfo (*this);
        append (result, subInfo.codeGenSubscript (destSym, lval->children ().at (1)));
        if (result.isNotOk ()) {
            return result;
        }


        const SPV& spv = subInfo.spv ();
        const std::vector<unsigned>& slices = subInfo.slices ();
        CodeGenStride stride (*this);
        append (result, stride.codeGenStride (destSym));
        if (result.isNotOk ()) {
            return result;
        }

        // Initialize required temporary symbols:
        std::vector<Symbol* > indices;
        Symbol* offset = st.appendTemporary(pubIntTy);
        Symbol* resultOffset = st.appendTemporary(pubIntTy);
        Symbol* tmpResult = st.appendTemporary(pubIntTy);
        Symbol* tmpValue = st.appendTemporary (pubIntTy);
        for (SPV::const_iterator it (spv.begin ()); it != spv.end (); ++ it) {
            Symbol* sym = st.appendTemporary(pubIntTy);
            indices.push_back(sym);
        }

        std::vector<Symbol*>::const_iterator idxIt;
        Imop* i = new Imop (e, Imop::ASSIGN, resultOffset, st.constantInt (0));
        pushImopAfter (result, i);

        Symbol* resSym = generateResultSymbol (result, e);

        if (!e->resultType ().isScalar ()) {
            unsigned count = 0;
            for (std::vector<unsigned>::const_iterator it (slices.begin ()); it != slices.end (); ++ it, ++ count) {
                unsigned k = *it;
                i = new Imop (e, Imop::SUB, resSym->getDim (count), spv[k].second, spv[k].first);
                code.push_imop (i);
            }

            codeGenSize (result);
            i = new Imop (e, Imop::FILL, resSym, st.constantInt (0), resSym->getSizeSym ());
        }
        else {
            i = new Imop (e, Imop::ASSIGN, resSym, st.constantInt (0));
        }

        code.push_imop (i);

        CodeGenLoop loop (*this);

        append (result, loop.enterLoop (spv, indices));
        if (result.isNotOk ()) {
            return result;
        }

        // compute offset:
        i = new Imop (e, Imop::ASSIGN, offset, st.constantInt (0));
        code.push_imop (i);

        idxIt = indices.begin ();
        for (unsigned k = 0; k < stride.size (); ++ k, ++ idxIt) {
            i = new Imop (e, Imop::MUL, tmpResult, stride.at (k), *idxIt);
            code.push_imop (i);

            i = new Imop (e, Imop::ADD, offset, offset, tmpResult);
            code.push_imop (i);
        }

        // increment the value:

        // t = x[offset]
        i = new Imop (e, Imop::LOAD, tmpValue, destSymSym, offset);
        code.push_imop (i);

        // t = t + 1
        i = new Imop (e, iType, tmpValue, tmpValue, st.constantInt (1));
        code.push_imop (i);

        // x[offset] = t
        i = new Imop (e, Imop::STORE, destSymSym, offset, tmpValue);
        code.push_imop (i);

        if (!e->resultType ().isScalar ()) {
            // r[resultOffset] = t
            i = new Imop (e, Imop::STORE, resSym, resultOffset, tmpValue);
            code.push_imop (i);

            // resultOffset = resultOffset + 1
            i = new Imop (e, Imop::ADD, resultOffset, resultOffset, st.constantInt (1));
            code.push_imop (i);
        }
        else {
            // r = t
            i = new Imop (e, Imop::ASSIGN, resSym, tmpValue);
            code.push_imop (i);
        }

        append (result, loop.exitLoop (indices));
        return result;
    }

    Symbol* one = st.constantInt (1);
    if (!e->resultType ().isScalar ()) {
        one = st.appendTemporary (static_cast<const TypeNonVoid&> (e->resultType ()));
        Imop* i = new Imop (e, Imop::FILL, one, st.constantInt (1), destSymSym->getSizeSym ());
        pushImopAfter (result,i );
    }

    // x = x `iType` 1
    Imop* i = newBinary (e, iType, destSymSym, destSymSym, one);
    pushImopAfter (result, i);

    // r = x

    Symbol* r = generateResultSymbol (result, e);
    r->inheritShape (destSymSym);
    i = newAssign (e, r, destSymSym);
    code.push_imop (i);
    return result;
}

ICode::Status TreeNodeExprPrefix::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV;
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    const TypeNonVoid& pubIntTy (TypeNonVoid (SECTYPE_PUBLIC, DATATYPE_INT, 0));

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
        SubscriptInfo subInfo = codegenSubscript (destSym, lval->children ().at (1), code, st, log);
        if (subInfo.status != ICode::OK) {
            return subInfo.status;
        }

        SPV& spv = subInfo.spv;
        std::vector<unsigned>& slices = subInfo.slices;
        std::vector<Symbol*> stride = codegenStride (destSym, code, st);
        std::vector<Symbol* > indices;
        Symbol* offset = st.appendTemporary(pubIntTy);
        Symbol* resultOffset = st.appendTemporary(pubIntTy);
        Symbol* tmpResult = st.appendTemporary(pubIntTy);
        Symbol* tmpValue = st.appendTemporary (pubIntTy);

        for (SPV::iterator it(spv.begin()); it != spv.end(); ++ it) {
            Symbol* sym = st.appendTemporary(pubIntTy);
            indices.push_back(sym);
        }

        std::stack<Imop*> jumpStack;
        std::vector<Symbol*>::iterator idxIt;
        Imop* i = 0;

        i = new Imop (this, Imop::ASSIGN, resultOffset, st.constantInt (0));
        code.push_imop (i);
        patchFirstImop (i);
        patchNextList(i, st);
        prevPatchNextList (i, st);

        // generate result symbol if needed;
        if (r != 0) {
            assert (r->secrecType().canAssign(resultType()));
            setResult (r);
            assert (false && "TODO!");
            if (!resultType ().isScalar ()) {

            }
        }
        else {
            generateResultSymbol (st);
            if (!resultType ().isScalar ()) {
                unsigned count = 0;
                for (std::vector<unsigned>::iterator it (slices.begin ()); it != slices.end (); ++ it, ++ count) {
                    unsigned k = *it;
                    i = new Imop (this, Imop::SUB, result ()->getDim (count), spv[k].second, spv[k].first);
                    code.push_imop (i);
                }

                computeSize (code, st);
                i = new Imop (this, Imop::FILL, result (), st.constantInt (0), result ()->getSizeSym ());
            }
            else {
                i = new Imop (this, Imop::ASSIGN, result (), st.constantInt (0));
            }

            code.push_imop (i);
        }

        // enter loop:
        idxIt = indices.begin ();
        for (SPV::iterator it (spv.begin ()); it != spv.end (); ++ it, ++ idxIt) {
            Symbol* i_lo = it->first;
            Symbol* i_hi = it->second;
            Symbol* idx  = *idxIt;

            i = new Imop (this, Imop::ASSIGN, idx, i_lo);
            code.push_imop (i);
            patchFirstImop (i);
            patchNextList(i, st);
            prevPatchNextList (i, st);

            i = new Imop (this, Imop::JGE, 0, idx, i_hi);
            code.push_imop (i);
            jumpStack.push (i);
        }

        // compute offset:
        i = new Imop (this, Imop::ASSIGN, offset, st.constantInt (0));
        code.push_imop(i);

        idxIt = indices.begin ();
        for (std::vector<Symbol*>::iterator it (stride.begin ()); it != stride.end (); ++ it, ++ idxIt) {
            i = new Imop (this, Imop::MUL, tmpResult, *it, *idxIt);
            code.push_imop (i);

            i = new Imop (this, Imop::ADD, offset, offset, tmpResult);
            code.push_imop (i);
        }

        // increment the value:

        // t = x[offset]
        i = new Imop (this, Imop::LOAD, tmpValue, destSymSym, offset);
        code.push_imop (i);

        // t = t + 1
        i = new Imop (this, cType, tmpValue, tmpValue, st.constantInt (1));
        code.push_imop (i);

        // x[offset] = t
        i = new Imop (this, Imop::STORE, destSymSym, offset, tmpValue);
        code.push_imop (i);

        if (!resultType ().isScalar ()) {
            // r[resultOffset] = t
            i = new Imop (this, Imop::STORE, result (), resultOffset, tmpValue);
            code.push_imop (i);

            // resultOffset = resultOffset + 1
            i = new Imop (this, Imop::ADD, resultOffset, resultOffset, st.constantInt (1));
            code.push_imop (i);
        }
        else {
            // r = t
            i = new Imop (this, Imop::ASSIGN, result (), tmpValue);
            code.push_imop (i);
        }

        // exit loop:
        Imop* prevJump = 0;
        for (std::vector<Symbol*>::reverse_iterator it (indices.rbegin ()); it != indices.rend (); ++ it) {
            Symbol* idx = *it;
            i = new Imop (this, Imop::ADD, idx, idx, st.constantInt (1));
            code.push_imop (i);
            if (prevJump != 0) {
                prevJump->setJumpDest (st.label (i));
            }

            i = new Imop (this, Imop::JUMP, (Symbol*) 0);
            code.push_imop (i);
            i->setJumpDest (st.label (jumpStack.top ()));
            prevJump = jumpStack.top ();
            jumpStack.pop ();
        }

        if (prevJump != 0) {
            addToNextList (prevJump);
        }

        return ICode::OK;
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

CGResult TreeNodeExprPostfix::codeGenWith (CodeGen &cg) {
    assert (children ().size () == 1);
    TreeNode *c0 = children ().at (0);
    assert (c0 != 0);
    assert (1 <= c0->children ().size () && c0->children ().size () <= 2);
    assert (dynamic_cast<TreeNodeIdentifier*>(c0->children ().at (0)) != 0);
    return cg.cgExprPostfix (this);
}

CGResult CodeGen::cgExprPostfix (TreeNodeExprPostfix *e) {
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV;

    // Type check:
    CGResult result;
    ICode::Status status = e->calculateResultType(st, log);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    const TypeNonVoid& pubIntTy (TypeNonVoid (SECTYPE_PUBLIC, DATATYPE_INT, 0));

    // Generate code for child expression:
    TreeNode* lval = e->children ().at (0);
    TreeNodeIdentifier* e1 = static_cast<TreeNodeIdentifier*>(lval->children ().at (0));
    Symbol *destSym = st.find (e1->value ());
    assert (destSym->symbolType() == Symbol::SYMBOL);
    assert (dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol* destSymSym = static_cast<SymbolSymbol*> (destSym);


    // either use ADD or SUB
    Imop::Type iType;
    switch (e->type ()) {
    case NODE_EXPR_POSTFIX_INC: iType = Imop::ADD; break;
    case NODE_EXPR_POSTFIX_DEC: iType = Imop::SUB; break;
    default:
        assert (false && "ICE: postfix operator on something other than increment or decrement (wut?)!");
        result.setStatus (ICode::E_NOT_IMPLEMENTED);
        return result;
    }

    // ++ x[e1,..,ek]
    if (lval->children().size() == 2) {
        CodeGenSubscript subInfo (*this);
        append (result, subInfo.codeGenSubscript (destSym, lval->children ().at (1)));
        if (result.isNotOk ()) {
            return result;
        }


        const SPV& spv = subInfo.spv ();
        const std::vector<unsigned>& slices = subInfo.slices ();
        CodeGenStride stride (*this);
        append (result, stride.codeGenStride (destSym));
        if (result.isNotOk ()) {
            return result;
        }

        // Initialize required temporary symbols:
        std::vector<Symbol* > indices;
        Symbol* offset = st.appendTemporary(pubIntTy);
        Symbol* resultOffset = st.appendTemporary(pubIntTy);
        Symbol* tmpResult = st.appendTemporary(pubIntTy);
        Symbol* tmpValue = st.appendTemporary (pubIntTy);
        for (SPV::const_iterator it (spv.begin ()); it != spv.end (); ++ it) {
            Symbol* sym = st.appendTemporary(pubIntTy);
            indices.push_back(sym);
        }

        std::vector<Symbol*>::const_iterator idxIt;
        Imop* i = new Imop (e, Imop::ASSIGN, resultOffset, st.constantInt (0));
        pushImopAfter (result, i);

        Symbol* resSym = generateResultSymbol (result, e);

        if (!e->resultType ().isScalar ()) {
            unsigned count = 0;
            for (std::vector<unsigned>::const_iterator it (slices.begin ()); it != slices.end (); ++ it, ++ count) {
                unsigned k = *it;
                i = new Imop (e, Imop::SUB, resSym->getDim (count), spv[k].second, spv[k].first);
                code.push_imop (i);
            }

            codeGenSize (result);
            i = new Imop (e, Imop::FILL, resSym, st.constantInt (0), resSym->getSizeSym ());
        }
        else {
            i = new Imop (e, Imop::ASSIGN, resSym, st.constantInt (0));
        }

        code.push_imop (i);

        CodeGenLoop loop (*this);

        append (result, loop.enterLoop (spv, indices));
        if (result.isNotOk ()) {
            return result;
        }

        // compute offset:
        i = new Imop (e, Imop::ASSIGN, offset, st.constantInt (0));
        code.push_imop (i);

        idxIt = indices.begin ();
        for (unsigned k = 0; k < stride.size (); ++ k, ++ idxIt) {
            i = new Imop (e, Imop::MUL, tmpResult, stride.at (k), *idxIt);
            code.push_imop (i);

            i = new Imop (e, Imop::ADD, offset, offset, tmpResult);
            code.push_imop (i);
        }

        // increment the value:

        // t = x[offset]
        i = new Imop (e, Imop::LOAD, tmpValue, destSymSym, offset);
        code.push_imop (i);

        if (!e->resultType ().isScalar ()) {
            // r[resultOffset] = t
            i = new Imop (e, Imop::STORE, resSym, resultOffset, tmpValue);
            code.push_imop (i);

            // resultOffset = resultOffset + 1
            i = new Imop (e, Imop::ADD, resultOffset, resultOffset, st.constantInt (1));
            code.push_imop (i);
        }
        else {
            // r = t
            i = new Imop (e, Imop::ASSIGN, resSym, tmpValue);
            code.push_imop (i);
        }

        // t = t + 1
        i = new Imop (e, iType, tmpValue, tmpValue, st.constantInt (1));
        code.push_imop (i);

        // x[offset] = t
        i = new Imop (e, Imop::STORE, destSymSym, offset, tmpValue);
        code.push_imop (i);

        append (result, loop.exitLoop (indices));
        return result;
    }

    Symbol* one = st.constantInt (1);
    if (!e->resultType ().isScalar ()) {
        one = st.appendTemporary (static_cast<const TypeNonVoid&> (e->resultType ()));
        Imop* i = new Imop (e, Imop::FILL, one, st.constantInt (1), destSymSym->getSizeSym ());
        pushImopAfter (result,i );
    }

    // r = x
    Symbol* r = generateResultSymbol (result, e);
    r->inheritShape (destSymSym);
    Imop* i = newAssign (e, r, destSymSym);
    pushImopAfter (result, i);

    // x = x `iType` 1
    i = newBinary (e, iType, destSymSym, destSymSym, one);
    code.push_imop (i);

    return result;
}

ICode::Status TreeNodeExprPostfix::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV;
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    const TypeNonVoid& pubIntTy (TypeNonVoid (SECTYPE_PUBLIC, DATATYPE_INT, 0));

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

    // x[e1,..,ek] ++
    if (lval->children().size() == 2) {
        SubscriptInfo subInfo = codegenSubscript (destSym, lval->children ().at (1), code, st, log);
        if (subInfo.status != ICode::OK) {
            return subInfo.status;
        }

        SPV& spv = subInfo.spv;
        std::vector<unsigned>& slices = subInfo.slices;
        std::vector<Symbol*> stride = codegenStride (destSym, code, st);
        std::vector<Symbol* > indices;
        Symbol* offset = st.appendTemporary(pubIntTy);
        Symbol* resultOffset = st.appendTemporary(pubIntTy);
        Symbol* tmpResult = st.appendTemporary(pubIntTy);
        Symbol* tmpValue = st.appendTemporary (pubIntTy);

        for (SPV::iterator it(spv.begin()); it != spv.end(); ++ it) {
            Symbol* sym = st.appendTemporary(pubIntTy);
            indices.push_back(sym);
        }

        std::stack<Imop*> jumpStack;
        std::vector<Symbol*>::iterator idxIt;
        Imop* i = 0;

        i = new Imop (this, Imop::ASSIGN, resultOffset, st.constantInt (0));
        code.push_imop (i);
        patchFirstImop (i);
        patchNextList(i, st);
        prevPatchNextList (i, st);

        // generate result symbol if needed;
        if (r != 0) {
            assert (r->secrecType().canAssign(resultType()));
            setResult (r);
            assert (false && "TODO!");
            if (!resultType ().isScalar ()) {

            }
        }
        else {
            generateResultSymbol (st);
            if (!resultType ().isScalar ()) {
                unsigned count = 0;
                for (std::vector<unsigned>::iterator it (slices.begin ()); it != slices.end (); ++ it, ++ count) {
                    unsigned k = *it;
                    i = new Imop (this, Imop::SUB, result ()->getDim (count), spv[k].second, spv[k].first);
                    code.push_imop (i);
                }

                computeSize (code, st);
                i = new Imop (this, Imop::FILL, result (), st.constantInt (0), result ()->getSizeSym ());
            }
            else {
                i = new Imop (this, Imop::ASSIGN, result (), st.constantInt (0));
            }

            code.push_imop (i);
        }

        // enter loop:
        idxIt = indices.begin ();
        for (SPV::iterator it (spv.begin ()); it != spv.end (); ++ it, ++ idxIt) {
            Symbol* i_lo = it->first;
            Symbol* i_hi = it->second;
            Symbol* idx  = *idxIt;

            i = new Imop (this, Imop::ASSIGN, idx, i_lo);
            code.push_imop (i);
            patchFirstImop (i);
            patchNextList(i, st);
            prevPatchNextList (i, st);

            i = new Imop (this, Imop::JGE, 0, idx, i_hi);
            code.push_imop (i);
            jumpStack.push (i);
        }

        // compute offset:
        i = new Imop (this, Imop::ASSIGN, offset, st.constantInt (0));
        code.push_imop(i);

        idxIt = indices.begin ();
        for (std::vector<Symbol*>::iterator it (stride.begin ()); it != stride.end (); ++ it, ++ idxIt) {
            i = new Imop (this, Imop::MUL, tmpResult, *it, *idxIt);
            code.push_imop (i);

            i = new Imop (this, Imop::ADD, offset, offset, tmpResult);
            code.push_imop (i);
        }

        // increment the value:

        // t = x[offset]
        i = new Imop (this, Imop::LOAD, tmpValue, destSymSym, offset);
        code.push_imop (i);

        if (!resultType ().isScalar ()) {
            // r[resultOffset] = t
            i = new Imop (this, Imop::STORE, result (), resultOffset, tmpValue);
            code.push_imop (i);

            // resultOffset = resultOffset + 1
            i = new Imop (this, Imop::ADD, resultOffset, resultOffset, st.constantInt (1));
            code.push_imop (i);
        }
        else {
            // r = t
            i = new Imop (this, Imop::ASSIGN, result (), tmpValue);
            code.push_imop (i);
        }

        // t = t + 1
        i = new Imop (this, cType, tmpValue, tmpValue, st.constantInt (1));
        code.push_imop (i);

        // x[offset] = t
        i = new Imop (this, Imop::STORE, destSymSym, offset, tmpValue);
        code.push_imop (i);

        // exit loop:
        Imop* prevJump = 0;
        for (std::vector<Symbol*>::reverse_iterator it (indices.rbegin ()); it != indices.rend (); ++ it) {
            Symbol* idx = *it;
            i = new Imop (this, Imop::ADD, idx, idx, st.constantInt (1));
            code.push_imop (i);
            if (prevJump != 0) {
                prevJump->setJumpDest (st.label (i));
            }

            i = new Imop (this, Imop::JUMP, (Symbol*) 0);
            code.push_imop (i);
            i->setJumpDest (st.label (jumpStack.top ()));
            prevJump = jumpStack.top ();
            jumpStack.pop ();
        }

        if (prevJump != 0) {
            addToNextList (prevJump);
        }

        return ICode::OK;
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
        copyShapeFrom (destSymSym, code, st);
    }

    Imop* i = 0;

    // r = x
    i = newAssign (this, result (), destSymSym);
    code.push_imop (i);
    patchFirstImop (i);
    prevPatchNextList (i, st);

    // x = x + 1
    i = newBinary (this, cType, destSymSym, destSymSym, one);
    code.push_imop (i);

    return ICode::OK;
}

ICode::Status TreeNodeExprPostfix::generateBoolCode(ICodeList&, SymbolTable&, CompileLog&)
{
     assert (false && "TODO");
     return ICode::E_NOT_IMPLEMENTED;
 }

} // namespace SecreC
