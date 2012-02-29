#include "codegen.h"
#include "treenode.h"

#include <stack>

#include <boost/foreach.hpp>

#include "symboltable.h"
#include "constant.h"
#include "misc.h"
#include "ModuleInfo.h"


/**
 * Code generation for expressions.
 */

namespace {

using namespace SecreC;

bool canShortCircuit (SecrecTreeNodeType nodeTy,
                      SecreC::Type* lhs, SecreC::Type* rhs) {
    return ((nodeTy == NODE_EXPR_BINARY_LAND || nodeTy == NODE_EXPR_BINARY_LOR) &&
            lhs->secrecSecType ()->isPublic () &&
            rhs->secrecSecType ()->isPublic () &&
            lhs->isScalar () && rhs->isScalar ());
}

}


namespace SecreC {

CGBranchResult CodeGen::cgBoolSimple (TreeNodeExpr *e) {
    CGBranchResult result (codeGen (e));
    if (result.isNotOk ()) {
        return result;
    }

    Imop *i = new Imop (e, Imop::JT, 0, result.symbol ());
    pushImopAfter (result, i);
    result.addToTrueList (i);

    i = new Imop (e, Imop::JUMP, 0);
    push_imop (i);
    result.addToFalseList (i);

    return result;
}

/******************************************************************
  TreeNodeExprCast
******************************************************************/

CGResult TreeNodeExprCast::codeGenWith (CodeGen &cg) {
    return cg.cgExprCast (this);
}

CGResult CodeGen::cgExprCast (TreeNodeExprCast *e) {
    CGResult result;
    ICode::Status status = m_tyChecker.visit (e);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    TreeNodeExpr* subExpr = e->expression ();
    const CGResult& subResult = codeGen (subExpr);
    append (result, subResult);
    Imop::Type iType = Imop::CAST;
    if (e->resultType ()->secrecDataType () ==
            subExpr->resultType ()->secrecDataType ()) {
        iType = Imop::ASSIGN;
    }
    SymbolSymbol* sym = generateResultSymbol (result, e->resultType ());
    if (subExpr->resultType ()->isScalar ()) {
        Imop* imop = new Imop (e, iType, sym, subResult.symbol ());
        pushImopAfter (result, imop);
    }
    else {
        copyShapeFrom (result, subResult.symbol ());
        allocResult (result);
        Imop* imop = new Imop (e, iType, sym, subResult.symbol (), sym->getSizeSym ());
        pushImopAfter (result, imop);
    }

    return result;
}

CGBranchResult TreeNodeExprCast::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolExprCast (this);
}

