/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "CodeGen.h"
#include "CodeGenResult.h"
#include "Constant.h"
#include "DataType.h"
#include "Log.h"
#include "Misc.h"
#include "ModuleInfo.h"
#include "SecurityType.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Types.h"


/**
 * Code generation for expressions.
 */

namespace SecreC {

namespace /* anonymous */ {

bool canShortCircuit(SecrecTreeNodeType nodeTy, const SecreC::Type * lhs, const SecreC::Type * rhs) {
    return ((nodeTy == NODE_EXPR_BINARY_LAND || nodeTy == NODE_EXPR_BINARY_LOR) &&
            lhs->secrecSecType()->isPublic() &&
            rhs->secrecSecType()->isPublic() &&
            lhs->isScalar() && rhs->isScalar());
}

bool getBinImopType(SecrecTreeNodeType type, Imop::Type& iType) {
    switch (type) {
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
    case NODE_EXPR_BITWISE_AND: iType = Imop::BAND; break;
    case NODE_EXPR_BITWISE_OR:  iType = Imop::BOR;  break;
    case NODE_EXPR_BITWISE_XOR: iType = Imop::XOR;  break;
    case NODE_EXPR_BINARY_SHL:  iType = Imop::SHL;  break;
    case NODE_EXPR_BINARY_SHR:  iType = Imop::SHR;  break;
    default:
        return false;
    }

    return true;
}

} // namespace anonymous

CGBranchResult CodeGen::cgBoolSimple(TreeNodeExpr * e) {
    CGBranchResult result(codeGen(e));
    if (result.isNotOk()) {
        return result;
    }

    auto i = new Imop(e, Imop::JT, nullptr, result.symbol());
    pushImopAfter(result, i);
    releaseTemporary(result, result.symbol());
    result.addToTrueList(i);

    i = new Imop(e, Imop::JUMP, nullptr);
    push_imop(i);
    result.addToFalseList(i);

    return result;
}

/******************************************************************
  TreeNodeExprNone
******************************************************************/

CGResult TreeNodeExprNone::codeGenWith (CodeGen&) {
    return CGResult ();
}

/******************************************************************
  TreeNodeExprCast
******************************************************************/

CGResult TreeNodeExprCast::codeGenWith(CodeGen & cg) {
    return cg.cgExprCast(this);
}

CGResult CodeGen::cgExprCast(TreeNodeExprCast * e) {
    if (m_tyChecker->visitExprCast(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    TreeNodeExpr * subExpr = e->expression();
    const CGResult & subResult = codeGen(subExpr);
    append(result, subResult);

    if (e->isOverloaded()) {
        SymbolProcedure* proc = e->procSymbol();
        std::vector<Symbol*> operands { subResult.symbol() };
        CGResult callRes(cgOverloadedExpr(e, e->resultType(), operands, proc));
        append(result, callRes);
        if (result.isNotOk())
            return result;
        result.setResult(callRes.symbol());
    }
    else {
        Imop::Type iType = Imop::CAST;
        if (e->resultType()->secrecDataType() ==
            subExpr->resultType()->secrecDataType()) {
            iType = Imop::ASSIGN;
        }
        SymbolSymbol * sym = generateResultSymbol(result, e->resultType());
        if (subExpr->resultType()->isScalar()) {
            emplaceImopAfter(result, e, Imop::DECLARE, sym);
            emplaceImop(e, iType, sym, subResult.symbol());
        }
        else {
            copyShapeFrom(result, subResult.symbol());
            allocTemporaryResult(result);
            emplaceImopAfter(result, e, iType, sym, subResult.symbol(), sym->getSizeSym());
        }

        releaseTemporary(result, subResult.symbol());
    }

    return result;
}

CGBranchResult TreeNodeExprCast::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolExprCast(this);
}

CGBranchResult CodeGen::cgBoolExprCast(TreeNodeExprCast * e) {
    return cgBoolSimple(e);
}

/******************************************************************
  TreeNodeExprIndex
******************************************************************/

CGResult TreeNodeExprIndex::codeGenWith(CodeGen & cg) {
    return cg.cgExprIndex(this);
}

CGResult CodeGen::cgExprIndex(TreeNodeExprIndex * e) {
    // Type check:
    if (m_tyChecker->visitExprIndex(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    bool isScalar = e->resultType()->isScalar();

    SymbolSymbol * resSym = generateResultSymbol(result, e);

    // 1. evaluate subexpressions
    TreeNodeExpr * eArg1 = e->expression ();
    const CGResult & argResult = codeGen(eArg1);
    append(result, argResult);
    if (result.isNotOk()) {
        return result;
    }

    Symbol * x = argResult.symbol();

    SubscriptInfo subscript;
    append(result, codeGenSubscript(subscript, x, e->indices ()));
    if (result.isNotOk()) {
        return result;
    }

    const SubscriptInfo::SPV & spv = subscript.spv();
    const SubscriptInfo::SliceIndices & slices = subscript.slices();

    // 5. compute resulting shape
    {
        pushComment("Computing shape:");
        unsigned count = 0;
        for (unsigned k : slices) {
            Symbol * sym = resSym->getDim(count);
            emplaceImopAfter(result, e, Imop::SUB, sym, spv[k].second, spv[k].first);
            ++ count;
        }

        codeGenSize(result);
    }

    // r = ALLOC def size (r = def)
    {
        if (!isScalar) {
            if (resSym->secrecType()->secrecDataType()->isUserPrimitive()) {
                assert(resSym->secrecType()->secrecSecType()->isPrivate());
                emplaceImopAfter(result, e, Imop::ALLOC, resSym, resSym->getSizeSym());
            }
            else {
                Symbol * def = defaultConstant(getContext(), e->resultType()->secrecDataType());
                emplaceImopAfter(result, e, Imop::ALLOC, resSym, resSym->getSizeSym(), def);
            }
        }
        else {
            emplaceImopAfter(result, e, Imop::DECLARE, resSym);
        }
    }

    // 4. initialze required temporary symbols
    LoopInfo loopInfo = prepareLoopInfo (subscript);
    const TypeBasic * pubIntTy = TypeBasic::getIndexType();
    Symbol * offset = m_st->appendTemporary(pubIntTy);
    Symbol * tmp_result = m_st->appendTemporary(
                TypeBasic::get(e->resultType()->secrecSecType(),
                               e->resultType()->secrecDataType()));
    Symbol * tmp_result2 = m_st->appendTemporary(pubIntTy);

    // 3. initialize strides
    std::vector<ArrayStrideInfo > strides;
    strides.push_back(x);
    strides.push_back(resSym);
    append(result, codeGenStride(strides[0]));
    append(result, codeGenStride(strides[1]));
    if (result.isNotOk()) {
        return result;
    }

    append(result, enterLoop(loopInfo, spv));
    if (result.isNotOk()) {
        return result;
    }

    // 8. compute offset for RHS
    {
        // old_ffset = 0
        pushComment("Compute offset:");
        emplaceImop(e, Imop::ASSIGN, offset, indexConstant(0));

        auto itIt = loopInfo.begin();
        auto itEnd = loopInfo.end();
        for (unsigned k = 0; itIt != itEnd; ++ k, ++ itIt) {
            // tmp_result2 = s[k] * idx[k]
            emplaceImop(e, Imop::MUL, tmp_result2, strides[0].at(k), *itIt);

            // old_offset = old_offset + tmp_result2
            emplaceImop(e, Imop::ADD, offset, offset, tmp_result2);
        }
    }

    // 9. load and store
    {
        pushComment("Load and store:");
        if (isScalar) {
            emplaceImop(e, Imop::LOAD, resSym, x, offset);
        }
        else {
            emplaceImop(e, Imop::DECLARE, tmp_result);
            emplaceImop(e, Imop::LOAD, tmp_result, x, offset);
            emplaceImop(e, Imop::ASSIGN, offset, indexConstant(0));

            unsigned count = 0;
            for (unsigned k : slices) {
                Symbol * idx = loopInfo.at(k);
                emplaceImop(e, Imop::SUB, tmp_result2, idx, spv[k].first);
                emplaceImop(e, Imop::MUL, tmp_result2, tmp_result2, strides[1].at(count));
                emplaceImop(e, Imop::ADD, offset, offset, tmp_result2);
                ++ count;
            }

            emplaceImop(e, Imop::STORE, resSym, offset, tmp_result);
            releaseTemporary(result, tmp_result);
        }
    }

    append(result, exitLoop(loopInfo));
    releaseTemporary(result, x);
    return result;
}

CGBranchResult TreeNodeExprIndex::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolSimple(this);
}

/******************************************************************
  TreeNodeExprSize
******************************************************************/

CGResult TreeNodeExprSize::codeGenWith(CodeGen & cg) {
    return cg.cgExprSize(this);
}

CGResult CodeGen::cgExprSize(TreeNodeExprSize * e) {
    if (m_tyChecker->visitExprSize(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result = codeGen(e->expression());
    if (result.isNotOk ()) {
        return result;
    }

    Symbol * size = indexConstant(1);
    if (!e->expression()->resultType()->isScalar()) {
        size = static_cast<SymbolSymbol *>(result.symbol())->getSizeSym();
    }

    releaseTemporary(result, result.symbol());
    result.setResult(size);
    return result;
}

/******************************************************************
  TreeNodeExprShape
******************************************************************/

CGResult TreeNodeExprShape::codeGenWith(CodeGen & cg) {
    return cg.cgExprShape(this);
}

CGResult CodeGen::cgExprShape(TreeNodeExprShape * e) {
    // Type check:
    if (m_tyChecker->visitExprShape(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    SymbolSymbol * resSym = generateResultSymbol(result, e);
    TreeNodeExpr * eArg = e->expression();
    const CGResult & argResult(codeGen(eArg));
    append(result, argResult);
    if (result.isNotOk()) {
        return result;
    }

    Symbol * n = indexConstant(eArg->resultType()->secrecDimType());
    emplaceImopAfter(result, e, Imop::ALLOC, resSym, n, indexConstant(0));
    emplaceImop(e, Imop::ASSIGN, resSym->getDim(0), n);

    unsigned count = 0;
    for (Symbol* sizeSym : dim_range(argResult.symbol())) {
        Symbol * indexSym = indexConstant(count);
        emplaceImop(e, Imop::STORE, resSym, indexSym, sizeSym);
        ++ count;
    }

    codeGenSize(result);
    releaseTemporary(result, argResult.symbol());
    return result;
}

/******************************************************************
  TreeNodeExprCat
******************************************************************/

CGResult TreeNodeExprCat::codeGenWith(CodeGen & cg) {
    return cg.cgExprCat(this);
}

CGResult CodeGen::cgExprCat(TreeNodeExprCat * e) {
    // Type check:
    if (m_tyChecker->visitExprCat(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    generateResultSymbol(result, e);

    const CGResult & arg1Result(codeGen(e->leftExpression()));
    append(result, arg1Result);
    if (result.isNotOk()) {
        return result;
    }

    const CGResult & arg2Result(codeGen(e->rightExpression()));
    append(result, arg2Result);
    if (result.isNotOk()) {
        return result;
    }

    const TypeBasic * const pubBoolTy = TypeBasic::getPublicBoolType();
    SecrecDimType k = e->dimensionality()->value();
    SecrecDimType n = e->resultType()->secrecDimType();
    SymbolSymbol * arg1ResultSymbol = static_cast<SymbolSymbol *>(arg1Result.symbol());
    SymbolSymbol * arg2ResultSymbol = static_cast<SymbolSymbol *>(arg2Result.symbol());
    SymbolSymbol * resSym = static_cast<SymbolSymbol *>(result.symbol());

    // Compute resulting shape and perform sanity check:
    std::stringstream ss;
    ss << "Different sized dimensions in concat at " << e->location() << '.';
    Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
    SymbolLabel * errLabel = m_st->label(err);
    for (SecrecDimType it = 0; it < e->resultType()->secrecDimType(); ++ it) {
        Symbol * s1 = arg1ResultSymbol->getDim(it);
        Symbol * s2 = arg2ResultSymbol->getDim(it);
        if (it == k) {
            emplaceImopAfter(result, e, Imop::ADD, resSym->getDim(it), s1, s2);
        }
        else {
            SymbolTemporary * temp_bool = m_st->appendTemporary(pubBoolTy);
            emplaceImopAfter(result, e, Imop::NE, temp_bool, s1, s2);
            emplaceImop(e, Imop::JT, errLabel, temp_bool);
            emplaceImop(e, Imop::ASSIGN, resSym->getDim(it), s1);
        }
    }

    auto jmp = new Imop(e, Imop::JUMP, (Symbol *) nullptr);
    pushImopAfter(result, jmp);
    result.addToNextList(jmp);

    push_imop(err);

    // Initialize strides:
    std::vector<ArrayStrideInfo > strides;
    strides.push_back(arg1ResultSymbol);
    strides.push_back(arg2ResultSymbol);
    strides.push_back(resSym);
    for (unsigned i = 0; i < 3; ++ i) {
        append(result, codeGenStride(strides[i]));
        if (result.isNotOk()) {
            return result;
        }
    }

    // Symbols for running indices:
    LoopInfo loopInfo;
    const TypeBasic * pubIntTy = TypeBasic::getIndexType();
    for (SecrecDimType it = 0; it < n; ++ it) {
        loopInfo.push_index(m_st->appendTemporary(pubIntTy));
    }

    // Compute size and allocate resulting array:
    codeGenSize(result);
    if (result.isNotOk()) {
        return result;
    }

    const TypeBasic * elemType = TypeBasic::get(e->resultType()->secrecSecType(),
                                                e->resultType()->secrecDataType());
    Symbol * offset = m_st->appendTemporary(pubIntTy);
    Symbol * tmpInt = m_st->appendTemporary(pubIntTy);
    Symbol * tmp_elem = m_st->appendTemporary(elemType);

    allocTemporaryResult(result);

    // Allocate memory for the temporary if needed:
    emplaceImopAfter(result, e, Imop::DECLARE, tmp_elem);

    append(result, enterLoop(loopInfo, resSym));

    // j = 0 (right hand side index)
    emplaceImopAfter(result, e, Imop::ASSIGN, offset, indexConstant (0));

    // IF (i_k >= d_k) GOTO T1;
    SymbolTemporary * temp_bool = m_st->appendTemporary(TypeBasic::getPublicBoolType());
    emplaceImop (e, Imop::GE, temp_bool, loopInfo.at(k), arg1ResultSymbol->getDim(k));

    auto i = new Imop(e, Imop::JT, nullptr, temp_bool);
    push_imop(i);
    result.addToNextList(i);

    // compute j if i < d (for e1)
    pushComment ("Left argument:");
    for (unsigned count = 0; count < strides[0].size(); ++ count) {
        emplaceImop(e, Imop::MUL, tmpInt, strides[0].at(count), loopInfo.at(count));
        emplaceImop(e, Imop::ADD, offset, offset, tmpInt);
    }

    // t = x[j]
    emplaceImop (e, Imop::LOAD, tmp_elem, arg1Result.symbol(), offset);

    // jump out
    auto jump_out = new Imop(e, Imop::JUMP, nullptr);
    push_imop(jump_out);

    // compute j if i >= d (for e2)
    for (SecrecDimType count = 0; static_cast<size_t>(count) < strides[1].size(); ++ count) {
        if (count == k) {
            emplaceImopAfter (result, e, Imop::SUB, tmpInt, loopInfo.at(count), arg1ResultSymbol->getDim(k));
            emplaceImop (e, Imop::MUL, tmpInt, strides[1].at(count), tmpInt);
        }
        else {
            emplaceImopAfter (result, e, Imop::MUL, tmpInt, strides[1].at(count), loopInfo.at(count));
        }

        emplaceImop (e, Imop::ADD, offset, offset, tmpInt);
    }

    // t = y[j]
    emplaceImopAfter (result, e, Imop::LOAD, tmp_elem, arg2Result.symbol(), offset);

    // out: r[i] = t
    i = new Imop(e, Imop::ASSIGN, offset, indexConstant(0));
    push_imop(i);
    jump_out->setDest(m_st->label(i));

    // compute j if i < d (for e1)
    for (unsigned count = 0; count != strides[2].size(); ++ count) {
        emplaceImop (e, Imop::MUL, tmpInt, strides[2].at(count), loopInfo.at(count));
        emplaceImop (e, Imop::ADD, offset, offset, tmpInt);
    }

    emplaceImop (e, Imop::STORE, resSym, offset, tmp_elem);

    append(result, exitLoop(loopInfo));

    releaseTemporary(result, tmp_elem);
    releaseTemporary(result, arg1ResultSymbol);
    releaseTemporary(result, arg2ResultSymbol);
    return result;
}

/******************************************************************
  TreeNodeExprReshape
******************************************************************/

CGResult TreeNodeExprReshape::codeGenWith(CodeGen & cg) {
    return cg.cgExprReshape(this);
}

CGResult CodeGen::cgExprReshape(TreeNodeExprReshape * e) {
    // Type check:
    if (m_tyChecker->visitExprReshape(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    // Evaluate subexpression:
    TreeNodeExpr * eArg = e->reshapee();
    CGResult result = codeGen(eArg);
    if (result.isNotOk()) {
        return result;
    }

    Symbol * rhs = result.symbol();
    SymbolSymbol * resSym = generateResultSymbol(result, e);

    {   // Eval subexpressions and copy dimensionalities:
        dim_iterator dimIt = dim_begin(resSym);
        for (TreeNodeExpr& dim : e->dimensions()) {
            assert(dimIt != dim_end(resSym));
            const CGResult & argResult = codeGen(&dim);
            append(result, argResult);
            if (result.isNotOk()) {
                return result;
            }

            auto i = new Imop(e, Imop::ASSIGN, *dimIt, argResult.symbol());
            pushImopAfter(result, i);
            ++ dimIt;
        }
    }

    // Compute new size:
    codeGenSize(result);
    Imop* i;

    if (!eArg->resultType()->isScalar()) {
        assert(dynamic_cast<SymbolSymbol *>(rhs) != nullptr);
        // Check that new and old sizes are equal:
        Symbol * sizeSymbol = static_cast<SymbolSymbol *>(rhs)->getSizeSym();
        SymbolTemporary * temp_bool = m_st->appendTemporary(TypeBasic::getPublicBoolType());

        emplaceImopAfter(result, e, Imop::EQ, temp_bool, sizeSymbol, resSym->getSizeSym());

        auto jmp = new Imop(e, Imop::JT, nullptr, temp_bool);
        push_imop(jmp);
        result.addToNextList(jmp);

        std::stringstream ss;
        ss << "ERROR: Mismatching sizes in reshape at " << e->location() << '.';
        Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
        push_imop(err);

        i = new Imop(e, Imop::COPY, resSym, rhs, resSym->getSizeSym ());
    }
    else {
        i = new Imop(e, Imop::ALLOC, resSym, resSym->getSizeSym (), rhs);
    }

    // Copy/alloc result:
    pushImopAfter(result, i);
    releaseTemporary (result, rhs);
    return result;
}

/*******************************************************************************
  TreeNodeExprToString
*******************************************************************************/

CGResult TreeNodeExprToString::codeGenWith(CodeGen & cg) {
    return cg.cgExprToString(this);
}

CGResult CodeGen::cgExprToString(TreeNodeExprToString * e) {
    // Type check:
    if (m_tyChecker->visitExprToString(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    // Evaluate subexpression:
    TreeNodeExpr * eArg = e->expression();
    CGResult result = codeGen(eArg);
    if (result.isNotOk()) {
        return result;
    }

    const auto tnv = static_cast<const TypeNonVoid *>(e->resultType());
    SymbolTemporary * temp = m_st->appendTemporary(tnv);
    auto imop = new Imop(e, Imop::TOSTRING, temp, result.symbol());
    pushImopAfter(result, imop);
    releaseTemporary(result, result.symbol());
    result.setResult(temp);
    return result;
}

/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/

void CodeGen::cgBinExprShapeCheck(TreeNodeExpr * e,
                                  Symbol * e1result,
                                  Symbol * e2result,
                                  CGResult & result)
{
    const TypeBasic * const pubBoolTy = TypeBasic::getPublicBoolType();
    std::stringstream ss;
    ss << "Mismatching shapes in arithmetic expression at " << e->location();
    Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
    SymbolLabel * errLabel = m_st->label(err);
    dim_iterator dj = dim_begin(e2result);
    for (Symbol* dim : dim_range(e1result)) {
        SymbolTemporary * temp_bool = m_st->appendTemporary(pubBoolTy);
        emplaceImopAfter(result, e, Imop::NE, temp_bool, dim, *dj);
        emplaceImop(e, Imop::JT, errLabel, temp_bool);
        ++dj;
    }

    Imop * jmp = new Imop(e, Imop::JUMP, (Symbol *) nullptr);
    pushImopAfter(result, jmp);
    result.addToNextList(jmp);
    push_imop(err);
}

// e is used for the 'created by' comment
Symbol* CodeGen::toVector(CGResult & result,
                          TreeNodeExpr * e,
                          Symbol * eArgRes,
                          Symbol * size)
{
    assert(dynamic_cast<const TypeBasic*>(eArgRes->secrecType()) != nullptr);
    const TypeBasic* eTy = static_cast<const TypeBasic*>(eArgRes->secrecType());
    const TypeBasic* eVecTy = TypeBasic::get(eTy->secrecSecType(), eTy->secrecDataType(), 1);
    SymbolSymbol* tmp = m_st->appendTemporary(eVecTy);
    SymbolSymbol* sizeSym;

    initShapeSymbols(m_st, tmp);
    sizeSym = tmp->getSizeSym();

    emplaceImopAfter(result, e, Imop::ASSIGN, sizeSym, size);
    tmp->setDim(0u, sizeSym);

    if (eTy->isScalar()) {
        emplaceImop(e, Imop::ALLOC, tmp, sizeSym, eArgRes);
    }
    else {
        emplaceImop(e, Imop::COPY, tmp, eArgRes, sizeSym);
    }

    releaseTemporary(result, eArgRes);

    return tmp;
}

CGResult CodeGen::cgOverloadedExpr (TreeNodeExpr* e,
                                    const Type* resTy,
                                    std::vector<Symbol*>& operands,
                                    SymbolProcedure* symProc)
{
    assert(operands.size() == 1 || operands.size() == 2);
    assert(symProc != nullptr);
    assert(e != nullptr);
    assert(resTy != nullptr);

    const TypeProc* procTy = symProc->decl()->procedureType();
    const std::vector<const TypeBasic*>& paramsTy = procTy->paramTypes();
    const Type* callRetTy = nullptr;
    bool unary = operands.size() == 1;
    CGResult result;
    SymbolSymbol* resSy = generateResultSymbol(result, resTy);
    Symbol* resSizeSy;

    assert(!resTy->isVoid());
    callRetTy = procTy->returnType();

    if (unary) {
        const Type* eArgTy = operands[0u]->secrecType();
        resSizeSy =
            eArgTy->isScalar()
            ? static_cast<Symbol*>(indexConstant(1u))
            : static_cast<Symbol*>(resSy->getSizeSym());

        // Shape of the result
        if (!eArgTy->isScalar()) {
            pushComment("Copying shape of the result from the argument:");
            append(result, copyShape(resSy, operands[0u]));
        }

        // Convert operand to vector if necessary
        if (eArgTy->secrecDimType() != 1u && !paramsTy[0u]->isScalar()) {
            pushComment("Converting operand to vector:");
            operands[0u] = toVector(result, e, operands[0u], resSizeSy);
        }
    }
    else {
        SecrecDimType e1Dim = operands[0u]->secrecType()->secrecDimType();
        SecrecDimType e2Dim = operands[1u]->secrecType()->secrecDimType();

        if (e1Dim == e2Dim && e1Dim != 0u) {
            pushComment("Checking if operand shapes match:");
            cgBinExprShapeCheck(e, operands[0u], operands[1u], result);
        }

        // Shape of the result
        assert (!resTy->isVoid());
        if (e1Dim == e2Dim && e1Dim == 0u) {
            // Both are scalar
            assert(resTy->isScalar());
            resSizeSy = indexConstant(1u);
        } else if (e1Dim > e2Dim) {
            pushComment("Shape of the result:");
            append(result, copyShape(resSy, operands[0u]));
            resSizeSy = resSy->getSizeSym();
        } else {
            pushComment("Shape of the result:");
            append(result, copyShape(resSy, operands[1u]));
            resSizeSy = resSy->getSizeSym();
        }

        // Convert operands to vectors if necessary. Note that the
        // typechecker adds classify nodes if needed, so we don't need to
        // classify.
        if (e1Dim == 0u && paramsTy[0u]->secrecDimType() == 1u) {
            // Stretch e1 to vector.
            pushComment("Stretching first operand to vector:");
            operands[0u] = toVector(result, e, operands[0u], resSizeSy);
        }
        else if (e1Dim > 1u) {
            // Reshape e1 to vector
            pushComment("Reshaping first operand to vector:");
            operands[0u] = toVector(result, e, operands[0u], resSizeSy);
        }

        if (e2Dim == 0u && paramsTy[1u]->secrecDimType() == 1u) {
            pushComment("Stretching second operand to vector:");
            operands[1u] = toVector(result, e, operands[1u], resSizeSy);
        }
        else if (e2Dim > 1u) {
            pushComment("Reshaping second operand to vector:");
            operands[1u] = toVector(result, e, operands[1u], resSizeSy);
        }
    }

    // Procedure call
    pushComment("Procedure call:");
    CGResult callRes(cgProcCall(symProc, callRetTy, operands));
    append(result, callRes);
    if (result.isNotOk())
        return result;

    if (!callRetTy->isVoid() && !callRetTy->isScalar()) {
        // Check result size
        pushComment("Checking if procedure call result has the correct size:");
        std::stringstream ss;
        ss << "Procedure at " << *symProc->location()
           << " returned a value with incorrect size in expression at " << e->location();
        Imop* err = newError(e, ConstantString::get(getContext(), ss.str()));
        SymbolLabel* errLabel = m_st->label(err);

        SymbolTemporary* tempBool = m_st->appendTemporary(TypeBasic::getPublicBoolType());
        Symbol* callResSizeSy = static_cast<SymbolSymbol*>(callRes.symbol())->getSizeSym();
        emplaceImopAfter(result, e, Imop::NE, tempBool, callResSizeSy, resSizeSy);
        emplaceImop(e, Imop::JT, errLabel, tempBool);

        Imop* jmp = new Imop(e, Imop::JUMP, nullptr);
        push_imop(jmp);
        result.addToNextList(jmp);

        push_imop(err);
    }

    // Copy to result so that it would have the correct shape
    if (callRetTy->secrecDimType() != resTy->secrecDimType()) {
        pushComment("Copying procedure call result to expression result:");

        if (resTy->isScalar()) {
            // Assign first (only) element of call result vector to
            // the scalar
            emplaceImopAfter(result, e, Imop::DECLARE, resSy);
            emplaceImop(e, Imop::LOAD, resSy, callRes.symbol(), indexConstant(0u));
            releaseTemporary(result, callRes.symbol());
        } else {
            emplaceImopAfter(result, e, Imop::COPY, resSy, callRes.symbol(), resSizeSy);
            releaseTemporary(result, callRes.symbol());
        }
    } else {
        result.setResult(callRes.symbol());
    }

    return result;
}

CGResult TreeNodeExprBinary::codeGenWith(CodeGen & cg) {
    return cg.cgExprBinary(this);
}

CGResult CodeGen::cgExprBinary(TreeNodeExprBinary * e) {
    // Type check:
    if (m_tyChecker->visitExprBinary(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    if (e->isOverloaded()) {
        CGResult result;

        const CGResult& arg1Result(codeGen(e->leftExpression()));
        append(result, arg1Result);
        if (result.isNotOk())
            return result;

        const CGResult& arg2Result(codeGen(e->rightExpression()));
        append(result, arg2Result);
        if (result.isNotOk())
            return result;

        std::vector<Symbol*> operands {arg1Result.symbol(), arg2Result.symbol()};
        const CGResult& callRes(cgOverloadedExpr(e, e->resultType(), operands, e->procSymbol()));
        append(result, callRes);
        if (result.isNotOk()) return result;
        result.setResult(callRes.symbol());

        return result;
    }

    TreeNodeExpr * eArg1 = e->leftExpression();
    TreeNodeExpr * eArg2 = e->rightExpression();

    /*
       If first sub-expression is public, then generate short-circuit code for
       logical && and logical ||.
       */
    if (canShortCircuit(e->type(), eArg1->resultType(), eArg2->resultType())) {
        // Generate code for first child expression:
        CGResult result(codeGen(eArg1));
        Symbol * oldSym = result.symbol();
        Symbol * resSym = generateResultSymbol(result, e);
        if (result.isNotOk()) {
            return result;
        }

        Imop * i = newAssign(e, resSym, oldSym);
        pushImopAfter(result, i);

        const Imop::Type iType = (e->type() == NODE_EXPR_BINARY_LAND) ? Imop::JF : Imop::JT;
        auto j = new Imop(e, iType, nullptr, resSym);
        push_imop(j);
        result.addToNextList(j);

        // Generate code for second child expression:
        CGResult arg2Result(codeGen(eArg2));
        Symbol * arg2Sym = arg2Result.symbol();
        if (arg2Result.isNotOk()) {
            return result;
        }

        i = new Imop(e, Imop::ASSIGN, resSym, arg2Sym);
        pushImopAfter(arg2Result, i);

        return result;
    }

    CGResult result;

    // Generate code for first child expression:
    const CGResult & arg1Result(codeGen(eArg1));
    append(result, arg1Result);
    if (result.isNotOk()) {
        return result;
    }

    // Generate code for first child expression:
    const CGResult & arg2Result(codeGen(eArg2));
    append(result, arg2Result);
    if (result.isNotOk()) {
        return result;
    }

    Symbol * e1result = arg1Result.symbol();
    Symbol * e2result = arg2Result.symbol();

    // Implicitly convert scalar to array if needed:
    if (eArg1->resultType()->secrecDimType() > eArg2->resultType()->secrecDimType()) {
        SymbolSymbol * tmpe1 = static_cast<SymbolSymbol *>(e1result);
        SymbolSymbol * tmpe2 = m_st->appendTemporary(static_cast<const TypeNonVoid *>(eArg1->resultType()));
        tmpe2->inheritShape(tmpe1);
        e1result = tmpe1;
        e2result = tmpe2;
        emplaceImopAfter(result, e, Imop::ALLOC, e2result, tmpe1->getSizeSym(), arg2Result.symbol());
        releaseTemporary(result, arg2Result.symbol());
    }
    else if (eArg2->resultType()->secrecDimType() > eArg1->resultType()->secrecDimType()) {
        SymbolSymbol * tmpe1 = m_st->appendTemporary(static_cast<const TypeNonVoid *>(eArg2->resultType()));
        SymbolSymbol * tmpe2 = static_cast<SymbolSymbol *>(e2result);
        tmpe1->inheritShape(tmpe2);
        e1result = tmpe1;
        e2result = tmpe2;
        emplaceImopAfter(result, e, Imop::ALLOC, e1result, tmpe2->getSizeSym(), arg1Result.symbol());
        releaseTemporary(result, arg1Result.symbol());
    }
    else {
        cgBinExprShapeCheck(e, e1result, e2result, result);
    }

    SymbolSymbol * resSym = generateResultSymbol(result, e);
    resSym->inheritShape(e1result);

    // Generate code for binary expression:
    Imop::Type iType;
    if (! getBinImopType(e->type(), iType)) {
        m_log.fatalInProc(e) << "Binary " << e->operatorString()
            << " not yet implemented. At " << e->location();
        result.setStatus(CGResult::ERROR_CONTINUE);
        return result;
    }

    allocTemporaryResult(result);
    pushImopAfter(result, newBinary(e, iType, resSym, e1result, e2result));
    releaseTemporary(result, e1result);
    releaseTemporary(result, e2result);
    return result;
}

CGBranchResult TreeNodeExprBinary::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolExprBinary(this);
}

CGBranchResult CodeGen::cgBoolExprBinary(TreeNodeExprBinary * e) {
    if (e->isOverloaded()) {
        return cgBoolSimple(e);
    }

    TreeNodeExpr * eArg1 = e->leftExpression();
    TreeNodeExpr * eArg2 = e->rightExpression();
    CGBranchResult result;

    switch (e->type()) {
    case NODE_EXPR_BINARY_LAND: // fall through
    case NODE_EXPR_BINARY_LOR:
        assert(!eArg1->resultType()->isVoid());
        assert(dynamic_cast<const TypeNonVoid *>(eArg1->resultType()) != nullptr);

        /*
           If first sub-expression is public, then generate short-circuit
           code for logical && and logical ||.
           */
        if (static_cast<const TypeNonVoid *>(eArg1->resultType())->secrecSecType()->isPublic()) {
            // Generate code for first child expression:
            result = codeGenBranch(eArg1);
            if (result.isNotOk()) {
                return result;
            }

            // Generate code for second child expression:
            const CGBranchResult & arg2Result = codeGenBranch(eArg2);
            result.patchFirstImop(arg2Result.firstImop());
            if (arg2Result.isNotOk()) {
                result.setStatus(arg2Result.status());
                return result;
            }

            // Short circuit the code:
            if (e->type() == NODE_EXPR_BINARY_LAND) {
                result.patchTrueList(m_st->label(arg2Result.firstImop()));
                result.setTrueList(arg2Result.trueList());
                result.addToFalseList(arg2Result.falseList());
            } else {
                assert(e->type() == NODE_EXPR_BINARY_LOR);
                result.patchFalseList(m_st->label(arg2Result.firstImop()));
                result.setFalseList(arg2Result.falseList());
                result.addToTrueList(arg2Result.trueList());
            }
        } else {
            result = codeGen(e);
            if (result.isNotOk()) {
                return result;
            }

            Imop * j1, *j2;
            if (e->type() == NODE_EXPR_BINARY_LAND) {
                j1 = new Imop(e, Imop::JF, nullptr);
                result.addToFalseList(j1);

                j2 = new Imop(e, Imop::JUMP, nullptr);
                result.addToTrueList(j2);
            } else {
                assert(e->type() == NODE_EXPR_BINARY_LOR);

                j1 = new Imop(e, Imop::JT, nullptr);
                result.addToTrueList(j1);

                j2 = new Imop(e, Imop::JUMP, nullptr);
                result.addToFalseList(j2);
            }

            j1->setArg1(result.symbol());
            push_imop(j1);
            pushImopAfter(result, j1);
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
    case NODE_EXPR_BITWISE_AND: // fall through
    case NODE_EXPR_BITWISE_OR:  // fall through
    case NODE_EXPR_BITWISE_XOR: // fall through
        {
            // Generate code for first child expression:
            const CGResult & arg1Result = codeGen(eArg1);
            append(result, arg1Result);
            if (result.isNotOk()) {
                return result;
            }

            // Generate code for second child expression:
            const CGResult & arg2Result = codeGen(eArg2);
            append(result, arg2Result);
            if (result.isNotOk()) {
                return result;
            }

            Imop::Type iType;
            if (! getBinImopType(e->type(), iType)) {
                assert(false && "Dont know how to handle the node type.");
                result.setStatus(CGResult::ERROR_FATAL);
                return result;
            }

            SymbolSymbol * temp = m_st->appendTemporary(TypeBasic::getPublicBoolType());
            pushImopAfter(result, new Imop(e, iType, temp, arg1Result.symbol(), arg2Result.symbol()));
            releaseTemporary(result, arg1Result.symbol());
            releaseTemporary(result, arg2Result.symbol());
            auto tj = new Imop(e, Imop::JT, nullptr, temp);
            pushImopAfter(result, tj);
            result.addToTrueList(tj);

            auto fj = new Imop(e, Imop::JUMP, nullptr);
            result.addToFalseList(fj);
            push_imop(fj);
            break;
        }
    default:
        assert(false && "Illegal binary operator");
        result.setStatus(CGResult::ERROR_FATAL);
        break;
    }

    return result;
}

/*******************************************************************************
  TreeNodeExprProcCall
*******************************************************************************/

CGResult TreeNodeExprProcCall::codeGenWith(CodeGen & cg) {
    return cg.cgExprProcCall(this);
}

CGResult CodeGen::cgProcCall (SymbolProcedure* symProc,
                              const SecreC::Type* returnType,
                              const std::vector<Symbol*>& args)
{
    CGResult result;
    SymbolSymbol* r = generateResultSymbol (result, returnType);
    std::vector<Symbol*> argList, retList;

    // Initialize arguments:
    for (Symbol * arg : args) {
        // Possibly copy arguments (if needed):
        for (Symbol* sym : flattenSymbol (arg)) {
            argList.push_back (copyNonTemporary (result, sym));
        }
    }

    // prep return values:
    if (!returnType->isVoid ()) {
        retList = flattenSymbol (r);
    }

    Imop* i = newCall (m_node, retList.begin (), retList.end (), argList.begin (), argList.end ());
    auto c = new Imop (m_node, Imop::RETCLEAN, nullptr, nullptr, nullptr);
    m_callsTo[symProc].insert (i);

    c->setArg2 (m_st->label (i));
    pushImopAfter (result, i);
    push_imop (c);

    if (! returnType->isVoid ()) {
        codeGenSize (result);
    }

    return result;
}

CGResult CodeGen::cgProcCall (SymbolProcedure* symProc,
                              const SecreC::Type* returnType,
                              const std::vector<TreeNodeExpr*>& args)
{
    CGResult result;
    SymbolSymbol* r = generateResultSymbol (result, returnType);
    std::vector<Symbol*> argList, retList;

    // Initialize arguments:
    for (TreeNodeExpr * arg : args) {
        const CGResult & argResult(codeGen(arg));
        append(result, argResult);
        if (result.isNotOk()) {
            return result;
        }

        // Possibly copy arguments (if needed):
        for (Symbol* sym : flattenSymbol (argResult.symbol ())) {
            argList.push_back (copyNonTemporary (result, sym));
        }
    }

    // prep return values:
    if (!returnType->isVoid ()) {
        retList = flattenSymbol (r);
    }

    Imop* i = newCall (m_node, retList.begin (), retList.end (), argList.begin (), argList.end ());
    auto c = new Imop (m_node, Imop::RETCLEAN, nullptr, nullptr, nullptr);
    m_callsTo[symProc].insert (i);

    c->setArg2 (m_st->label (i));
    pushImopAfter (result, i);
    push_imop (c);

    if (! returnType->isVoid ()) {
        codeGenSize (result);
    }

    return result;
}

CGResult CodeGen::cgExprProcCall(TreeNodeExprProcCall * e) {
    // Type check:
    if (m_tyChecker->visitExprProcCall(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    std::vector<TreeNodeExpr * > args;
    for (TreeNodeExpr& param : e->params ()) {
        args.push_back (&param);
    }

    return cgProcCall(e->symbolProcedure(), e->resultType(), args);
}

CGBranchResult TreeNodeExprProcCall::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolSimple(this);
}

/*******************************************************************************
  TreeNodeExprRVariable
*******************************************************************************/

CGResult TreeNodeExprRVariable::codeGenWith(CodeGen & cg) {
    return cg.cgExprRVariable(this);
}

CGResult CodeGen::cgExprRVariable(TreeNodeExprRVariable * e) {
    // Type check:
    if (m_tyChecker->visitExprRVariable(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    Symbol * sym = e->valueSymbol ();
    CGResult result;
    result.setResult(sym);
    return result;
}

CGBranchResult TreeNodeExprRVariable::codeGenBoolWith(CodeGen & cg) {
    return cg.cgBoolExprRVariable(this);
}

CGBranchResult CodeGen::cgBoolExprRVariable(TreeNodeExprRVariable * e) {
    CGBranchResult result;
    Symbol * sym = e->valueSymbol ();
    auto i = new Imop(e, Imop::JT, nullptr, sym);
    push_imop(i);
    result.setFirstImop(i);
    result.addToTrueList(i);
    i = new Imop(e, Imop::JUMP, nullptr);
    push_imop(i);
    result.addToFalseList(i);
    return result;
}

/*******************************************************************************
  TreeNodeExprDomainID
*******************************************************************************/

CGResult TreeNodeExprDomainID::codeGenWith(CodeGen & cg) {
    return cg.cgExprDomainID(this);
}

CGResult CodeGen::cgExprDomainID(TreeNodeExprDomainID * e) {
    // Type check:
    if (m_tyChecker->visitExprDomainID(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    assert(dynamic_cast<const TypeNonVoid *>(e->resultType()) != nullptr);
    const auto resultType = static_cast<const TypeNonVoid *>(e->resultType());
    SymbolSymbol * t = m_st->appendTemporary(resultType);
    Symbol * s = findIdentifier (SYM_DOMAIN, e->securityType()->identifier());
    if (s == nullptr || s->symbolType() != SYM_DOMAIN) {
        assert(false && "ICE: Type checker must guarantee that!");
        return CGResult::ERROR_FATAL;
    }

    auto i = new Imop(e, Imop::DOMAINID, t, static_cast<SymbolDomain *>(s));
    CGResult result;
    result.setResult(t);
    pushImopAfter(result, i);
    return result;
}

/*******************************************************************************
  TreeNodeExprQualified
*******************************************************************************/

CGResult TreeNodeExprQualified::codeGenWith(CodeGen & cg) {
    return cg.cgExprQualified(this);
}

CGResult CodeGen::cgExprQualified(TreeNodeExprQualified * e) {
    // Type check:
    if (m_tyChecker->visitExprQualified(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    return codeGen(e->expression());
}

CGBranchResult TreeNodeExprQualified::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolExprQualified(this);
}

CGBranchResult CodeGen::cgBoolExprQualified(TreeNodeExprQualified * e) {
    return codeGenBranch(e->expression());
}

/*******************************************************************************
  TreeNodeExprStringFromBytes
*******************************************************************************/

CGResult TreeNodeExprStringFromBytes::codeGenWith (CodeGen& cg) {
    return cg.cgExprStringFromBytes (this);
}

CGResult CodeGen::cgExprStringFromBytes(TreeNodeExprStringFromBytes * e) {
    // Type check:
    if (m_tyChecker->visitExprStringFromBytes(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    SymbolSymbol * resSym = generateResultSymbol(result, e);
    const CGResult & argResult = codeGen(e->expression());
    append(result, argResult);
    if (result.isNotOk()) {
        return result;
    }

    SymbolSymbol * arrSym = static_cast<SymbolSymbol *>(argResult.symbol());
    Symbol * sizeSym = m_st->appendTemporary(TypeBasic::getIndexType());
    Symbol * tempElem = m_st->appendTemporary(TypeBasic::get(DATATYPE_UINT8));

    /**
     * Allocate memory for the string:
     */

    emplaceImopAfter(result, e, Imop::ASSIGN, sizeSym, arrSym->getDim(0));

    // XXX TODO: giant hack
    emplaceImop(e, Imop::ALLOC, resSym, sizeSym, ConstantInt::get(DATATYPE_UINT8, 0)); // allocates 1 byte more
    emplaceImop(e, Imop::STORE, resSym, sizeSym, ConstantInt::get(DATATYPE_UINT8, 0)); // initialize last byte to zero

    /**
     * Copy the data from array to the string:
     */

    LoopInfo loopInfo;
    loopInfo.push_index(m_st->appendTemporary(TypeBasic::getIndexType()));
    append(result, enterLoop(loopInfo, arrSym));
    emplaceImop(e, Imop::LOAD, tempElem, arrSym, loopInfo.at(0));
    emplaceImop(e, Imop::STORE, resSym, loopInfo.at(0), tempElem);
    append(result, exitLoop(loopInfo));
    releaseTemporary(result, arrSym);
    return result;
}

/*******************************************************************************
  TreeNodeExprBytesFromString
*******************************************************************************/

CGResult TreeNodeExprBytesFromString::codeGenWith(CodeGen & cg) {
    return cg.cgExprBytesFromString(this);
}

CGResult CodeGen::cgExprBytesFromString(TreeNodeExprBytesFromString * e) {
    // Type check:
    if (m_tyChecker->visitExprBytesFromString(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    SymbolSymbol * resSym = generateResultSymbol(result, e);
    const CGResult & argResult = codeGen(e->expression());
    append(result, argResult);
    if (result.isNotOk()) {
        return result;
    }

    Symbol * sizeSym = resSym->getDim(0);
    Symbol * indexSym = m_st->appendTemporary(TypeBasic::getIndexType ());
    Symbol * strSym = argResult.symbol();
    Symbol * charSym = m_st->appendTemporary(TypeBasic::get(DATATYPE_UINT8));
    Symbol * tempBool = m_st->appendTemporary(TypeBasic::getPublicBoolType());
    Symbol * zeroByte = ConstantInt::get(DATATYPE_UINT8, 0);

    /**
     * Compute length of the array:
     */

    // size = strlen (str)
    emplaceImopAfter(result, e, Imop::STRLEN, sizeSym, strSym);

    // r = ALLOC 0 i
    emplaceImop(e, Imop::ALLOC, resSym, sizeSym, zeroByte);

    /**
     * Copy the data:
     */

    // i = 0
    emplaceImop(e, Imop::ASSIGN, indexSym, indexConstant(0));

    // BACK:
    // t = i < size
    auto cmp = new Imop (e, Imop::LT, tempBool, indexSym, sizeSym);
    push_imop (cmp);

    // JF OUT t
    auto jmp = new Imop (e, Imop::JF, nullptr, tempBool);
    result.addToNextList (jmp);
    push_imop (jmp);

    // L1: c = str[i]
    emplaceImop(e, Imop::LOAD, charSym, strSym, indexSym);

    // r[i] = c
    emplaceImop(e, Imop::STORE, resSym, indexSym, charSym);

    // i = i + 1
    emplaceImop(e, Imop::ADD, indexSym, indexSym, indexConstant(1));

    // JUMP BACK
    emplaceImop(e, Imop::JUMP, m_st->label (cmp));

    // OUT:
    codeGenSize(result);
    releaseTemporary(result, strSym);
    return result;
}

/*******************************************************************************
  TreeNodeStringPart
*******************************************************************************/

CGResult CodeGen::cgStringPart (TreeNodeStringPart* p) {
    return p->codeGenWith (*this);
}

/*******************************************************************************
  TreeNodeStringPartFragment
*******************************************************************************/

CGResult TreeNodeStringPartFragment::codeGenWith (CodeGen& cg) {
    return cg.cgStringPartFragment (this);
}

CGResult CodeGen::cgStringPartFragment (TreeNodeStringPartFragment* p) {
    CGResult result;
    result.setResult (ConstantString::get (getContext (), p->staticValue ()));
    return result;
}

/*******************************************************************************
  TreeNodeStringPartIdentifier
*******************************************************************************/

CGResult TreeNodeStringPartIdentifier::codeGenWith (CodeGen& cg) {
    return cg.cgStringPartIdentifier (this);
}

CGResult CodeGen::cgStringPartIdentifier (TreeNodeStringPartIdentifier* p) {
    CGResult result;
    if (p->isConstant ()) {
        result.setResult (ConstantString::get (getContext (), p->staticValue ()));
    }
    else {
        const TypeNonVoid* stringType = TypeBasic::get (DATATYPE_STRING);
        Symbol* argSymbol = m_st->find (SYM_SYMBOL, p->name ());

        if (argSymbol->secrecType () == stringType) {
            result.setResult (argSymbol);
        }
        else {
            SymbolSymbol* resultSymbol = m_st->appendTemporary (stringType);
            pushImopAfter (result, new Imop (p, Imop::TOSTRING, resultSymbol, argSymbol));
            result.setResult (resultSymbol);
        }
    }

    return result;
}

/*******************************************************************************
  TreeNodeExprString
*******************************************************************************/

CGResult TreeNodeExprString::codeGenWith(CodeGen & cg) {
    return cg.cgExprString(this);
}

CGResult CodeGen::cgExprString(TreeNodeExprString * e) {
    // Type check:
    if (m_tyChecker->visitExprString(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    std::vector<Symbol*> partResults;
    for (TreeNodeStringPart& part : e->parts ()) {
        const CGResult& partResult = cgStringPart (&part);
        append (result, partResult);
        if (result.isNotOk ())
            return result;

        Symbol* sym = partResult.symbol ();
        if (sym->symbolType () == SYM_CONSTANT) {
            if (static_cast<ConstantString*>(sym)->value ().empty ())
                continue;
        }

        partResults.push_back (sym);
    }

    std::vector<Symbol*> allocs; // temporary allocations
    const auto stringType = static_cast<const TypeNonVoid*>(e->resultType ());

    while (partResults.size () > 1) {
        std::vector<Symbol*> temp;
        for (size_t i = 1, n = partResults.size (); i < n; i += 2) {
            Symbol* prev = partResults[i-1];
            Symbol* curr = partResults[i];

            if (prev->symbolType () == SYM_CONSTANT && curr->symbolType () == SYM_CONSTANT) {
                std::stringstream ss;
                ss << static_cast<ConstantString*>(prev)->value ();
                ss << static_cast<ConstantString*>(curr)->value ();
                temp.push_back (ConstantString::get (getContext (), ss.str ()));
            }
            else {
                Symbol* resultSym = m_st->appendTemporary (stringType);
                pushImopAfter (result, new Imop (e, Imop::ADD, resultSym, prev, curr));
                temp.push_back (resultSym);
            }

            if (prev->symbolType () != SYM_CONSTANT) allocs.push_back (prev);
            if (curr->symbolType () != SYM_CONSTANT) allocs.push_back (curr);
        }

        if (partResults.size () % 2 == 1) {
            temp.push_back (partResults.back ());
        }

        std::swap (temp, partResults);
    }

    assert (partResults.size () <= 1);
    if (partResults.empty ()) {
        partResults.push_back (ConstantString::get (getContext (), ""));
    }

    for (Symbol* sym : allocs) {
        releaseTemporary (result, sym);
    }

    result.setResult(partResults.front ());
    return result;
}

/*******************************************************************************
  TreeNodeExprFloat
*******************************************************************************/

CGResult TreeNodeExprFloat::codeGenWith (CodeGen &cg) {
    return cg.cgExprFloat (this);
}

CGResult CodeGen::cgExprFloat(TreeNodeExprFloat * e) {
    // Type check:
    if (m_tyChecker->visitExprFloat(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    result.setResult(ConstantFloat::get (e->resultType()->secrecDataType(),
                                         e->value ()));
    return result;
}

/*******************************************************************************
  TreeNodeExprTernary
*******************************************************************************/

CGResult TreeNodeExprTernary::codeGenWith(CodeGen & cg) {
    return cg.cgExprTernary(this);
}

CGResult CodeGen::cgExprTernary(TreeNodeExprTernary * e) {
    // Type check:
    if (m_tyChecker->visitExprTernary(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    TreeNodeExpr * e1 = e->conditional();
    TreeNodeExpr * e2 = e->trueBranch();
    TreeNodeExpr * e3 = e->falseBranch();
    const TypeBasic * const pubBoolTy = TypeBasic::getPublicBoolType();

    if (e1->havePublicBoolType()) {
        generateResultSymbol(result, e);

        // Generate code for boolean expression:
        CGBranchResult e1Result = codeGenBranch(e1);
        append(result, e1Result);
        if (result.isNotOk()) {
            return result;
        }

        // Generate code for first value child expression:
        CGResult eTrueResult(codeGen(e2));
        if (eTrueResult.isNotOk()) {
            result.setStatus(eTrueResult.status());
            return result;
        }

        if (!e->resultType()->isVoid()) {
            if (!eTrueResult.symbol()->secrecType()->isScalar()) {
                SymbolSymbol * resultSymbol = static_cast<SymbolSymbol *>(result.symbol());
                append (eTrueResult, copyShape (resultSymbol, eTrueResult.symbol ()));
                if (eTrueResult.isNotOk ())
                    return result;

                if (eTrueResult.symbol()->secrecType()->secrecSecType()->isPrivate()) {
                    emplaceImopAfter(eTrueResult, e, Imop::ALLOC, resultSymbol,
                                     resultSymbol->getSizeSym());
                }
                else {
                    Symbol * def =
                        defaultConstant(getContext(),
                                        eTrueResult.symbol()->secrecType()->secrecDataType());
                    emplaceImopAfter(eTrueResult, e, Imop::ALLOC, resultSymbol,
                                     resultSymbol->getSizeSym(), def);
                }
            }

            pushImopAfter(eTrueResult, newAssign(e, result.symbol(), eTrueResult.symbol()));
            releaseTemporary(eTrueResult, eTrueResult.symbol());
        }

        result.patchFirstImop(eTrueResult.firstImop());

        // Jump out of the ternary construct:
        auto i = new Imop(e, Imop::JUMP, nullptr);
        result.addToNextList(i);
        result.patchFirstImop(i);
        push_imop(i);

        CGResult eFalseResult(codeGen(e3));
        if (eFalseResult.isNotOk()) {
            result.setStatus(eFalseResult.status());
            return result;
        }

        if (!e->resultType()->isVoid()) {
            if (!eFalseResult.symbol()->secrecType()->isScalar()) {
                SymbolSymbol * resultSymbol = static_cast<SymbolSymbol *>(result.symbol());
                append (eFalseResult, copyShape (resultSymbol, eFalseResult.symbol ()));
                if (eFalseResult.isNotOk ())
                    return result;

                if (eFalseResult.symbol()->secrecType()->secrecSecType()->isPrivate()) {
                    emplaceImopAfter(eFalseResult, e, Imop::ALLOC, resultSymbol,
                                     resultSymbol->getSizeSym());
                }
                else {
                    Symbol * def =
                        defaultConstant(getContext(),
                                        eFalseResult.symbol()->secrecType()->secrecDataType());
                    emplaceImopAfter(eFalseResult, e, Imop::ALLOC, resultSymbol,
                                     resultSymbol->getSizeSym(), def);
                }
            }

            pushImopAfter(eFalseResult, newAssign(e, result.symbol(), eFalseResult.symbol()));
            releaseTemporary(eFalseResult, eFalseResult.symbol());
        }

        // Link boolean expression code to the rest of the code:
        e1Result.patchTrueList(m_st->label(eTrueResult.firstImop()));
        e1Result.patchFalseList(m_st->label(eFalseResult.firstImop()));
    }
    else {
        // Evaluate subexpressions:
        CGResult e1Result(codeGen(e1));
        append(result, e1Result);
        if (result.isNotOk()) {
            return result;
        }

        CGResult e2Result(codeGen(e2));
        append(result, e2Result);
        if (result.isNotOk()) {
            return result;
        }

        CGResult e3Result(codeGen(e3));
        append(result, e3Result);
        if (result.isNotOk()) {
            return result;
        }

        // Generate temporary for the result of the ternary expression, if needed:
        SymbolSymbol * resSym = generateResultSymbol(result, e);
        resSym->inheritShape(e1Result.symbol());
        allocTemporaryResult(result);

        // check that shapes match
        std::stringstream ss;
        ss << "Mismatching shapes at " << e->location();
        auto jmp = new Imop(e, Imop::JUMP, (Symbol *) nullptr);
        Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
        SymbolLabel * errLabel = m_st->label(err);
        dim_iterator
            di = dim_begin(e1Result.symbol()),
               dj = dim_begin(e2Result.symbol()),
               dk = dim_begin(e3Result.symbol()),
               de = dim_end(e1Result.symbol());
        for (; di != de; ++ di, ++ dj, ++ dk) {
            SymbolTemporary * temp_bool = m_st->appendTemporary(pubBoolTy);

            emplaceImopAfter (result, e, Imop::NE, temp_bool, *di, *dj);
            emplaceImop (e, Imop::JT, errLabel, temp_bool);
            emplaceImop (e, Imop::NE, temp_bool, *dj, *dk);
            emplaceImop (e, Imop::JT, errLabel, temp_bool);
        }

        result.patchNextList(m_st->label(jmp));
        push_imop(jmp);
        push_imop(err);

        // loop to set all values of resulting array

        // Set up some temporary scalars:
        Symbol * counter = m_st->appendTemporary(TypeBasic::getIndexType());
        Symbol * b = m_st->appendTemporary(
                    TypeBasic::get(e1->resultType()->secrecSecType(),
                                   e1->resultType()->secrecDataType()));
        Symbol * t = m_st->appendTemporary(
                    TypeBasic::get(e->resultType()->secrecSecType(),
                                   e->resultType()->secrecDataType()));

        // r = e1
        Imop * i = newAssign(e, resSym, e2Result.symbol());
        push_imop(i);
        jmp->setDest(m_st->label(i));

        // counter = 0
        i = new Imop(e, Imop::ASSIGN, counter, indexConstant(0));
        push_imop(i);

        SymbolTemporary * temp_bool = m_st->appendTemporary(TypeBasic::getPublicBoolType());
        auto test = new Imop(e, Imop::GE, temp_bool, counter, resSym->getSizeSym());
        push_imop(test);

        // L0: if (counter >= size) goto next;
        i = new Imop(e, Imop::JT, (Symbol *) nullptr, temp_bool);
        push_imop(i);
        result.addToNextList(i);

        // b = e1[counter]
        i = new Imop(e, Imop::LOAD, b, e1Result.symbol(), counter);
        push_imop(i);

        auto t0 = new Imop(e, Imop::ADD, counter, counter, indexConstant(1));
        auto t1 = new Imop(e, Imop::STORE, resSym, counter, t);

        // if b goto T0
        i = new Imop(e, Imop::JT, (Symbol *) nullptr, b);
        push_imop(i);
        i->setDest(m_st->label(t0));

        // t = e3[counter]
        // T1: result[counter] = t
        i = new Imop(e, Imop::LOAD, t, e3Result.symbol(), counter);
        push_imop(i);
        push_imop(t1);

        // T0: counter = counter + 1
        push_imop(t0);

        // goto L0
        i = new Imop(e, Imop::JUMP, (Symbol *) nullptr);
        push_imop(i);
        i->setDest(m_st->label(test));

        releaseTemporary(result, e1Result.symbol());
        releaseTemporary(result, e2Result.symbol());
        releaseTemporary(result, e3Result.symbol());
    }

    return result;
}

CGBranchResult TreeNodeExprTernary::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolExprTernary(this);
}

CGBranchResult CodeGen::cgBoolExprTernary(TreeNodeExprTernary * e) {

    CGBranchResult result = codeGenBranch(e->conditional());
    if (result.isNotOk()) {
        return result;
    }

    // Generate code for first value child expression:
    const CGBranchResult & trueResult = codeGenBranch(e->trueBranch());
    if (trueResult.isNotOk()) {
        return trueResult;
    }

    // Generate code for second value child expression:
    const CGBranchResult & falseResult = codeGenBranch(e->falseBranch());
    if (falseResult.isNotOk()) {
        return falseResult;
    }

    // Link conditional expression code to the rest of the code:
    result.patchTrueList(m_st->label(trueResult.firstImop()));
    result.patchFalseList(m_st->label(falseResult.firstImop()));

    result.addToTrueList(trueResult.trueList());
    result.addToTrueList(falseResult.trueList());
    result.addToFalseList(trueResult.falseList());
    result.addToFalseList(falseResult.falseList());

    return result;
}

/*******************************************************************************
  TreeNodeExprArrayConstructor
*******************************************************************************/

CGResult TreeNodeExprArrayConstructor::codeGenWith(CodeGen & cg) {
    return cg.cgExprArrayConstructor(this);
}

CGResult CodeGen::cgExprArrayConstructor(TreeNodeExprArrayConstructor * e) {
    // Type check:
    if (m_tyChecker->visitExprArrayConstructor(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    SymbolSymbol * resultSymbol = generateResultSymbol (result, e);
    Symbol* sizeValue = indexConstant (e->expressions ().size ());

    emplaceImopAfter (result, e, Imop::ASSIGN, resultSymbol->getDim (0), sizeValue);
    emplaceImopAfter (result, e, Imop::ASSIGN, resultSymbol->getSizeSym (), sizeValue);
    allocTemporaryResult (result);

    unsigned index = 0;
    for (TreeNodeExpr& child : e->expressions ()) {
        const CGResult& exprResult = codeGen (&child);
        append (result, exprResult);
        if (result.isNotOk ())
            return result;

        emplaceImopAfter (result, e, Imop::STORE, resultSymbol, indexConstant (index), exprResult.symbol ());
        releaseTemporary (result, exprResult.symbol ());
        ++ index;
    }

    return result;
}

/*******************************************************************************
  TreeNodeExprInt
*******************************************************************************/

CGResult TreeNodeExprInt::codeGenWith(CodeGen & cg) {
    return cg.cgExprInt(this);
}

CGResult CodeGen::cgExprInt(TreeNodeExprInt * e) {
    // Type check:
    if (m_tyChecker->visitExprInt(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    result.setResult(numericConstant(e->resultType()->secrecDataType(),
                                     e->value()));
    return result;
}

/*******************************************************************************
  TreeNodeExprBool
*******************************************************************************/

CGResult TreeNodeExprBool::codeGenWith(CodeGen & cg) {
    return cg.cgExprBool(this);
}

CGResult CodeGen::cgExprBool(TreeNodeExprBool * e) {
    // Type check:
    if (m_tyChecker->visitExprBool(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    result.setResult(ConstantInt::getBool (e->value()));
    return result;
}

CGBranchResult TreeNodeExprBool::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolExprBool(this);
}

CGBranchResult CodeGen::cgBoolExprBool(TreeNodeExprBool * e) {
    CGBranchResult result;
    auto i = new Imop(e, Imop::JUMP, nullptr);
    push_imop(i);
    result.setFirstImop(i);

    if (e->value()) {
        result.addToTrueList(i);
    } else {
        result.addToFalseList(i);
    }

    return result;
}

/*******************************************************************************
  TreeNodeExprClassify
*******************************************************************************/

CGResult TreeNodeExprClassify::codeGenWith(CodeGen & cg) {
    return cg.cgExprClassify(this);
}

CGResult CodeGen::cgExprClassify(TreeNodeExprClassify * e) {
    // Type check:
    if (m_tyChecker->visitExprClassify(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    // Generate code for child expression
    TreeNodeExpr * eArg = e->expression();
    CGResult result(codeGen(eArg));
    if (result.isNotOk()) {
        return result;
    }

    // Generate temporary for the result of the classification, if needed:
    Symbol * argSym = result.symbol();
    SymbolSymbol * resSym = generateResultSymbol(result, e);
    resSym->inheritShape(argSym);

    if (argSym->isConstant ()) {
        SymbolSymbol* sizeSym = m_st->appendTemporary(TypeBasic::get (DATATYPE_UINT64));
        Symbol* one = static_cast<Symbol*> (ConstantInt::get(DATATYPE_UINT64, 1));
        pushImopAfter(result, newUnary(e, Imop::ASSIGN, sizeSym, one));
        resSym->setSizeSym (sizeSym);
    }

    allocTemporaryResult(result);
    pushImopAfter(result, newUnary(e, Imop::CLASSIFY, resSym, argSym));
    releaseTemporary(result, argSym);
    return result;
}

/*******************************************************************************
  TreeNodeExprDeclassify
*******************************************************************************/

CGResult TreeNodeExprDeclassify::codeGenWith(CodeGen & cg) {
    return cg.cgExprDeclassify(this);
}

CGResult CodeGen::cgExprDeclassify(TreeNodeExprDeclassify * e) {
    // Type check:
    if (m_tyChecker->visitExprDeclassify(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    // Generate code for child expression
    CGResult result(codeGen(e->expression()));
    if (result.isNotOk()) {
        return result;
    }

    // Generate temporary for the result of the classification, if needed:
    Symbol * argSym = result.symbol();
    assert(argSym != nullptr);
    SymbolSymbol * resSym = generateResultSymbol(result, e);
    resSym->inheritShape(argSym);
    allocTemporaryResult(result);
    pushImopAfter(result, newUnary(e, Imop::DECLASSIFY, resSym, argSym));
    releaseTemporary(result, argSym);
    return result;
}

CGBranchResult TreeNodeExprDeclassify::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolSimple(this);
}

/*******************************************************************************
  TreeNodeExprUnary
*******************************************************************************/

CGResult TreeNodeExprUnary::codeGenWith(CodeGen & cg) {
    return cg.cgExprUnary(this);
}

CGResult CodeGen::cgExprUnary(TreeNodeExprUnary * e) {
    // Type check:
    if (m_tyChecker->visitExprUnary(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    if (e->isOverloaded()) {
        CGResult result;

        const CGResult& argResult(codeGen(e->expression ()));
        append(result, argResult);
        if (result.isNotOk())
            return result;

        std::vector<Symbol*> operands {argResult.symbol ()};
        const CGResult& callRes(cgOverloadedExpr(e, e->resultType(), operands, e->procSymbol()));
        append(result, callRes);
        if (result.isNotOk()) return result;
        result.setResult(callRes.symbol());

        return result;
    }

    // Generate code for child expression:
    TreeNodeExpr * eArg = e->expression();
    CGResult result(codeGen(eArg));
    if (!result.isOk()) {
        return result;
    }

    Symbol * eResult = result.symbol();
    SymbolSymbol * resSym = generateResultSymbol(result, e);
    resSym->inheritShape(eResult);  // no need to copy the symbols
    allocTemporaryResult(result);

    // Generate code for unary expression:
    Imop::Type iType;
    switch (e->type ()) {
    case NODE_EXPR_UINV: iType = Imop::UINV; break;
    case NODE_EXPR_UNEG: iType = Imop::UNEG; break;
    default:
        assert(e->type() == NODE_EXPR_UMINUS);
        iType = Imop::UMINUS;
        break;
    }

    pushImopAfter(result, newUnary(e, iType, result.symbol(), eResult));
    releaseTemporary(result, eResult);
    return result;
}

CGBranchResult TreeNodeExprUnary::codeGenBoolWith (CodeGen &cg) {
    assert (havePublicBoolType ());
    return cg.cgBoolExprUnary (this);
}

CGBranchResult CodeGen::cgBoolExprUnary(TreeNodeExprUnary * e) {
    // Generate code for child expression:
    if (e->isOverloaded()) {
        return cgBoolSimple(e);
    }
    else {
        CGBranchResult result = codeGenBranch(e->expression());
        if (!result.isOk()) {
            return result;
        }

        result.swapTrueFalse();
        return result;
    }
}

/******************************************************************
  TreeNodeExprPrefix
******************************************************************/

// Used in the loop when the expression is overloaded and indexed
// (e.g. x[a:b]++)
CGResult CodeGen::cgOverloadedPrefixPostfix(TreeNodeExpr* e,
                                            SymbolProcedure* procSymbol,
                                            Symbol* x,
                                            Symbol* one)
{
    CGResult result;
    Symbol* firstArg;
    Symbol* secondArg;

    // Copy arguments for the procedure call
    if (x->secrecType()->secrecSecType()->isPublic()) {
        firstArg = x;
    }
    else {
        assert(dynamic_cast<SymbolSymbol*>(x) != nullptr);
        SymbolSymbol* xSym = static_cast<SymbolSymbol*>(x);
        firstArg = m_st->appendTemporary(xSym->secrecType());
        emplaceImopAfter(result, e, Imop::DECLARE, firstArg);
        emplaceImop(e, Imop::ASSIGN, firstArg, xSym);
    }

    if (one->secrecType()->secrecSecType()->isPublic()) {
        secondArg = one;
    }
    else {
        assert(dynamic_cast<SymbolSymbol*>(one) != nullptr);
        SymbolSymbol* oneSym = static_cast<SymbolSymbol*>(one);
        secondArg = m_st->appendTemporary(oneSym->secrecType());
        emplaceImopAfter(result, e, Imop::DECLARE, secondArg);
        emplaceImop(e, Imop::ASSIGN, secondArg, oneSym);
    }

    std::vector<Symbol*> args {firstArg, secondArg};
    CGResult callRes(cgOverloadedExpr(e, firstArg->secrecType(), args, procSymbol));
    append(result, callRes);
    if (result.isNotOk()) return result;
    result.setResult(callRes.symbol());

    return result;
}

CGResult TreeNodeExprPrefix::codeGenWith(CodeGen & cg) {
    return cg.cgExprPrefix(this);
}

CGResult CodeGen::cgExprPrefix(TreeNodeExprPrefix * e) {
    // Type check:
    if (m_tyChecker->visitExprPrefix(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    TreeNodeLValue * lval = e->lvalue ();
    const bool isScalar = e->resultType ()->isScalar ();
    const bool isOverloaded = e->isOverloaded ();

    // Generate code for the lvalue:
    SubscriptInfo subscript;
    bool isIndexed = false;
    const CGResult& lvalResult = cgLValue (lval, subscript, isIndexed);
    append (result, lvalResult);
    if (result.isNotOk ()) {
        return result;
    }

    SymbolSymbol * x = static_cast<SymbolSymbol*>(lvalResult.symbol ());
    SymbolSymbol * r = generateResultSymbol(result, e);
    const TypeNonVoid * pubIntTy = TypeBasic::getIndexType();

    assert(dynamic_cast<const TypeNonVoid*>(e->resultType()) != nullptr);
    const TypeNonVoid * resTy = static_cast<const TypeNonVoid*>(e->resultType());
    const DataType * pubResTy = dtypeDeclassify(resTy->secrecSecType(),
                                                resTy->secrecDataType());
    const TypeNonVoid * elemType = TypeBasic::get(e->resultType()->secrecSecType(),
                                                  e->resultType()->secrecDataType());
    Symbol * one = numericConstant(pubResTy, 1);
    Symbol* const idxOne = indexConstant (1);
    bool classifyOverloaded = false;

    if (isOverloaded) {
        SymbolProcedure* symProc = e->procSymbol ();
        const TypeProc* tyProc = symProc->decl ()->procedureType ();
        assert (tyProc->paramTypes ().size () == 2u);
        if (tyProc->paramTypes ()[1u]->secrecSecType ()->isPrivate ())
            classifyOverloaded = true;
    }

    // either use ADD or SUB
    Imop::Type iType;
    switch (e->type()) {
    case NODE_EXPR_PREFIX_INC: iType = Imop::ADD; break;
    case NODE_EXPR_PREFIX_DEC: iType = Imop::SUB; break;
    default:
        assert(false && "ICE: prefix operator on something other than increment or decrement (wut?)!");
        result.setStatus(CGResult::ERROR_FATAL);
        return result;
    }

    if (isIndexed) {
        // r = (++ x[is])

        const SubscriptInfo::SPV & spv = subscript.spv();
        ArrayStrideInfo stride(x);
        append(result, codeGenStride(stride));
        if (result.isNotOk()) {
            return result;
        }

        // Initialize required temporary symbols:
        LoopInfo loopInfo = prepareLoopInfo (subscript);
        Symbol * offset = m_st->appendTemporary(pubIntTy);
        Symbol * resultOffset = m_st->appendTemporary (pubIntTy);
        Symbol * tmpResult = m_st->appendTemporary(pubIntTy);
        Symbol * tmpElem = m_st->appendTemporary(elemType);

        // compute the shape and the size of the result symbol "r"
        {
            unsigned count = 0;
            for (unsigned k : subscript.slices()) {
                Symbol * sym = r->getDim(count);
                emplaceImopAfter(result, e, Imop::SUB, sym, spv[k].second, spv[k].first);
                ++ count;
            }

            codeGenSize(result);
        }

        // allocate memory for the result symbol "r"
        if (! isScalar) {
            if (e->resultType()->secrecSecType()->isPrivate()) {
                emplaceImopAfter(result, e, Imop::ALLOC, r, r->getSizeSym());
            }
            else {
                Symbol * def = defaultConstant(getContext(), e->resultType()->secrecDataType());
                emplaceImopAfter(result, e, Imop::ALLOC, r, r->getSizeSym(), def);
            }
        }
        else {
            emplaceImopAfter(result, e, Imop::DECLARE, r);
        }

        emplaceImopAfter(result, e, Imop::DECLARE, tmpElem);

        if (classifyOverloaded) {
            Symbol * t = m_st->appendTemporary(elemType);
            emplaceImopAfter(result, e, Imop::DECLARE, t);
            emplaceImop(e, Imop::CLASSIFY, t, one);
            one = t;
        }

        // resultOffset = 0
        pushImopAfter (result, newAssign (e, resultOffset, indexConstant (0)));

        // Enter the loop:
        append(result, enterLoop(loopInfo, spv));
        if (result.isNotOk()) {
            return result;
        }

        // Loop body:
        {
            // compute offset:
            emplaceImop(e, Imop::ASSIGN, offset, indexConstant(0));
            LoopInfo::const_iterator idxIt = loopInfo.begin();
            for (unsigned k = 0; k < stride.size(); ++ k, ++ idxIt) {
                emplaceImop(e, Imop::MUL, tmpResult, stride.at(k), *idxIt);
                emplaceImop(e, Imop::ADD, offset, offset, tmpResult);
            }

            // t = x[offset]
            emplaceImop(e, Imop::LOAD, tmpElem, x, offset);

            // t = t + 1
            if (isOverloaded) {
                const CGResult& callRes(cgOverloadedPrefixPostfix(e, e->procSymbol(),
                                                                  tmpElem, one));
                append(result, callRes);
                if (result.isNotOk())
                    return result;

                // Copy result
                emplaceImopAfter(result, e, Imop::ASSIGN, tmpElem, callRes.symbol());
                releaseTemporary(result, callRes.symbol());
            }
            else {
                emplaceImop(e, iType, tmpElem, tmpElem, one);
            }

            // x[offset] = t
            emplaceImop(e, Imop::STORE, x, offset, tmpElem);

            if (isScalar) {
                // r = t
                emplaceImop(e, Imop::ASSIGN, r, tmpElem);
            }
            else{
                // r[resultOffset] = t
                emplaceImop(e, Imop::STORE, r, resultOffset, tmpElem);
            }

            // Increment the result "r" offset
            // resultOffset = resultOffset + 1
            emplaceImop (e, Imop::ADD, resultOffset, resultOffset, idxOne);
        }

        // Exit the loop:
        append(result, exitLoop(loopInfo));

        releaseTemporary (result, tmpElem);
        releaseTemporary (result, one);

        return result;
    }
    else {
        // r = (++ x)

        if (isOverloaded) {
            if (classifyOverloaded) {
                // elemType - scalar in x
                SymbolSymbol * t = m_st->appendTemporary(elemType);
                emplaceImopAfter(result, e, Imop::DECLARE, t);
                emplaceImop(e, Imop::CLASSIFY, t, one);
                one = t;
            }
        }
        else if (!isScalar) {
            SymbolSymbol * t = m_st->appendTemporary(resTy);
            emplaceImopAfter(result, e, Imop::ALLOC, t, x->getSizeSym(), one);
            one = t;
        }

        // x = x `iType` 1
        if (isOverloaded) {
            std::vector<Symbol*> args {x, one};
            const CGResult& callRes(cgOverloadedExpr(e, e->resultType(),
                                                     args, e->procSymbol ()));
            append(result, callRes);
            if (result.isNotOk())
                return result;

            // Copy result
            if (isScalar) {
                if (callRes.symbol()->secrecType()->secrecDimType() != 0u)
                    emplaceImopAfter(result, e, Imop::ASSIGN, x, callRes.symbol(), indexConstant(0u));
                else
                    emplaceImopAfter(result, e, Imop::ASSIGN, x, callRes.symbol());
            }
            else {
                emplaceImopAfter(result, e, Imop::RELEASE, nullptr, x);
                emplaceImop(e, Imop::COPY, x, callRes.symbol(), x->getSizeSym());
            }

            releaseTemporary(result, callRes.symbol());
        }
        else {
            pushImopAfter(result, newBinary(e, iType, x, x, one));
        }

        // Copy the value of e to the result "r"
        if (!isScalar) {
            copyShapeFrom(result, x);
            emplaceImopAfter(result, e, Imop::COPY, r, x, x->getSizeSym());
        }
        else {
            emplaceImopAfter(result, e, Imop::DECLARE, r);
            push_imop(newAssign(e, r, x));
        }

        if (!isOverloaded)
            // If the type is overloaded the procedure call will release it
            releaseTemporary(result, one);

        return result;
    }
}

/******************************************************************
  TreeNodeExprPostfix
******************************************************************/

CGResult TreeNodeExprPostfix::codeGenWith(CodeGen & cg) {
    return cg.cgExprPostfix(this);
}

// TODO: this has tons of common code with cgExprIndex. We can probably refactor
// this using a higher order function.
CGResult CodeGen::cgExprPostfix(TreeNodeExprPostfix * e) {

    // Type check:
    if (m_tyChecker->visitExprPostfix(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    const TypeNonVoid * pubIntTy = TypeBasic::getIndexType();
    TreeNodeLValue * lval = e->lvalue ();

    assert(dynamic_cast<const TypeNonVoid*>(e->resultType()) != nullptr);
    const TypeNonVoid * resTy = static_cast<const TypeNonVoid*>(e->resultType());
    const TypeNonVoid * elemType = TypeBasic::get(e->resultType()->secrecSecType(),
                                                  e->resultType()->secrecDataType());
    const DataType * pubResTy = dtypeDeclassify(resTy->secrecSecType(),
                                                resTy->secrecDataType());
    Symbol* one = numericConstant(pubResTy, 1);
    Symbol* const idxOne = indexConstant (1);
    bool classifyOverloaded = false;
    bool isOverloaded = e->isOverloaded();

    if (isOverloaded) {
        SymbolProcedure* symProc = e->procSymbol ();
        const TypeProc* tyProc = symProc->decl ()->procedureType ();
        assert (tyProc->paramTypes ().size () == 2u);
        if (tyProc->paramTypes ()[1u]->secrecSecType ()->isPrivate ())
            classifyOverloaded = true;
    }

    // Generate code for the lvalue:
    SubscriptInfo subscript;
    bool isIndexed = false;
    const CGResult& lvalResult = cgLValue (lval, subscript, isIndexed);
    append (result, lvalResult);
    if (result.isNotOk ()) {
        return result;
    }

    SymbolSymbol * lvalSym = static_cast<SymbolSymbol*>(lvalResult.symbol ());
    SymbolSymbol * r = generateResultSymbol(result, e);

    // either use ADD or SUB
    Imop::Type iType;
    switch (e->type()) {
    case NODE_EXPR_POSTFIX_INC: iType = Imop::ADD; break;
    case NODE_EXPR_POSTFIX_DEC: iType = Imop::SUB; break;
    default:
        assert(false && "ICE: postfix operator on something other than increment or decrement (wut?)!");
        result.setStatus(CGResult::ERROR_FATAL);
        return result;
    }

    const bool isScalar = e->resultType ()->isScalar ();

    if (isIndexed) {
        // r = (e[is] ++)

        const auto & spv = subscript.spv();
        ArrayStrideInfo stride(lvalSym);
        append(result, codeGenStride(stride));
        if (result.isNotOk()) {
            return result;
        }

        // Initialize required temporary symbols:
        LoopInfo loopInfo = prepareLoopInfo (subscript);
        Symbol * rhsOffset = m_st->appendTemporary(pubIntTy);
        Symbol * resultOffset = m_st->appendTemporary (pubIntTy);
        Symbol * tmpResult = m_st->appendTemporary(pubIntTy);
        Symbol * tmpElem = m_st->appendTemporary(elemType);

        // compute the shape and the size of the result symbol "r"
        {
            unsigned count = 0;
            for (unsigned k : subscript.slices()) {
                Symbol * sym = r->getDim(count);
                emplaceImopAfter(result, e, Imop::SUB, sym, spv[k].second, spv[k].first);
                ++ count;
            }

            codeGenSize(result);
        }

        // allocate memory for the result symbol "r"
        if (! isScalar) {
            if (e->resultType()->secrecSecType()->isPrivate()) {
                emplaceImopAfter(result, e, Imop::ALLOC, r, r->getSizeSym());
            }
            else {
                Symbol * def = defaultConstant(getContext(), e->resultType()->secrecDataType());
                emplaceImopAfter(result, e, Imop::ALLOC, r, r->getSizeSym(), def);
            }
        }
        else {
            emplaceImopAfter(result, e, Imop::DECLARE, r);
        }

        emplaceImopAfter(result, e, Imop::DECLARE, tmpElem);

        // make the symbol "one" private if need be
        if (classifyOverloaded) {
            Symbol * t = m_st->appendTemporary(elemType);
            emplaceImopAfter(result, e, Imop::DECLARE, t);
            emplaceImop(e, Imop::CLASSIFY, t, one);
            one = t;
        }

        // resultOffset = 0
        pushImopAfter (result, newAssign (e, resultOffset, indexConstant (0)));

        // Enter the loop:
        append(result, enterLoop(loopInfo, spv));
        if (result.isNotOk()) {
            return result;
        }

        // Loop body:
        {
            // compute RHS offset:
            emplaceImop(e, Imop::ASSIGN, rhsOffset, indexConstant(0));
            LoopInfo::const_iterator idxIt = loopInfo.begin();
            for (unsigned k = 0; k < stride.size(); ++ k, ++ idxIt) {
                emplaceImop(e, Imop::MUL, tmpResult, stride.at(k), *idxIt);
                emplaceImop(e, Imop::ADD, rhsOffset, rhsOffset, tmpResult);
            }

            // t = x[rhsOffset]
            emplaceImop(e, Imop::LOAD, tmpElem, lvalSym, rhsOffset);

            if (isScalar) {
                // r = t
                emplaceImop(e, Imop::ASSIGN, r, tmpElem);
            }
            else {
                // r[resultOffset] = t
                emplaceImop(e, Imop::STORE, r, resultOffset, tmpElem);
            }

            // t = t + 1
            if (isOverloaded) {
                CGResult callRes(cgOverloadedPrefixPostfix(e, e->procSymbol(), tmpElem, one));
                append(result, callRes);
                if (result.isNotOk())
                    return result;

                // Copy the result of the call to tmpElem
                emplaceImopAfter(result, e, Imop::ASSIGN, tmpElem, callRes.symbol());
                releaseTemporary(result, callRes.symbol());
            }
            else {
                emplaceImop(e, iType, tmpElem, tmpElem, one);
            }

            // x[rhsOffset] = t
            emplaceImop(e, Imop::STORE, lvalSym, rhsOffset, tmpElem);

            // Increment the result "r" offset
            // resultOffset = resultOffset + 1
            emplaceImop (e, Imop::ADD, resultOffset, resultOffset, idxOne);
        }

        // Exit the loop:
        append(result, exitLoop(loopInfo));

        releaseTemporary(result, tmpElem);
        releaseTemporary(result, one);

        return result;
    }
    else {
        // r = (e ++)

        // Copy the value of e to the result "r"
        if (! isScalar) {
            copyShapeFrom(result, lvalSym);
            emplaceImopAfter(result, e, Imop::COPY, r, lvalSym, lvalSym->getSizeSym());
        }
        else {
            emplaceImopAfter(result, e, Imop::DECLARE, r);
            push_imop(newAssign(e, r, lvalSym));
        }

        // Construct the "1" value to add to the lvalue. The value could be private and/or array.
        if (isOverloaded) {
            if (classifyOverloaded) {
                SymbolSymbol * t = m_st->appendTemporary(elemType);
                emplaceImopAfter(result, e, Imop::DECLARE, t);
                emplaceImop(e, Imop::CLASSIFY, t, one);
                one = t;
            }
        }
        else if (!isScalar) {
            SymbolSymbol * t = m_st->appendTemporary(resTy);
            emplaceImopAfter(result, e, Imop::ALLOC, t, lvalSym->getSizeSym(), one);
            one = t;
        }

        // x = x `iType` 1
        if (isOverloaded) {
            std::vector<Symbol*> args {lvalSym, one};
            CGResult callRes(cgOverloadedExpr(e, e->resultType(), args, e->procSymbol ()));
            append(result, callRes);
            if (result.isNotOk())
                return result;

            // Copy result
            if (isScalar) {
                if (callRes.symbol()->secrecType()->secrecDimType() != 0u)
                    emplaceImopAfter(result, e, Imop::ASSIGN, lvalSym, callRes.symbol(), indexConstant(0u));
                else
                    emplaceImopAfter(result, e, Imop::ASSIGN, lvalSym, callRes.symbol());
            }
            else {
                emplaceImopAfter(result, e, Imop::RELEASE, nullptr, lvalSym);
                emplaceImop(e, Imop::COPY, lvalSym, callRes.symbol(), lvalSym->getSizeSym());
            }

            releaseTemporary(result, callRes.symbol());
        }
        else {
            pushImopAfter(result, newBinary(e, iType, lvalSym, lvalSym, one));
        }

        if (!isOverloaded)
            releaseTemporary(result, one);

        return result;
    }
}

/*******************************************************************************
  TreeNodeExprSelection
*******************************************************************************/

CGResult TreeNodeExprSelection::codeGenWith(CodeGen & cg) {
    return cg.cgExprSelection(this);
}

CGResult CodeGen::cgExprSelection(TreeNodeExprSelection* root) {
    if (m_tyChecker->visitExprSelection(root) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    TreeNodeExpr* e = root->expression ();
    CGResult result;

    // Generate code for the subexpression:
    CGResult exprResult = codeGen (e);
    append (result, exprResult);
    if (result.isNotOk ())
        return result;

    // Pick the proper field:
    assert (dynamic_cast<SymbolSymbol*>(exprResult.symbol ()) != nullptr);
    SymbolSymbol* exprValue = static_cast<SymbolSymbol*>(exprResult.symbol ());
    StringRef fieldName = root->identifier ()->value ();
    if (SymbolSymbol* fieldValue = lookupField (exprValue, fieldName)) {
        result.setResult (fieldValue);
        releaseTemporary (result, exprResult.symbol (), fieldValue);
        // TODO: Remove temporaries?
        return result;
    }

    // The following should have been rules out by the type checker:
    m_log.fatal () << "ICE: invalid code generation for attribute selection expression at " << root->location () << ".";
    return CGResult::ERROR_CONTINUE;
}

CGBranchResult TreeNodeExprSelection::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolExprSelection(this);
}

CGBranchResult CodeGen::cgBoolExprSelection(TreeNodeExprSelection* e) {
    return cgBoolSimple(e);
}

/*******************************************************************************
  TreeNodeExprStrlen
*******************************************************************************/

CGResult TreeNodeExprStrlen::codeGenWith (CodeGen& cg) {
    return cg.cgExprStrlen(this);
}

CGResult CodeGen::cgExprStrlen(TreeNodeExprStrlen* e) {
    if (m_tyChecker->visitExprStrlen (e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;
    const CGResult & argResult = codeGen(e->expression());
    append(result, argResult);
    if (result.isNotOk()) {
        return result;
    }

    Symbol* strSym = argResult.symbol();
    SymbolSymbol* resSym = generateResultSymbol(result, e);
    emplaceImopAfter (result, e, Imop::STRLEN, resSym, strSym);
    releaseTemporary(result, strSym);
    return result;
}

} // namespace SecreC
