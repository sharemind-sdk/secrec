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

#include "../CodeGen.h"
#include "../CodeGenResult.h"
#include "../Constant.h"
#include "../DataType.h"
#include "../Misc.h"
#include "../SecurityType.h"
#include "../SymbolTable.h"
#include "../TreeNode.h"
#include "../TypeChecker.h"
#include "../Types.h"

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
    SymbolSymbol * arg2ResultSymbol = static_cast<SymbolSymbol *>(arg2Result.symbol());


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

        // This is a special case for better performance
        if (slices.empty() && e->type() == NODE_EXPR_BINARY_ASSIGN) // only one element is updated
        {
            pushComment("scalar expr_binary_assign cgExprAssign");
            Symbol * offset = m_st->appendTemporary(TypeBasic::get(DATATYPE_UINT64));
            emplaceImopAfter(result, e, Imop::ASSIGN, offset, spv[0].first);
            for (unsigned i = 1; i < spv.size(); i++)
            {
                emplaceImopAfter(result, e, Imop::MUL, offset, offset, destSym->dims()[i]);
                emplaceImopAfter(result, e, Imop::ADD, offset, offset, spv[i].first);
            }
            emplaceImopAfter(result, e, Imop::STORE, destSym, offset, arg2Result.symbol());
            return result;
        }

        // check that rhs has correct dimensions
        // eArg2 scalar is supported, it will be filled into all selected destination elements
        if (!eArg2->resultType()->isScalar()) {
            assert(static_cast<size_t>(eArg2->resultType()->secrecDimType()) == slices.size());

            std::stringstream ss;
            ss << "Shape of RHS doesnt match shape of LHS in assignment at "
                << e->location().printer(m_pathStyle) << '.';
            auto jmp = new Imop(e, Imop::JUMP, nullptr);
            Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
            SymbolLabel * errLabel = m_st->label(err);

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


        const TypeBasic * indexArrayType = TypeBasic::get(DATATYPE_UINT64, 1);
        auto xndim = destSym->dims().size();

        // dimensions array: xndim elements (full dimensionality of x)
        Symbol * dest_dims = m_st->appendTemporary(indexArrayType);
        emplaceImopAfter(result, e, Imop::ALLOC, dest_dims, indexConstant(xndim), indexConstant(0));
        for (unsigned i = 0; i < xndim; ++i) {
            Symbol * dimSym = destSym->dims()[i];
            emplaceImop(e, Imop::STORE, dest_dims, indexConstant(i), dimSym);
        }

        // starts array: xndim elements (all dimensions)
        Symbol * begins = m_st->appendTemporary(indexArrayType);
        emplaceImopAfter(result, e, Imop::ALLOC, begins, indexConstant(xndim), indexConstant(0));
        for (unsigned i = 0; i < xndim; ++i) {
            Symbol * startSym = spv[i].first;
            emplaceImop(e, Imop::STORE, begins, indexConstant(i), startSym);
        }

        // ends array: xndim elements (all dimensions)
        Symbol * ends = m_st->appendTemporary(indexArrayType);
        emplaceImopAfter(result, e, Imop::ALLOC, ends, indexConstant(xndim), indexConstant(0));
        for (unsigned i = 0; i < xndim; ++i) {
            Symbol * endSym;
            if (spv[i].second) {
                // It's a slice, use the end value
                endSym = spv[i].second;
            } else {
                // It's a scalar index, set end = start + 1
                Symbol * temp = m_st->appendTemporary(TypeBasic::get(DATATYPE_UINT64));
                emplaceImopAfter(result, e, Imop::ADD, temp, spv[i].first, indexConstant(1));
                endSym = temp;
            }
            emplaceImop(e, Imop::STORE, ends, indexConstant(i), endSym);
        }

        SymbolSymbol * scatter_source = nullptr;
        if (e->type() == NODE_EXPR_BINARY_ASSIGN)
        {
            // no extra copying needed
            scatter_source = arg2ResultSymbol;
        } else {
            // Create op_arg1 with the correct dimensionality
            const TypeBasic* temp_type = TypeBasic::get(
                destSym->secrecType()->secrecSecType(),
                destSym->secrecType()->secrecDataType(),
                slices.size()
                );
            
            SymbolSymbol * op_arg1 = m_st->appendTemporary(temp_type);
            scatter_source = m_st->appendTemporary(op_arg1->secrecType());
            // Set the dimensions properly
            unsigned count = 0;
            for (unsigned i = 0; i < xndim; ++i) {
                if (std::find(slices.begin(), slices.end(), i) != slices.end()) {
                    // Slice dimension -> keep it
                    SymbolSymbol* sym = m_st->appendTemporary(TypeBasic::get(DATATYPE_UINT64));
                    emplaceImopAfter(result, e, Imop::SUB, sym, spv[i].second, spv[i].first);
                    op_arg1->setDim(count, sym);
                    scatter_source->setDim(count, sym);
                    releaseTemporary(result, sym);
                    ++count;
                }
            }
            op_arg1->setSizeSym(m_st->appendTemporary(TypeBasic::get(DATATYPE_UINT64)));
            codeGenSize(result, op_arg1);
            assert(op_arg1->getSizeSym());
            scatter_source->setSizeSym(op_arg1->getSizeSym());
            emplaceImopAfter(result, e, Imop::ALLOC, op_arg1, op_arg1->getSizeSym());
            emplaceImopAfter(result, e, Imop::ALLOC, scatter_source, scatter_source->getSizeSym());

            // get elements from dest to temp array
            SyscallPassingConvention passing = destSym->secrecType()->secrecSecType()->isPrivate() ? Push : PushRef;
            SyscallOperands syscallOperands;
            syscallOperands.emplace_back(destSym, passing);  // source array
            syscallOperands.emplace_back(op_arg1, passing);  // destination array
            syscallOperands.emplace_back(dest_dims, SyscallPassingConvention::PushCRef);
            syscallOperands.emplace_back(begins, SyscallPassingConvention::PushCRef);
            syscallOperands.emplace_back(ends, SyscallPassingConvention::PushCRef);
            ConstantString* syscallName = ConstantString::get(getContext(), "arrgather");
            pushImopAfter(result, new Imop(e, syscallName, syscallOperands));

            if (isOverloaded) {
                // PD-specific (overloaded) operators
                std::vector<Symbol*> operands {op_arg1, arg2Result.symbol()};
                const CGResult& callRes(cgOverloadedExpr(e, e->resultType(), operands, e->procSymbol()));
                append(result, callRes);
                if (result.isNotOk())
                    return result;

                if (scatter_source->isArray()) {
                    emplaceImopAfter(result, e, Imop::COPY, scatter_source, callRes.symbol(), scatter_source->getSizeSym());
                }
                else {
                    emplaceImopAfter(result, e, Imop::ASSIGN, scatter_source, callRes.symbol());
                }
            } else {
                // Operators on cleardata
                Imop::Type iType;
                if (getAssignBinImopType(e->type(), iType)) {
                    if (op_arg1->isArray() && !eArg2->resultType()->isScalar())
                    {
                        emplaceImop(e, iType, scatter_source, op_arg1, arg2ResultSymbol, scatter_source->getSizeSym());
                    } else if (op_arg1->isArray() && eArg2->resultType()->isScalar()) {
                        Symbol * rhsSym = m_st->appendTemporary(op_arg1->secrecType());
                        emplaceImopAfter(result, e, Imop::ALLOC, rhsSym, op_arg1->getSizeSym(), arg2Result.symbol());
                        emplaceImop(e, iType, scatter_source, op_arg1, rhsSym, scatter_source->getSizeSym());
                        releaseTemporary(result, rhsSym);
                    } else if (!op_arg1->isArray() && eArg2->resultType()->isScalar()) {
                        emplaceImop(e, iType, scatter_source, op_arg1, arg2ResultSymbol);
                    } else {
                        assert(false);  // shouldn't happen
                        result |= CGResult::ERROR_FATAL;
                        return result;
                    }
                }
                else {
                    assert(false);  // shouldn't happen
                    result |= CGResult::ERROR_FATAL;
                    return result;
                }
                releaseTemporary(result, arg2Result.symbol());
            }
        }

        SyscallPassingConvention passing = scatter_source->secrecType()->secrecSecType()->isPrivate() ? Push : PushRef;
        SyscallOperands syscallOperands;
        syscallOperands.emplace_back(scatter_source, passing);
        syscallOperands.emplace_back(destSym, passing);
        syscallOperands.emplace_back(dest_dims, SyscallPassingConvention::PushCRef);
        syscallOperands.emplace_back(begins, SyscallPassingConvention::PushCRef);
        syscallOperands.emplace_back(ends, SyscallPassingConvention::PushCRef);
        ConstantString* syscallName = ConstantString::get(getContext(), "arrscatter");
        pushImopAfter(result, new Imop(e, syscallName, syscallOperands));

        releaseTemporary(result, dest_dims);
        releaseTemporary(result, begins);
        releaseTemporary(result, ends);
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
                ss << "Shape of RHS doesn't match shape of LHS in assignment at "
                   << e->location().printer(m_pathStyle) << '.';
                auto jmp = new Imop(e, Imop::JUMP, static_cast<Symbol *>(nullptr));
                Imop * err = newError(e, ConstantString::get(getContext(), ss.str()));
                SymbolLabel * errLabel = m_st->label(err);
                SymbolSymbol * arg2ResultSymbol = static_cast<SymbolSymbol *>(arg2Result.symbol());

                auto di(arg2ResultSymbol->dims().begin());
                auto de(arg2ResultSymbol->dims().end());
                auto dj(static_cast<SymbolSymbol *>(destSym)->dims().begin());
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
                    emplaceImopAfter(result, e, Imop::COPY, destSym, callRes.symbol(), destSym->getSizeSym());
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