CGBranchResult CodeGen::cgBoolExprCast (TreeNodeExprCast *e) {
    return cgBoolSimple (e);
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

    SubscriptInfo subscript;
    append (result, codeGenSubscript (subscript, x, e->children ().at (1)));
    if (result.isNotOk ()) {
        return result;
    }

    const SubscriptInfo::SPV& spv = subscript.spv ();
    const SubscriptInfo::SliceIndices& slices = subscript.slices ();

    // 5. compute resulting shape
    {
        pushComment ("Computing shape:");
        unsigned count = 0;
        BOOST_FOREACH (unsigned k, subscript.slices ()) {
            Symbol* sym = resSym->getDim (count);
            Imop* i = new Imop (e, Imop::SUB, sym, spv[k].second, spv[k].first);
            pushImopAfter (result, i);
            ++ count;
        }

        codeGenSize (result);
    }

    // r = ALLOC def size
    if (!isScalar) {
        Symbol* def = defaultConstant (getContext (), e->resultType ()->secrecDataType ());
        Imop* i = 0;
        i = new Imop (e, Imop::ALLOC, resSym, def, resSym->getSizeSym ());
        result.addTempResource (resSym);
        pushImopAfter (result, i);
    }

    // 4. initialze required temporary symbols
    LoopInfo loopInfo;
    Context& cxt = getContext ();
    TypeNonVoid* pubIntTy = TypeNonVoid::getIndexType (cxt);
    for (SPV::const_iterator it(spv.begin()); it != spv.end(); ++ it) {
        Symbol* sym = m_st->appendTemporary(pubIntTy);
        loopInfo.push_index (sym);
    }

    Symbol* offset = m_st->appendTemporary(pubIntTy);
    Symbol* tmp_result = m_st->appendTemporary(TypeNonVoid::get (cxt,
        e->resultType ()->secrecSecType(), e->resultType ()->secrecDataType()));
    Symbol* tmp_result2 = m_st->appendTemporary(pubIntTy);

    // 3. initialize strides
    std::vector<ArrayStrideInfo > strides;
    strides.push_back (x);
    strides.push_back (resSym);
    append (result, codeGenStride (strides[0]));
    append (result, codeGenStride (strides[1]));
    if (result.isNotOk ()) {
        return result;
    }

    append (result, enterLoop (loopInfo, spv));
    if (result.isNotOk ()) {
        return result;
    }

    // 8. compute offset for RHS
    {
        // old_ffset = 0
        pushComment ("Compute offset:");
        Imop* i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (),0));
        push_imop(i);

        LoopInfo::iterator itIt = loopInfo.begin();
        LoopInfo::iterator itEnd = loopInfo.end();
        for (unsigned k = 0; itIt != itEnd; ++ k, ++ itIt) {
            // tmp_result2 = s[k] * idx[k]
            i = new Imop (e, Imop::MUL, tmp_result2, strides[0].at (k), *itIt);
            push_imop(i);

            // old_offset = old_offset + tmp_result2
            i = new Imop (e, Imop::ADD, offset, offset, tmp_result2);
            push_imop(i);
        }
    }

    // 9. load and store
    {
        pushComment("Load and store:");

        // tmp = x[old_offset] or r = x[old_offset] if scalar
        Imop* i = new Imop (e, Imop::LOAD, (isScalar ? resSym : tmp_result), x, offset);
        push_imop (i);

        // r[offset] = tmp if not scalar
        if (!isScalar) {
            i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (),0));
            push_imop (i);
            unsigned count = 0;
            BOOST_FOREACH (unsigned k, slices) {
                Symbol* idx = loopInfo.at (k);

                i = new Imop (e, Imop::SUB, tmp_result2, idx, spv[k].first);
                push_imop (i);

                i = new Imop (e, Imop::MUL, tmp_result2, tmp_result2, strides[1].at (count));
                push_imop (i);

                i = new Imop (e, Imop::ADD, offset, offset, tmp_result2);
                push_imop (i);
                ++ count;
            }

            i = new Imop (e, Imop::STORE, resSym, offset, tmp_result);
            push_imop(i);
        }
    }

    append (result, exitLoop (loopInfo));
    return result;
}

