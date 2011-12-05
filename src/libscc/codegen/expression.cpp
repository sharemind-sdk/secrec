#include <stack>

#include "codegen.h"
#include "treenode.h"
#include "symboltable.h"
#include "constant.h"
#include "misc.h"


/**
 * Code generation for expressions.
 */

namespace SecreC {

/******************************************************************
  TreeNodeExprCast
******************************************************************/

CGResult TreeNodeExprCast::codeGenWith (CodeGen &cg) {
    return cg.cgExprCast (this);
}

CGResult CodeGen::cgExprCast (TreeNodeExprCast *e) {
    TreeNodeExpr* eArg (static_cast<TreeNodeExpr*>(e->children ().at (1)));
    log.warning () << "Generating code for partially implemented expression.";
    return eArg->codeGenWith (*this);
}

CGBranchResult TreeNodeExprCast::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprCast (this);
}

CGBranchResult CodeGen::cgBoolExprCast (TreeNodeExprCast *e) {
    TreeNodeExpr* eArg (static_cast<TreeNodeExpr*>(e->children ().at (1)));
    log.warning () << "Generating code for partially implemented expression.";
    return eArg->codeGenBoolWith (*this);
}

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
    ICode::Status status =  m_tyChecker.visit (e);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    bool isScalar = e->resultType ()->isScalar ();

    SymbolSymbol* resSym = generateResultSymbol (result, e);

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
        pushComment ("Computing shape:");
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

    // r = ALLOC def size
    {
        Symbol* def = defaultConstant (getContext (), e->resultType ()->secrecDataType ());
        Imop* i = 0;
        if (!isScalar)
            i = new Imop (e, Imop::ALLOC, resSym, def, resSym->getSizeSym ());
        else
            i = newAssign (e, resSym, def);
        pushImopAfter (result, i);
    }

    // 4. initialze required temporary symbols
    std::vector<Symbol* > indices;
    Context& cxt = getContext ();
    TypeNonVoid* pubIntTy = TypeNonVoid::get (cxt, DATATYPE_INT);
    for (SPV::const_iterator it(spv.begin()); it != spv.end(); ++ it) {
        Symbol* sym = st->appendTemporary(pubIntTy);
        indices.push_back(sym);
    }

    Symbol* offset = st->appendTemporary(pubIntTy);
    Symbol* tmp_result = st->appendTemporary(TypeNonVoid::get (cxt,
        e->resultType ()->secrecSecType(), e->resultType ()->secrecDataType()));
    Symbol* tmp_result2 = st->appendTemporary(pubIntTy);

    // 3. initialize strides
    std::vector<CodeGenStride > strides (2, CodeGenStride (*this));
    append (result, strides[0].codeGenStride (x));
    append (result, strides[1].codeGenStride (resSym));
    if (result.isNotOk ()) {
        return result;
    }

    if (!isScalar) {
        Imop* i = newAssign (e, tmp_result, defaultConstant (getContext (), tmp_result->secrecType ()->secrecDataType ()));
        pushImopAfter (result, i);
    }

    CodeGenLoop loop (*this);

    append (result, loop.enterLoop (spv, indices));
    if (result.isNotOk ()) {
        return result;
    }

    // 8. compute offset for RHS
    {
        // old_ffset = 0
        pushComment ("Compute offset:");
        Imop* i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (),0));
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
        pushComment("Load and store:");

        // tmp = x[old_offset] or r = x[old_offset] if scalar
        Imop* i = new Imop (e, Imop::LOAD, (isScalar ? resSym : tmp_result), x, offset);
        code.push_imop (i);

        // r[offset] = tmp if not scalar
        if (!isScalar) {
            i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (),0));
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

/******************************************************************
  TreeNodeExprSize
******************************************************************/

CGResult TreeNodeExprSize::codeGenWith (CodeGen &cg) {
    return cg.cgExprSize (this);
}

CGResult CodeGen::cgExprSize (TreeNodeExprSize* e) {
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result (codeGen (e->expression ()));
    if (!result.isOk ()) {
        return result;
    }

    Symbol* size = ConstantInt::get (getContext (), 1);
    if (!e->expression ()->resultType ()->isScalar()) {
        size = static_cast<SymbolSymbol*>(result.symbol())->getSizeSym();
    }

    result.setResult (size);
    return result;
}

/******************************************************************
  TreeNodeExprShape
******************************************************************/

