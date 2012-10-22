/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include <boost/range.hpp>
#include <boost/foreach.hpp>

#include "imop.h"
#include "log.h"
#include "symbol.h"
#include "treenode.h"
#include "typechecker.h"

namespace SecreC {

namespace /* anonymous */ {

SecrecDataType getResultDType(SecrecTreeNodeType type, SecrecDataType d1, SecrecDataType d2) {
    switch (type) {
    case NODE_EXPR_BINARY_ADD:
        if (d1 != d2) break;
        if (! isNumericDataType(d1) && d1 != DATATYPE_STRING)
            break;
        return d1;
    case NODE_EXPR_BINARY_SUB:
    case NODE_EXPR_BINARY_MUL:
    case NODE_EXPR_BINARY_MOD:
    case NODE_EXPR_BINARY_DIV:
        if (d1 != d2) break;
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
    assert((root->children().at(0)->type() == NODE_LVALUE) != 0x0);
    TreeNode * lval = root->children().at(0);
    assert(lval->children().size() <= 2);
    assert(dynamic_cast<TreeNodeIdentifier * >(lval->children().at(0)) != 0);

    TreeNodeIdentifier * e =
        static_cast<TreeNodeIdentifier *>(lval->children().at(0));
    SecreC::Type * eType = getSymbol(e)->secrecType();
    SecrecDimType destDim = eType->secrecDimType();
    if (lval->children().size() == 2) {
        const Status s = checkIndices(lval->children().at(1), destDim);
        if (s != OK)
            return s;
    }

    const char * m1 = isPrefix ? "Prefix " : "Postfix ";
    const char * m2 = isInc ? "increment" : "decrement";

    // check that argument is a variable
    if (e->type() == NODE_EXPR_RVARIABLE) {
        m_log.fatalInProc(root) << m1 << m2
            << " expects variable at " << root->location() << '.';
        return E_TYPE;
    }
    // increment or decrement of void
    if (eType->isVoid()) {
        m_log.fatalInProc(root) << m1 << m2 << " of void type expression at "
            << root->location() << '.';
        return E_TYPE;
    }

    // check that we are operating on numeric types
    if (!isNumericDataType(eType->secrecDataType())) {
        m_log.fatalInProc(root) << m1 << m2
            << " operator expects numeric type, given "
            << *eType << " at " << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(TypeNonVoid::get(m_context,
                eType->secrecSecType(),
                eType->secrecDataType(),
                eType->secrecDimType()));
    return OK;
}

/*******************************************************************************
  TreeNodeExprAssign
*******************************************************************************/

TypeChecker::Status TreeNodeExprAssign::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprAssign * e) {
    typedef DataTypeVar DTV;
    typedef TypeNonVoid TNV;

    if (e->haveResultType())
        return OK;

    // Get symbol for l-value:
    TreeNodeIdentifier * id = e->identifier();
    SymbolSymbol * dest = getSymbol(id);
    if (dest == 0)
        return E_TYPE;

    assert(dest->secrecType()->isVoid() == false);
    assert(dynamic_cast<TNV *>(dest->secrecType()) != 0);
    TypeNonVoid * varType = static_cast<TNV *>(dest->secrecType());
    SecrecDimType destDim = varType->secrecDimType();


    // Check the slice:
    if (e->slice()) {
        if (e->slice()->children().size() != destDim) {
            m_log.fatalInProc(e) << "Incorrect number of indices at "
                                 << e->location()
                                 << ".";
            return E_TYPE;
        }

        const Status s = checkIndices(e->slice(), destDim);
        if (s != OK)
            return s;
    }

    // Calculate type of r-value:
    TreeNodeExpr *& src = e->rightHandSidePtrRef();
    TypeNonVoid * lhsType = TypeNonVoid::get(getContext(),
            varType->secrecSecType(), varType->secrecDataType(), destDim);
    src->setContext(lhsType);

    const Status s = visitExpr(src);
    if (s != OK)
        return s;

    if (checkAndLogIfVoid(src))
        return E_TYPE;

    TNV * srcType = static_cast<TNV *>(src->resultType());
    if (! srcType->latticeLEQ(lhsType)) {
        m_log.fatalInProc(e) << "Invalid assignment from value of type " << *srcType
            << " to variable of type " << *lhsType << " at "
            << e->location() << '.';
        return E_TYPE;
    }

    // Add implicit classify node if needed:
    classifyIfNeeded(src, varType->secrecSecType());
    varType = static_cast<TypeNonVoid *>(src->resultType());
    e->setResultType(lhsType);
    return OK;
}

/*******************************************************************************
  TreeNodeExprCast
*******************************************************************************/

TypeChecker::Status TreeNodeExprCast::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprCast * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * subExpr = root->expression();
    SecrecDataType resultingDType = root->dataType()->dataType();
    subExpr->setContextSecType(root->contextSecType());
    subExpr->setContextDimType(root->contextDimType());
    const Status status = visitExpr(subExpr);
    if (status != OK)
        return status;

    subExpr->instantiateDataType(getContext());
    SecreC::Type * ty = subExpr->resultType();
    SecrecDataType givenDType = ty->secrecDataType();
    if (! latticeExplicitLEQ(givenDType, resultingDType)) {
        m_log.fatalInProc(root) << "Unable to perform cast at "
            << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(
            TypeNonVoid::get(getContext(),
                ty->secrecSecType(),
                resultingDType,
                ty->secrecDimType()));

    return OK;
}

/*******************************************************************************
  TreeNodeExprIndex
*******************************************************************************/

TypeChecker::Status TreeNodeExprIndex::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprIndex * root) {
    typedef TypeNonVoid TNV;

    if (root->haveResultType())
        return TypeChecker::OK;

    TreeNodeExpr * e = root->expression();
    e->setContextSecType(root->contextSecType());
    e->setContextDataType(root->contextDataType());
    e->setContextDimType(root->indices()->children().size());
    Status s = visitExpr(e);
    if (s != OK)
        return s;
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    TNV * eType = static_cast<TNV *>(e->resultType());
    SecrecDimType k = 0;
    SecrecDimType n = eType->secrecDimType();

    if (root->indices()->children().size() != static_cast<size_t>(n)) {
        m_log.fatalInProc(root) << "Incorrent number of indices at"
            << e->location() << '.';
        return E_TYPE;
    }

    {
        s = checkIndices(root->indices(), k);
        if (s != OK)
            return s;
    }

    root->setResultType(TypeNonVoid::get(getContext(),
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

TypeChecker::Status TreeNodeExprSize::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprSize * root) {
    typedef TypeNonVoid TNV;

    if (! root->haveResultType()) {
        TreeNodeExpr * e = root->expression();
        const Status s = visitExpr(e);
        if (s != OK)
            return s;
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        e->instantiateDataType(getContext());
        root->setResultType(TypeNonVoid::getIndexType(getContext()));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprShape
*******************************************************************************/

TypeChecker::Status TreeNodeExprShape::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprShape * root) {
    typedef TypeNonVoid TNV;

    if (! root->haveResultType()) {
        TreeNodeExpr * e = root->expression();
        const Status s = visitExpr(e);
        if (s != OK)
            return s;
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        e->instantiateDataType(getContext());
        root->setResultType(TypeNonVoid::get(getContext(), DATATYPE_UINT64, 1));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprCat
*******************************************************************************/

TypeChecker::Status TreeNodeExprCat::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprCat * root) {
    typedef TypeNonVoid TNV;

    if (root->haveResultType())
        return OK;

    // missing argument is interpreted as 0
    if (root->dimensionality() == 0) {
        TreeNode * e = new TreeNodeExprInt(0, root->location());
        root->appendChild(e);
    }

    TNV * eTypes[2];

    // check that first subexpressions 2 are arrays and of equal dimensionalities
    for (int i = 0; i < 2; ++ i) {
        assert(dynamic_cast<TreeNodeExpr *>(root->children().at(i)) != 0);
        TreeNodeExpr * e = static_cast<TreeNodeExpr *>(root->children().at(i));
        e->setContext(root->typeContext());
        const Status s = visitExpr(e);
        if (s != OK)
            return s;
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        eTypes[i] = static_cast<TNV *>(e->resultType());
        if (eTypes[i]->isScalar()) {
            m_log.fatalInProc(root) << "Concatenation of scalar values at "
                << e->location() << '.';
            return E_TYPE;
        }
    }

    SecrecDataType d0 = upperDataType(eTypes[0]->secrecDataType(),eTypes[1]->secrecDataType());
    root->leftExpression()->instantiateDataType(getContext(), d0);
    root->rightExpression()->instantiateDataType(getContext(), d0);

    {
        SecurityType * lSecTy = eTypes[0]->secrecSecType();
        SecurityType * rSecTy = eTypes[1]->secrecSecType();
        TreeNodeExpr *& left = root->leftExpressionPtrRef();
        TreeNodeExpr *& right = root->rightExpressionPtrRef();
        classifyIfNeeded(left, rSecTy);
        classifyIfNeeded(right, lSecTy);
        eTypes[0] = static_cast<TNV *>(left->resultType());
        eTypes[1] = static_cast<TNV *>(right->resultType());
    }

    SecurityType * resSecType = upperSecType(eTypes[0]->secrecSecType(),
            eTypes[1]->secrecSecType());
    if (resSecType == 0) {
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

    // type checker actually allows for aribtrary expression here
    // but right now parser expects integer literals, this is OK
    TreeNodeExpr * e3 = root->dimensionality();
    e3->setContextIndexType(getContext());
    const Status s = visitExpr(e3);
    if (s != OK)
        return s;
    if (checkAndLogIfVoid(e3))
        return E_TYPE;

    TNV * e3Type = static_cast<TNV *>(e3->resultType());
    if (! e3Type->isPublicIntScalar()) {
        m_log.fatalInProc(root) << "Expected public scalar integer at "
            << root->dimensionality()->location()
            << " got " << *e3Type << '.';
        return E_TYPE;
    }

    root->setResultType(TypeNonVoid::get(m_context,
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

TypeChecker::Status TreeNodeExprReshape::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprReshape * root) {
    typedef TypeNonVoid TNV;

    if (root->haveResultType())
        return OK;

    SecrecDimType resDim = boost::size(root->dimensions());
    TreeNodeExpr * e = root->reshapee();
    e->setContextSecType(root->contextSecType());
    e->setContextDataType(root->contextDataType());
    e->setContextDimType(resDim);
    Status s = visitExpr(e);
    if (s != OK)
        return s;
    if (checkAndLogIfVoid(e))
        return E_TYPE;

    TNV * eType = static_cast<TNV *>(e->resultType());
    BOOST_FOREACH (TreeNode * _dim, root->dimensions()) {
        assert(dynamic_cast<TreeNodeExpr *>(_dim));
        TreeNodeExpr * dim = static_cast<TreeNodeExpr *>(_dim);
        dim->setContextIndexType(getContext());
        s = visitExpr(dim);
        if (s != OK)
            return s;

        if (checkAndLogIfVoid(dim))
            return E_TYPE;
        TNV * dimType = static_cast<TNV *>(dim->resultType());
        if (! dimType->isPublicIntScalar()) {
            m_log.fatalInProc(root) << "Expected public integer scalar at "
                << dim->location()
                << " got " << dimType->toString() << '.';
            return E_TYPE;
        }
    }

    if (resDim == 0) {
        m_log.fatalInProc(root) << "Conversion from non-scalar to scalar at "
            << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(TypeNonVoid::get(getContext(),
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

TypeChecker::Status TreeNodeExprToString::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprToString * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e = root->expression();
    e->setContextSecType(PublicSecType::get(getContext()));
    e->setContextDimType(0);
    const Status s = visitExpr(e);
    if (s != OK)
        return s;
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    e->instantiateDataType(getContext());
    TypeNonVoid * tnv = static_cast<TypeNonVoid *>(e->resultType());
    if (tnv->secrecDimType() != 0
            || tnv->secrecSecType()->isPrivate()
            || tnv->secrecDataType() == DATATYPE_STRING) {
        m_log.fatalInProc(root) << "Invalid argument passed to \"tostring\" expression at "
                                << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(TypeNonVoid::get(getContext(), DATATYPE_STRING));

    return OK;
}

/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/

TypeChecker::Status TreeNodeExprBinary::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprBinary * root) {
    typedef TypeNonVoid TNV;

    if (root->haveResultType())
        return OK;

    TreeNodeExpr *& e1 = root->leftExpressionPtrRef();
    TreeNodeExpr *& e2 = root->rightExpressionPtrRef();
    TypeNonVoid * eType1 = 0, *eType2 = 0;

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
        e1->setContext(root);
        e2->setContext(root);
        break;
    default:
        e1->setContextSecType(root->contextSecType());
        e2->setContextSecType(root->contextSecType());
        break;
    }

    {
        Status s = visitExpr(e1);
        if (s != OK)
            return s;
        if (checkAndLogIfVoid(e1))
            return E_TYPE;

        s = visitExpr(e2);
        if (s != OK)
            return s;
        if (checkAndLogIfVoid(e2))
            return E_TYPE;
    }

    {
        const SecrecDataType lDType = e1->resultType()->secrecDataType();
        const SecrecDataType rDType = e2->resultType()->secrecDataType();
        const SecrecDataType uDType = upperDataType(lDType, rDType);
        e1->instantiateDataType(getContext(), uDType);
        e2->instantiateDataType(getContext(), uDType);

        assert(dynamic_cast<TNV *>(e1->resultType()) != 0);
        eType1 = static_cast<TNV *>(e1->resultType());

        assert(dynamic_cast<TNV *>(e2->resultType()) != 0);
        eType2 = static_cast<TNV *>(e2->resultType());
    }

    if (static_cast<const TNV *>(eType1)->kind() == TNV::BASIC
            && static_cast<const TNV *>(eType2)->kind() == TNV::BASIC)
    {
        {   // check if operator is overloaded
            SymbolProcedure * match = 0;
            std::vector<DataType *> argumentDataTypes;
            DataTypeProcedureVoid * argTypes = 0;
            argumentDataTypes.push_back(eType1->dataType());
            argumentDataTypes.push_back(eType2->dataType());
            argTypes = DataTypeProcedureVoid::get(getContext(),
                    argumentDataTypes);
            Status s = findBestMatchingProc(match, root->operatorName(),
                                            *root, argTypes, root);
            if (s != OK)
                return s;

            if (match != 0) { // overloaded operator
                SecreC::Type * resultType = 0;
                s = checkProcCall(match, argTypes, resultType);
                if (s != OK)
                    return s;

                assert(resultType != 0);
                root->setResultType(resultType);
                root->setProcSymbol(match);
                return OK;
            }
        }


        SecurityType * s1 = eType1->secrecSecType();
        SecurityType * s2 = eType2->secrecSecType();
        SecurityType * s0 = upperSecType(s1, s2);

        // Add implicit classify nodes if needed:
        classifyIfNeeded(e1, s0);
        eType1 = static_cast<TNV *>(e1->resultType());
        classifyIfNeeded(e2, s0);
        eType2 = static_cast<TNV *>(e2->resultType());

        SecrecDataType d1 = eType1->secrecDataType();
        SecrecDataType d2 = eType2->secrecDataType();
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

                root->setResultType(TNV::get(m_context, s0, d0, n0));
                return OK;
            }
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

TypeChecker::Status TreeNodeExprUnary::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprUnary * root) {
    typedef DataTypeBasic DTB;

    if (root->haveResultType())
        return OK;

    assert(root->type() == NODE_EXPR_UMINUS ||
            root->type() == NODE_EXPR_UNEG ||
            root->type() == NODE_EXPR_UINV);
    TreeNodeExpr * e = root->expression();
    e->setContext(root);
    Status s = visitExpr(e);
    if (s != OK)
        return s;
    SecreC::Type * eType = e->resultType();

    if (!eType->isVoid()
            && (assert(dynamic_cast<TypeNonVoid *>(eType) != 0), true)
            && static_cast<TypeNonVoid *>(eType)->kind() == TypeNonVoid::BASIC)
    {
        TypeNonVoid * et = static_cast<TypeNonVoid *>(eType);
        assert(dynamic_cast<DTB *>(et->dataType()) != 0);
        DTB * bType = static_cast<DTB *>(et->dataType());

        {   // check if operator is overloaded
            SymbolProcedure * match = 0;
            std::vector<DataType *> argumentDataTypes;
            DataTypeProcedureVoid * argTypes = 0;
            argumentDataTypes.push_back(bType);
            argTypes = DataTypeProcedureVoid::get(getContext(),
                    argumentDataTypes);
            s = findBestMatchingProc(match, root->operatorName(),
                                     *root, argTypes, root);
            if (s != OK)
                return s;

            if (match != 0) { // overloaded operator
                SecreC::Type * resultType = 0;
                s = checkProcCall(match, argTypes, resultType);
                if (s != OK)
                    return s;

                assert(resultType != 0);
                root->setResultType(resultType);
                root->setProcSymbol(match);
                return OK;
            }
        }

        if (root->type() == NODE_EXPR_UINV) {
            if (isNumericDataType(bType->dataType())
                    || isXorDataType(bType->dataType())
                    || bType->dataType() == DATATYPE_NUMERIC
                    || bType->dataType() == DATATYPE_BOOL)
            {
                root->setResultType(et);
                return OK;
            }
        } else if (root->type() == NODE_EXPR_UNEG) {
            if (bType->dataType() == DATATYPE_BOOL) {
                root->setResultType(et);
                return OK;
            }
        }
        else if (root->type() == NODE_EXPR_UMINUS) {
            if (isNumericDataType(bType->dataType()) || bType->dataType() == DATATYPE_NUMERIC) {
                root->setResultType(et);
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

void  TreeNodeExprUnary::instantiateDataTypeV (Context &cxt, SecrecDataType dType) {
    resetDataType (cxt, dType);
    expression ()->instantiateDataType (cxt, dType);
}

/*******************************************************************************
  TreeNodeExprBool
*******************************************************************************/

TypeChecker::Status TreeNodeExprBool::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprBool * e) {
    if (! e->haveResultType()) {
        e->setResultType(TypeNonVoid::get(m_context, DATATYPE_BOOL));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprInt
*******************************************************************************/

TypeChecker::Status TreeNodeExprInt::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprInt * e) {
    if (! e->haveResultType()) {
        SecrecDataType dtype = DATATYPE_NUMERIC; /* default */;
        if (e->haveContextDataType()) {
            dtype = dtypeDeclassify(e->contextDataType());
            if (! isNumericDataType(dtype)) {
                m_log.fatalInProc(e) << "Expected numeric context at " << e->location() << '.';
                return E_TYPE;
            }
        }

        e->setResultType(TypeNonVoid::get(getContext(), dtype));
    }

    return OK;
}

void TreeNodeExprInt::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType(cxt, dType);
}

/*******************************************************************************
  TreeNodeExprClassify
*******************************************************************************/

TypeChecker::Status TreeNodeExprClassify::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprClassify * root) {
    if (! root->haveResultType()) {
        m_log.fatalInProc(root) << "ICE: type checking classify node at " << root->location() << '.';
        return E_OTHER;
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprDeclassify
*******************************************************************************/

TypeChecker::Status TreeNodeExprDeclassify::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprDeclassify * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e = root->expression();
    e->setContextDimType(root->contextDimType());
    const Status s = visitExpr(e);
    if (s != OK)
        return s;

    Type * childType = e->resultType();
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    if (childType->secrecSecType()->isPublic()) {
        m_log.fatalInProc(root) << "Argument of type " << *childType
            << " passed to declassify operator at "
            << root->location() << '.';
        return E_TYPE;
    }

    root->setResultType(TypeNonVoid::get(getContext(),
                dtypeDeclassify(childType->secrecDataType()),
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

TypeChecker::Status TreeNodeExprRVariable::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprRVariable * e) {
    if (e->haveResultType())
        return OK;

    TreeNodeIdentifier * id = e->identifier();
    SymbolSymbol * s = getSymbol(id);
    if (s == 0)
        return E_TYPE;

    assert(!s->secrecType()->isVoid());
    assert(dynamic_cast<TypeNonVoid *>(s->secrecType()) != 0);
    TypeNonVoid * type = static_cast<TypeNonVoid *>(s->secrecType());
    assert(type->dataType()->kind() == DataType::VAR);
    assert(dynamic_cast<DataTypeVar *>(type->dataType()) != 0);
    e->setResultType(TypeNonVoid::get(m_context,
                static_cast<DataTypeVar *>(type->dataType())->dataType()));
    return OK;
}

/*******************************************************************************
  TreeNodeExprString
*******************************************************************************/

TypeChecker::Status TreeNodeExprString::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprString * e) {
    if (!e->haveResultType()) {
        e->setResultType(TypeNonVoid::get(m_context, DATATYPE_STRING));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprFloat
*******************************************************************************/

TypeChecker::Status TreeNodeExprFloat::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprFloat * e) {
    if (!e->haveResultType()) {
        if (e->haveContextDataType()) {
            SecrecDataType dType = e->contextDataType();
            switch (dType) {
            case DATATYPE_NUMERIC:
                break;
            case DATATYPE_FLOAT32:
            case DATATYPE_FLOAT64:
                e->setResultType(TypeNonVoid::get(m_context, dType));
                return OK;
            default:
                m_log.fatalInProc(e) << "Expecting floating point, got "
                    << dType << " at " << e->location() << '.';
                return E_TYPE;
            }
        }

        e->setResultType(TypeNonVoid::get(m_context, DATATYPE_FLOAT32));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprTernary
*******************************************************************************/

TypeChecker::Status TreeNodeExprTernary::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprTernary * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e1 = root->conditional();
    e1->setContextSecType(PublicSecType::get(getContext()));
    e1->setContextDataType(DATATYPE_BOOL);
    Status s = visitExpr(e1);
    if (s != OK)
        return s;
    if (checkAndLogIfVoid(e1))
        return E_TYPE;

    TreeNodeExpr * e2 = root->trueBranch();
    e2->setContext(root);
    s = visitExpr(e2);
    if (s != OK)
        return s;

    TreeNodeExpr * e3 = root->falseBranch();
    e3->setContext(root);
    s = visitExpr(e3);
    if (s != OK)
        return s;

    SecreC::Type * eType1 = e1->resultType();
    SecreC::Type * eType2 = e2->resultType();
    SecreC::Type * eType3 = e3->resultType();

    assert(dynamic_cast<TypeNonVoid *>(eType1) != 0);
    TypeNonVoid * cType = static_cast<TypeNonVoid *>(eType1);

    // check if conditional expression is of public boolean type
    if (cType->secrecDataType() != DATATYPE_BOOL
            || cType->secrecSecType()->isPrivate())
    {
        m_log.fatalInProc(root) << "Conditional subexpression at " << e1->location()
            << " of ternary expression has to be public boolean, got "
            << *cType << '.';
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
        if (s0 == 0) {
            m_log.fatalInProc(root) << "Incompatible security types in ternary expression at "
                << e2->location() << " and " << e3->location() << '.';
            m_log.fatal() << "Unable to match "
                << *e2->resultType()->secrecSecType() << " with "
                << *e3->resultType()->secrecSecType() << '.';
            return E_TYPE;
        }

        classifyIfNeeded(e2, s0);
        classifyIfNeeded(e3, s0);

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

TypeChecker::Status TreeNodeExprDomainID::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprDomainID * e) {
    if (e->haveResultType())
        return OK;

    if (e->securityType()->isPublic()) {
        m_log.fatalInProc(e) << "Public security type does not have a domain ID at "
                             << e->location() << '.';
        return E_TYPE;
    }

    const Status status = visit(e->securityType());
    if (status != OK)
        return status;
    e->setResultType(TypeNonVoid::get(getContext(), DATATYPE_UINT64));
    return OK;
}

/*******************************************************************************
  TreeNodeExprQualified
*******************************************************************************/

TypeChecker::Status TreeNodeExprQualified::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprQualified * e) {
    if (e->haveResultType())
        return OK;

    Status status = OK;
    TreeNodeExpr * subExpr = e->expression();
    subExpr->setContext(e);
    bool checkSecType = false, checkDataType = false, checkDimType = false;
    BOOST_FOREACH (TreeNode * _node, e->types()) {
        switch (_node->type()) {
        case NODE_SECTYPE_F: {
            TreeNodeSecTypeF * secTy = static_cast<TreeNodeSecTypeF *>(_node);
            status = visit(secTy);
            if (status != OK)
                return status;
            subExpr->setContextSecType(secTy->cachedType());
            checkSecType = true;
        }
            break;
        case NODE_DATATYPE_F:
            subExpr->setContextDataType(
                    static_cast<TreeNodeDataTypeF *>(_node)->dataType());
            checkDataType = true;
            break;
        case NODE_DIMTYPE_F:
            subExpr->setContextDimType(
                    static_cast<TreeNodeDimTypeF *>(_node)->dimType());
            checkDimType = true;
            break;
        default:
            assert(false && "ICE: expression qualified over non-type!");
            break;
        }
    }

    status = visitExpr(subExpr);
    if (status != OK)
        return status;

    /* Check that the actual type matches the qualified type: */

    if (checkSecType) {
        if (subExpr->contextSecType() !=
                subExpr->resultType()->secrecSecType()) {
            m_log.fatalInProc(e) << "Security type of the expression at "
                << subExpr->location()
                << " does not match the qualified type.";
            return E_TYPE;
        }
    }

    if (checkDataType) {
        if (subExpr->contextDataType() !=
                subExpr->resultType()->secrecDataType()) {
            m_log.fatalInProc(e) << "Data type of the expression at "
                << subExpr->location()
                << " does not match the qualified type.";

            return E_TYPE;
        }
    }

    if (checkDimType) {
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

TypeChecker::Status TreeNodeExprStringFromBytes::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprStringFromBytes * e) {
    if (e->haveResultType())
        return OK;

    TreeNodeExpr * subExpr = e->expression();
    subExpr->setContextSecType(PublicSecType::get(getContext()));
    subExpr->setContextDataType(DATATYPE_UINT8);
    subExpr->setContextDimType(1);
    const Status status = visitExpr(subExpr);
    if (status != OK)
        return status;

    Type * ty = subExpr->resultType();
    if (ty->secrecSecType()->isPrivate() ||
            ty->secrecDataType() != DATATYPE_UINT8 ||
            ty->secrecDimType() != 1)
    {
        m_log.fatalInProc(e) << "Invalid argument. Expected public byte array, got "
                             << ty->toString() << " at " << e->location() << '.';
        return E_TYPE;
    }

    TypeNonVoid * resultType = TypeNonVoid::get(getContext(),
            PublicSecType::get(getContext()), DATATYPE_STRING);
    e->setResultType(resultType);
    return OK;
}

/*******************************************************************************
  TreeNodeExprBytesFromString
*******************************************************************************/

TypeChecker::Status TreeNodeExprBytesFromString::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprBytesFromString * e) {
    if (e->haveResultType())
        return OK;

    TreeNodeExpr * subExpr = e->expression();
    subExpr->setContext(TypeNonVoid::get(getContext(), DATATYPE_STRING));
    const Status status = visitExpr(subExpr);
    if (status != OK)
        return status;

    Type * ty = subExpr->resultType();
    if (ty->secrecSecType()->isPrivate() ||
            ty->secrecDataType() != DATATYPE_STRING ||
            ty->secrecDimType() != 0)
    {
        m_log.fatalInProc(e) << "Invalid argument. Expected public string, got "
                             << ty->toString() << " at " << e->location() << '.';
        return E_TYPE;
    }

    TypeNonVoid * resultType = TypeNonVoid::get(getContext(),
            PublicSecType::get(getContext()), DATATYPE_UINT8, 1);
    e->setResultType(resultType);
    return OK;
}

/*******************************************************************************
  TreeNodeExprPrefix
*******************************************************************************/

TypeChecker::Status TreeNodeExprPrefix::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprPrefix * root) {
    return checkPostfixPrefixIncDec(root, false,
            root->type() == NODE_EXPR_PREFIX_INC);
}

/*******************************************************************************
  TreeNodeExprPostfix
*******************************************************************************/

TypeChecker::Status TreeNodeExprPostfix::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprPostfix * root) {
    return checkPostfixPrefixIncDec(root, true,
            root->type() == NODE_EXPR_POSTFIX_INC);
}

} // namespace SecreC