CGBranchResult TreeNodeExprIndex::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolSimple (this);
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

    Symbol* n = ConstantInt::get (getContext (), eArg->resultType ()->secrecDimType());
    Imop* i = new Imop (e, Imop::ALLOC, resSym, ConstantInt::get (getContext (), 0), n);
    pushImopAfter (result, i);
    result.addTempResource (resSym);

    i = new Imop (e, Imop::ASSIGN, resSym->getDim (0), n);
    push_imop(i);

    unsigned count = 0;
    BOOST_FOREACH (Symbol* sizeSym, dim_range (argResult.symbol ())) {
        Symbol* indexSym = ConstantInt::get (getContext (), count);
        Imop* i = new Imop (e, Imop::STORE, resSym, indexSym, sizeSym);
        push_imop(i);
        ++ count;
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

    SecrecDimType k = e->dimensionality ()->value ();
    SecrecDimType n = e->resultType ()->secrecDimType();
    SymbolSymbol* arg1ResultSymbol = static_cast<SymbolSymbol*>(arg1Result.symbol ());
    SymbolSymbol* arg2ResultSymbol = static_cast<SymbolSymbol*>(arg2Result.symbol ());
    SymbolSymbol* resSym = static_cast<SymbolSymbol*>(result.symbol ());

    // Compute resulting shape and perform sanity check:
    std::stringstream ss;
    ss << "Different sized dimensions in concat at " << e->location() << ".";
    Imop* err = newError (e, ConstantString::get (getContext (), ss.str ()));
    SymbolLabel* errLabel = m_st->label(err);
    for (SecrecDimType it = 0; it < e->resultType ()->secrecDimType(); ++ it) {
        Symbol* s1 = arg1ResultSymbol->getDim(it);
        Symbol* s2 = arg2ResultSymbol->getDim(it);
        if (it == k) {
            Imop* i = new Imop (e, Imop::ADD, resSym->getDim(it), s1, s2);
            pushImopAfter (result, i);
        }
        else {
            Imop* i = new Imop (e, Imop::JNE, (Symbol*) 0, s1, s2);
            pushImopAfter (result, i);
            i->setJumpDest(errLabel);

            i = new Imop (e, Imop::ASSIGN, resSym->getDim(it), s1);
            push_imop(i);
        }
    }

    Imop* jmp = new Imop (e, Imop::JUMP, (Symbol*) 0);
    pushImopAfter (result, jmp);
    result.addToNextList (jmp);

    push_imop(err);

    // Initialize strides:
    std::vector<ArrayStrideInfo > strides;
    strides.push_back (arg1ResultSymbol);
    strides.push_back (arg2ResultSymbol);
    strides.push_back (resSym);
    for (unsigned i = 0; i < 3; ++ i) {
        append (result, codeGenStride (strides[i]));
        if (result.isNotOk ()) {
            return result;
        }
    }

    // Symbols for running indices:
    LoopInfo loopInfo;
    TypeNonVoid* pubIntTy = TypeNonVoid::getIndexType (getContext ());
    for (SecrecDimType it = 0; it < n; ++ it) {
        Symbol* sym = m_st->appendTemporary(pubIntTy);
        loopInfo.push_index (sym);
    }

    // Compute size and allocate resulting array:
    codeGenSize (result);
    if (result.isNotOk ()) {
        return result;
    }

    allocResult (result);

    append (result, enterLoop (loopInfo, resSym));

    Symbol* offset = m_st->appendTemporary(pubIntTy);
    Symbol* tmpInt = m_st->appendTemporary(pubIntTy);

    // j = 0 (right hand side index)
    Imop* i = new Imop(e, Imop::ASSIGN, offset, ConstantInt::get (getContext (),0));
    push_imop(i);

    // IF (i_k >= d_k) GOTO T1;
    i = new Imop(e, Imop::JGE, (Symbol*) 0, loopInfo.at (k), arg1ResultSymbol->getDim(k));
    push_imop(i);
    result.addToNextList (i);

    // compute j if i < d (for e1)
    for (unsigned count = 0; count < strides[0].size (); ++ count) {
        Imop* i = new Imop(e, Imop::MUL, tmpInt, strides[0].at (count), loopInfo.at (count));
        push_imop(i);

        i = new Imop(e, Imop::ADD, offset, offset, tmpInt);
        push_imop(i);
    }

    // t = x[j]
    TypeNonVoid* elemType = TypeNonVoid::get (getContext (),
        e->resultType ()->secrecSecType(), e->resultType ()->secrecDataType());
    Symbol* tmp_elem = m_st->appendTemporary(elemType);
    i = new Imop (e, Imop::LOAD, tmp_elem, arg1Result.symbol (), offset);
    push_imop(i);

    // jump out
    Imop* jump_out = new Imop(e, Imop::JUMP, (Symbol*) 0);
    push_imop (jump_out);

    // compute j if i >= d (for e2)
    for (SecrecDimType count = 0; static_cast<size_t>(count) < strides[1].size (); ++ count) {
        if (count == k) {
            i = new Imop (e, Imop::SUB, tmpInt, loopInfo.at (count), arg1ResultSymbol->getDim(k));
            pushImopAfter (result, i);

            i = new Imop (e, Imop::MUL, tmpInt, strides[1].at (count), tmpInt);
            push_imop(i);
        }
        else {
            i = new Imop (e, Imop::MUL, tmpInt, strides[1].at (count), loopInfo.at (count));
            pushImopAfter (result, i);
        }

        i = new Imop (e, Imop::ADD, offset, offset, tmpInt);
        push_imop(i);
    }

    // t = y[j]
    i = new Imop (e, Imop::LOAD, tmp_elem, arg2Result.symbol (), offset);
    pushImopAfter (result, i);

    // out: r[i] = t
    i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (),0));
    push_imop(i);
    jump_out->setJumpDest (m_st->label (i));

    // compute j if i < d (for e1)
    for (unsigned count = 0; count != strides[2].size (); ++ count) {
        Imop* i = new Imop (e, Imop::MUL, tmpInt, strides[2].at (count), loopInfo.at (count));
        push_imop (i);

        i = new Imop (e, Imop::ADD, offset, offset, tmpInt);
        push_imop (i);
    }

    i = new Imop (e, Imop::STORE, resSym, offset, tmp_elem);
    push_imop(i);

    append (result, exitLoop (loopInfo));

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
    ScopedAllocations allocs (*this, result);

    { // Eval subexpressions and copy dimensionalities:
        dim_iterator dimIt = dim_begin (resSym);
        BOOST_FOREACH (TreeNode* _dim, e->dimensions ()) {
            assert (dimIt != dim_end (resSym));
            assert (dynamic_cast<TreeNodeExpr*>(_dim) != 0);
            TreeNodeExpr* dim = static_cast<TreeNodeExpr*>(_dim);
            const CGResult& argResult (codeGen (dim));
            append (result, argResult);
            if (result.isNotOk ()) {
                return result;
            }

            Imop* i = new Imop (e, Imop::ASSIGN, *dimIt, argResult.symbol ());
            pushImopAfter (result, i);
            ++ dimIt;
        }
    }

    // Compute new size:
    codeGenSize (result);

    if (!eArg->resultType ()->isScalar()) {
        assert (dynamic_cast<SymbolSymbol*>(rhs) != 0);
        // Check that new and old sizes are equal:
        Symbol* sizeSymbol = static_cast<SymbolSymbol*>(rhs)->getSizeSym ();
        Imop* jmp = new Imop (e, Imop::JE, (Symbol*) 0, sizeSymbol, resSym->getSizeSym ());
        pushImopAfter (result, jmp);
        result.addToNextList (jmp);
        std::stringstream ss;
        ss << "ERROR: Mismatching sizes in reshape at " << e->location () << ".";
        Imop* err = newError (e, ConstantString::get (getContext (), ss.str ()));
        push_imop (err);
    }
    else {
        // Convert scalar to constant array:
        Symbol* tmp = rhs;
        rhs = m_st->appendTemporary (TypeNonVoid::get (getContext (),
            eArg->resultType ()->secrecSecType (),
            eArg->resultType ()->secrecDataType (),
            e->resultType ()->secrecDimType ()));
        allocs.allocTemporary (rhs, tmp, resSym->getSizeSym ());
    }

    // Copy result:
    Imop* i = new Imop (e, Imop::COPY, resSym, rhs, resSym->getSizeSym ());
    pushImopAfter (result, i);
    m_allocs.push_back (resSym);
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

    if (e->isOverloaded ()) {
        std::vector<TreeNodeExpr* > params;
        params.push_back (eArg1);
        params.push_back (eArg2);
        return cgProcCall (e->procSymbol (), e->resultType (), params);
    }

    /*
      If first sub-expression is public, then generate short-circuit code for
      logical && and logical ||.
    */
    if (canShortCircuit (e->type (), eArg1->resultType (), eArg2->resultType ())) {
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
        push_imop (j);
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
    ScopedAllocations allocs (*this, result);

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
        SymbolSymbol* tmpe2 = m_st->appendTemporary (static_cast<TypeNonVoid*> (eArg1->resultType ()));
        tmpe2->inheritShape (tmpe1);
        e1result = tmpe1;
        e2result = tmpe2;
        allocs.allocTemporary (e2result, arg2Result.symbol (), tmpe1->getSizeSym ());
    }
    else
    if (eArg2->resultType ()->secrecDimType () > eArg1->resultType ()->secrecDimType ()) {
        SymbolSymbol* tmpe1 = m_st->appendTemporary (static_cast<TypeNonVoid*> (eArg2->resultType ()));
        SymbolSymbol* tmpe2 = static_cast<SymbolSymbol*>(e2result);
        tmpe1->inheritShape (tmpe2);
        e1result = tmpe1;
        e2result = tmpe2;
        allocs.allocTemporary (e1result, arg1Result.symbol (), tmpe2->getSizeSym ());
    }
    else {
        std::stringstream ss;
        ss << "Mismaching shapes in addition at " << e->location();
        Imop* err = newError (e, ConstantString::get (getContext (), ss.str ()));
        SymbolLabel* errLabel = m_st->label (err);
        dim_iterator dj = dim_begin (e2result);
        BOOST_FOREACH (Symbol* dim, dim_range (e1result)) {
            Imop* i = new Imop (e, Imop::JNE, (Symbol*) 0, dim, *dj);
            i->setJumpDest (errLabel);
            pushImopAfter (result, i);
            ++ dj;
        }

        jmp = new Imop(e, Imop::JUMP, (Symbol*) 0);
        pushImopAfter (result, jmp);
        result.addToNextList (jmp);
        push_imop (err);
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
            m_log.fatal() << "Binary " << e->operatorString ()
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
    assert (havePublicBoolType ());
    return cg.cgBoolExprBinary (this);
}