CGResult TreeNodeExprShape::codeGenWith (CodeGen &cg) {
    return cg.cgExprShape (this);
}

CGResult CodeGen::cgExprShape (TreeNodeExprShape *e) {
    // Type check:
    CGResult result;
    ICode::Status status = m_tyChecker.visit (e);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    SymbolSymbol* resSym = generateResultSymbol (result, e);
    TreeNodeExpr* eArg = e->expression ();
    const CGResult& argResult (codeGen (eArg));
    append (result, argResult);
    if (result.isNotOk ()) {
        return result;
    }

    Symbol* n = ConstantInt::get (getContext (),eArg->resultType ()->secrecDimType());
    Imop* i = 0;

    if (eArg->resultType ()->isScalar())
        i = new Imop (m_node, Imop::ASSIGN, resSym, ConstantInt::get (getContext (), 0));
    else
        i = new Imop (m_node, Imop::ALLOC, resSym, ConstantInt::get (getContext (), 0), n);
    pushImopAfter (result, i);

    i = new Imop (m_node, Imop::ASSIGN, resSym->getDim (0), n);
    code.push_imop(i);

    dim_iterator
            dti = dim_begin (argResult.symbol()),
            dte = dim_end (argResult.symbol());
    for (unsigned count = 0; dti != dte; ++ dti, ++ count) {
        Imop* i = new Imop(m_node, Imop::STORE, resSym, ConstantInt::get (getContext (),count), *dti);
        code.push_imop(i);
    }

    codeGenSize (result);
    return result;
}

/******************************************************************
  TreeNodeExprCat
******************************************************************/

CGResult TreeNodeExprCat::codeGenWith (CodeGen &cg) {
    return cg.cgExprCat (this);
}

