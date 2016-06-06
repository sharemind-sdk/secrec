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
#include "Misc.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Types.h"

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

CGResult TreeNodeExprAssign::codeGenWith (CodeGen &cg) {
    return cg.cgExprAssign (this);
}

CGResult CodeGen::cgAssign (SymbolSymbol* dest, Symbol* src) {
    assert (dest != nullptr);
    assert (src != nullptr);

    CGResult result;
    result.setResult (dest);

    if (dest->secrecType ()->secrecDataType ()->isComposite ()) {
        const auto& destFields = dest->fields ();
        const auto& srcFields = static_cast<SymbolSymbol*>(src)->fields ();
        assert (destFields.size () == srcFields.size ());
        for (size_t i = 0; i < destFields.size (); ++ i) {
            append (result, cgAssign (destFields[i], srcFields[i]));
            if (result.isNotOk ())
                return result;
        }
    }
    else {
        copyShapeFrom (result, src);
        if (result.isNotOk()) {
            return result;
        }

        if (dest->isArray()) {
            emplaceImopAfter (result, m_node, Imop::RELEASE, nullptr, dest);

            if (src->secrecType ()->isScalar ())
                emplaceImop (m_node, Imop::ALLOC, dest, dest->getSizeSym(), src);
            else
                emplaceImop (m_node, Imop::COPY, dest, src, dest->getSizeSym());
        } else {
            releaseResource (result, dest);
            emplaceImopAfter (result, m_node, Imop::ASSIGN, dest, src);
        }
    }

    return result;
}