CGBranchResult CodeGen::cgBoolExprBinary (TreeNodeExprBinary *e) {
    typedef TypeNonVoid TNV;

    if (e->isOverloaded ()) {
        return cgBoolSimple (e);
    }


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
                    result.patchTrueList (m_st->label(arg2Result.firstImop ()));
                    result.setTrueList (arg2Result.trueList ());
                    result.addToFalseList (arg2Result.falseList ());
                } else {
                    assert (e->type() == NODE_EXPR_BINARY_LOR);
                    result.patchFalseList (m_st->label (arg2Result.firstImop()));
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
                push_imop(j1);
                pushImopAfter (result, j1);
                push_imop(j2);

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
            push_imop (fj);
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

CGResult CodeGen::cgProcCall (SymbolProcedure* symProc,
                              SecreC::Type* returnType,
                              const std::vector<TreeNodeExpr*>& args)
{
    CGResult result;
    SymbolSymbol* r = generateResultSymbol (result, returnType);
    std::list<Symbol*> argList, retList;

    // Initialize arguments:
    BOOST_FOREACH (TreeNodeExpr* arg, args) {
        const CGResult& argResult (codeGen (arg));
        append (result, argResult);
        if (result.isNotOk ()) {
           return result;
        }

        Symbol* sym = argResult.symbol ();
        argList.push_back (sym);
        argList.insert (argList.end (), dim_begin (sym), dim_end (sym));
    }

    // prep return values:
    if (!returnType->isVoid ()) {
        retList.insert (retList.end (), dim_begin (r), dim_end (r));
        retList.push_back (r);
    }

    Imop* i = newCall (m_node, retList.begin (), retList.end (), argList.begin (), argList.end ());
    Imop* c = new Imop (m_node, Imop::RETCLEAN, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
    m_callsTo[symProc->decl ()].insert (i);

    c->setArg2 (m_st->label (i));
    pushImopAfter (result, i);
    push_imop (c);

    if (! returnType->isVoid ()) {
        codeGenSize (result);
        if (! returnType->isScalar ()) {
            result.addTempResource (result.symbol ());
        }
    }

    return result;
}

CGResult CodeGen::cgExprProcCall (TreeNodeExprProcCall *e) {
    typedef TreeNode::ChildrenListConstIterator CLCI;

    // Type check:
    ICode::Status s = m_tyChecker.visit (e);
    if (s != ICode::OK) {
        return CGResult (s);
    }

    std::vector<TreeNodeExpr* > args;
    BOOST_FOREACH (TreeNode* _arg, e->paramRange ()) {
        assert ((_arg->type() & NODE_EXPR_MASK) != 0x0);
        assert (dynamic_cast<TreeNodeExpr*> (_arg) != 0);
        args.push_back (static_cast<TreeNodeExpr*> (_arg));
    }

    return cgProcCall (e->symbolProcedure (), e->resultType (), args);
}

CGBranchResult TreeNodeExprProcCall::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType());
    return cg.cgBoolSimple (this);
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
    push_imop (i);
    result.setFirstImop (i);
    result.addToTrueList (i);
    i = new Imop (e, Imop::JUMP, 0);
    push_imop (i);
    result.addToFalseList (i);
    return result;
}