CGResult CodeGen::cgExprCat (TreeNodeExprCat *e) {

    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    generateResultSymbol (result, e);

    const CGResult& arg1Result (codeGen (e->leftExpression ()));
    append (result, arg1Result);
    if (result.isNotOk ()) {
        return result;
    }


    const CGResult& arg2Result (codeGen (e->rightExpression ()));
    append (result, arg2Result);
    if (result.isNotOk ()) {
        return result;
    }

    unsigned k = e->dimensionality ()->value ();
    unsigned n = e->resultType ()->secrecDimType();
    SymbolSymbol* arg1ResultSymbol = static_cast<SymbolSymbol*>(arg1Result.symbol ());
    SymbolSymbol* arg2ResultSymbol = static_cast<SymbolSymbol*>(arg2Result.symbol ());
    SymbolSymbol* resSym = static_cast<SymbolSymbol*>(result.symbol ());

    // Compute resulting shape and perform sanity check:
    std::stringstream ss;
    ss << "Different sized dimensions in concat at " << e->location() << ".";
    Imop* err = newError (m_node, ConstantString::get (getContext (), ss.str ()));
    SymbolLabel* errLabel = st->label(err);
    for (unsigned it = 0; it < e->resultType ()->secrecDimType(); ++ it) {
        Symbol* s1 = arg1ResultSymbol->getDim(it);
        Symbol* s2 = arg2ResultSymbol->getDim(it);
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
    append (result, strides[0].codeGenStride (arg1ResultSymbol));
    append (result, strides[1].codeGenStride (arg2ResultSymbol));
    append (result, strides[2].codeGenStride (resSym));
    if (result.isNotOk ()) {
        return result;
    }

    // Symbols for running indices:
    std::vector<Symbol* > indices;
    TypeNonVoid* pubIntTy = TypeNonVoid::get (getContext (), DATATYPE_INT);
    for (unsigned it = 0; it < n; ++ it) {
        Symbol* sym = st->appendTemporary(pubIntTy);
        indices.push_back(sym);
    }

    // Compute size and allocate resulting array:
    codeGenSize (result);
    if (result.isNotOk ()) {
        return result;
    }

    Symbol* rhs = defaultConstant (getContext (), e->resultType ()->secrecDataType ());
    Imop* i = new Imop (m_node, Imop::ALLOC, resSym, rhs, resSym->getSizeSym());
    code.push_imop(i);

    CodeGenLoop loop (*this);
    append (result, loop.enterLoop (resSym, indices));

    Symbol* offset = st->appendTemporary(pubIntTy);
    Symbol* tmpInt = st->appendTemporary(pubIntTy);

    // j = 0 (right hand side index)
    i = new Imop(m_node, Imop::ASSIGN, offset, ConstantInt::get (getContext (),0));
    code.push_imop(i);

    // IF (i_k >= d_k) GOTO T1;
    i = new Imop(m_node, Imop::JGE, (Symbol*) 0, indices[k], arg1ResultSymbol->getDim(k));
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
    TypeNonVoid* elemType = TypeNonVoid::get (getContext (),
        e->resultType ()->secrecSecType(), e->resultType ()->secrecDataType());
    Symbol* tmp_elem = st->appendTemporary(elemType);
    i = new Imop (m_node, Imop::LOAD, tmp_elem, arg1Result.symbol (), offset);
    code.push_imop(i);

    // jump out
    Imop* jump_out = new Imop(m_node, Imop::JUMP, (Symbol*) 0);
    code.push_imop (jump_out);

    // compute j if i >= d (for e2)
    for (unsigned count = 0; count < strides[1].size (); ++ count) {
        if (count == k) {
            i = new Imop (m_node, Imop::SUB, tmpInt, indices[count], arg1ResultSymbol->getDim(k));
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
    i = new Imop (m_node, Imop::ASSIGN, offset, ConstantInt::get (getContext (),0));
    code.push_imop(i);
    jump_out->setJumpDest (st->label (i));

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

/******************************************************************
  TreeNodeExprReshape
******************************************************************/

CGResult TreeNodeExprReshape::codeGenWith (CodeGen &cg) {
    return cg.cgExprReshape (this);
}

CGResult CodeGen::cgExprReshape (TreeNodeExprReshape *e) {
    // Type check:
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    // Evaluate subexpression:
    TreeNodeExpr* eArg = e->reshapee ();
    CGResult result (codeGen (eArg));
    if (result.isNotOk ()) {
        return result;
    }

    Symbol* rhs = result.symbol ();
    SymbolSymbol* resSym = generateResultSymbol (result, e);

    { // Eval subexpressions and copy dimensionalities:
        TreeNode::ChildrenListConstIterator it = e->children ().begin () + 1;
        dim_iterator dimIt = dim_begin (resSym);
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
    allocResult (result);

    if (!eArg->resultType ()->isScalar()) {
        // Check that new and old sizes are equal:
        Symbol* sizeSymbol = static_cast<SymbolSymbol*>(rhs)->getSizeSym ();
        Imop* jmp = new Imop (e, Imop::JE, (Symbol*) 0, sizeSymbol, resSym->getSizeSym ());
        pushImopAfter (result, jmp);
        result.addToNextList (jmp);
        std::stringstream ss;
        ss << "ERROR: Mismatching sizes in reshape at " << e->location ();
        Imop* err = newError (e, ConstantString::get (getContext (), ss.str ()));
        code.push_imop (err);
    }
    else {
        // Convert scalar to constant array:
        Symbol* tmp = rhs;
        rhs = st->appendTemporary (TypeNonVoid::get (getContext (),
            eArg->resultType ()->secrecSecType (),
            eArg->resultType ()->secrecDataType (),
            e->resultType ()->secrecDimType ()));
        Imop* i = new Imop (e, Imop::ALLOC, rhs, tmp, resSym->getSizeSym ());
        code.push_imop (i);
    }

    // Copy result:
    Imop* i = new Imop (e, Imop::ASSIGN, resSym, rhs, resSym->getSizeSym ());
    pushImopAfter (result, i);
    return result;
}

/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/

CGResult TreeNodeExprBinary::codeGenWith (CodeGen &cg) {
    return cg.cgExprBinary (this);
}

CGResult CodeGen::cgExprBinary (TreeNodeExprBinary *e) {
    // Type check:
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    TreeNodeExpr* eArg1 = e->leftExpression ();
    TreeNodeExpr* eArg2 = e->rightExpression ();

    /*
      If first sub-expression is public, then generate short-circuit code for
      logical && and logical ||.
    */
    if (   eArg1->resultType ()->secrecSecType ()->isPublic ()
        && eArg1->resultType ()->isScalar ()
        && eArg2->resultType ()->isScalar ()
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
    if (eArg1->resultType ()->secrecDimType () > eArg2->resultType ()->secrecDimType ()) {
        SymbolSymbol* tmpe1 = static_cast<SymbolSymbol*>(e1result);
        SymbolSymbol* tmpe2 = st->appendTemporary (static_cast<TypeNonVoid*> (eArg1->resultType ()));
        tmpe2->inheritShape (tmpe1);
        e1result = tmpe1;
        e2result = tmpe2;
        Imop* i = new Imop (e, Imop::ALLOC, e2result, arg2Result.symbol (), tmpe1->getSizeSym ());
        pushImopAfter (result, i);
    }
    else
    if (eArg2->resultType ()->secrecDimType () > eArg1->resultType ()->secrecDimType ()) {
        SymbolSymbol* tmpe1 = st->appendTemporary (static_cast<TypeNonVoid*> (eArg2->resultType ()));
        SymbolSymbol* tmpe2 = static_cast<SymbolSymbol*>(e2result);
        tmpe1->inheritShape (tmpe2);
        e1result = tmpe1;
        e2result = tmpe2;
        Imop* i = new Imop (e, Imop::ALLOC, e1result, arg1Result.symbol (), tmpe2->getSizeSym ());
        pushImopAfter (result, i);
    }
    else {
        std::stringstream ss;
        ss << "Mismaching shapes in addition at " << e->location();
        Imop* err = newError (e, ConstantString::get (getContext (), ss.str ()));
        SymbolLabel* errLabel = st->label (err);
        dim_iterator
                di = dim_begin (e1result),
                dj = dim_begin (e2result),
                de = dim_end (e1result);
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

    SymbolSymbol* resSym = generateResultSymbol (result, e);
    resSym->inheritShape(e1result);

    // Generate code for binary expression:
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

    allocResult (result);
    Imop* i = newBinary (e, iType, resSym, e1result, e2result);
    pushImopAfter (result, i);

    return result;
}

CGBranchResult TreeNodeExprBinary::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprBinary (this);
}

CGBranchResult CodeGen::cgBoolExprBinary (TreeNodeExprBinary *e) {
    typedef TypeNonVoid TNV;


    TreeNodeExpr *eArg1 = e->leftExpression ();
    TreeNodeExpr *eArg2 = e->rightExpression ();
    CGBranchResult result;

    switch (e->type()) {
        case NODE_EXPR_BINARY_LAND: // fall through
        case NODE_EXPR_BINARY_LOR:
            assert(!eArg1->resultType ()->isVoid());
            assert(dynamic_cast<TNV*>(eArg1->resultType()) != 0);

            /*
              If first sub-expression is public, then generate short-circuit
              code for logical && and logical ||.
            */
            if (static_cast<TNV*>(eArg1->resultType())->secrecSecType()->isPublic ()) {
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
                    result.patchTrueList (st->label(arg2Result.firstImop ()));
                    result.setTrueList (arg2Result.trueList ());
                    result.addToFalseList (arg2Result.falseList ());
                } else {
                    assert (e->type() == NODE_EXPR_BINARY_LOR);
                    result.patchFalseList (st->label (arg2Result.firstImop()));
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
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    SymbolSymbol* r = generateResultSymbol (result, e);
    std::list<Symbol*> argList, retList;

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

        Symbol* sym = argResult.symbol ();
        argList.push_back (sym);
        argList.insert (argList.end (), dim_begin (sym), dim_end (sym));
    }

    // prep return values:
    if (!e->resultType ()->isVoid ()) {
        retList.insert (retList.end (), dim_begin (r), dim_end (r));
        retList.push_back (r);
    }

    Imop* i = newCall (e, retList.begin (), retList.end (), argList.begin (), argList.end ());
    Imop *c = new Imop (e, Imop::RETCLEAN, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
    m_callsTo[e->symbolProcedure ()->decl ()].insert (i);

    c->setArg2 (st->label (i));
    pushImopAfter (result, i);
    code.push_imop (c);

    if (!e->resultType ()->isVoid ()) {
        codeGenSize (result);
    }

    return result;
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

/*******************************************************************************
  TreeNodeExprRVariable
*******************************************************************************/

CGResult TreeNodeExprRVariable::codeGenWith (CodeGen &cg) {
    return cg.cgExprRVariable (this);
}

CGResult CodeGen::cgExprRVariable (TreeNodeExprRVariable *e) {
    // Type check:
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    SymbolSymbol* sym = m_tyChecker.getSymbol (e->identifier ());
    CGResult result;
    result.setResult (sym);
    return result;
}

CGBranchResult TreeNodeExprRVariable::codeGenBoolWith (CodeGen &cg) {
    return cg.cgBoolExprRVariable (this);
}

CGBranchResult CodeGen::cgBoolExprRVariable (TreeNodeExprRVariable *e) {
    CGBranchResult result;
    SymbolSymbol* sym = m_tyChecker.getSymbol (e->identifier ());
    Imop *i = new Imop (e, Imop::JT, 0, sym);
    code.push_imop (i);
    result.setFirstImop (i);
    result.addToTrueList (i);
    i = new Imop (e, Imop::JUMP, 0);
    code.push_imop (i);
    result.addToFalseList (i);
    return result;
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
    ICode::Status status = m_tyChecker.visit (e);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    result.setResult (ConstantString::get (getContext (), e->value ()));
    return result;
}

/*******************************************************************************
  TreeNodeExprTernary
*******************************************************************************/

CGResult TreeNodeExprTernary::codeGenWith (CodeGen &cg) {
    return cg.cgExprTernary (this);
}

CGResult CodeGen::cgExprTernary (TreeNodeExprTernary *e) {
    // Type check:
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    TreeNodeExpr *e1 = e->conditional ();
    TreeNodeExpr *e2 = e->trueBranch ();
    TreeNodeExpr *e3 = e->falseBranch ();

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

        if (!e->resultType ()->isVoid ()) {
            if (!eTrueResult.symbol ()->secrecType ()->isScalar ()) {
                SymbolSymbol* resultSymbol = static_cast<SymbolSymbol*>(result.symbol ());
                resultSymbol->inheritShape (eTrueResult.symbol ());
                Imop* i = new Imop (e, Imop::ALLOC, resultSymbol,
                                    defaultConstant (getContext (), eTrueResult.symbol ()->secrecType ()->secrecDataType ()),
                                    resultSymbol->getSizeSym ());
                pushImopAfter (eTrueResult, i);
            }

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

        if (!e->resultType ()->isVoid ()) {
            if (!eFalseResult.symbol ()->secrecType ()->isScalar ()) {
                SymbolSymbol* resultSymbol = static_cast<SymbolSymbol*>(result.symbol ());
                resultSymbol->inheritShape (eFalseResult.symbol ());
                Imop* i = new Imop (e, Imop::ALLOC, resultSymbol,
                                    defaultConstant (getContext (), eFalseResult.symbol ()->secrecType ()->secrecDataType ()),
                                    resultSymbol->getSizeSym ());
                pushImopAfter (eFalseResult, i);
            }

            Imop* i = newAssign (e, result.symbol (), eFalseResult.symbol ());
            pushImopAfter (eFalseResult, i);
        }        

        // Link boolean expression code to the rest of the code:
        e1Result.patchTrueList (st->label (eTrueResult.firstImop ()));
        e1Result.patchFalseList (st->label (eFalseResult.firstImop ()));
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
        SymbolSymbol* resSym = generateResultSymbol (result, e);
        resSym->inheritShape (e1Result.symbol ());
        allocResult (result);

        // check that shapes match
        std::stringstream ss;
        ss << "Mismatching shapes at " << e->location();
        Imop* jmp = new Imop (e, Imop::JUMP, (Symbol*) 0);
        Imop* err = newError (e, ConstantString::get (getContext (), ss.str ()));
        SymbolLabel* errLabel = st->label(err);
        dim_iterator
                di = dim_begin (e1Result.symbol ()),
                dj = dim_begin (e2Result.symbol ()),
                dk = dim_begin (e3Result.symbol ()),
                de = dim_end (e1Result.symbol ());
        for (; di != de; ++ di, ++ dj, ++ dk) {
            Imop* i = new Imop (e, Imop::JNE, (Symbol*) 0, *di, *dj);
            pushImopAfter (result, i);
            i->setJumpDest(errLabel);

            i = new Imop (e, Imop::JNE, (Symbol*) 0, *dj, *dk);
            code.push_imop (i);
            i->setJumpDest (errLabel);
        }

        result.patchNextList (st->label (jmp));
        code.push_imop(jmp);
        code.push_imop(err);

        // loop to set all values of resulting array

        // Set up some temporary scalars:
        Context& cxt = getContext ();
        Symbol* counter = st->appendTemporary(TypeNonVoid::get (cxt, DATATYPE_INT));
        Symbol* b = st->appendTemporary(TypeNonVoid::get (cxt,
            e1->resultType ()->secrecSecType (), e1->resultType ()->secrecDataType ()));
        Symbol* t = st->appendTemporary(TypeNonVoid::get (cxt,
            e->resultType ()->secrecSecType (), e->resultType ()->secrecDataType ()));

        // r = e1
        Imop* i = newAssign (e, resSym, e2Result.symbol ());
        code.push_imop (i);
        jmp->setJumpDest (st->label(i));

        // counter = 0
        i = new Imop (e, Imop::ASSIGN, counter, ConstantInt::get (getContext (), 0));
        code.push_imop (i);

        // L0: if (counter >= size) goto next;
        Imop* jge = new Imop (e, Imop::JGE, (Symbol*) 0, counter, resSym->getSizeSym ());
        code.push_imop (jge);
        result.addToNextList (jge);

        // b = e1[counter]
        i = new Imop (e, Imop::LOAD, b, e1Result.symbol (), counter);
        code.push_imop (i);

        Imop* t0 = new Imop (e, Imop::ADD, counter, counter, ConstantInt::get (getContext (), 1));
        Imop* t1 = new Imop (e, Imop::STORE, resSym, counter, t);

        // if b goto T0
        i = new Imop (e, Imop::JT, (Symbol*) 0, b);
        code.push_imop(i);
        i->setJumpDest (st->label(t0));

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
        i->setJumpDest (st->label (jge));
    }

    return result;
}

CGBranchResult TreeNodeExprTernary::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprTernary (this);
}

CGBranchResult CodeGen::cgBoolExprTernary (TreeNodeExprTernary *e) {

    CGBranchResult result = codeGenBranch ( e->conditional ());
    if (result.isNotOk ()) {
        return result;
    }

    // Generate code for first value child expression:
    const CGBranchResult& trueResult = codeGenBranch (e->trueBranch ());
    if (trueResult.isNotOk ()) {
        return trueResult;
    }

    // Generate code for second value child expression:
    const CGBranchResult& falseResult = codeGenBranch (e->falseBranch ());
    if (falseResult.isNotOk ()) {
        return falseResult;
    }

    // Link conditional expression code to the rest of the code:
    result.patchTrueList (st->label (trueResult.firstImop ()));
    result.patchFalseList (st->label (falseResult.firstImop ()));

    result.addToTrueList (trueResult.trueList ());
    result.addToTrueList (falseResult.trueList ());
    result.addToFalseList (trueResult.falseList ());
    result.addToFalseList (falseResult.falseList ());

    return result;
}

/*******************************************************************************
  TreeNodeExprInt
*******************************************************************************/

CGResult TreeNodeExprInt::codeGenWith (CodeGen &cg) {
    return cg.cgExprInt (this);
}

CGResult CodeGen::cgExprInt (TreeNodeExprInt *e) {
    // Type check:
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    result.setResult (ConstantInt::get (getContext (), e->value ()));
    return result;
}

/*******************************************************************************
  TreeNodeExprUInt
*******************************************************************************/

CGResult TreeNodeExprUInt::codeGenWith (CodeGen &cg) {
    return cg.cgExprUInt (this);
}

CGResult CodeGen::cgExprUInt (TreeNodeExprUInt *e) {
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    result.setResult (ConstantUInt::get (getContext (), e->value ()));
    return result;
}

/*******************************************************************************
  TreeNodeExprBool
*******************************************************************************/

CGResult TreeNodeExprBool::codeGenWith (CodeGen &cg) {
    return cg.cgExprBool (this);
}

CGResult CodeGen::cgExprBool (TreeNodeExprBool *e) {
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    CGResult result;
    Context& cxt = getContext ();
    result.setResult (ConstantBool::get (cxt, e->value ()));
    return result;
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

/*******************************************************************************
  TreeNodeExprClassify
*******************************************************************************/

CGResult TreeNodeExprClassify::codeGenWith (CodeGen &cg) {
    return cg.cgExprClassify (this);
}

CGResult CodeGen::cgExprClassify (TreeNodeExprClassify *e) {
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    // Generate code for child expression
    TreeNodeExpr* eArg = e->expression ();
    CGResult result (codeGen (eArg));
    if (result.isNotOk ()) {
        return result;
    }

    // Generate temporary for the result of the classification, if needed:
    Symbol* argSym = result.symbol ();
    SymbolSymbol* resSym = generateResultSymbol (result, e);
    resSym->inheritShape (argSym);
    allocResult (result);

    Imop *i = newUnary (m_node, Imop::CLASSIFY, resSym, argSym);
    pushImopAfter (result, i);
    return result;
}

/*******************************************************************************
  TreeNodeExprDeclassify
*******************************************************************************/

CGResult TreeNodeExprDeclassify::codeGenWith (CodeGen &cg) {
    return cg.cgExprDeclassify (this);
}

CGResult CodeGen::cgExprDeclassify (TreeNodeExprDeclassify *e) {
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    // Generate code for child expression
    CGResult result (codeGen (e->expression ()));
    if (result.isNotOk ()) {
        return result;
    }

    // Generate temporary for the result of the classification, if needed:
    Symbol* argSym = result.symbol ();
    SymbolSymbol* resSym = generateResultSymbol (result, e);
    resSym->inheritShape (argSym);
    allocResult (result);

    Imop *i = newUnary (e, Imop::DECLASSIFY, resSym, argSym);
    pushImopAfter (result, i);

    return result;
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

/*******************************************************************************
  TreeNodeExprUnary
*******************************************************************************/

CGResult TreeNodeExprUnary::codeGenWith (CodeGen &cg) {
    return cg.cgExprUnary (this);
}

CGResult CodeGen::cgExprUnary (TreeNodeExprUnary *e) {
    // Type check:
    ICode::Status s = m_tyChecker.visit (e);
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
    SymbolSymbol* resSym = generateResultSymbol (result, e);
    resSym->inheritShape (eResult); // no need to copy the symbols
    allocResult (result);

    // Generate code for unary expression:
    Imop::Type iType = (e->type() == NODE_EXPR_UNEG) ? Imop::UNEG : Imop::UMINUS;
    Imop *i = newUnary (m_node, iType, result.symbol (), eResult);
    pushImopAfter (result, i);
    return result;
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

/******************************************************************
  TreeNodeExprPrefix
******************************************************************/

CGResult TreeNodeExprPrefix::codeGenWith (CodeGen &cg) {
    assert (children ().size () == 1);
    TreeNode *c0 = children ().at (0);
    (void) c0;
    assert (c0 != 0);
    assert (1 <= c0->children ().size () && c0->children ().size () <= 2);
    assert (dynamic_cast<TreeNodeIdentifier*>(c0->children ().at (0)) != 0);
    return cg.cgExprPrefix (this);
}

CGResult CodeGen::cgExprPrefix (TreeNodeExprPrefix *e) {
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV;

    // Type check:
    CGResult result;
    ICode::Status status = m_tyChecker.visit (e);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    TypeNonVoid* pubIntTy = TypeNonVoid::get (getContext (), DATATYPE_INT);

    // Generate code for child expression:
    TreeNode* lval = e->children ().at (0);
    TreeNodeIdentifier* e1 = static_cast<TreeNodeIdentifier*>(lval->children ().at (0));
    Symbol *destSym = st->find (e1->value ());
    assert (destSym->symbolType() == Symbol::SYMBOL);
    assert (dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol* destSymSym = static_cast<SymbolSymbol*> (destSym);
    result.setResult (destSymSym);

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
        assert (!e->resultType ()->isScalar ());
        CodeGenSubscript subInfo (*this);
        append (result, subInfo.codeGenSubscript (destSym, lval->children ().at (1)));
        if (result.isNotOk ()) {
            return result;
        }

        const SPV& spv = subInfo.spv ();
        CodeGenStride stride (*this);
        append (result, stride.codeGenStride (destSym));
        if (result.isNotOk ()) {
            return result;
        }

        // Initialize required temporary symbols:
        std::vector<Symbol* > indices;
        Symbol* offset = st->appendTemporary(pubIntTy);
        Symbol* tmpResult = st->appendTemporary(pubIntTy);
        Symbol* tmpValue = st->appendTemporary (pubIntTy);
        for (SPV::const_iterator it (spv.begin ()); it != spv.end (); ++ it) {
            Symbol* sym = st->appendTemporary(pubIntTy);
            indices.push_back(sym);
        }

        std::vector<Symbol*>::const_iterator idxIt;
        CodeGenLoop loop (*this);

        append (result, loop.enterLoop (spv, indices));
        if (result.isNotOk ()) {
            return result;
        }

        // compute offset:
        Imop* i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (), 0));
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
        i = new Imop (e, iType, tmpValue, tmpValue, ConstantInt::get (getContext (), 1));
        code.push_imop (i);

        // x[offset] = t
        i = new Imop (e, Imop::STORE, destSymSym, offset, tmpValue);
        code.push_imop (i);

        append (result, loop.exitLoop (indices));
        return result;
    }

    Symbol* one = ConstantInt::get (getContext (), 1);
    if (!e->resultType ()->isScalar ()) {
        one = st->appendTemporary (static_cast<TypeNonVoid*> (e->resultType ()));
        Imop* i = new Imop (e, Imop::ALLOC, one, ConstantInt::get (getContext (), 1), destSymSym->getSizeSym ());
        pushImopAfter (result, i);
    }

    // x = x `iType` 1
    Imop* i = newBinary (e, iType, destSymSym, destSymSym, one);
    pushImopAfter (result, i);
    return result;
}

/******************************************************************
  TreeNodeExprPostfix
******************************************************************/

CGResult TreeNodeExprPostfix::codeGenWith (CodeGen &cg) {
    assert (children ().size () == 1);
    TreeNode *c0 = children ().at (0);
    (void) c0;
    assert (c0 != 0);
    assert (1 <= c0->children ().size () && c0->children ().size () <= 2);
    assert (dynamic_cast<TreeNodeIdentifier*>(c0->children ().at (0)) != 0);
    return cg.cgExprPostfix (this);
}

CGResult CodeGen::cgExprPostfix (TreeNodeExprPostfix *e) {
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV;

    // Type check:
    CGResult result;
    ICode::Status status = m_tyChecker.visit (e);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    TypeNonVoid* pubIntTy = TypeNonVoid::get (getContext (), DATATYPE_INT);

    // Generate code for child expression:
    TreeNode* lval = e->children ().at (0);
    TreeNodeIdentifier* e1 = static_cast<TreeNodeIdentifier*>(lval->children ().at (0));
    Symbol *destSym = st->find (e1->value ());
    assert (destSym->symbolType() == Symbol::SYMBOL);
    assert (dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol* destSymSym = static_cast<SymbolSymbol*> (destSym);

    // r = x
    SymbolSymbol* r = generateResultSymbol (result, e);
    copyShapeFrom (result, destSymSym);
    allocResult (result);
    Imop* i = newAssign (e, r, destSymSym);
    pushImopAfter (result, i);


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
        assert (!e->resultType ()->isScalar ());
        CodeGenSubscript subInfo (*this);
        append (result, subInfo.codeGenSubscript (destSym, lval->children ().at (1)));
        if (result.isNotOk ()) {
            return result;
        }

        const SPV& spv = subInfo.spv ();
        CodeGenStride stride (*this);
        append (result, stride.codeGenStride (destSym));
        if (result.isNotOk ()) {
            return result;
        }

        // Initialize required temporary symbols:
        std::vector<Symbol* > indices;
        Symbol* offset = st->appendTemporary(pubIntTy);
        Symbol* tmpResult = st->appendTemporary(pubIntTy);
        Symbol* tmpValue = st->appendTemporary (pubIntTy);
        for (SPV::const_iterator it (spv.begin ()); it != spv.end (); ++ it) {
            Symbol* sym = st->appendTemporary(pubIntTy);
            indices.push_back(sym);
        }

        std::vector<Symbol*>::const_iterator idxIt;

        CodeGenLoop loop (*this);

        append (result, loop.enterLoop (spv, indices));
        if (result.isNotOk ()) {
            return result;
        }

        // compute offset:
        i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (), 0));
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
        i = new Imop (e, iType, tmpValue, tmpValue, ConstantInt::get (getContext (), 1));
        code.push_imop (i);

        // x[offset] = t
        i = new Imop (e, Imop::STORE, destSymSym, offset, tmpValue);
        code.push_imop (i);

        append (result, loop.exitLoop (indices));
        return result;
    }

    Symbol* one = ConstantInt::get (getContext (), 1);
    if (!e->resultType ()->isScalar ()) {
        one = st->appendTemporary (static_cast<TypeNonVoid*> (e->resultType ()));
        Imop* i = new Imop (e, Imop::ALLOC, one, ConstantInt::get (getContext (), 1), destSymSym->getSizeSym ());
        pushImopAfter (result,i );
    }

    // x = x `iType` 1
    i = newBinary (e, iType, destSymSym, destSymSym, one);
    code.push_imop (i);

    return result;
}

} // namespace SecreC