CGResult CodeGen::cgExprAssign(TreeNodeExprAssign * e) {

    // Type check:
    if (m_tyChecker->visitExprAssign(e) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGResult result;

    // Generate code for the righthand side:
    TreeNodeExpr * eArg2 = e->rightHandSide();
    const CGResult & arg2Result = codeGen(eArg2);
    append(result, arg2Result);
    if (result.isNotOk()) {
        return result;
    }


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
    const TypeBasic * pubIntTy = TypeBasic::getIndexType();
    const TypeBasic * pubBoolTy = TypeBasic::getPublicBoolType();

    // struct to struct assignment
    if (destSym->secrecType ()->secrecDataType ()->isComposite ()) {
        append (result, cgAssign (destSym, arg2Result.symbol ()));
        if (result.isNotOk ())
            return result;

        releaseTemporary (result,  arg2Result.symbol ());
        return result;
    }

    bool isOverloaded = e->isOverloaded ();

    // x[e1,...,ek] = e
    if (isIndexed) {
        const SubscriptInfo::SPV & spv = cgSub.spv();
        const SubscriptInfo::SliceIndices & slices = cgSub.slices();

        // check that rhs has correct dimensions
        if (!eArg2->resultType()->isScalar()) {
            assert(static_cast<size_t>(eArg2->resultType()->secrecDimType()) == slices.size());

            std::stringstream ss;
            ss << "Shape of RHS doesnt match shape of LHS in assignment at " << e->location() << '.';
            auto jmp = new Imop(e, Imop::JUMP, nullptr);
            Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
            SymbolLabel * errLabel = m_st->label(err);
            SymbolSymbol * arg2ResultSymbol = static_cast<SymbolSymbol *>(arg2Result.symbol());

            for (SecrecDimType k = 0; k < eArg2->resultType()->secrecDimType(); ++ k) {
                Symbol * tsym = m_st->appendTemporary(pubIntTy);
                Symbol * bsym = m_st->appendTemporary(pubBoolTy);
                emplaceImopAfter(result, e, Imop::SUB, tsym, spv[slices[k]].second, spv[slices[k]].first);
                emplaceImop(e, Imop::NE, bsym, tsym, arg2ResultSymbol->getDim(k));
                emplaceImop(e, Imop::JT, errLabel, bsym);
            }

            push_imop(jmp);
            result.addToNextList(jmp);
            push_imop(err);
        }

        // initialize stride
        ArrayStrideInfo stride(destSym);
        append(result, codeGenStride(stride));
        if (result.isNotOk()) {
            return result;
        }

        // initialze running indices
        LoopInfo loopInfo = prepareLoopInfo (cgSub);

        // initialze symbols for offsets and temporary results
        Symbol * offset = m_st->appendTemporary(pubIntTy);
        Symbol * old_offset = m_st->appendTemporary(pubIntTy);
        Symbol * tmp_result2 = m_st->appendTemporary(pubIntTy);
        SymbolSymbol * r = generateResultSymbol(result, e);
        Symbol * resultOffset = m_st->appendTemporary (pubIntTy);
        bool resultScalar = r->secrecType()->isScalar();

        // compute the shape and the size of the result symbol "r"
        {
            unsigned count = 0;
            for (unsigned k : cgSub.slices()) {
                Symbol * sym = r->getDim(count);
                emplaceImopAfter(result, e, Imop::SUB, sym, spv[k].second, spv[k].first);
                ++ count;
            }

            codeGenSize(result);
        }

        // allocate memory for the result symbol "r"
        if (!resultScalar) {
            const DataType* dt = e->resultType()->secrecDataType();
            if (dt->isUserPrimitive()) {
                emplaceImopAfter(result, e, Imop::ALLOC, r, r->getSizeSym());
            }
            else {
                SecrecDataType prim = static_cast<const DataTypeBuiltinPrimitive*>(dt)->secrecDataType();
                emplaceImopAfter(result, e, Imop::ALLOC, r, r->getSizeSym(),
                                 defaultConstant(getContext(), prim));
            }
        }
        else {
            emplaceImopAfter(result, e, Imop::DECLARE, r);
        }

        // resultOffset = 0
        pushImopAfter (result, newAssign (e, resultOffset, indexConstant (0)));

        // offset = 0
        pushImopAfter (result, newAssign (e, offset, indexConstant(0)));

        // Declare temporaries that might require allocation for the inner assignment:
        const TypeBasic * ty = TypeBasic::get(e->resultType()->secrecSecType(),
                                        e->resultType()->secrecDataType());
        const TypeBasic * rTy = TypeBasic::get(eArg2->resultType()->secrecSecType(),
                                         eArg2->resultType()->secrecDataType());
        Symbol * t1 = m_st->appendTemporary(ty);
        Symbol * t2 = m_st->appendTemporary(ty);
        emplaceImop(e, Imop::DECLARE, t1);
        emplaceImop(e, Imop::DECLARE, t2);

        // start
        append(result, enterLoop(loopInfo, spv));
        if (result.isNotOk()) {
            return result;
        }

        // compute offset for RHS
        {
            // old_ffset = 0
            emplaceImopAfter(result, e, Imop::ASSIGN, old_offset, indexConstant(0));

            unsigned count = 0;
            auto itIt = loopInfo.begin();
            auto itEnd = loopInfo.end();
            for (; itIt != itEnd; ++ count, ++ itIt) {
                // tmp_result2 = s[k] * idx[k]
                emplaceImop(e, Imop::MUL, tmp_result2, stride.at(count), *itIt);

                // old_offset = old_offset + tmp_result2
                emplaceImop(e, Imop::ADD, old_offset, old_offset, tmp_result2);
            }
        }

        // load and store
        {
            if (e->type() == NODE_EXPR_BINARY_ASSIGN) {
                if (!eArg2->resultType()->isScalar()) {
                    emplaceImop(e, Imop::LOAD, t1, arg2Result.symbol(), offset);
                    emplaceImop(e, Imop::STORE, destSym, old_offset, t1);

                    if (resultScalar) {
                        emplaceImop(e, Imop::ASSIGN, r, t1);
                    }
                    else {
                        emplaceImop(e, Imop::STORE, r, resultOffset, t1);
                    }
                }
                else {
                    emplaceImop(e, Imop::STORE, destSym, old_offset, arg2Result.symbol());

                    if (resultScalar) {
                        emplaceImop(e, Imop::ASSIGN, r, arg2Result.symbol());
                    }
                    else {
                        emplaceImop(e, Imop::STORE, r, resultOffset, arg2Result.symbol());
                    }
                }
            }
            else if (! isOverloaded) {
                Imop::Type iType;
                if (! getAssignBinImopType(e->type(), iType)) {
                    assert(false);  // shouldn't happen
                    result |= CGResult::ERROR_FATAL;
                    return result;
                }

                emplaceImop(e, Imop::LOAD, t1, destSym, old_offset);

                if (!eArg2->resultType()->isScalar()) {
                    emplaceImop(e, Imop::LOAD, t2, arg2Result.symbol(), offset);
                    emplaceImop(e, iType, t1, t1, t2);
                    emplaceImop(e, Imop::STORE, destSym, old_offset, t1);
                }
                else {
                    emplaceImop(e, iType, t1, t1, arg2Result.symbol());
                    emplaceImop(e, Imop::STORE, destSym, old_offset, t1);
                }

                if (resultScalar) {
                    emplaceImop(e, Imop::ASSIGN, r, t1);
                }
                else {
                    emplaceImop(e, Imop::STORE, r, resultOffset, t1);
                }
            }
            else {
                // isOverloaded

                Symbol * t1 = m_st->appendTemporary(ty);
                Symbol * t2 = m_st->appendTemporary(rTy);

                emplaceImop(e, Imop::DECLARE, t1);
                emplaceImop(e, Imop::DECLARE, t2);
                emplaceImop(e, Imop::LOAD, t1, destSym, old_offset);

                if (!eArg2->resultType()->isScalar()) {
                    emplaceImop(e, Imop::LOAD, t2, arg2Result.symbol(), offset);
                }
                else {
                    emplaceImop(e, Imop::ASSIGN, t2, arg2Result.symbol());
                }

                std::vector<Symbol*> operands {t1, t2};
                const CGResult& callRes(cgOverloadedExpr(e, ty, operands, e->procSymbol()));
                append(result, callRes);
                if (result.isNotOk())
                    return result;

                emplaceImop(e, Imop::STORE, destSym, old_offset, callRes.symbol());
                if (resultScalar) {
                    emplaceImop(e, Imop::ASSIGN, r, callRes.symbol());
                }
                else {
                    emplaceImop(e, Imop::STORE, r, resultOffset, callRes.symbol());
                }
            }

            // offset = offset + 1
            emplaceImop(e, Imop::ADD, offset, offset, indexConstant(1));
            // resultOffset = resultOffset + 1
            emplaceImop(e, Imop::ADD, resultOffset, resultOffset, indexConstant(1));
        }

        // loop exit
        append(result, exitLoop(loopInfo));
        if (result.isNotOk()) {
            return result;
        }

        // Free temporaries
        releaseResource (result, t1);
        releaseResource (result, t2);
        releaseTemporary (result, arg2Result.symbol());
        return result;
    }
    else {
        // isIndexed = false

        result.setResult (destSym);

        // Generate code for regular x = e assignment
        if (e->type() == NODE_EXPR_BINARY_ASSIGN) {
            if (!eArg2->resultType()->isScalar()) {
                result.setResult (destSym); // hack!
                copyShapeFrom(result, arg2Result.symbol());
                if (result.isNotOk()) {
                    return result;
                }
            }

            if (destSym->isArray()) {
                emplaceImopAfter(result, e, Imop::RELEASE, nullptr, destSym);
                if (eArg2->resultType()->isScalar())
                    emplaceImop(e, Imop::ALLOC, destSym, destSym->getSizeSym(), arg2Result.symbol());
                else
                    emplaceImop(e, Imop::COPY, destSym, arg2Result.symbol(), destSym->getSizeSym());
                releaseTemporary (result, arg2Result.symbol());
            } else {
                emplaceImopAfter(result, e, Imop::ASSIGN, destSym, arg2Result.symbol());
                releaseTemporary (result, arg2Result.symbol());
            }
        } else {
            // Arithmetic assignments
            if (destSym->isArray() && !eArg2->resultType()->isScalar()) {
                // Check shapes
                std::stringstream ss;
                ss << "Shape of RHS doesn't match shape of LHS in assignment at " << e->location() << '.';
                auto jmp = new Imop(e, Imop::JUMP, (Symbol *) nullptr);
                Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
                SymbolLabel * errLabel = m_st->label(err);
                SymbolSymbol * arg2ResultSymbol = static_cast<SymbolSymbol *>(arg2Result.symbol());

                dim_iterator
                    di = dim_begin(arg2ResultSymbol),
                    dj = dim_begin(destSym),
                    de = dim_end(arg2ResultSymbol);
                for (; di != de; ++ di, ++ dj) {
                    SymbolTemporary * temp_bool = m_st->appendTemporary(pubBoolTy);
                    emplaceImop(e, Imop::NE, temp_bool, *di, *dj);
                    emplaceImop(e, Imop::JT, errLabel, temp_bool);
                }

                push_imop(jmp);
                result.addToNextList(jmp);
                push_imop(err);
            }

            if (isOverloaded) {
                std::vector<Symbol*> operands {destSym, arg2Result.symbol()};
                const CGResult& callRes(cgOverloadedExpr(e, e->resultType(), operands, e->procSymbol()));
                append(result, callRes);
                if (result.isNotOk())
                    return result;

                // Copy result
                if (destSym->isArray()) {
                    emplaceImopAfter(result, e, Imop::ASSIGN, destSym, callRes.symbol(), destSym->getSizeSym());
                }
                else {
                    emplaceImopAfter(result, e, Imop::ASSIGN, destSym, callRes.symbol());
                }
            }
            else {
                // Not overloaded
                Imop::Type iType;
                if (! getAssignBinImopType(e->type(), iType)) {
                    assert(false);  // shouldn't happen
                    result |= CGResult::ERROR_FATAL;
                    return result;
                }

                if (destSym->isArray()) {
                    if (eArg2->resultType()->isScalar()) {
                        Symbol * rhsSym = m_st->appendTemporary(destSym->secrecType());
                        emplaceImopAfter(result, e, Imop::ALLOC, rhsSym, destSym->getSizeSym(), arg2Result.symbol());
                        emplaceImop(e, iType, destSym, destSym, rhsSym, destSym->getSizeSym());
                        releaseTemporary(result, rhsSym);
                        releaseTemporary(result, arg2Result.symbol());
                    }
                    else {
                        emplaceImopAfter(result, e, iType, destSym, destSym, arg2Result.symbol(), destSym->getSizeSym());
                        releaseTemporary(result, arg2Result.symbol ());
                    }
                }
                else {
                    emplaceImopAfter(result, e, iType, destSym, destSym, arg2Result.symbol());
                    releaseTemporary(result, arg2Result.symbol ());
                }
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

    auto i = new Imop(e, Imop::JT, nullptr, result.symbol());
    pushImopAfter(result, i);
    result.addToTrueList(i);

    i = new Imop(e, Imop::JUMP, nullptr);
    push_imop(i);
    result.addToFalseList(i);

    return result;
}

} // namespace SecreC