/*******************************************************************************
  TreeNodeExprDomainID
*******************************************************************************/

CGResult TreeNodeExprDomainID::codeGenWith (CodeGen& cg) {
    return cg.cgExprDomainID (this);
}

CGResult CodeGen::cgExprDomainID (TreeNodeExprDomainID* e) {
    CGResult result;
    ICode::Status status = m_tyChecker.visit (e);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    assert (dynamic_cast<TypeNonVoid*>(e->resultType ()) != 0);
    TypeNonVoid* resultType = static_cast<TypeNonVoid*>(e->resultType ());
    SymbolSymbol* t = m_st->appendTemporary (resultType);
    Symbol* s = m_st->find (e->securityType ()->identifier ()->value ());
    if (s == 0 || s->symbolType () != Symbol::PDOMAIN) {
        assert (false && "ICE: Type checker must guarantee that!");
        result.setStatus (ICode::E_TYPE);
        return result;
    }

    Imop* i = new Imop (e, Imop::DOMAINID, t, static_cast<SymbolDomain*>(s));
    result.setResult (t);
    pushImopAfter (result, i);
    return result;
}

/*******************************************************************************
  TreeNodeExprQualified
*******************************************************************************/

CGResult TreeNodeExprQualified::codeGenWith (CodeGen& cg) {
    return cg.cgExprQualified (this);
}

CGResult CodeGen::cgExprQualified (TreeNodeExprQualified* e) {
    CGResult result;

    ICode::Status status = m_tyChecker.visit (e);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    return codeGen (e->expression ());
}

CGBranchResult TreeNodeExprQualified::codeGenBoolWith (CodeGen& cg) {
    assert (havePublicBoolType ());
    return cg.cgBoolExprQualified (this);
}

