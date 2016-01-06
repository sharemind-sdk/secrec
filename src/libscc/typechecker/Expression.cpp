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
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Types.h"
#include "SecurityType.h"
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

SecrecDataType getResultDType(SecrecTreeNodeType type, const DataType* d1, const DataType* d2) {
    if (d1->isPrimitive () && d2->isPrimitive ()) {
        return getResultDType (type,
            static_cast<const DataTypePrimitive*>(d1)->secrecDataType (),
            static_cast<const DataTypePrimitive*>(d2)->secrecDataType ());
    }

    return DATATYPE_UNDEFINED;
}

const TypeNonVoid* upperTypeNonVoid (Context& cxt, const TypeNonVoid* a, const TypeNonVoid* b) {
    const SecurityType* const secType = upperSecType (a->secrecSecType (), b->secrecSecType ());
    const SecrecDimType dimType = upperDimType (a->secrecDimType (), b->secrecDimType ());
    const DataType* const dataType = upperDataType (cxt, a->secrecDataType (), b->secrecDataType ());
    if (secType == nullptr || dimType == (~ SecrecDimType(0)) || dataType == nullptr)
        return nullptr;

    return TypeBasic::get (cxt, secType, dataType, dimType);
}

} // namespace anonymous

/*******************************************************************************
  TypeChecker
*******************************************************************************/

