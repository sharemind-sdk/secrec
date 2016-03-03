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

#include "Constant.h"
#include "DataType.h"
#include "Imop.h"
#include "Log.h"
#include "Misc.h"
#include "SecurityType.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Types.h"
#include "Visitor.h"

#include <boost/range.hpp>


namespace SecreC {

namespace /* anonymous */ {

SecrecDataType getResultDType(SecrecTreeNodeType type, SecrecDataType d1, SecrecDataType d2) {
    switch (type) {
    case NODE_EXPR_BINARY_ADD:
        if (d1 != d2) break;
        if (! isNumericDataType(d1) && d1 != DATATYPE_STRING)
            break;
        return d1;
    case NODE_EXPR_BINARY_SHL:
    case NODE_EXPR_BINARY_SHR:
        if (isSignedNumericDataType (d1))
            break;
    case NODE_EXPR_BINARY_SUB:
    case NODE_EXPR_BINARY_MUL:
    case NODE_EXPR_BINARY_MOD:
    case NODE_EXPR_BINARY_DIV:
        if (d1 != d2) break;
        if (d1 == DATATYPE_STRING) break;
        if (d1 != DATATYPE_NUMERIC && ! isNumericDataType(d1))
            break;
        return d1;
    case NODE_EXPR_BITWISE_AND:
    case NODE_EXPR_BITWISE_OR:
    case NODE_EXPR_BITWISE_XOR:
        if (d1 != d2) break;
        if (d1 == DATATYPE_STRING) break;
        return d1;
    case NODE_EXPR_BINARY_EQ:
    case NODE_EXPR_BINARY_GE:
    case NODE_EXPR_BINARY_GT:
    case NODE_EXPR_BINARY_LE:
    case NODE_EXPR_BINARY_LT:
    case NODE_EXPR_BINARY_NE:
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

SecrecDataType getResultDType(SecrecTreeNodeType type, DataType* d1, DataType* d2) {
    if (d1->isBuiltinPrimitive () && d2->isBuiltinPrimitive ()) {
        return getResultDType (type,
            static_cast<DataTypeBuiltinPrimitive*>(d1)->secrecDataType (),
            static_cast<DataTypeBuiltinPrimitive*>(d2)->secrecDataType ());
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

    TypeNonVoid* eType = lval->secrecType ();

    if (eType->secrecDataType ()->isUserPrimitive ()) {
        auto dt = static_cast<DataTypeUserPrimitive*> (eType->secrecDataType ());
        assert (eType->secrecSecType ()->isPrivate ());

        SymbolKind* kind = static_cast<PrivateSecType*> (eType->secrecSecType ())->securityKind ();
        assert (dt->inKind (kind));
        auto publicType = dt->publicType (kind);

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

        std::vector<TypeBasic*> argumentDataTypes;
        DataType* publicDataType = dtypeDeclassify (getContext (),
                                                    eType->secrecSecType(),
                                                    eType->secrecDataType());
        TypeBasic* publicType = TypeBasic::get (getContext (),
                                                publicDataType,
                                                dim);
        TypeBasic* varType = TypeBasic::get (getContext (),
                                             eType->secrecSecType (),
                                             eType->secrecDataType (),
                                             dim);

        argumentDataTypes.push_back (varType);
        argumentDataTypes.push_back (publicType);

        SymbolProcedure* symProc;
        TypeProc* callType = TypeProc::get (getContext (), argumentDataTypes);
        TCGUARD (findBestMatchingOpDef (symProc,
                                        op->operatorName (),
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

    // check that we are operating on numeric types
    if (! isNumericDataType (eType->secrecDataType ())) {
        m_log.fatalInProc (root) << m1 << m2
            << " operator expects numeric type, given "
            << *eType << " at " << root->location () << '.';
        return E_TYPE;
    }

    root->setResultType (eType);
    return OK;
}

TypeNonVoid* TypeChecker::checkSelect (const Location& loc, Type* ty,
                                       TreeNodeIdentifier* id)
{
    assert (ty != nullptr);

    if (ty->kind () != Type::BASIC || ! ty->secrecDataType ()->isComposite ()) {
        m_log.fatal () << "Expecting structure, got " << *ty << ". "
                       << "Error at " << loc << ".";
        return nullptr;
    }

    // Verify attribute access:
    DataTypeStruct* structType = static_cast<DataTypeStruct*>(ty->secrecDataType ());
    StringRef fieldName = id->value ();
    TypeBasic* matchingFieldType = nullptr;
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
    TypeNonVoid* fieldType = checkSelect (structExpr->location (), structExpr->resultType (), e->identifier ());
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
    TypeNonVoid * lhsType = lval->secrecType ();

    // Calculate type of r-value:
    TreeNodeExpr * src = e->rightHandSide ();
    src->setContext(lhsType);

    TCGUARD (visitExpr(src));

    if (checkAndLogIfVoid(src))
        return E_TYPE;

    TypeNonVoid * srcType = static_cast<TypeNonVoid *>(src->resultType());
    if (! srcType->latticeLEQ(getContext (), lhsType)) {
        m_log.fatalInProc(e) << "Invalid assignment from value of type " << *srcType
            << " to variable of type " << *lhsType << " at "
            << e->location() << '.';
        return E_TYPE;
    }

    // Search for overloaded operator
    if (e->type() != NODE_EXPR_BINARY_ASSIGN) {
        SecrecOperator op = e->getOperator();
        assert(op != SCOP_NONE);

        bool isIndexed = lval->type () == NODE_LVALUE_INDEX;
        // If the lval is indexed, codegen will create a loop
        // operating on scalars
        SecrecDimType lDim = 0;
        SecrecDimType rDim = 0;
        if (! isIndexed)  {
            lDim = lhsType->secrecDimType();
            rDim = srcType->secrecDimType();
        }

        std::vector<TypeBasic*> argumentDataTypes;
        TypeBasic* lType = TypeBasic::get(getContext(),
                                          lhsType->secrecSecType(),
                                          lhsType->secrecDataType(),
                                          lDim);
        TypeBasic* rType = TypeBasic::get(getContext(),
                                          srcType->secrecSecType(),
                                          srcType->secrecDataType(),
                                          rDim);
        argumentDataTypes.push_back(lType);
        argumentDataTypes.push_back(rType);

        SymbolProcedure* symProc;
        TypeProc* callType = TypeProc::get (getContext(), argumentDataTypes);
        TCGUARD (findBestMatchingOpDef(symProc,
                                       e->operatorName(),
                                       callType,
                                       e));

        if (symProc != nullptr) {
            TypeProc* procType = static_cast<TypeProc*>(symProc->secrecType());
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

    TCGUARD (visitDataTypeF (root->dataType (), root->contextSecType ()));

    DataType* resultingDType = root->dataType()->cachedType ();
    TreeNodeExpr * subExpr = root->expression();
    subExpr->setContextSecType(root->contextSecType());
    subExpr->setContextDimType(root->contextDimType());
    TCGUARD (visitExpr(subExpr));

    subExpr->instantiateDataType(getContext());

    if (subExpr->resultType()->isVoid()) {
        m_log.fatalInProc(root) << "Unable to cast from void at "
                                << root->location() << '.';
        return E_TYPE;
    }

    assert(dynamic_cast<TypeBasic*>(subExpr->resultType()) != nullptr);
    TypeBasic * ty = static_cast<TypeBasic*>(subExpr->resultType());
    DataType * givenDType = ty->secrecDataType();

    if (ty->secrecSecType()->isPrivate()) {

        // Check if the kind has the type that we are casting to
        SymbolKind* kind = static_cast<PrivateSecType*>(ty->secrecSecType())->securityKind();
        StringRef name;

        if (resultingDType->isBuiltinPrimitive()) {
            name = SecrecFundDataTypeToString(static_cast<DataTypeBuiltinPrimitive*>(resultingDType)->secrecDataType());
        }
        else {
            name = static_cast<DataTypeUserPrimitive*>(resultingDType)->name();
        }

        DataTypeUserPrimitive* resultUserPrim = kind->findType(name);
        if (resultUserPrim == nullptr) {
            m_log.fatalInProc(root)
                << "Kind " << *kind << " does not have data type " << *resultingDType
                << " at " << root->location() << '.';
            return E_TYPE;
        }

        TypeBasic* want = TypeBasic::get(getContext(),
                                         ty->secrecSecType(),
                                         resultUserPrim,
                                         ty->secrecDimType());
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

    TypeBasic * want = TypeBasic::get(getContext(),
                                      ty->secrecSecType(),
                                      resultingDType,
                                      ty->secrecDimType());

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
    const auto eType = static_cast<TypeNonVoid*>(e->resultType());
    SecrecDimType k = 0;
    SecrecDimType n = eType->secrecDimType();

    if (root->indices()->children().size() != n) {
        m_log.fatalInProc(root) << "Incorrect number of indices at "
            << e->location() << '.';
        return E_TYPE;
    }

    TCGUARD (checkIndices(root->indices(), k));

    root->setResultType(TypeBasic::get(getContext(),
                eType->secrecSecType(), eType->secrecDataType(), k));
    return OK;
}

void TreeNodeExprIndex::instantiateDataTypeV (Context &cxt, SecrecDataType dType) {
    resetDataType (cxt, dType);
    expression ()->instantiateDataType (cxt, dType);
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
        e->instantiateDataType(getContext());
        root->setResultType(TypeBasic::getIndexType(getContext()));
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
        e->instantiateDataType(getContext());
        root->setResultType(TypeBasic::get(getContext(), DATATYPE_UINT64, 1));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprCat
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprCat(TreeNodeExprCat * root) {
    if (root->haveResultType())
        return OK;

    // missing argument is interpreted as 0
    if (root->dimensionality() == nullptr) {
        TreeNode * e = new TreeNodeExprInt(0, root->location());
        root->appendChild(e);
    }

    TypeNonVoid* eTypes[2];

    // check that first subexpressions 2 are arrays and of equal dimensionalities
    for (int i = 0; i < 2; ++ i) {
        assert(dynamic_cast<TreeNodeExpr *>(root->children().at(i)) != nullptr);
        TreeNodeExpr * e = static_cast<TreeNodeExpr *>(root->children().at(i));
        e->setContext(root->typeContext());
        TCGUARD (visitExpr(e));
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        eTypes[i] = static_cast<TypeNonVoid*>(e->resultType());
        if (eTypes[i]->isScalar()) {
            m_log.fatalInProc(root) << "Concatenation of scalar values at "
                << e->location() << '.';
            return E_TYPE;
        }
    }

    DataType* d0 = upperDataType(getContext (),
                                 static_cast<TypeBasic*> (eTypes[0]),
                                 static_cast<TypeBasic*> (eTypes[1]));
    root->leftExpression()->instantiateDataType(getContext(), d0);
    root->rightExpression()->instantiateDataType(getContext(), d0);

    {
        SecurityType * lSecTy = eTypes[0]->secrecSecType();
        SecurityType * rSecTy = eTypes[1]->secrecSecType();
        TreeNodeExpr * left = root->leftExpression();
        TreeNodeExpr * right = root->rightExpression();
        left = classifyIfNeeded(left, rSecTy);
        right = classifyIfNeeded(right, lSecTy);
        eTypes[0] = static_cast<TypeNonVoid*>(left->resultType());
        eTypes[1] = static_cast<TypeNonVoid*>(right->resultType());
    }

    SecurityType * resSecType = upperSecType(eTypes[0]->secrecSecType(),
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

    if (root->dimensionality ()->value () >=  eTypes[0]->secrecDimType()) {
        m_log.fatalInProc (root) << "Array concatenation over invalid dimensionality at "
            << root->dimensionality ()->location () << '.';
        return E_TYPE;
    }

    // type checker actually allows for aribtrary expression here
    // but right now parser expects integer literals, this is OK
    TreeNodeExpr * e3 = root->dimensionality();
    e3->setContextIndexType(getContext());
    TCGUARD (visitExpr(e3));
    if (checkAndLogIfVoid(e3))
        return E_TYPE;

    const auto e3Type = static_cast<TypeNonVoid *>(e3->resultType());
    if (! e3Type->isPublicUIntScalar()) {
        m_log.fatalInProc(root) << "Expected public scalar integer at "
            << root->dimensionality()->location()
            << " got " << *e3Type << '.';
        return E_TYPE;
    }

    root->setResultType(TypeBasic::get(m_context,
                resSecType,
                eTypes[0]->secrecDataType(),
                eTypes[0]->secrecDimType()));
    return OK;
}

void TreeNodeExprCat::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType(cxt, dType);
    leftExpression()->instantiateDataType(cxt, dType);
    rightExpression()->instantiateDataType(cxt, dType);
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

    const auto eType = static_cast<TypeNonVoid *>(e->resultType());
    for (TreeNodeExpr& dim : root->dimensions()) {
        dim.setContextIndexType (getContext());
        TCGUARD (visitExpr(&dim));

        if (checkAndLogIfVoid(&dim))
            return E_TYPE;

        const auto dimType = static_cast<TypeNonVoid *>(dim.resultType());
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

    root->setResultType(TypeBasic::get(getContext(),
                eType->secrecSecType(), eType->secrecDataType(), resDim));
    return OK;
}

void TreeNodeExprReshape::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType(cxt, dType);
    reshapee()->instantiateDataType(cxt, dType);
}

/*******************************************************************************
  TreeNodeExprToString
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprToString(TreeNodeExprToString * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e = root->expression();
    e->setContextSecType(PublicSecType::get(getContext()));
    e->setContextDimType(0);
    TCGUARD (visitExpr(e));
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    e->instantiateDataType(getContext());
    TypeNonVoid * tnv = static_cast<TypeNonVoid *>(e->resultType());
    if (tnv->secrecDimType() != 0
            || tnv->secrecSecType()->isPrivate()
            || tnv->secrecDataType()->isString ()) {
        m_log.fatalInProc(root) << "Invalid argument passed to \"tostring\" expression at "
                                << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(TypeBasic::get(getContext(), DATATYPE_STRING));

    return OK;
}

/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/

bool checkUserPrimPublic (TypeBasic* lType, TypeBasic* rType) {
    DataType* lData = lType->secrecDataType ();

    if (lData->isUserPrimitive ()) {
        auto lPrim = static_cast<DataTypeUserPrimitive*> (lData);
        assert (lType->secrecSecType ()->isPrivate ());
        SymbolKind* kind = static_cast<PrivateSecType*> (lType->secrecSecType ())->securityKind ();

        assert (lPrim->inKind (kind));
        auto publicType = lPrim->publicType (kind);

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
    TypeBasic * eType1 = nullptr, *eType2 = nullptr;
    TypeBasic * eType1Orig = nullptr, *eType2Orig = nullptr;

    //set context data type
    switch (root->type()) {
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

    {
        TCGUARD (visitExpr(e1));
        if (checkAndLogIfVoid(e1))
            return E_TYPE;

        TCGUARD (visitExpr(e2));
        if (checkAndLogIfVoid(e2))
            return E_TYPE;
    }

    assert(dynamic_cast<TypeBasic *>(e1->resultType()) != nullptr);
    eType1Orig = static_cast<TypeBasic *>(e1->resultType());

    assert(dynamic_cast<TypeBasic *>(e2->resultType()) != nullptr);
    eType2Orig = static_cast<TypeBasic *>(e2->resultType());

    {
        TypeBasic* const lType = static_cast<TypeBasic*> (e1->resultType());
        TypeBasic* const rType = static_cast<TypeBasic*> (e2->resultType());

        // Can't have an expression with a private-only type and a public type
        if (! checkUserPrimPublic (lType, rType) ||
            ! checkUserPrimPublic (rType, lType))
        {
            m_log.fatalInProc (root)
                << "Binary expression operands at " << root->location ()
                << " have a public type and a user defined type without a public representation.";
            return E_TYPE;
        }

        DataType* upper = upperDataType (getContext (), lType, rType);

        if (upper != nullptr) {
            if (upper->isUserPrimitive ()) {
                // One of the operands must have been private. Use its
                // public type to instantiate.

                SecurityType* secType = lType->secrecSecType()->isPrivate()
                    ? lType->secrecSecType()
                    : rType->secrecSecType();

                auto publicType = dtypeDeclassify (getContext (), secType, upper);

                if (publicType != nullptr) {
                    SecrecDataType upperDT = static_cast<DataTypeBuiltinPrimitive*>(publicType)->secrecDataType();
                    e1->instantiateDataType(getContext(), upperDT);
                    e2->instantiateDataType(getContext(), upperDT);
                }
            }
            else {
                SecrecDataType upperDT = dtypeDeclassify (static_cast<DataTypeBuiltinPrimitive*> (upper)->secrecDataType ());
                e1->instantiateDataType(getContext(), upperDT);
                e2->instantiateDataType(getContext(), upperDT);
            }
        }

        assert(dynamic_cast<TypeBasic *>(e1->resultType()) != nullptr);
        eType1 = static_cast<TypeBasic *>(e1->resultType());

        assert(dynamic_cast<TypeBasic *>(e2->resultType()) != nullptr);
        eType2 = static_cast<TypeBasic *>(e2->resultType());
    }

    SecrecDimType n1 = eType1->secrecDimType();
    SecrecDimType n2 = eType2->secrecDimType();

    if (n1 == 0 || n2 == 0 || n1 == n2) {
        SecrecDimType n0 = upperDimType(n1, n2);

        // Find user definition
        SymbolProcedure* match = nullptr;
        if (overloadedOpGood(eType1) && overloadedOpGood(eType2)) {
            std::vector<TypeBasic*> argumentDataTypes;
            argumentDataTypes.push_back(eType1);
            argumentDataTypes.push_back(eType2);
            TypeProc* argTypes = TypeProc::get(getContext(), argumentDataTypes);
            TCGUARD(findBestMatchingOpDef(match, root->operatorName(), argTypes, root));
        }

        if (match != nullptr) { // Overloaded operator
            // Add implicit classify nodes if needed:
            const std::vector<TypeBasic*>& params =
                match->decl()->procedureType()->paramTypes();

            e1->setContextDataType(params[0u]->secrecDataType());
            e2->setContextDataType(params[1u]->secrecDataType());

            e1 = classifyIfNeeded(e1, params[0u]->secrecSecType());
            eType1 = static_cast<TypeBasic *>(e1->resultType());
            e2 = classifyIfNeeded(e2, params[1u]->secrecSecType());
            eType2 = static_cast<TypeBasic *>(e2->resultType());

            Type* rtv = match->decl()->procedureType()->returnType();
            assert(dynamic_cast<TypeBasic*>(rtv) != nullptr);
            TypeBasic* rt = static_cast<TypeBasic*>(rtv);

            root->setResultType(
                TypeBasic::get(m_context, rt->secrecSecType(), rt->secrecDataType(), n0));
            root->setProcSymbol(match);

            return OK;
        } else { // Not overloaded
            SecurityType * s1 = eType1->secrecSecType();
            SecurityType * s2 = eType2->secrecSecType();
            SecurityType * s0 = upperSecType(s1, s2);

            // Add implicit classify nodes if needed:
            e1 = classifyIfNeeded(e1, s0);
            eType1 = static_cast<TypeBasic *>(e1->resultType());
            e2 = classifyIfNeeded(e2, s0);
            eType2 = static_cast<TypeBasic *>(e2->resultType());

            DataType* d1 = eType1->secrecDataType();
            DataType* d2 = eType2->secrecDataType();
            SecrecDataType d0 = getResultDType(root->type(), d1, d2);

            if (d0 != DATATYPE_UNDEFINED) {
                switch (root->type()) {
                    case NODE_EXPR_BINARY_EQ:
                    case NODE_EXPR_BINARY_GE:
                    case NODE_EXPR_BINARY_GT:
                    case NODE_EXPR_BINARY_LE:
                    case NODE_EXPR_BINARY_LT:
                    case NODE_EXPR_BINARY_NE:
                        e1->instantiateDataType(getContext());
                        e2->instantiateDataType(getContext());
                    default:
                        break;
                }

                root->setResultType(TypeBasic::get (m_context, s0, d0, n0));
                return OK;
            }
        }
    }

    m_log.fatalInProc(root) << "Invalid binary operation " << root->operatorString()
        << " between operands of type " << *eType1Orig << " and " << *eType2Orig
        << " at " << root->location() << '.';
    return E_TYPE;
}

void TreeNodeExprBinary::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType(cxt, dType);
    leftExpression()->instantiateDataType(cxt, dType);
    rightExpression()->instantiateDataType(cxt, dType);
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
    SecreC::Type * eType = e->resultType();

    if (eType->kind () == Type::BASIC) {
        TypeBasic * et = static_cast<TypeBasic *>(eType);

        {   // check if operator is overloaded
            SymbolProcedure * match = nullptr;
            std::vector<TypeBasic *> argumentDataTypes;
            argumentDataTypes.push_back (et);
            TypeProc* argTypes = TypeProc::get(getContext(), argumentDataTypes);
            TCGUARD(findBestMatchingOpDef(match, root->operatorName(),
                                          argTypes, root));
            if (match != nullptr) { // overloaded operator
                root->setProcSymbol(match);

                Type* rtv = match->decl()->procedureType()->returnType();
                assert(dynamic_cast<TypeBasic*>(rtv) != nullptr);
                TypeBasic* rt = static_cast<TypeBasic*>(rtv);

                root->setResultType(
                    TypeBasic::get(m_context, rt->secrecSecType(),
                                   rt->secrecDataType(),
                                   et->secrecDimType()));

                return OK;
            }
        }

        if (root->type() == NODE_EXPR_UINV) {
            if (isNumericDataType(et->secrecDataType ())
                    || isXorDataType(et->secrecDataType ())
                    || et->secrecDataType ()->equals (DATATYPE_NUMERIC)
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
                    || et->secrecDataType()->equals (DATATYPE_NUMERIC)) {
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

void TreeNodeExprUnary::instantiateDataTypeV (Context &cxt, SecrecDataType dType) {
    resetDataType (cxt, dType);
    expression ()->instantiateDataType (cxt, dType);
}

/*******************************************************************************
  TreeNodeExprBool
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprBool(TreeNodeExprBool * e) {
    if (! e->haveResultType()) {
        e->setResultType(TypeBasic::get(m_context, DATATYPE_BOOL));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprArrayConstructor
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprArrayConstructor(TreeNodeExprArrayConstructor * e) {
    if (e->haveResultType ()) {
        return OK;
    }

    TypeBasic* elemType = nullptr;
    for (TreeNodeExpr& child : e->expressions ()) {
        child.setContextSecType (e->contextSecType ());
        child.setContextDataType (e->contextDataType ());
        child.setContextDimType (0);
        TCGUARD (visitExpr (&child));

        if (checkAndLogIfVoid (&child))
            return E_TYPE;

        TypeBasic* childType = static_cast<TypeBasic*>(child.resultType ());

        if (! childType->isScalar ()) {
            m_log.fatalInProc (e) << "Expecting scalar elements in array constructor at "
                                  << child.location () << ".";
            return E_TYPE;
        }

        assert (childType != nullptr);
        if (elemType == nullptr) {
            elemType = childType;
        }
        else {
            elemType = upperTypeBasic (getContext (), childType, elemType);
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


    e->setResultType (TypeBasic::get (getContext (),
        elemType->secrecSecType (), elemType->secrecDataType (), 1));
    return OK;
}

void TreeNodeExprArrayConstructor::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType (cxt, dType);
    for (TreeNodeExpr& child : expressions ()) {
        child.instantiateDataType (cxt, dType);
    }
}

/*******************************************************************************
  TreeNodeExprInt
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprInt(TreeNodeExprInt * e) {
    if (! e->haveResultType()) {
        DataType* dtype = DataTypeBuiltinPrimitive::get(getContext(), DATATYPE_NUMERIC); /* default */
        if (e->haveContextDataType()) {
            dtype = dtypeDeclassify(getContext (), e->contextSecType(), e->contextDataType());
            if ((dtype == nullptr) || ! isNumericDataType(dtype)) {
                m_log.fatalInProc(e) << "Expecting numeric context at "
                                     << e->location() << '.';
                return E_TYPE;
            }
        }

        e->setResultType(TypeBasic::get(getContext(), dtype));
    }

    return OK;
}

void TreeNodeExprInt::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType(cxt, dType);
}

/*******************************************************************************
  TreeNodeExprFloat
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprFloat(TreeNodeExprFloat * e) {
    if (!e->haveResultType()) {
        DataType* dType = DataTypeBuiltinPrimitive::get(getContext(), DATATYPE_NUMERIC); /* default */
        if (e->haveContextDataType()) {
            dType = dtypeDeclassify(getContext(), e->contextSecType(), e->contextDataType());
            if ((dType == nullptr) || ! (isFloatingDataType (dType) ||
                                         dType->equals (DATATYPE_NUMERIC)))
            {
                m_log.fatalInProc(e) << "Expecting floating point, got "
                    << *dType << " at " << e->location() << '.';
                return E_TYPE;
            }
        }

        e->setResultType(TypeBasic::get(getContext(), dType));
    }

    return OK;
}

void TreeNodeExprFloat::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    assert (dType == DATATYPE_FLOAT32 || dType == DATATYPE_FLOAT64);
    resetDataType(cxt, dType);
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

    Type * childType = e->resultType();
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    if (childType->secrecSecType()->isPublic()) {
        m_log.fatalInProc(root) << "Argument of type " << *childType
            << " passed to declassify operator at "
            << root->location() << '.';
        return E_TYPE;
    }

    if (childType->secrecDataType ()->isUserPrimitive ()) {
        auto dt = static_cast<DataTypeUserPrimitive*> (childType->secrecDataType ());
        SymbolKind* kind = static_cast<PrivateSecType*> (childType->secrecSecType ())->securityKind ();
        assert (dt->inKind (kind));
        auto publicType = dt->publicType (kind);

        if (! publicType) {
            m_log.fatalInProc (root)
                << "Type '" << dt->name () << "' does not have a public representation at "
                << root->location () << '.';
            return E_TYPE;
        }
    }

    root->setResultType(TypeBasic::get(getContext(),
                                       dtypeDeclassify(getContext(),
                                                       childType->secrecSecType(),
                                                       childType->secrecDataType()),
                                       childType->secrecDimType()));

    return OK;
}

void TreeNodeExprDeclassify::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType(cxt, dType);
    expression()->instantiateDataType(cxt, dType);
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

        s = ConstantInt::get (getContext (), DATATYPE_UINT64, symDim->dimType ());
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

    e->setResultType(TypeBasic::get(m_context, DATATYPE_STRING));
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
        SecurityType* secTy = symDom->securityType ();
        if (secTy->isPublic ()) {
            p->setValue (ConstantString::get (getContext (), "public"));
        }
        else {
            p->setValue (ConstantString::get (getContext (),
                static_cast<PrivateSecType*>(secTy)->name ()));
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
        TypeNonVoid* ty = symSym->secrecType ();
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
    e1->setContextSecType(PublicSecType::get(getContext()));
    e1->setContextDataType(DataTypeBuiltinPrimitive::get (getContext (), DATATYPE_BOOL));
    TCGUARD (visitExpr(e1));
    if (checkAndLogIfVoid(e1))
        return E_TYPE;

    TreeNodeExpr * e2 = root->trueBranch();
    e2->setContext(root);
    TCGUARD (visitExpr(e2));

    TreeNodeExpr * e3 = root->falseBranch();
    e3->setContext(root);
    TCGUARD (visitExpr(e3));

    SecreC::Type * eType1 = e1->resultType();
    SecreC::Type * eType2 = e2->resultType();
    SecreC::Type * eType3 = e3->resultType();

    assert(dynamic_cast<TypeNonVoid *>(eType1) != nullptr);
    TypeNonVoid * cType = static_cast<TypeNonVoid *>(eType1);

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
        e2->instantiateDataType(getContext(), e3->resultType()->secrecDataType());
        e3->instantiateDataType(getContext(), e2->resultType()->secrecDataType());
        eType2 = e2->resultType();
        eType3 = e3->resultType();

        SecurityType * s0 = upperSecType(e2->resultType()->secrecSecType(),
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

void TreeNodeExprTernary::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType(cxt, dType);
    trueBranch()->instantiateDataType(cxt, dType);
    falseBranch()->instantiateDataType(cxt, dType);
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
    e->setResultType(TypeBasic::get(getContext(), DATATYPE_UINT64));
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

void TreeNodeExprQualified::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType(cxt, dType);
    expression()->instantiateDataType(cxt, dType);
}

/*******************************************************************************
  TreeNodeExprStringFromBytes
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprStringFromBytes(TreeNodeExprStringFromBytes * e) {
    if (e->haveResultType())
        return OK;

    TreeNodeExpr * subExpr = e->expression();
    subExpr->setContextSecType(PublicSecType::get(getContext()));
    subExpr->setContextDataType(DataTypeBuiltinPrimitive::get (getContext (), DATATYPE_UINT8));
    subExpr->setContextDimType(1);
    TCGUARD (visitExpr(subExpr));

    Type * ty = subExpr->resultType();
    if (ty->secrecSecType()->isPrivate() ||
            ! ty->secrecDataType()->equals (DATATYPE_UINT8) ||
            ty->secrecDimType() != 1)
    {
        m_log.fatalInProc(e) << "Invalid argument. Expected public byte array, got "
                             << PrettyPrint(ty) << " at " << e->location() << '.';
        return E_TYPE;
    }

    TypeBasic * resultType = TypeBasic::get(getContext(),
            PublicSecType::get(getContext()), DATATYPE_STRING);
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
    subExpr->setContext(TypeBasic::get(getContext(), DATATYPE_STRING));
    TCGUARD (visitExpr(subExpr));

    Type * ty = subExpr->resultType();
    if (ty->secrecSecType()->isPrivate() ||
            ! ty->secrecDataType()->isString () ||
            ty->secrecDimType() != 0)
    {
        m_log.fatalInProc(e) << "Invalid argument. Expected public string, got "
                             << PrettyPrint(ty) << " at " << e->location() << '.';
        return E_TYPE;
    }

    TypeBasic * resultType = TypeBasic::get(getContext(),
            PublicSecType::get(getContext()), DATATYPE_UINT8, 1);
    e->setResultType(resultType);
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

void TreeNodeExprNone::instantiateDataTypeV (Context&, SecrecDataType) {}

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
    auto stringType = TypeBasic::get(getContext(), DATATYPE_STRING);
    subExpr->setContext(stringType);
    TCGUARD (visitExpr(subExpr));
    if (subExpr->resultType () != stringType) {
        m_log.fatalInProc (e) << "Expecting string as argument to 'strlen' got "
                              << *subExpr->resultType () << " at " << subExpr->location () << ".";
        return E_TYPE;
    }

    e->setResultType (TypeBasic::getIndexType (getContext ()));
    return OK;
}

} // namespace SecreC