CGBranchResult CodeGen::cgBoolExprQualified (TreeNodeExprQualified *e) {
    return codeGenBranch (e->expression ());
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

        Symbol* needToFree = 0;

        if (!e->resultType ()->isVoid ()) {
            if (!eTrueResult.symbol ()->secrecType ()->isScalar ()) {
                SymbolSymbol* resultSymbol = static_cast<SymbolSymbol*>(result.symbol ());
                resultSymbol->inheritShape (eTrueResult.symbol ());
                Imop* i = new Imop (e, Imop::ALLOC, resultSymbol,
                                    defaultConstant (getContext (), eTrueResult.symbol ()->secrecType ()->secrecDataType ()),
                                    resultSymbol->getSizeSym ());
                pushImopAfter (eTrueResult, i);
                needToFree = resultSymbol;
            }

            Imop* i = newAssign (e, result.symbol (), eTrueResult.symbol ());
            pushImopAfter (eTrueResult, i);
            releaseTempAllocs (eTrueResult);
        }

        result.patchFirstImop (eTrueResult.firstImop ());

        // Jump out of the ternary construct:
        Imop* i = new Imop (e, Imop::JUMP, 0);
        result.addToNextList (i);
        result.patchFirstImop (i);
        push_imop (i);

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
                needToFree = resultSymbol;
            }

            Imop* i = newAssign (e, result.symbol (), eFalseResult.symbol ());
            pushImopAfter (eFalseResult, i);
            releaseTempAllocs (eFalseResult);
        }

        if (needToFree) {
            result.addTempResource (needToFree);
        }

        // Link boolean expression code to the rest of the code:
        e1Result.patchTrueList (m_st->label (eTrueResult.firstImop ()));
        e1Result.patchFalseList (m_st->label (eFalseResult.firstImop ()));
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
        SymbolLabel* errLabel = m_st->label(err);
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
            push_imop (i);
            i->setJumpDest (errLabel);
        }

        result.patchNextList (m_st->label (jmp));
        push_imop(jmp);
        push_imop(err);

        // loop to set all values of resulting array

        // Set up some temporary scalars:
        Context& cxt = getContext ();
        Symbol* counter = m_st->appendTemporary(TypeNonVoid::getIndexType (cxt));
        Symbol* b = m_st->appendTemporary(TypeNonVoid::get (cxt,
            e1->resultType ()->secrecSecType (), e1->resultType ()->secrecDataType ()));
        Symbol* t = m_st->appendTemporary(TypeNonVoid::get (cxt,
            e->resultType ()->secrecSecType (), e->resultType ()->secrecDataType ()));

        // r = e1
        Imop* i = newAssign (e, resSym, e2Result.symbol ());
        push_imop (i);
        jmp->setJumpDest (m_st->label(i));

        // counter = 0
        i = new Imop (e, Imop::ASSIGN, counter, ConstantInt::get (getContext (), 0));
        push_imop (i);

        // L0: if (counter >= size) goto next;
        Imop* jge = new Imop (e, Imop::JGE, (Symbol*) 0, counter, resSym->getSizeSym ());
        push_imop (jge);
        result.addToNextList (jge);

        // b = e1[counter]
        i = new Imop (e, Imop::LOAD, b, e1Result.symbol (), counter);
        push_imop (i);

        Imop* t0 = new Imop (e, Imop::ADD, counter, counter, ConstantInt::get (getContext (), 1));
        Imop* t1 = new Imop (e, Imop::STORE, resSym, counter, t);

        // if b goto T0
        i = new Imop (e, Imop::JT, (Symbol*) 0, b);
        push_imop(i);
        i->setJumpDest (m_st->label(t0));

        // t = e3[counter]
        // T1: result[counter] = t
        i = new Imop (e, Imop::LOAD, t, e3Result.symbol (), counter);
        push_imop (i);
        push_imop (t1);

        // T0: counter = counter + 1
        push_imop (t0);

        // goto L0
        i = new Imop (e, Imop::JUMP, (Symbol*) 0);
        push_imop (i);
        i->setJumpDest (m_st->label (jge));
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
    result.patchTrueList (m_st->label (trueResult.firstImop ()));
    result.patchFalseList (m_st->label (falseResult.firstImop ()));

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
    result.setResult (
        numericConstant (getContext (),
            e->resultType ()->secrecDataType (), e->value ()));
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
    result.setResult (
        numericConstant (getContext (),
            e->resultType ()->secrecDataType (), e->value ()));
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
    Imop *i = new Imop(e, Imop::JUMP, 0);
    push_imop (i);
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

    Imop *i = newUnary (e, Imop::CLASSIFY, resSym, argSym);
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
    return cg.cgBoolSimple (this);
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

    if (e->isOverloaded ()) {
        std::vector<TreeNodeExpr* > params;
        params.push_back (e->expression ());
        return cgProcCall (e->procSymbol (), e->resultType (), params);
    }

    // Generate code for child expression:
    TreeNodeExpr *eArg = e->expression ();
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
    Imop *i = newUnary (e, iType, result.symbol (), eResult);
    pushImopAfter (result, i);
    return result;
}