TypeChecker::Status TypeChecker::checkPostfixPrefixIncDec(TreeNodeExpr * root,
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

    // check that we are operating on numeric types
    if (!isNumericDataType(eType->secrecDataType())) {
        m_log.fatalInProc(root) << m1 << m2
            << " operator expects numeric type, given "
            << *eType << " at " << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(eType);
    return OK;
}

const TypeNonVoid* TypeChecker::checkSelect (const Location& loc,
                                             const Type* ty,
                                             TreeNodeIdentifier* id)
{
    assert (ty != nullptr);

    if (ty->kind () != Type::BASIC || ! ty->secrecDataType ()->isComposite ()) {
        m_log.fatal () << "Expecting structure, got" << *ty << ". "
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
    if (! srcType->latticeLEQ(getContext (), lhsType)) {
        m_log.fatalInProc(e) << "Invalid assignment from value of type " << *srcType
            << " to variable of type " << *lhsType << " at "
            << e->location() << '.';
        return E_TYPE;
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

    TCGUARD (visitDataTypeF (root->dataType ()));

    const DataType* resultingDType = root->dataType()->cachedType ();;
    TreeNodeExpr * subExpr = root->expression();
    subExpr->setContextSecType(root->contextSecType());
    subExpr->setContextDimType(root->contextDimType());
    TCGUARD (visitExpr(subExpr));

    subExpr->instantiateDataType(getContext());
    const SecreC::Type * ty = subExpr->resultType();
    const DataType* givenDType = ty->secrecDataType();
    if (! latticeExplicitLEQ(givenDType, resultingDType)) {
        m_log.fatalInProc(root) << "Unable to perform cast at "
            << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(
            TypeBasic::get(getContext(),
                ty->secrecSecType(),
                resultingDType,
                ty->secrecDimType()));

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

    std::array<const TypeNonVoid*, 2> eTypes = {nullptr, nullptr};

    // check that first subexpressions 2 are arrays and of equal dimensionalities
    for (int i = 0; i < 2; ++ i) {
        assert(dynamic_cast<TreeNodeExpr *>(root->children().at(i)) != nullptr);
        TreeNodeExpr * e = static_cast<TreeNodeExpr *>(root->children().at(i));
        e->setContext(root->typeContext());
        TCGUARD (visitExpr(e));
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        eTypes[i] = static_cast<const TypeNonVoid*>(e->resultType());
        if (eTypes[i]->isScalar()) {
            m_log.fatalInProc(root) << "Concatenation of scalar values at "
                << e->location() << '.';
            return E_TYPE;
        }
    }

    const DataType* d0 = upperDataType(getContext (),
                                       eTypes[0]->secrecDataType(),
                                       eTypes[1]->secrecDataType());
    root->leftExpression()->instantiateDataType(getContext(), d0);
    root->rightExpression()->instantiateDataType(getContext(), d0);

    {
        const SecurityType * lSecTy = eTypes[0]->secrecSecType();
        const SecurityType * rSecTy = eTypes[1]->secrecSecType();
        TreeNodeExpr * left = root->leftExpression();
        TreeNodeExpr * right = root->rightExpression();
        left = classifyIfNeeded(left, rSecTy);
        right = classifyIfNeeded(right, lSecTy);
        eTypes[0] = static_cast<const TypeNonVoid*>(left->resultType());
        eTypes[1] = static_cast<const TypeNonVoid*>(right->resultType());
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

    const auto e3Type = static_cast<const TypeNonVoid *>(e3->resultType());
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

    const auto eType = static_cast<const TypeNonVoid *>(e->resultType());
    for (TreeNodeExpr& dim : root->dimensions()) {
        dim.setContextIndexType (getContext());
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
    const auto tnv = static_cast<const TypeNonVoid *>(e->resultType());
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

TypeChecker::Status TypeChecker::visitExprBinary(TreeNodeExprBinary * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e1 = root->leftExpression ();
    TreeNodeExpr * e2 = root->rightExpression ();
    const TypeBasic * eType1 = nullptr, *eType2 = nullptr;

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

    {
        const DataType* const lDType = e1->resultType()->secrecDataType();
        const DataType* const rDType = e2->resultType()->secrecDataType();
        const DataType* const uDType = upperDataType(getContext (), lDType, rDType);
        e1->instantiateDataType(getContext(), uDType);
        e2->instantiateDataType(getContext(), uDType);

        assert(dynamic_cast<const TypeBasic *>(e1->resultType()) != nullptr);
        eType1 = static_cast<const TypeBasic *>(e1->resultType());

        assert(dynamic_cast<const TypeBasic *>(e2->resultType()) != nullptr);
        eType2 = static_cast<const TypeBasic *>(e2->resultType());
    }

    {   // check if operator is overloaded
        SymbolProcedure * match = nullptr;
        std::vector<const TypeBasic*> argumentDataTypes;
        argumentDataTypes.push_back (eType1);
        argumentDataTypes.push_back (eType2);
        const TypeProc* argTypes = TypeProc::get(getContext(), argumentDataTypes);
        TCGUARD (findBestMatchingProc(match, root->operatorName(),
                                      *root, argTypes, root));

        if (match != nullptr) { // overloaded operator
            const SecreC::Type* resultType = match->decl()->procedureType()->returnType ();
            root->setResultType(resultType);
            root->setProcSymbol(match);
            return OK;
        }
    }


    const SecurityType * s1 = eType1->secrecSecType();
    const SecurityType * s2 = eType2->secrecSecType();
    const SecurityType * s0 = upperSecType(s1, s2);

    // Add implicit classify nodes if needed:
    e1 = classifyIfNeeded(e1, s0);
    eType1 = static_cast<const TypeBasic *>(e1->resultType());
    e2 = classifyIfNeeded(e2, s0);
    eType2 = static_cast<const TypeBasic *>(e2->resultType());

    const DataType* d1 = eType1->secrecDataType();
    const DataType* d2 = eType2->secrecDataType();
    SecrecDimType n1 = eType1->secrecDimType();
    SecrecDimType n2 = eType2->secrecDimType();
    SecrecDataType d0 = getResultDType(root->type(), d1, d2);

    if (n1 == 0 || n2 == 0 || n1 == n2) {
        SecrecDimType n0 = upperDimType(n1,n2);
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

    m_log.fatalInProc(root) << "Invalid binary operation " << root->operatorString()
        << " between operands of type " << *eType1 << " and " << *eType2
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
    const SecreC::Type * eType = e->resultType();

    if (eType->kind () == Type::BASIC) {
        const auto et = static_cast<const TypeBasic *>(eType);

        {   // check if operator is overloaded
            SymbolProcedure * match = nullptr;
            std::vector<const TypeBasic *> argumentDataTypes;
            argumentDataTypes.push_back (et);
            const TypeProc* argTypes = TypeProc::get (getContext(), argumentDataTypes);
            TCGUARD (findBestMatchingProc(match, root->operatorName(),
                                          *root, argTypes, root));
            if (match != nullptr) { // overloaded operator
                const SecreC::Type * resultType = match->decl()->procedureType()->returnType ();
                root->setResultType (resultType);
                root->setProcSymbol (match);
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

    const TypeNonVoid* elemType = nullptr;
    for (TreeNodeExpr& child : e->expressions ()) {
        child.setContextSecType (e->contextSecType ());
        child.setContextDataType (e->contextDataType ());
        child.setContextDimType (0);
        TCGUARD (visitExpr (&child));

        if (checkAndLogIfVoid (&child))
            return E_TYPE;

        const auto childType = static_cast<const TypeNonVoid*>(child.resultType ());

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
            elemType = upperTypeNonVoid (getContext (), childType, elemType);
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
        const DataType* dtype = DataTypePrimitive::get (getContext (), DATATYPE_NUMERIC); /* default */
        if (e->haveContextDataType()) {
            dtype = dtypeDeclassify(getContext (), e->contextDataType());
            if (! isNumericDataType(dtype)) {
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
        const DataType* dType = DataTypePrimitive::get (getContext (), DATATYPE_NUMERIC); /* default */
        if (e->haveContextDataType()) {
            dType = dtypeDeclassify(getContext (), e->contextDataType());
            if (! (isFloatingDataType (dType) || dType->equals (DATATYPE_NUMERIC))) {
                m_log.fatalInProc(e) << "Expecting floating point, got "
                    << dType << " at " << e->location() << '.';
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

    const Type * childType = e->resultType();
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    if (childType->secrecSecType()->isPublic()) {
        m_log.fatalInProc(root) << "Argument of type " << *childType
            << " passed to declassify operator at "
            << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(TypeBasic::get(getContext(),
                dtypeDeclassify(getContext (), childType->secrecDataType()),
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
    e1->setContextSecType(PublicSecType::get(getContext()));
    e1->setContextDataType(DataTypePrimitive::get (getContext (), DATATYPE_BOOL));
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

    assert(dynamic_cast<const TypeNonVoid *>(eType1) != nullptr);
    const auto cType = static_cast<const TypeNonVoid *>(eType1);

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
        if (subExpr->contextDataType() !=
                subExpr->resultType()->secrecDataType()) {
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
    subExpr->setContextDataType(DataTypePrimitive::get (getContext (), DATATYPE_UINT8));
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

    const TypeBasic * resultType = TypeBasic::get(getContext(),
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

    const Type * ty = subExpr->resultType();
    if (ty->secrecSecType()->isPrivate() ||
            ! ty->secrecDataType()->isString () ||
            ty->secrecDimType() != 0)
    {
        m_log.fatalInProc(e) << "Invalid argument. Expected public string, got "
                             << PrettyPrint(ty) << " at " << e->location() << '.';
        return E_TYPE;
    }

    const TypeBasic * resultType = TypeBasic::get(getContext(),
            PublicSecType::get(getContext()), DATATYPE_UINT8, 1);
    e->setResultType(resultType);
    return OK;
}

/*******************************************************************************
  TreeNodeExprPrefix
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprPrefix(TreeNodeExprPrefix * root) {
    return checkPostfixPrefixIncDec(root, false,
            root->type() == NODE_EXPR_PREFIX_INC);
}

/*******************************************************************************
  TreeNodeExprPostfix
*******************************************************************************/

TypeChecker::Status TypeChecker::visitExprPostfix(TreeNodeExprPostfix * root) {
    return checkPostfixPrefixIncDec(root, true,
            root->type() == NODE_EXPR_POSTFIX_INC);
}

/*******************************************************************************
  TreeNodeExprNone
*******************************************************************************/

void TreeNodeExprNone::instantiateDataTypeV (Context&, SecrecDataType) { }

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
