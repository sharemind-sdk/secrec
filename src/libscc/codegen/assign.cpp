#include "codegen.h"
#include "constant.h"
#include "misc.h"
#include "symboltable.h"
#include "treenode.h"

#include <sstream>
#include <vector>


/**
 * Code generation for assignment expression
 */

namespace SecreC {

namespace /* anonymous */ {

bool getAssignBinImopType(SecrecTreeNodeType type, Imop::Type& iType) {
    switch (type) {
    case NODE_EXPR_BINARY_ASSIGN_MUL: iType = Imop::MUL;  break;
    case NODE_EXPR_BINARY_ASSIGN_DIV: iType = Imop::DIV;  break;
    case NODE_EXPR_BINARY_ASSIGN_MOD: iType = Imop::MOD;  break;
    case NODE_EXPR_BINARY_ASSIGN_ADD: iType = Imop::ADD;  break;
    case NODE_EXPR_BINARY_ASSIGN_SUB: iType = Imop::SUB;  break;
    case NODE_EXPR_BINARY_ASSIGN_AND: iType = Imop::BAND; break;
    case NODE_EXPR_BINARY_ASSIGN_OR:  iType = Imop::BOR;  break;
    case NODE_EXPR_BINARY_ASSIGN_XOR: iType = Imop::XOR;  break;
    default:
        return false;
    }

    return true;
}

} // namespace anonymous

/*******************************************************************************
  TreeNodeExprAssign
*******************************************************************************/

CGResult CodeGen::cgLValue (TreeNodeLValue* lval, SubscriptInfo& subInfo, bool& isIndexed) {
    assert (lval != NULL);

    if (lval->isIndex ()) {
        TreeNodeExprIndex* lvalIndex = lval->index ();
        assert ((lvalIndex->expression ()->type () == NODE_EXPR_RVARIABLE) && "TODO: how do we index a field?");
        TreeNodeIdentifier* eArg1 = static_cast<TreeNodeExprRVariable*>(lvalIndex->expression ())->identifier ();
        SymbolSymbol * destSym = m_st->find<SYM_SYMBOL>(eArg1->value());

        isIndexed = true;
        CGResult result;
        result.setResult (destSym);
        append(result, codeGenSubscript(subInfo, destSym, lvalIndex->indices ()));
        return result;
    }

    if (lval->isIdentifier ()) {
        TreeNodeIdentifier* ident = lval->identifier ();
        SymbolSymbol * destSym = m_st->find<SYM_SYMBOL>(ident->value());

        isIndexed = false;
        CGResult result;
        result.setResult (destSym);
        return result;
    }

    if (lval->isSelection ()) {
        isIndexed = false;
        return codeGen (lval->selection ());
    }

    return CGResult::ERROR_CONTINUE;
}

CGResult TreeNodeExprAssign::codeGenWith (CodeGen &cg) {
    return cg.cgExprAssign (this);
}

CGResult CodeGen::cgExprAssign(TreeNodeExprAssign * e) {
    typedef SubscriptInfo::SPV SPV; // symbol pair vector

    // Type check:
    if (m_tyChecker->visit(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;

    // Generate code for the righthand side:
    TreeNodeExpr * eArg2 = e->rightHandSide();
    const CGResult & arg2Result = codeGen(eArg2);
    append(result, arg2Result);
    if (result.isNotOk()) {
        return result;
    }

    result.setResult (arg2Result.symbol ());

    // Generate code for the left hand side;
    TreeNodeLValue* lval = e->leftHandSide ();
    SubscriptInfo cgSub;
    bool isIndexed = false;
    const CGResult & lvalResult = cgLValue (lval, cgSub, isIndexed);
    append (result, lvalResult);
    if (result.isNotOk ()) {
        return result;
    }

    SymbolSymbol * destSym = static_cast<SymbolSymbol*>(lvalResult.symbol ());
    TypeBasic * pubIntTy = TypeBasic::getIndexType(getContext());
    TypeBasic * pubBoolTy = TypeBasic::getPublicBoolType(getContext());

    // x[e1,...,ek] = e
    if (isIndexed) {
        const SPV & spv = cgSub.spv();
        const SubscriptInfo::SliceIndices & slices = cgSub.slices();

        // 2. check that rhs has correct dimensions
        if (!eArg2->resultType()->isScalar()) {
            assert(static_cast<size_t>(eArg2->resultType()->secrecDimType()) == slices.size());

            std::stringstream ss;
            ss << "Shape of RHS doesnt match shape of LHS in assignment at " << e->location() << '.';
            Imop * jmp = new Imop(e, Imop::JUMP, (Symbol *) 0);
            Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
            SymbolLabel * errLabel = m_st->label(err);
            SymbolSymbol * arg2ResultSymbol = static_cast<SymbolSymbol *>(arg2Result.symbol());

            for (SecrecDimType k = 0; k < eArg2->resultType()->secrecDimType(); ++ k) {
                Symbol * tsym = m_st->appendTemporary(pubIntTy);
                Symbol * bsym = m_st->appendTemporary(pubBoolTy);
                Imop * i = new Imop(e, Imop::SUB, tsym, spv[slices[k]].second, spv[slices[k]].first);
                pushImopAfter(result, i);

                i = new Imop(e, Imop::NE, bsym, tsym, arg2ResultSymbol->getDim(k));
                push_imop(i);

                i = new Imop(e, Imop::JT, errLabel, bsym);
                push_imop(i);
            }

            push_imop(jmp);
            result.addToNextList(jmp);
            push_imop(err);
        }

        // 3. initialize stride
        ArrayStrideInfo stride(destSym);
        append(result, codeGenStride(stride));
        if (result.isNotOk()) {
            return result;
        }

        // 4. initialze running indices
        LoopInfo loopInfo;
        for (SPV::const_iterator it(spv.begin()); it != spv.end(); ++ it) {
            Symbol * sym = m_st->appendTemporary(pubIntTy);
            loopInfo.push_index(sym);
        }

        // 6. initialze symbols for offsets and temporary results
        Symbol * offset = m_st->appendTemporary(pubIntTy);
        Symbol * old_offset = m_st->appendTemporary(pubIntTy);
        Symbol * tmp_result2 = m_st->appendTemporary(pubIntTy);

        // offset = 0
        Imop * i = new Imop(e, Imop::ASSIGN, offset, indexConstant(0));
        pushImopAfter(result, i);

        // 7. start
        append(result, enterLoop(loopInfo, spv));
        if (result.isNotOk()) {
            return result;
        }

        // 8. compute offset for RHS
        {
            // old_ffset = 0
            Imop * i = new Imop(e, Imop::ASSIGN, old_offset, indexConstant(0));
            pushImopAfter(result, i);

            unsigned count = 0;
            LoopInfo::iterator itIt = loopInfo.begin();
            LoopInfo::iterator itEnd = loopInfo.end();
            for (; itIt != itEnd; ++ count, ++ itIt) {
                // tmp_result2 = s[k] * idx[k]
                i = new Imop(e, Imop::MUL, tmp_result2, stride.at(count), *itIt);
                push_imop(i);

                // old_offset = old_offset + tmp_result2
                i = new Imop(e, Imop::ADD, old_offset, old_offset, tmp_result2);
                push_imop(i);
            }
        }

        // 9. load and store
        {
            TypeBasic * ty = TypeBasic::get(getContext(),
                    e->resultType()->secrecSecType(), e->resultType()->secrecDataType());
            if (e->type() == NODE_EXPR_BINARY_ASSIGN) {
                if (!eArg2->resultType()->isScalar()) {
                    Symbol * t1 = m_st->appendTemporary(ty);

                    Imop * i = new Imop(e, Imop::LOAD, t1, arg2Result.symbol(), offset);
                    push_imop(i);

                    i = new Imop(e, Imop::STORE, destSym, old_offset, t1);
                    push_imop(i);
                }
                else {
                    Imop * i = new Imop(e, Imop::STORE, destSym, old_offset, arg2Result.symbol());
                    push_imop(i);
                }
            }
            else {
                Symbol * t1 = m_st->appendTemporary(ty);
                Symbol * t2 = m_st->appendTemporary(ty);
                Imop::Type iType;
                if (! getAssignBinImopType(e->type(), iType)) {
                    assert(false);  // shouldn't happen
                    result |= CGResult::ERROR_FATAL;
                    return result;
                }

                Imop * i = new Imop(e, Imop::LOAD, t1, destSym, old_offset);
                push_imop(i);

                if (!eArg2->resultType()->isScalar()) {
                    i = new Imop(e, Imop::LOAD, t2, arg2Result.symbol(), offset);
                    push_imop(i);

                    i = new Imop(e, iType, t1, t1, t2);
                    push_imop(i);

                    i = new Imop(e, Imop::STORE, destSym, old_offset, t1);
                    push_imop(i);
                }
                else {
                    i = new Imop(e, iType, t1, t1, arg2Result.symbol());
                    push_imop(i);

                    i = new Imop(e, Imop::STORE, destSym, old_offset, t1);
                    push_imop(i);
                }
            }

            // offset = offset + 1
            Imop * i = new Imop(e, Imop::ADD, offset, offset, indexConstant(1));
            push_imop(i);
        }

        // 9. loop exit
        append(result, exitLoop(loopInfo));
        if (result.isNotOk()) {
            return result;
        }

        return result;
    }
    else {
        // Generate code for regular x = e assignment
        if (e->type() == NODE_EXPR_BINARY_ASSIGN) {
            if (!eArg2->resultType()->isScalar()) {
                result.setResult (destSym); // hack!
                copyShapeFrom(result, arg2Result.symbol());
                result.setResult (arg2Result.symbol());
                if (result.isNotOk()) {
                    return result;
                }
            }

            Imop * i = NULL;
            if (destSym->isArray()) {
                i = new Imop(e, Imop::RELEASE, 0, destSym);
                pushImopAfter(result, i);

                if (eArg2->resultType()->isScalar()) {
                    i = new Imop(e, Imop::ALLOC, destSym, arg2Result.symbol(), destSym->getSizeSym());
                }
                else {
                    i = new Imop(e, Imop::COPY, destSym, arg2Result.symbol(), destSym->getSizeSym());
                }

                pushImopAfter(result, i);
            } else {
                i = new Imop(e, Imop::ASSIGN, destSym, arg2Result.symbol());
                pushImopAfter(result, i);
            }
        } else {
            // Arithmetic assignments

            Imop::Type iType;
            if (! getAssignBinImopType(e->type(), iType)) {
                assert(false);  // shouldn't happen
                result |= CGResult::ERROR_FATAL;
                return result;
            }

            Imop * i = NULL;
            if (destSym->isArray()) {
                if (eArg2->resultType()->isScalar()) {
                    Symbol * rhsSym = m_st->appendTemporary(destSym->secrecType());
                    i = new Imop(e, Imop::ALLOC, rhsSym, arg2Result.symbol(), destSym->getSizeSym());
                    pushImopAfter(result, i);
                    i = new Imop(e, iType, destSym, destSym, rhsSym, destSym->getSizeSym());
                    pushImopAfter(result, i);
                    releaseTemporary(result, rhsSym);
                }
                else {
                    std::stringstream ss;
                    ss << "Shape of RHS doesn't match shape of LHS in assignment at " << e->location() << '.';
                    Imop * jmp = new Imop(e, Imop::JUMP, (Symbol *) 0);
                    Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
                    SymbolLabel * errLabel = m_st->label(err);
                    SymbolSymbol * arg2ResultSymbol = static_cast<SymbolSymbol *>(arg2Result.symbol());

                    dim_iterator
                        di = dim_begin(arg2ResultSymbol),
                           dj = dim_begin(destSym),
                           de = dim_end(arg2ResultSymbol);
                    for (; di != de; ++ di, ++ dj) {
                        SymbolTemporary * temp_bool = m_st->appendTemporary(pubBoolTy);

                        i = new Imop(e, Imop::NE, temp_bool, *di, *dj);
                        push_imop(i);

                        i = new Imop(e, Imop::JT, errLabel, temp_bool);
                        push_imop(i);
                    }

                    push_imop(jmp);
                    result.addToNextList(jmp);
                    push_imop(err);
                    i = new Imop(e, iType, destSym, destSym, arg2Result.symbol(), destSym->getSizeSym());
                    pushImopAfter(result, i);
                }
            }
            else {
                i = new Imop(e, iType, destSym, destSym, arg2Result.symbol());
                pushImopAfter(result, i);
            }
        }
    }

    return result;
}

CGBranchResult TreeNodeExprAssign::codeGenBoolWith(CodeGen & cg) {
    assert(havePublicBoolType());
    return cg.cgBoolExprAssign(this);
}

CGBranchResult CodeGen::cgBoolExprAssign(TreeNodeExprAssign * e) {
    CGBranchResult result = codeGen(e);
    if (result.isNotOk()) {
        return result;
    }

    Imop * i = new Imop(e, Imop::JT, 0, result.symbol());
    pushImopAfter(result, i);
    result.addToTrueList(i);

    i = new Imop(e, Imop::JUMP, 0);
    push_imop(i);
    result.addToFalseList(i);

    return result;
}

} // namespace SecreC