CGBranchResult TreeNodeExprUnary::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType ());
    return cg.cgBoolExprUnary (this);
}

CGBranchResult CodeGen::cgBoolExprUnary (TreeNodeExprUnary *e) {
    // Generate code for child expression:
    if (e->isOverloaded ()) {
        return cgBoolSimple (e);
    }
    else {
        CGBranchResult result = codeGenBranch (e->expression ());
        if (!result.isOk ()) {
            return result;
        }

        result.swapTrueFalse ();
        return result;
    }
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

    TypeNonVoid* pubIntTy = TypeNonVoid::getIndexType (getContext ());

    // Generate code for child expression:
    Symbol* one = numericConstant (getContext (), e->resultType ()->secrecDataType (), 1);
    TreeNode* lval = e->children ().at (0);
    TreeNodeIdentifier* e1 = static_cast<TreeNodeIdentifier*>(lval->children ().at (0));
    Symbol *destSym = m_st->find (e1->value ());
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

    ScopedAllocations allocs (*this, result);

    // ++ x[e1,..,ek]
    if (lval->children().size() == 2) {
        assert (!e->resultType ()->isScalar ());
        SubscriptInfo subInfo;
        append (result, codeGenSubscript (subInfo, destSym, lval->children ().at (1)));
        if (result.isNotOk ()) {
            return result;
        }

        const SubscriptInfo::SPV& spv = subInfo.spv ();
        ArrayStrideInfo stride (destSym);
        append (result, codeGenStride (stride));
        if (result.isNotOk ()) {
            return result;
        }

        // Initialize required temporary symbols:
        LoopInfo loopInfo;
        Symbol* offset = m_st->appendTemporary(pubIntTy);
        TypeNonVoid* elemType = TypeNonVoid::get (getContext (),
            e->resultType ()->secrecSecType (),
            e->resultType ()->secrecDataType ());
        Symbol* tmpResult = m_st->appendTemporary(pubIntTy);
        Symbol* tmpElem = m_st->appendTemporary (elemType);
        for (SPV::const_iterator it (spv.begin ()); it != spv.end (); ++ it) {
            Symbol* sym = m_st->appendTemporary(pubIntTy);
            loopInfo.push_index (sym);
        }

        if (elemType->secrecSecType ()->isPrivate ()) {
            Symbol* t = m_st->appendTemporary (static_cast<TypeNonVoid*> (elemType));
            allocs.classifyTemporary (t, one);
            one = t;
        }

        append (result, enterLoop (loopInfo, spv));
        if (result.isNotOk ()) {
            return result;
        }

        // compute offset:
        Imop* i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (), 0));
        push_imop (i);

        LoopInfo::const_iterator idxIt = loopInfo.begin ();
        for (unsigned k = 0; k < stride.size (); ++ k, ++ idxIt) {
            i = new Imop (e, Imop::MUL, tmpResult, stride.at (k), *idxIt);
            push_imop (i);

            i = new Imop (e, Imop::ADD, offset, offset, tmpResult);
            push_imop (i);
        }

        // increment the value:

        // t = x[offset]
        i = new Imop (e, Imop::LOAD, tmpElem, destSymSym, offset);
        push_imop (i);

        // t = t + 1
        i = new Imop (e, iType, tmpElem, tmpElem, one);
        push_imop (i);

        // x[offset] = t
        i = new Imop (e, Imop::STORE, destSymSym, offset, tmpElem);
        push_imop (i);

        append (result, exitLoop (loopInfo));
        return result;
    }

    if (!e->resultType ()->isScalar ()) {
        SymbolSymbol* t = m_st->appendTemporary (static_cast<TypeNonVoid*>(e->resultType ()));
        t->inheritShape (destSymSym);
        allocs.allocTemporary (t, one,  destSymSym->getSizeSym ());
        one = t;
    }
    else
    if (e->resultType ()->secrecSecType ()->isPrivate ()) {
        SymbolSymbol* t = m_st->appendTemporary (static_cast<TypeNonVoid*> (e->resultType ()));
        allocs.classifyTemporary (t, one);
        one = t;
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
    typedef SubscriptInfo::SPV SPV;

    // Type check:
    CGResult result;
    ICode::Status status = m_tyChecker.visit (e);
    if (status != ICode::OK) {
        result.setStatus (status);
        return result;
    }

    TypeNonVoid* pubIntTy = TypeNonVoid::getIndexType (getContext ());

    // Generate code for child expression:
    TreeNode* lval = e->children ().at (0);
    TreeNodeIdentifier* e1 = static_cast<TreeNodeIdentifier*>(lval->children ().at (0));
    Symbol *destSym = m_st->find (e1->value ());
    assert (destSym->symbolType() == Symbol::SYMBOL);
    assert (dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol* destSymSym = static_cast<SymbolSymbol*> (destSym);
    Symbol* one = numericConstant (getContext (), e->resultType ()->secrecDataType (), 1);

    // r = x
    SymbolSymbol* r = generateResultSymbol (result, e);
    if (! destSymSym->secrecType ()->isScalar ()) {
        copyShapeFrom (result, destSymSym);
        Imop* i = new Imop (e, Imop::COPY, r, destSymSym, destSymSym->getSizeSym ());
        addAlloc (r);
        pushImopAfter (result, i);
    }
    else {
        Imop* i = newAssign (e, r, destSymSym);
        pushImopAfter (result, i);
    }

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

    ScopedAllocations allocs (*this, result);

    // x[e1,..,ek] ++
    if (lval->children().size() == 2) {
        assert (!e->resultType ()->isScalar ());
        SubscriptInfo subInfo;
        append (result, codeGenSubscript (subInfo, destSym, lval->children ().at (1)));
        if (result.isNotOk ()) {
            return result;
        }

        const SPV& spv = subInfo.spv ();
        ArrayStrideInfo stride (destSym);
        append (result, codeGenStride (stride));
        if (result.isNotOk ()) {
            return result;
        }

        // Initialize required temporary symbols:
        LoopInfo loopInfo;
        TypeNonVoid* elemType = TypeNonVoid::get (getContext (),
            e->resultType ()->secrecSecType (),
            e->resultType ()->secrecDataType ());
        Symbol* offset = m_st->appendTemporary(pubIntTy);
        Symbol* tmpResult = m_st->appendTemporary(pubIntTy);
        Symbol* tmpElem = m_st->appendTemporary (elemType);
        for (SPV::const_iterator it (spv.begin ()); it != spv.end (); ++ it) {
            Symbol* sym = m_st->appendTemporary(pubIntTy);
            loopInfo.push_index (sym);
        }

        if (elemType->secrecSecType ()->isPrivate ()) {
            Symbol* t = m_st->appendTemporary (static_cast<TypeNonVoid*> (elemType));
            allocs.classifyTemporary (t, one);
            one = t;
        }

        append (result, enterLoop (loopInfo, spv));
        if (result.isNotOk ()) {
            return result;
        }

        // compute offset:
        Imop* i = new Imop (e, Imop::ASSIGN, offset, ConstantInt::get (getContext (), 0));
        push_imop (i);

        LoopInfo::const_iterator idxIt = loopInfo.begin ();
        for (unsigned k = 0; k < stride.size (); ++ k, ++ idxIt) {
            i = new Imop (e, Imop::MUL, tmpResult, stride.at (k), *idxIt);
            push_imop (i);

            i = new Imop (e, Imop::ADD, offset, offset, tmpResult);
            push_imop (i);
        }

        // increment the value:

        // t = x[offset]
        i = new Imop (e, Imop::LOAD, tmpElem, destSymSym, offset);
        push_imop (i);

        // t = t + 1
        i = new Imop (e, iType, tmpElem, tmpElem, one);
        push_imop (i);

        // x[offset] = t
        i = new Imop (e, Imop::STORE, destSymSym, offset, tmpElem);
        push_imop (i);

        append (result, exitLoop (loopInfo));
        return result;
    }

    // x ++

    if (!e->resultType ()->isScalar ()) {
        SymbolSymbol* t = m_st->appendTemporary (static_cast<TypeNonVoid*> (e->resultType ()));
        allocs.allocTemporary (t, one,  destSymSym->getSizeSym ());
        t->inheritShape (destSymSym);
        one = t;
    }
    else
    if (e->resultType ()->secrecSecType ()->isPrivate ()) {
        SymbolSymbol* t = m_st->appendTemporary (static_cast<TypeNonVoid*> (e->resultType ()));
        allocs.classifyTemporary (t, one);
        one = t;
   }

    // x = x `iType` 1
    Imop* i = newBinary (e, iType, destSymSym, destSymSym, one);
    push_imop (i);

    return result;
}

} // namespace SecreC
