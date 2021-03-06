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

#include "../Constant.h"
#include "../DataType.h"
#include "../Imop.h"
#include "../Log.h"
#include "../Misc.h"
#include "../SecurityType.h"
#include "../Symbol.h"
#include "../SymbolTable.h"
#include "../TreeNode.h"
#include "../TypeChecker.h"
#include "../Types.h"
#include "../SecurityType.h"
#include "../Visitor.h"

#include "../Context.h"
#include "../ContextImpl.h"


namespace SecreC {

namespace /* anonymous */ {

/**
 * Use for statically casting base class to subclass.
 * NULL pointer input is not allowed.
 * Asserts that dynamic_cast returns non-null.
 */
template <typename DestPtr, typename SrcPtr>
inline DestPtr cast(SrcPtr ptr) {
    static_assert(std::is_convertible<DestPtr, SrcPtr>::value, "");
    assert (ptr != nullptr);
    assert (dynamic_cast<DestPtr>(ptr) != nullptr);
    return static_cast<DestPtr>(ptr);
};

bool isAbstractNumeric(SecrecDataType d) {
    switch (d) {
    case DATATYPE_NUMERIC:
    case DATATYPE_NUMERIC_FLOAT:
        return true;
    default:
        return false;
    }
}

bool compatibleAbstractNumericDType(SecrecDataType d1, SecrecDataType d2,
                                    SecrecDataType * result)
{
    if (isAbstractNumeric(d1) || isAbstractNumeric(d2)) {
        *result = upperDataType(d1, d2);
        return true;
    }

    return false;
}

SecrecDataType getResultDType(SecrecTreeNodeType type, SecrecDataType d1, SecrecDataType d2) {
    SecrecDataType result = DATATYPE_UNDEFINED;
    switch (type) {
    case NODE_EXPR_BINARY_ADD:
        if (compatibleAbstractNumericDType(d1, d2, &result)) return result;
        if (d1 != d2) break;
        if (! isNumericDataType(d1) && d1 != DATATYPE_STRING)
            break;
        return d1;
    case NODE_EXPR_BINARY_SHL:
    case NODE_EXPR_BINARY_SHR:
        if (isSignedNumericDataType (d1)) break;
        if (isSignedNumericDataType (d2)) break;
    case NODE_EXPR_BINARY_SUB:
    case NODE_EXPR_BINARY_MUL:
    case NODE_EXPR_BINARY_MOD:
    case NODE_EXPR_BINARY_DIV:
        if (compatibleAbstractNumericDType(d1, d2, &result)) return result;
        if (d1 == DATATYPE_STRING) break;
        if (d1 != d2) break;
        if (! isNumericDataType(d1)) break;
        return d1;
    case NODE_EXPR_BITWISE_AND:
    case NODE_EXPR_BITWISE_OR:
    case NODE_EXPR_BITWISE_XOR:
        if (d1 == DATATYPE_NUMERIC_FLOAT || d2 == DATATYPE_NUMERIC_FLOAT) break;
        if (isFloatingDataType(d1) || isFloatingDataType(d2)) break;
        if (d1 == DATATYPE_STRING) break;
        if (compatibleAbstractNumericDType(d1, d2, &result)) return result;
        if (d1 != d2) break;
        return d1;
    case NODE_EXPR_BINARY_EQ:
    case NODE_EXPR_BINARY_GE:
    case NODE_EXPR_BINARY_GT:
    case NODE_EXPR_BINARY_LE:
    case NODE_EXPR_BINARY_LT:
    case NODE_EXPR_BINARY_NE:
        if (compatibleAbstractNumericDType(d1, d2, &result)) {
            if (result == DATATYPE_UNDEFINED) {
                break;
            }
        }
        if (d1 != d2) break;
        return DATATYPE_BOOL;
    case NODE_EXPR_BINARY_LAND:
    case NODE_EXPR_BINARY_LOR:
        if (d1 != DATATYPE_BOOL || d2 != DATATYPE_BOOL) break;
        return DATATYPE_BOOL;
    default:
        break;
    }

    return DATATYPE_UNDEFINED;
}

SecrecDataType getResultDType(SecrecTreeNodeType type, const DataType* d1, const DataType* d2) {
    if (d1->isBuiltinPrimitive () && d2->isBuiltinPrimitive ()) {
        return getResultDType (type,
            static_cast<const DataTypeBuiltinPrimitive*>(d1)->secrecDataType (),
            static_cast<const DataTypeBuiltinPrimitive*>(d2)->secrecDataType ());
    }

    return DATATYPE_UNDEFINED;
}

bool overloadedOpGood (const TypeBasic* ty) {
    if (ty->secrecDataType ()->isComposite () ||
        ty->secrecDataType ()->isString ())
        return false;

    return true;
}

} // namespace anonymous

/*******************************************************************************
  TypeChecker
*******************************************************************************/

TypeChecker::Status TypeChecker::checkPostfixPrefixIncDec(TreeNodeExpr * root,
                                                          OverloadableOperator * op,
                                                          bool isPrefix,
                                                          bool isInc)
{
    if (root->haveResultType())
        return OK;

    assert(root->children().size() == 1);
    // TODO: a bit of a hack
    TreeNodeLValue * lval = static_cast<TreeNodeLValue*>(root->children().at(0));
    const char * m1 = isPrefix ? "Prefix " : "Postfix ";
    const char * m2 = isInc ? "increment" : "decrement";

    TCGUARD (visitLValue (lval));

    const TypeNonVoid* eType = lval->secrecType ();

    if (eType->secrecDataType ()->isUserPrimitive ()) {
        auto dt = static_cast<const DataTypeUserPrimitive*> (eType->secrecDataType ());
        assert (eType->secrecSecType ()->isPrivate ());

        SymbolKind* kind =
            static_cast<const PrivateSecType*> (eType->secrecSecType ())->securityKind ();
        assert (kind->findType (dt->name ()) != nullptr);
        auto publicType = kind->findType (dt->name ())->publicType;

        if (! publicType) {
            m_log.fatalInProc (root)
                << m1 << m2 << " operator expects a type with a public representation, given "
                << *eType << " at " << root->location () << '.';
            return E_TYPE;
        }
    }

    // Find overloaded + or -
    {
        bool isIndexed = lval->type () == NODE_LVALUE_INDEX;
        // If the lval is indexed, codegen will create a loop
        // operating on scalars
        SecrecDimType dim = isIndexed ? 0 : eType->secrecDimType ();

        std::vector<const TypeBasic*> argumentDataTypes;
        const DataType* publicDataType = dtypeDeclassify (eType->secrecSecType(),
                                                          eType->secrecDataType());
        const TypeBasic* publicType = TypeBasic::get (publicDataType, dim);
        const TypeBasic* varType = TypeBasic::get (eType->secrecSecType (),
                                                   eType->secrecDataType (),
                                                   dim);

        // check that we are operating on numeric types
        if (! isNumericDataType (publicDataType)) {
            m_log.fatalInProc (root)
                << m1 << m2 << " operator expects numeric type, given "
                << *eType << " at " << root->location () << '.';
            return E_TYPE;
        }

        argumentDataTypes.push_back (varType);
        argumentDataTypes.push_back (publicType);

        SymbolProcedure* symProc = nullptr;
        const TypeProc* callType = TypeProc::get (argumentDataTypes);
        TCGUARD (findBestMatchingOpDef (symProc,
                                        op->operatorName (),
                                        *root,
                                        callType,
                                        root));

        if (symProc != nullptr) {
            op->setProcSymbol (symProc);
            root->setResultType (eType);
            return OK;
        }
        else if (eType->secrecSecType ()->isPrivate ()) {
            m_log.fatalInProc (root)
                << m1 << m2 << " at " << root->location ()
                << " can not be performed due to missing operator definition.";
            return E_TYPE;
        }
    }

    // Built-in operator on public type
    root->setResultType (eType);
    return OK;
}

const TypeNonVoid* TypeChecker::checkSelect (const Location& loc,
                                             const Type* ty,
                                             TreeNodeIdentifier* id)
{
    assert (ty != nullptr);

    if (ty->kind () != Type::BASIC || ! ty->secrecDataType ()->isComposite ()) {
        m_log.fatal () << "Expecting structure, got " << *ty << ". "
                       << "Error at " << loc << ".";
        return nullptr;
    }

    // Verify attribute access:
    const auto structType = static_cast<const DataTypeStruct*>(ty->secrecDataType ());
    StringRef fieldName = id->value ();
    const TypeBasic* matchingFieldType = nullptr;
    for (const auto& field : structType->fields ()) {
        if (fieldName == field.name) {
            matchingFieldType = field.type;
            break;
        }
    }

    if (matchingFieldType == nullptr) {
        m_log.fatal () << "Invalid attribute \'" << fieldName << "\'. "
                       << "Error at " << id->location () << ".";
        return nullptr;
    }

    return matchingFieldType;
}

/*******************************************************************************
  TreeNodeExpr
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExpr(TreeNodeExpr * e) {
    return dispatchExpr (*this, e);
}

/*******************************************************************************
  TreeNodeExprSelection
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprSelection (TreeNodeExprSelection * e) {
    if (e->haveResultType())
        return OK;

    // Check subexpression:
    TreeNodeExpr* structExpr = e->expression ();
    TCGUARD (visitExpr (structExpr));
    const TypeNonVoid* fieldType = checkSelect (structExpr->location (), structExpr->resultType (), e->identifier ());
    if (fieldType != nullptr) {
        e->setResultType (fieldType);
        return OK;
    }

    return E_TYPE;
}

/*******************************************************************************
  TreeNodeExprAssign
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprAssign(TreeNodeExprAssign * e) {
    if (e->haveResultType())
        return OK;

    // Get symbol for l-value:
    TreeNodeLValue* lval = e->leftHandSide ();

    TCGUARD (visitLValue (lval));
    const TypeNonVoid * lhsType = lval->secrecType ();

    // Calculate type of r-value:
    TreeNodeExpr * src = e->rightHandSide ();
    src->setContext(lhsType);

    TCGUARD (visitExpr(src));

    if (checkAndLogIfVoid(src))
        return E_TYPE;

    const auto srcType = static_cast<const TypeNonVoid *>(src->resultType());
    if (! srcType->latticeLEQ(lhsType)) {
        m_log.fatalInProc(e) << "Invalid assignment from value of type " << *srcType
            << " to variable of type " << *lhsType << " at "
            << e->location() << '.';
        return E_TYPE;
    }

    // Search for overloaded operator
    if (e->type() != NODE_EXPR_BINARY_ASSIGN) {
        bool isIndexed = lval->type () == NODE_LVALUE_INDEX;
        // If the lval is indexed, codegen will create a loop
        // operating on scalars
        SecrecDimType lDim = 0;
        SecrecDimType rDim = 0;
        if (! isIndexed)  {
            lDim = lhsType->secrecDimType();
            rDim = srcType->secrecDimType();
        }

        std::vector<const TypeBasic*> argumentDataTypes;
        const TypeBasic* lType = TypeBasic::get(lhsType->secrecSecType(),
                                                lhsType->secrecDataType(),
                                                lDim);
        const TypeBasic* rType = TypeBasic::get(srcType->secrecSecType(),
                                                srcType->secrecDataType(),
                                                rDim);
        argumentDataTypes.push_back(lType);
        argumentDataTypes.push_back(rType);

        SymbolProcedure* symProc = nullptr;
        const TypeProc* callType = TypeProc::get (argumentDataTypes);
        TCGUARD (findBestMatchingOpDef(symProc,
                                       e->operatorName(),
                                       *e,
                                       callType,
                                       e));

        if (symProc != nullptr) {
            const TypeProc* procType = static_cast<const TypeProc*>(symProc->secrecType());
            assert(procType->paramTypes().size() == 2);
            if (procType->paramTypes()[1]->secrecSecType() !=
                srcType->secrecSecType())
            {
                src->setContextDataType(lhsType->secrecDataType());
                src = classifyIfNeeded(src, lhsType->secrecSecType());
            }

            e->setProcSymbol(symProc);
            e->setResultType(lhsType);
            return OK;
        }
        else if (lhsType->secrecSecType()->isPrivate()) {
            m_log.fatalInProc(e)
                << "Assignment at " << e->location()
                << " can not be performed due to missing operator definition.";
            return E_TYPE;
        }
    }

    // Add implicit classify node if needed:
    src = classifyIfNeeded(src, lhsType->secrecSecType());
    e->setResultType(lhsType);
    return OK;
}

/*******************************************************************************
  TreeNodeExprCast
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprCast(TreeNodeExprCast * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * subExpr = root->expression();
    subExpr->setContextSecType(root->contextSecType());
    subExpr->setContextDimType(root->contextDimType());
    TCGUARD (visitExpr(subExpr));

    subExpr->instantiateDataType();

    if (subExpr->resultType()->isVoid()) {
        m_log.fatalInProc(root) << "Unable to cast from void at "
                                << root->location() << '.';
        return E_TYPE;
    }

    const TypeBasic * ty = cast<const TypeBasic*>(subExpr->resultType());
    const DataType * givenDType = ty->secrecDataType();

    TCGUARD (visitDataTypeF (root->dataType (), ty->secrecSecType ()));
    const DataType* resultingDType = root->dataType()->cachedType ();
    const TypeBasic* want = TypeBasic::get(ty->secrecSecType(),
                                           resultingDType,
                                           ty->secrecDimType());

    if (ty->secrecSecType()->isPrivate()) {
        SymbolProcedure * symProc;
        TCGUARD(findBestMatchingCastDef(symProc, ty, want, root));

        if (! symProc) {
            m_log.fatalInProc(root)
                << "Unable to perform cast at " << root->location()
                << " due to missing cast definition.";
            return E_TYPE;
        }

        root->setResultType(want);
        root->setProcSymbol(symProc);

        return OK;
    }
    else if (! latticeExplicitLEQ(givenDType, resultingDType)) {
        m_log.fatalInProc(root) << "Unable to perform cast at "
                                << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(want);

    return OK;
}

/*******************************************************************************
  TreeNodeExprIndex
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprIndex(TreeNodeExprIndex * root) {
    if (root->haveResultType())
        return TypeChecker::OK;

    TreeNodeExpr * e = root->expression();
    e->setContextSecType(root->contextSecType());
    e->setContextDataType(root->contextDataType());
    e->setContextDimType(root->indices()->children().size());
    TCGUARD (visitExpr(e));
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    const auto eType = static_cast<const TypeNonVoid*>(e->resultType());
    SecrecDimType k = 0;
    SecrecDimType n = eType->secrecDimType();

    if (root->indices()->children().size() != n) {
        m_log.fatalInProc(root) << "Incorrect number of indices at "
            << e->location() << '.';
        return E_TYPE;
    }

    TCGUARD (checkIndices(root->indices(), k));

    root->setResultType(TypeBasic::get(eType->secrecSecType(),
                                       eType->secrecDataType(), k));
    return OK;
}

void TreeNodeExprIndex::instantiateDataTypeV (SecrecDataType dType) {
    resetDataType (dType);
    expression ()->instantiateDataType (dType);
}

/*******************************************************************************
  TreeNodeExprSize
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprSize(TreeNodeExprSize * root) {
    if (! root->haveResultType()) {
        TreeNodeExpr * e = root->expression();
        TCGUARD (visitExpr(e));
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        e->instantiateDataType();
        root->setResultType(TypeBasic::getIndexType());
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprShape
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprShape(TreeNodeExprShape * root) {
    if (! root->haveResultType()) {
        TreeNodeExpr * e = root->expression();
        TCGUARD (visitExpr(e));
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        e->instantiateDataType();
        root->setResultType(TypeBasic::get(DATATYPE_UINT64, 1));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprCat
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprCat(TreeNodeExprCat * root) {
    if (root->haveResultType())
        return OK;

    std::array<const TypeNonVoid*, 2> eTypes = {{nullptr, nullptr}};

    // check that first subexpressions 2 are arrays and of equal dimensionalities
    for (std::size_t i = 0; i < 2; ++ i) {
        auto const e = cast<TreeNodeExpr *>(root->children().at(i));
        e->setContext(root->typeContext());
        TCGUARD (visitExpr(e));
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        eTypes[i] = cast<const TypeNonVoid*>(e->resultType());
        if (eTypes[i]->isScalar()) {
            m_log.fatalInProc(root) << "Concatenation of scalar values at "
                << e->location() << '.';
            return E_TYPE;
        }
    }

    const DataType* d0 = upperDataType(cast<const TypeBasic*> (eTypes[0]),
                                       cast<const TypeBasic*> (eTypes[1]));
    root->leftExpression()->instantiateDataType(d0);
    root->rightExpression()->instantiateDataType(d0);

    {
        const SecurityType * lSecTy = eTypes[0]->secrecSecType();
        const SecurityType * rSecTy = eTypes[1]->secrecSecType();
        TreeNodeExpr * left = root->leftExpression();
        TreeNodeExpr * right = root->rightExpression();
        left = classifyIfNeeded(left, rSecTy);
        right = classifyIfNeeded(right, lSecTy);
        eTypes[0] = cast<const TypeNonVoid*>(left->resultType());
        eTypes[1] = cast<const TypeNonVoid*>(right->resultType());
    }

    const SecurityType * resSecType = upperSecType(eTypes[0]->secrecSecType(),
            eTypes[1]->secrecSecType());
    if (resSecType == nullptr) {
        m_log.fatalInProc(root) << "Concatenating arrays of incompatable security types at "
                                << root->location() << '.';
        return E_TYPE;
    }

    if (eTypes[0]->secrecDataType() != eTypes[1]->secrecDataType()) {
        m_log.fatalInProc(root) << "Data type mismatch at "
            << root->leftExpression()->location() << " and "
            << root->rightExpression()->location() << '.';
        return E_TYPE;
    }

    if (eTypes[0]->secrecDimType() != eTypes[1]->secrecDimType()) {
        m_log.fatalInProc(root) << "Dimensionalities mismatch at "
            << root->leftExpression()->location() << " and "
            << root->rightExpression()->location() << '.';
        return E_TYPE;
    }

    // missing argument is interpreted as 0
    if (root->dimensionality() == nullptr) {
        // A bit awkard to intern string for just this
        auto strRef = getContext().pImpl()->stringTable().addString("0", 1);
        TreeNode * e = new TreeNodeExprInt(*strRef, root->location());
        root->appendChild(e);
    }

    auto const dimExpr = root->dimensionality();
    dimExpr->setContextIndexType();
    TCGUARD (visitExpr(dimExpr));
    if (checkAndLogIfVoid(dimExpr) || ! dimExpr->haveActualValue()) {
        return E_TYPE;
    }

    const auto dimType = cast<const TypeNonVoid *>(dimExpr->resultType());
    if (! dimType->isPublicUIntScalar()) {
        m_log.fatalInProc(root) << "Expected public scalar integer at "
            << root->dimensionality()->location()
            << " got " << *dimType << '.';
        return E_TYPE;
    }

    if (dimExpr->actualValue () >= eTypes[0]->secrecDimType()) {
        m_log.fatalInProc (root) << "Array concatenation over invalid dimensionality at "
                                 << dimExpr->location () << '.';
        return E_TYPE;
    }

    root->setResultType(
                TypeBasic::get(resSecType,
                               eTypes[0]->secrecDataType(),
                               eTypes[0]->secrecDimType()));
    return OK;
}

void TreeNodeExprCat::instantiateDataTypeV(SecrecDataType dType) {
    resetDataType(dType);
    leftExpression()->instantiateDataType(dType);
    rightExpression()->instantiateDataType(dType);
}

/*******************************************************************************
  TreeNodeExprReshape
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprReshape(TreeNodeExprReshape * root) {
    if (root->haveResultType())
        return OK;

    SecrecDimType resDim = root->dimensions().size ();
    TreeNodeExpr * e = root->reshapee();
    e->setContextSecType(root->contextSecType());
    e->setContextDataType(root->contextDataType());
    TCGUARD (visitExpr(e));
    if (checkAndLogIfVoid(e))
        return E_TYPE;

    const auto eType = static_cast<const TypeNonVoid *>(e->resultType());
    for (TreeNodeExpr& dim : root->dimensions()) {
        dim.setContextIndexType ();
        TCGUARD (visitExpr(&dim));

        if (checkAndLogIfVoid(&dim))
            return E_TYPE;

        const auto dimType = static_cast<const TypeNonVoid *>(dim.resultType());
        if (! dimType->isPublicUIntScalar()) {
            m_log.fatalInProc(root) << "Expecting 'uint' at "
                << dim.location() << " got " << *dimType << '.';
            return E_TYPE;
        }
    }

    if (resDim == 0) {
        m_log.fatalInProc(root) << "Conversion from non-scalar to scalar at "
            << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(TypeBasic::get(eType->secrecSecType(),
                                       eType->secrecDataType(),
                                       resDim));
    return OK;
}

void TreeNodeExprReshape::instantiateDataTypeV(SecrecDataType dType) {
    resetDataType(dType);
    reshapee()->instantiateDataType(dType);
}

/*******************************************************************************
  TreeNodeExprToString
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprToString(TreeNodeExprToString * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e = root->expression();
    e->setContextSecType(PublicSecType::get());
    e->setContextDimType(0);
    TCGUARD (visitExpr(e));
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    e->instantiateDataType();
    const auto tnv = static_cast<const TypeNonVoid *>(e->resultType());
    if (tnv->secrecDimType() != 0
            || tnv->secrecSecType()->isPrivate()
            || tnv->secrecDataType()->isString ()) {
        m_log.fatalInProc(root) << "Invalid argument passed to \"tostring\" expression at "
                                << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(TypeBasic::get(DATATYPE_STRING));

    return OK;
}

/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/

bool checkUserPrimPublic (const TypeBasic* lType, const TypeBasic* rType) {
    const DataType* lData = lType->secrecDataType ();

    if (lData->isUserPrimitive ()) {
        auto lPrim = cast<const DataTypeUserPrimitive*> (lData);
        assert (lType->secrecSecType ()->isPrivate ());
        SymbolKind* kind =
            cast<const PrivateSecType*> (lType->secrecSecType ())->securityKind ();

        assert (kind->findType (lPrim->name ()) != nullptr);
        auto publicType = kind->findType (lPrim->name ())->publicType;

        if (! publicType && rType->secrecSecType ()->isPublic ())
            return false;
    }

    return true;
}

TypeChecker::Status TypeChecker::visitExprBinary(TreeNodeExprBinary * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e1 = root->leftExpression ();
    TreeNodeExpr * e2 = root->rightExpression ();
    const TypeBasic * eType1 = nullptr, *eType2 = nullptr;

    //set context data type
    switch (root->type()) {
    case NODE_EXPR_BINARY_COMMA:
        e2->setContext(root);
        TCGUARD (visitExpr(e1));
        TCGUARD (visitExpr(e2));
        root->setResultType(e2->resultType());
        return OK;
    case NODE_EXPR_BINARY_ADD:
    case NODE_EXPR_BINARY_DIV:
    case NODE_EXPR_BINARY_LAND:
    case NODE_EXPR_BINARY_LOR:
    case NODE_EXPR_BINARY_MOD:
    case NODE_EXPR_BINARY_MUL:
    case NODE_EXPR_BINARY_SUB:
    case NODE_EXPR_BITWISE_AND:
    case NODE_EXPR_BITWISE_OR:
    case NODE_EXPR_BITWISE_XOR:
    case NODE_EXPR_BINARY_SHL:
    case NODE_EXPR_BINARY_SHR:
        e1->setContext(root);
        e2->setContext(root);
        break;
    default:
        e1->setContextSecType(root->contextSecType());
        e2->setContextSecType(root->contextSecType());
        break;
    }

    TCGUARD (visitExpr(e1));
    if (checkAndLogIfVoid(e1))
        return E_TYPE;

    TCGUARD (visitExpr(e2));
    if (checkAndLogIfVoid(e2))
        return E_TYPE;

    auto const lTypeOrig = cast<const TypeBasic *>(e1->resultType());
    auto const rTypeOrig = cast<const TypeBasic *>(e2->resultType());

    // Can't have an expression with a private-only type and a public type
    if (! checkUserPrimPublic (lTypeOrig, rTypeOrig) ||
        ! checkUserPrimPublic (rTypeOrig, lTypeOrig))
    {
        m_log.fatalInProc (root)
            << "Binary expression operands at " << root->location ()
            << " have a public type and a user defined type without a public representation.";
        return E_TYPE;
    }

    const DataType* upper = upperDataType (lTypeOrig, rTypeOrig);
    if (upper != nullptr) {
        if (upper->isUserPrimitive ()) {
            // One of the operands must have been private. Use its
            // public type to instantiate.

            const SecurityType* secType = lTypeOrig->secrecSecType()->isPrivate()
                ? lTypeOrig->secrecSecType()
                : rTypeOrig->secrecSecType();

            auto publicType = dtypeDeclassify (secType, upper);

            if (publicType != nullptr) {
                SecrecDataType upperDT =
                    cast<const DataTypeBuiltinPrimitive*>(publicType)
                    ->secrecDataType();
                e1->instantiateDataType (upperDT);
                e2->instantiateDataType (upperDT);
            }
        }
        else {
            SecrecDataType upperDT = cast<const DataTypeBuiltinPrimitive*> (upper)->secrecDataType ();
            e1->instantiateDataType (upperDT);
            e2->instantiateDataType (upperDT);
        }
    }

    eType1 = cast<const TypeBasic *>(e1->resultType());
    eType2 = cast<const TypeBasic *>(e2->resultType());

    SecrecDimType const n1 = eType1->secrecDimType();
    SecrecDimType const n2 = eType2->secrecDimType();

    if (n1 != n2 && n1 > 0 && n2 > 0) {
        m_log.fatalInProc(root) << "Binary operation " << root->operatorString()
                                << " between operands with dimensions " << n1 << " and " << n2
                                << " at " << root->location() << '.';
        return E_TYPE;
    }

    SecrecDimType const n0 = upperDimType(n1, n2);

    // Find user definition
    SymbolProcedure* match = nullptr;
    if (overloadedOpGood(eType1) && overloadedOpGood(eType2)) {
        std::vector<const TypeBasic*> argumentDataTypes;
        argumentDataTypes.push_back(eType1);
        argumentDataTypes.push_back(eType2);
        const TypeProc* argTypes = TypeProc::get(argumentDataTypes);
        TCGUARD(findBestMatchingOpDef(match, root->operatorName(), *root, argTypes, root));
    }

    if (match != nullptr) { // Overloaded operator
        // Add implicit classify nodes if needed:
        const std::vector<const TypeBasic*>& params =
            match->decl()->procedureType()->paramTypes();

        e1->setContextDataType(params[0u]->secrecDataType());
        e2->setContextDataType(params[1u]->secrecDataType());

        (void) classifyIfNeeded(e1, params[0u]->secrecSecType());
        (void) classifyIfNeeded(e2, params[1u]->secrecSecType());

        const Type* rtv = match->decl()->procedureType()->returnType();
        auto const rt = cast<const TypeBasic*>(rtv);

        root->setResultType(
            TypeBasic::get(rt->secrecSecType(), rt->secrecDataType(), n0));
        root->setProcSymbol(match);

        return OK;
    }
    else { // Not overloaded
        auto const s1 = eType1->secrecSecType();
        auto const s2 = eType2->secrecSecType();
        auto const s0 = upperSecType(s1, s2);

        if (s0 == nullptr || s0->isPrivate()) {
            m_log.fatalInProc(root) << "Binary expression on private operands at "
                                    << root->location()
                                    << " has no matching operator definition.";
            return E_TYPE;
        }

        assert (s1->isPublic() && s2->isPublic());

        eType1 = cast<const TypeBasic *>(e1->resultType());
        eType2 = cast<const TypeBasic *>(e2->resultType());

        auto const d1 = eType1->secrecDataType();
        auto const d2 = eType2->secrecDataType();
        auto const d0 = getResultDType(root->type(), d1, d2);

        if (d0 != DATATYPE_UNDEFINED) {
            switch (root->type()) {
                case NODE_EXPR_BINARY_EQ:
                case NODE_EXPR_BINARY_GE:
                case NODE_EXPR_BINARY_GT:
                case NODE_EXPR_BINARY_LE:
                case NODE_EXPR_BINARY_LT:
                case NODE_EXPR_BINARY_NE:
                    e1->instantiateDataType();
                    e2->instantiateDataType();
                default:
                    break;
            }

            root->setResultType(TypeBasic::get (s0, d0, n0));
            return OK;
        }
        else {
            m_log.fatalInProc(root) << "Binary operation " << root->operatorString()
                                    << " between operands of type " << *lTypeOrig << " and " << *rTypeOrig
                                    << " at " << root->location() << '.';
            return E_TYPE;
        }
    }
}

void TreeNodeExprBinary::instantiateDataTypeV(SecrecDataType dType) {
    resetDataType(dType);
    leftExpression()->instantiateDataType(dType);
    rightExpression()->instantiateDataType(dType);
}

/*******************************************************************************
  TreeNodeExprUnary
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprUnary(TreeNodeExprUnary * root) {
    if (root->haveResultType())
        return OK;

    assert(root->type() == NODE_EXPR_UMINUS ||
            root->type() == NODE_EXPR_UNEG ||
            root->type() == NODE_EXPR_UINV);
    TreeNodeExpr * e = root->expression();
    e->setContext(root);
    TCGUARD (visitExpr(e));
    const SecreC::Type * eType = e->resultType();

    if (eType->kind () == Type::BASIC) {
        const TypeBasic * et = static_cast<const TypeBasic *>(eType);

        {   // check if operator is overloaded
            SymbolProcedure * match = nullptr;
            std::vector<const TypeBasic *> argumentDataTypes;
            argumentDataTypes.push_back (et);
            const TypeProc* argTypes = TypeProc::get(argumentDataTypes);
            TCGUARD(findBestMatchingOpDef(match, root->operatorName(), *root,
                                          argTypes, root));
            if (match != nullptr) { // overloaded operator
                root->setProcSymbol(match);

                const Type* rtv = match->decl()->procedureType()->returnType();
                const TypeBasic* rt = cast<const TypeBasic*>(rtv);

                root->setResultType(
                    TypeBasic::get(rt->secrecSecType(),
                                   rt->secrecDataType(),
                                   et->secrecDimType()));

                return OK;
            }
            else if (et->secrecSecType()->isPrivate()) {
                m_log.fatalInProc(root) << "Unary expression on private operand at "
                                        << root->location()
                                        << " has no matching operator definition.";
                return E_TYPE;
            }
        }

        if (root->type() == NODE_EXPR_UINV) {
            if (isUnsignedNumericDataType(et->secrecDataType ())
                    || et->secrecDataType ()->isAbstractNumeric()
                    || et->secrecDataType ()->isBool ())
            {
                root->setResultType (et);
                return OK;
            }
        } else if (root->type() == NODE_EXPR_UNEG) {
            if (et->secrecDataType ()->isBool ()) {
                root->setResultType (et);
                return OK;
            }
        }
        else if (root->type() == NODE_EXPR_UMINUS) {
            if (isNumericDataType(et->secrecDataType())
                    || et->secrecDataType()->isAbstractNumeric())
            {
                root->setResultType (et);
                return OK;
            }
        }
    }

    m_log.fatalInProc(root) << "Invalid expression of type (" << *eType
        << ") given to unary "
        << (root->type() == NODE_EXPR_UINV
                ? "bitwise negation"
                : (root->type() == NODE_EXPR_UNEG
                    ? "negation"
                    : "minus"))
        << " operator at " << root->location() << '.';
    return E_TYPE;
}

void TreeNodeExprUnary::instantiateDataTypeV (SecrecDataType dType) {
    resetDataType (dType);
    expression ()->instantiateDataType (dType);
}

/*******************************************************************************
  TreeNodeExprBool
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprBool(TreeNodeExprBool * e) {
    if (! e->haveResultType()) {
        e->setResultType(TypeBasic::get(DATATYPE_BOOL));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprArrayConstructor
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprArrayConstructor(TreeNodeExprArrayConstructor * e) {
    assert (e != nullptr);
    assert (! e->expressions().empty() &&
            "Parser verifies that array constructors are not empty.");

    if (e->haveResultType ()) {
        return OK;
    }

    const TypeBasic* elemType = nullptr;
    for (TreeNodeExpr& child : e->expressions ()) {
        child.setContextSecType (e->contextSecType ());
        child.setContextDataType (e->contextDataType ());
        child.setContextDimType (0);
        TCGUARD (visitExpr (&child));

        if (checkAndLogIfVoid (&child))
            return E_TYPE;

        const auto childType = static_cast<const TypeBasic*>(child.resultType ());
        assert (childType != nullptr);

        if (! childType->isScalar ()) {
            m_log.fatalInProc (e) << "Expecting scalar elements in array constructor at "
                                  << child.location () << ".";
            return E_TYPE;
        }

        if (childType->isString()) {
            m_log.fatalInProc(e)
                    << "Array of strings declared at " << child.location() << ".";
            m_log.fatal() << "This feature is currently not supported.";
            return E_TYPE;
        }

        if (elemType == nullptr) {
            elemType = childType;
        }
        else {
            elemType = upperTypeBasic (childType, elemType);
            if (elemType == nullptr) {
                m_log.fatalInProc (e) << "Array element of invalid type at "
                                      << child.location () << ".";
                return E_TYPE;
            }
        }
    }

    for (TreeNodeExpr& child : e->expressions ()) {
        classifyIfNeeded (&child, elemType->secrecSecType ());
    }


    e->setResultType (TypeBasic::get (elemType->secrecSecType (),
                                      elemType->secrecDataType (),
                                      1));
    return OK;
}

void TreeNodeExprArrayConstructor::instantiateDataTypeV(SecrecDataType dType) {
    resetDataType (dType);
    for (TreeNodeExpr& child : expressions ()) {
        child.instantiateDataType (dType);
    }
}

/*******************************************************************************
  TreeNodeExprInt
*******************************************************************************/

namespace /* anonymous */ {

uint64_t charToDigit(char c) {
    switch (c) {
        #define X(c,d) case c: return d;
        X('0', 0u) X('1', 1u) X('2', 2u) X('3', 3u) X('4', 4u)
        X('5', 5u) X('6', 6u) X('7', 7u) X('8', 8u) X('9', 9u)
        X('a',10u) X('b',11u) X('c',12u) X('d',13u) X('e',14u) X('f',15u)
        X('A',10u) X('B',11u) X('C',12u) X('D',13u) X('E',14u) X('F',15u)
        #undef X
        default:
            assert(0 && "Invalid digit character!");
            return 0;
    }
}

bool readIntLiteralSuffix(StringRef input,
                          std::size_t * length,
                          SecrecDataType * dataType)
{
    assert (length);
    assert (dataType);

    static std::array<std::pair<std::string, SecrecDataType>, 10> const
        suffixes = {{
            {"u8",  DATATYPE_UINT8},
            {"u16", DATATYPE_UINT16},
            {"u32", DATATYPE_UINT32},
            {"u64", DATATYPE_UINT64},
            {"i8",  DATATYPE_INT8 },
            {"i16", DATATYPE_INT16},
            {"i32", DATATYPE_INT32},
            {"i64", DATATYPE_INT64},
            {"f32", DATATYPE_FLOAT32},
            {"f64", DATATYPE_FLOAT64}
        }};

    if (input.size() > 2) {
        for (auto const & suffix : suffixes) {
            if (input.endsWith(suffix.first)) {
                *length -= suffix.first.size();
                *dataType = suffix.second;
                return true;
            }
        }
    }

    return false;
}

bool convertNumber(StringRef input,
                   SecrecDataType * dataType,
                   uint64_t * result)
{
    assert (dataType);
    assert (result);

    const char* start = input.data(); // start of the actual number part
    size_t length = input.size(); // length of the actual number part

    // Read suffix:
    SecrecDataType tempDataType;
    bool const haveSuffix = readIntLiteralSuffix(input, &length, &tempDataType);

    // Read prefix:
    uint64_t base = 10;
    if (length > 2 && start[0] == '0') {
        switch (start[1]) {
        case 'b': base = 2; start += 2; length -= 2; break;
        case 'o': base = 8; start += 2; length -= 2; break;
        case 'x': base = 16; start += 2; length -= 2; break;
        default:
            break;
        }
    }

    // Read value:
    uint64_t value = 0;
    for (std::size_t offset = 0; offset < length; ++ offset) {
        uint64_t const digit = charToDigit(start[offset]);
        assert(digit < base);
        uint64_t const newValue = value*base + digit;
        if (newValue < value) {
            return false;
        }

        value = newValue;
    }

    // Commit:
    *dataType = haveSuffix ? tempDataType : DATATYPE_NUMERIC;
    *result = value;
    return true;
}

uint64_t maxIntLiteralValue(SecrecDataType dataType) {
    switch (dataType) {
    case DATATYPE_INT8: return std::numeric_limits<int8_t>::max();
    case DATATYPE_INT16: return std::numeric_limits<int16_t>::max();
    case DATATYPE_INT32: return std::numeric_limits<int32_t>::max();
    case DATATYPE_INT64: return std::numeric_limits<int64_t>::max();
    case DATATYPE_UINT8: return std::numeric_limits<uint8_t>::max();
    case DATATYPE_UINT16: return std::numeric_limits<uint16_t>::max();
    case DATATYPE_UINT32: return std::numeric_limits<uint32_t>::max();
    case DATATYPE_UINT64: return std::numeric_limits<uint64_t>::max();
    case DATATYPE_NUMERIC: return std::numeric_limits<uint64_t>::max();
    default: return 0; /* unexpected, will fail later on */
    }
}


} // namespace anonymous

TypeChecker::Status TypeChecker::visitExprInt(TreeNodeExprInt * e) {
    if (! e->haveResultType()) {
        uint64_t actualValue = 0;
        SecrecDataType dataType = DATATYPE_NUMERIC;
        if (! convertNumber(e->stringValue(), &dataType, &actualValue)) {
            m_log.fatalInProc(e)
                << "Integer literal overflow at "
                << e->location() << '.';
            return E_TYPE;
        }

        const DataType* dType = DataTypeBuiltinPrimitive::get(dataType);
        if (actualValue > maxIntLiteralValue(dataType)) {
            m_log.fatalInProc(e)
                << "Integer literal can not be represented by type \'"
                << *dType << "\' at " << e->location() << '.';
            return E_TYPE;
        }

        if (e->haveContextDataType()) {
            dType = dtypeDeclassify(e->contextSecType(), e->contextDataType());
            if ((dType == nullptr) || ! isNumericDataType(dType)) {
                m_log.fatalInProc(e) << "Expecting numeric context at "
                                     << e->location() << '.';
                return E_TYPE;
            }
        }

        e->setActualValue(actualValue);
        e->setResultType(TypeBasic::get(dType));
    }

    return OK;
}

void TreeNodeExprInt::instantiateDataTypeV(SecrecDataType dType) {
    resetDataType(dType);
}

/*******************************************************************************
  TreeNodeExprFloat
*******************************************************************************/

namespace /* anonymous */ {

SecrecDataType readFloatLiteralSuffix(StringRef input)  {

    if (input.endsWith("f32")) return DATATYPE_FLOAT32;
    if (input.endsWith("f64")) return DATATYPE_FLOAT64;

    return DATATYPE_NUMERIC_FLOAT;
}

} // namespace anonymous

TypeChecker::Status TypeChecker::visitExprFloat(TreeNodeExprFloat * e) {
    if (!e->haveResultType()) {
        auto const secrecDataType = readFloatLiteralSuffix(e->value());
        DataType const * dType = DataTypeBuiltinPrimitive::get(secrecDataType);
        if (e->haveContextDataType()) {
            dType = dtypeDeclassify(e->contextSecType(), e->contextDataType());
            if (dType == nullptr) {
                m_log.fatalInProc(e) << "ICE: Unknown type at " << e->location() << ".";
                return E_TYPE;
            }
        }

        e->setResultType(TypeBasic::get(dType));
    }

    return OK;
}

void TreeNodeExprFloat::instantiateDataTypeV(SecrecDataType dType) {
    resetDataType(dType);
}

/*******************************************************************************
  TreeNodeExprClassify
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprClassify(TreeNodeExprClassify * root) {
    if (! root->haveResultType()) {
        m_log.fatalInProc(root) << "ICE: type checking classify node at "
                                << root->location() << '.';
        return E_OTHER;
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprDeclassify
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprDeclassify(TreeNodeExprDeclassify * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e = root->expression();
    e->setContextDimType(root->contextDimType());
    TCGUARD (visitExpr(e));

    const Type * childType = e->resultType();
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    if (childType->secrecSecType()->isPublic()) {
        m_log.fatalInProc(root) << "Argument of type " << *childType
            << " passed to declassify operator at "
            << root->location() << '.';
        return E_TYPE;
    }

    if (childType->secrecDataType ()->isUserPrimitive ()) {
        auto dt = static_cast<const DataTypeUserPrimitive*> (childType->secrecDataType ());
        SymbolKind* kind =
            static_cast<const PrivateSecType*> (childType->secrecSecType ())->securityKind ();
        assert (kind->findType (dt->name ()) != nullptr);
        auto publicType = kind->findType (dt->name ())->publicType;

        if (! publicType) {
            m_log.fatalInProc (root)
                << "Type '" << dt->name () << "' does not have a public representation at "
                << root->location () << '.';
            return E_TYPE;
        }
    }

    root->setResultType(
        TypeBasic::get(
            dtypeDeclassify(childType->secrecSecType(), childType->secrecDataType()),
            childType->secrecDimType()));
    return OK;
}

void TreeNodeExprDeclassify::instantiateDataTypeV(SecrecDataType dType) {
    resetDataType(dType);
    expression()->instantiateDataType(dType);
}

/*******************************************************************************
  TreeNodeExprRVariable
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprRVariable (TreeNodeExprRVariable * e) {
    if (e->haveResultType())
        return OK;

    TreeNodeIdentifier * id = e->identifier();
    Symbol * s = findIdentifier (SYM_SYMBOL, id);

    if (s == nullptr) {
        // TODO: this is not the prettiest solution
        SymbolDimensionality* symDim = findIdentifier<SYM_DIM>(id);
        if (symDim == nullptr)
            return E_TYPE;

        s = ConstantInt::get (DATATYPE_UINT64, symDim->dimType ());
    }

    e->setResultType(s->secrecType());
    e->setValueSymbol(s);
    return OK;
}

/*******************************************************************************
  TreeNodeExprString
*******************************************************************************/

bool TreeNodeExprString::isConstant () const {
    for (TreeNodeStringPart& part : parts ()) {
        if (! part.isConstant ())
            return false;
    }

    return true;
}

TypeChecker::Status TypeChecker::visitExprString(TreeNodeExprString * e) {
    if (e->haveResultType())
        return OK;

    for (TreeNodeStringPart& part : e->parts ()) {
        TCGUARD (visitStringPart (&part));
    }

    e->setResultType(TypeBasic::get(DATATYPE_STRING));
    return OK;
}

/*******************************************************************************
  TreeNodeStringPart
*******************************************************************************/

TypeChecker::Status TypeChecker::visitStringPart (TreeNodeStringPart * e) {
    return dispatchStringPart (*this, e);
}

/*******************************************************************************
  TreeNodeStringPartFragment
*******************************************************************************/

TypeChecker::Status TypeChecker::visitStringPartFragment(TreeNodeStringPartFragment *) {
    return OK;
}

/*******************************************************************************
  TreeNodeStringPartIdentifier
*******************************************************************************/

StringRef TreeNodeStringPartIdentifier::staticValue () const {
    assert (isConstant ());
    return m_value->value ();
}

TypeChecker::Status TypeChecker::visitStringPartIdentifier(TreeNodeStringPartIdentifier * p) {
    if (p->value () || p->secrecType ())
        return OK;

    const StringRef name = p->name ();
    SymbolDomain* symDom = m_st->find<SYM_DOMAIN>(name);
    SymbolDataType* symTy = m_st->find<SYM_TYPE>(name);
    SymbolSymbol* symSym = m_st->find<SYM_SYMBOL>(name);

    if (symDom == nullptr && symTy == nullptr && symSym == nullptr) {
        m_log.fatalInProc (p) << "Identifier \'" << name <<
                                 "\' at " << p->location () << " not in scope.";
        return E_TYPE;
    }

    if ((!!symDom + !!symTy + !!symSym) != 1) {
        m_log.fatalInProc (p) << "Ambiguous use of identifier \'" << name <<
                                 "\' at " << p->location () << ".";
        return E_TYPE;
    }

    if (symDom != nullptr) {
        const SecurityType* secTy = symDom->securityType ();
        if (secTy->isPublic ()) {
            p->setValue (ConstantString::get (getContext (), "public"));
        }
        else {
            p->setValue (ConstantString::get (getContext (),
                static_cast<const PrivateSecType*>(secTy)->name ()));
        }
    }
    else
    if (symTy != nullptr) {
        std::ostringstream os;
        os << *symTy->dataType ();
        p->setValue (ConstantString::get (getContext (), os.str ()));
    }
    else
    if (symSym != nullptr) {
        const TypeNonVoid* ty = symSym->secrecType ();
        if (! canPrintValue (ty)) {
            m_log.fatalInProc(p)
                << "Unable to convert variable \"" << symSym->name () << "\" to string. "
                << "Got " << PrettyPrint (ty) << " at " << p->location() << ". "
                << "Expecting a public scalar or a string.";
            return E_TYPE;
        }

        p->setSecrecType (ty);
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprTernary
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprTernary(TreeNodeExprTernary * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e1 = root->conditional();
    e1->setContextSecType(PublicSecType::get());
    e1->setContextDataType(DataTypeBuiltinPrimitive::get (DATATYPE_BOOL));
    TCGUARD (visitExpr(e1));
    if (checkAndLogIfVoid(e1))
        return E_TYPE;

    TreeNodeExpr * e2 = root->trueBranch();
    e2->setContext(root);
    TCGUARD (visitExpr(e2));

    TreeNodeExpr * e3 = root->falseBranch();
    e3->setContext(root);
    TCGUARD (visitExpr(e3));

    const SecreC::Type * eType1 = e1->resultType();
    const SecreC::Type * eType2 = e2->resultType();
    const SecreC::Type * eType3 = e3->resultType();

    const auto cType = cast<const TypeNonVoid *>(eType1);

    // check if conditional expression is of public boolean type
    if (! cType->secrecDataType()->isBool () || cType->secrecSecType()->isPrivate())
    {
        m_log.fatalInProc(root) << "Conditional subexpression at " << e1->location()
            << " of ternary expression has to be public boolean, got "
            << PrettyPrint(cType) << '.';
        return E_TYPE;
    }

    // check the types of results
    if (eType2->isVoid() != eType3->isVoid()) {
        m_log.fatalInProc(root) << "Subxpression at " << e2->location() << " is"
            << (eType2->isVoid() ? "" : "not ")
            << " void while subexpression at " << e3->location()
            << (eType3->isVoid() ? " is." : " isn't.");
        return E_TYPE;
    }

    if (!eType2->isVoid()) {
        e2->instantiateDataType(e3->resultType()->secrecDataType());
        e3->instantiateDataType(e2->resultType()->secrecDataType());
        eType2 = e2->resultType();
        eType3 = e3->resultType();

        const SecurityType * s0 = upperSecType(e2->resultType()->secrecSecType(),
                e3->resultType()->secrecSecType());
        if (s0 == nullptr) {
            m_log.fatalInProc(root) << "Incompatible security types in ternary expression at "
                << e2->location() << " and " << e3->location() << '.';
            m_log.fatal() << "Unable to match "
                << *e2->resultType()->secrecSecType() << " with "
                << *e3->resultType()->secrecSecType() << '.';
            return E_TYPE;
        }

        e2 = classifyIfNeeded(e2, s0);
        e3 = classifyIfNeeded(e3, s0);

        if (eType2->secrecDataType() != eType3->secrecDataType()) {
            m_log.fatalInProc(root) << "Results of ternary expression  at "
                << root->location()
                << " have to be of same data types, got "
                << *eType2 << " and " << *eType3 << '.';
            return E_TYPE;
        }

        SecrecDimType n1 = eType1->secrecDimType();
        SecrecDimType n2 = eType2->secrecDimType();
        SecrecDimType n3 = eType3->secrecDimType();

        if (n2 != n3) {
            m_log.fatalInProc(root) << "Branches of ternary expression at "
                << root->location()
                << " aren't of equal dimensionalities.";
            return E_TYPE;
        }

        if (n1 != 0 && n1 != n2) {
            m_log.fatalInProc(root) << "Conditional expression at "
                << e1->location()
                << " is non-scalar and doesn't match resulting subexpressions.";
            return E_TYPE;
        }
    }

    assert(e2->resultType() == e3->resultType());
    root->setResultType(e2->resultType());
    return OK;
}

void TreeNodeExprTernary::instantiateDataTypeV(SecrecDataType dType) {
    resetDataType(dType);
    trueBranch()->instantiateDataType(dType);
    falseBranch()->instantiateDataType(dType);
}

/*******************************************************************************
  TreeNodeExprDomainID
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprDomainID(TreeNodeExprDomainID * e) {
    if (e->haveResultType())
        return OK;

    if (e->securityType()->isPublic()) {
        m_log.fatalInProc(e) << "Public security type does not have a domain ID at "
                             << e->location() << '.';
        return E_TYPE;
    }

    TCGUARD (visitSecTypeF (e->securityType()));
    e->setResultType(TypeBasic::get(DATATYPE_UINT64));
    return OK;
}

/*******************************************************************************
  TreeNodeExprQualified
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprQualified(TreeNodeExprQualified * e) {
    if (e->haveResultType())
        return OK;

    TypeContext suppliedContext;
    TreeNodeExpr * subExpr = e->expression();
    subExpr->setContext(e);
    for (TreeNodeTypeF& node : e->types()) {
        TCGUARD (visitTypeF (&node));
        node.setTypeContext (suppliedContext);
        node.setTypeContext (*subExpr); // not the nicest solution
    }

    TCGUARD (visitExpr(subExpr));

    /* Check that the actual type matches the qualified type: */

    if (suppliedContext.haveContextSecType ()) {
        if (subExpr->contextSecType() !=
                subExpr->resultType()->secrecSecType()) {
            m_log.fatalInProc(e) << "Security type of the expression at "
                << subExpr->location()
                << " does not match the qualified type.";
            return E_TYPE;
        }
    }

    if (suppliedContext.haveContextDataType ()) {
        if (! subExpr->contextDataType()->
            equals (subExpr->resultType()->secrecDataType()))
        {
            m_log.fatalInProc(e) << "Data type of the expression at "
                << subExpr->location()
                << " does not match the qualified type.";

            return E_TYPE;
        }
    }

    if (suppliedContext.haveContextDimType ()) {
        if (subExpr->contextDimType() !=
                subExpr->resultType()->secrecDimType()) {
            m_log.fatalInProc(e) << "Dimensionality type of the expression at "
                << subExpr->location()
                << " does not match the qualified type.";
            return E_TYPE;
        }
    }

    e->setResultType(subExpr->resultType());
    return OK;
}

void TreeNodeExprQualified::instantiateDataTypeV(SecrecDataType dType) {
    resetDataType(dType);
    expression()->instantiateDataType(dType);
}

/*******************************************************************************
  TreeNodeExprStringFromBytes
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprStringFromBytes(TreeNodeExprStringFromBytes * e) {
    if (e->haveResultType())
        return OK;

    TreeNodeExpr * subExpr = e->expression();
    subExpr->setContextSecType(PublicSecType::get());
    subExpr->setContextDataType(DataTypeBuiltinPrimitive::get (DATATYPE_UINT8));
    subExpr->setContextDimType(1);
    TCGUARD (visitExpr(subExpr));

    const Type * ty = subExpr->resultType();
    if (ty->secrecSecType()->isPrivate() ||
            ! ty->secrecDataType()->equals (DATATYPE_UINT8) ||
            ty->secrecDimType() != 1)
    {
        m_log.fatalInProc(e) << "Invalid argument. Expected public byte array, got "
                             << PrettyPrint(ty) << " at " << e->location() << '.';
        return E_TYPE;
    }

    const TypeBasic * resultType = TypeBasic::get(PublicSecType::get(), DATATYPE_STRING);
    e->setResultType(resultType);
    return OK;
}

/*******************************************************************************
  TreeNodeExprBytesFromString
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprBytesFromString(TreeNodeExprBytesFromString * e) {
    if (e->haveResultType())
        return OK;

    TreeNodeExpr * subExpr = e->expression();
    subExpr->setContext(TypeBasic::get(DATATYPE_STRING));
    TCGUARD (visitExpr(subExpr));

    const Type * ty = subExpr->resultType();
    if (ty->secrecSecType()->isPrivate() ||
            ! ty->secrecDataType()->isString () ||
            ty->secrecDimType() != 0)
    {
        m_log.fatalInProc(e) << "Invalid argument. Expected public string, got "
                             << PrettyPrint(ty) << " at " << e->location() << '.';
        return E_TYPE;
    }

    const TypeBasic * resultType = TypeBasic::get(PublicSecType::get(), DATATYPE_UINT8, 1);
    e->setResultType(resultType);
    return OK;
}

/*******************************************************************************
  TreeNodeExprGetFpuState
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprGetFpuState(TreeNodeExprGetFpuState * e) {
    if (e->haveResultType())
        return OK;

    e->setResultType(TypeBasic::get(DATATYPE_UINT64));
    return OK;
}

/*******************************************************************************
  TreeNodeExprSetFpuState
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprSetFpuState(TreeNodeExprSetFpuState * e) {
    if (e->haveResultType())
        return OK;

    auto * subExpr = e->expression();
    subExpr->setContext(TypeBasic::get(DATATYPE_UINT64));
    TCGUARD(visitExpr(subExpr));

    assert(subExpr->haveResultType());
    auto const * ty = subExpr->resultType();
    if (ty->isVoid()
        || !ty->secrecDataType()->equals(DATATYPE_UINT64)
        || !ty->secrecSecType()->isPublic()
        || !ty->isScalar())
    {
        m_log.fatalInProc(e) << "Argument to __set_fpu_state must be of type "
                                "public uint64 in " << e->location();
        return E_TYPE;
    }
    e->setResultType(TypeVoid::get());
    return OK;
}

/*******************************************************************************
  TreeNodeExprPrefix
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprPrefix(TreeNodeExprPrefix * root) {
    return checkPostfixPrefixIncDec(root, root, true,
            root->type() == NODE_EXPR_PREFIX_INC);
}

/*******************************************************************************
  TreeNodeExprPostfix
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprPostfix(TreeNodeExprPostfix * root) {
    return checkPostfixPrefixIncDec(root, root, false,
            root->type() == NODE_EXPR_POSTFIX_INC);
}

/*******************************************************************************
  TreeNodeExprNone
*******************************************************************************/

void TreeNodeExprNone::instantiateDataTypeV (SecrecDataType) { }

TypeChecker::Status TypeChecker::visitExprNone (TreeNodeExprNone *e) {
    m_log.fatalInProc (e) << "Invalid expression node at " << e->location () << ". "
                          << "Most likely internal compiler error!";
    return E_TYPE;
}

/*******************************************************************************
  TreeNodeExprStrlen
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprStrlen(TreeNodeExprStrlen* e) {
    if (e->haveResultType ())
        return OK;

    auto subExpr = e->expression ();
    auto stringType = TypeBasic::get(DATATYPE_STRING);
    subExpr->setContext(stringType);
    TCGUARD (visitExpr(subExpr));
    if (subExpr->resultType () != stringType) {
        m_log.fatalInProc (e) << "Expecting string as argument to 'strlen' got "
                              << *subExpr->resultType () << " at " << subExpr->location () << ".";
        return E_TYPE;
    }

    e->setResultType (TypeBasic::getIndexType ());
    return OK;
}


} // namespace SecreC
