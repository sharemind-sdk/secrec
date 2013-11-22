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
#include "constant.h"
#include "treenode.h"
#include "typechecker.h"
#include "symboltable.h"
#include "types.h"

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

TypeNonVoid* upperTypeNonVoid (Context& cxt, TypeNonVoid* a, TypeNonVoid* b) {
    SecurityType* secType = upperSecType (a->secrecSecType (), b->secrecSecType ());
    SecrecDimType dimType = upperDimType (a->secrecDimType (), b->secrecDimType ());
    SecrecDataType dataType = upperDataType (a->secrecDataType (), b->secrecDataType ());
    if (secType == 0 || dimType == (~ SecrecDimType(0)) || dataType == DATATYPE_UNDEFINED)
        return 0;

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
    assert((root->children().at(0)->type() == NODE_LVALUE) != 0x0);
    TreeNode * lval = root->children().at(0);
    assert(lval->children().size() <= 2);
    assert(dynamic_cast<TreeNodeIdentifier * >(lval->children().at(0)) != 0);

    TreeNodeIdentifier * e =
        static_cast<TreeNodeIdentifier *>(lval->children().at(0));
    SecreC::Type * eType = NULL;
    if (SymbolSymbol* sym = getSymbol (e))
        eType = sym->secrecType ();
    else
        return E_TYPE;

    SecrecDimType destDim = eType->secrecDimType();
    if (lval->children().size() == 2) {
        TCGUARD (checkIndices(lval->children().at(1), destDim));
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

    root->setResultType(TypeBasic::get(m_context,
                eType->secrecSecType(),
                eType->secrecDataType(),
                eType->secrecDimType()));
    return OK;
}

/*******************************************************************************
  TreeNodeExprNone
*******************************************************************************/

TypeChecker::Status TreeNodeExprNone::accept(TypeChecker&) {
    return TypeChecker::OK;
}

void TreeNodeExprNone::instantiateDataTypeV (Context&, SecrecDataType) { }

/*******************************************************************************
  TreeNodeExprAssign
*******************************************************************************/

TypeChecker::Status TreeNodeExprAssign::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprAssign * e) {
    if (e->haveResultType())
        return OK;

    // Get symbol for l-value:
    TreeNodeIdentifier * id = e->identifier();
    SymbolSymbol * dest = getSymbol(id);
    if (dest == 0)
        return E_TYPE;

    assert(dest->secrecType()->isVoid() == false);
    assert(dynamic_cast<TypeNonVoid *>(dest->secrecType()) != 0);
    TypeNonVoid * varType = static_cast<TypeNonVoid *>(dest->secrecType());
    SecrecDimType destDim = varType->secrecDimType();


    // Check the slice:
    if (e->slice()) {
        if (e->slice()->children().size() != destDim) {
            m_log.fatalInProc(e) << "Incorrect number of indices at "
                                 << e->location()
                                 << ".";
            return E_TYPE;
        }

        TCGUARD (checkIndices(e->slice(), destDim));
    }

    // Calculate type of r-value:
    TreeNodeExpr * src = e->rightHandSide ();
    TypeNonVoid * lhsType = TypeBasic::get(getContext(),
            varType->secrecSecType(), varType->secrecDataType(), destDim);
    src->setContext(lhsType);

    TCGUARD (visitExpr(src));

    if (checkAndLogIfVoid(src))
        return E_TYPE;

    TypeNonVoid * srcType = static_cast<TypeNonVoid *>(src->resultType());
    if (! srcType->latticeLEQ(lhsType)) {
        m_log.fatalInProc(e) << "Invalid assignment from value of type " << *srcType
            << " to variable of type " << *lhsType << " at "
            << e->location() << '.';
        return E_TYPE;
    }

    // Add implicit classify node if needed:
    src = classifyIfNeeded(src, varType->secrecSecType());
    e->setResultType(src->resultType ());
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

    TCGUARD (visit (root->dataType ()));

    SecrecDataType resultingDType = root->dataType()->cachedType ();
    TreeNodeExpr * subExpr = root->expression();
    subExpr->setContextSecType(root->contextSecType());
    subExpr->setContextDimType(root->contextDimType());
    TCGUARD (visitExpr(subExpr));

    subExpr->instantiateDataType(getContext());
    SecreC::Type * ty = subExpr->resultType();
    SecrecDataType givenDType = ty->secrecDataType();
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
    TCGUARD (visitExpr(e));
    if (checkAndLogIfVoid(e))
        return E_TYPE;
    TNV * eType = static_cast<TNV *>(e->resultType());
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

TypeChecker::Status TreeNodeExprSize::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprSize * root) {
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

TypeChecker::Status TreeNodeExprShape::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprShape * root) {
    typedef TypeNonVoid TNV;

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
        TCGUARD (visitExpr(e));
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
        TreeNodeExpr * left = root->leftExpression();
        TreeNodeExpr * right = root->rightExpression();
        left = classifyIfNeeded(left, rSecTy);
        right = classifyIfNeeded(right, lSecTy);
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

    TNV * e3Type = static_cast<TNV *>(e3->resultType());
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

TypeChecker::Status TreeNodeExprReshape::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprReshape * root) {
    typedef TypeNonVoid TNV;

    if (root->haveResultType())
        return OK;

    SecrecDimType resDim = root->dimensions().size ();
    TreeNodeExpr * e = root->reshapee();
    e->setContextSecType(root->contextSecType());
    e->setContextDataType(root->contextDataType());
    TCGUARD (visitExpr(e));
    if (checkAndLogIfVoid(e))
        return E_TYPE;

    TNV * eType = static_cast<TNV *>(e->resultType());
    BOOST_FOREACH (TreeNodeExpr& dim, root->dimensions()) {
        dim.setContextIndexType (getContext());
        TCGUARD (visitExpr(&dim));

        if (checkAndLogIfVoid(&dim))
            return E_TYPE;

        TNV * dimType = static_cast<TNV *>(dim.resultType());
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

TypeChecker::Status TreeNodeExprToString::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprToString * root) {
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
            || tnv->secrecDataType() == DATATYPE_STRING) {
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

TypeChecker::Status TreeNodeExprBinary::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprBinary * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e1 = root->leftExpression ();
    TreeNodeExpr * e2 = root->rightExpression ();
    TypeBasic * eType1 = 0, *eType2 = 0;

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
        TCGUARD (visitExpr(e1));
        if (checkAndLogIfVoid(e1))
            return E_TYPE;

        TCGUARD (visitExpr(e2));
        if (checkAndLogIfVoid(e2))
            return E_TYPE;
    }

    {
        const SecrecDataType lDType = e1->resultType()->secrecDataType();
        const SecrecDataType rDType = e2->resultType()->secrecDataType();
        const SecrecDataType uDType = upperDataType(lDType, rDType);
        e1->instantiateDataType(getContext(), uDType);
        e2->instantiateDataType(getContext(), uDType);

        assert(dynamic_cast<TypeBasic *>(e1->resultType()) != 0);
        eType1 = static_cast<TypeBasic *>(e1->resultType());

        assert(dynamic_cast<TypeBasic *>(e2->resultType()) != 0);
        eType2 = static_cast<TypeBasic *>(e2->resultType());
    }

    {   // check if operator is overloaded
        SymbolProcedure * match = 0;
        std::vector<TypeBasic*> argumentDataTypes;
        argumentDataTypes.push_back (eType1);
        argumentDataTypes.push_back (eType2);
        TypeProc* argTypes = TypeProc::get(getContext(), argumentDataTypes);
        TCGUARD (findBestMatchingProc(match, root->operatorName(),
                                      *root, argTypes, root));

        if (match != 0) { // overloaded operator
            SecreC::Type* resultType = match->decl()->procedureType()->returnType ();
            root->setResultType(resultType);
            root->setProcSymbol(match);
            return OK;
        }
    }


    SecurityType * s1 = eType1->secrecSecType();
    SecurityType * s2 = eType2->secrecSecType();
    SecurityType * s0 = upperSecType(s1, s2);

    // Add implicit classify nodes if needed:
    e1 = classifyIfNeeded(e1, s0);
    eType1 = static_cast<TypeBasic *>(e1->resultType());
    e2 = classifyIfNeeded(e2, s0);
    eType2 = static_cast<TypeBasic *>(e2->resultType());

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

TypeChecker::Status TreeNodeExprUnary::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprUnary * root) {
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
            SymbolProcedure * match = 0;
            std::vector<TypeBasic *> argumentDataTypes;
            argumentDataTypes.push_back (et);
            TypeProc* argTypes = TypeProc::get (getContext(), argumentDataTypes);
            TCGUARD (findBestMatchingProc(match, root->operatorName(),
                                          *root, argTypes, root));
            if (match != 0) { // overloaded operator
                SecreC::Type * resultType = match->decl()->procedureType()->returnType ();
                root->setResultType (resultType);
                root->setProcSymbol (match);
                return OK;
            }
        }

        if (root->type() == NODE_EXPR_UINV) {
            if (isNumericDataType(et->secrecDataType ())
                    || isXorDataType(et->secrecDataType ())
                    || et->secrecDataType () == DATATYPE_NUMERIC
                    || et->secrecDataType () == DATATYPE_BOOL)
            {
                root->setResultType (et);
                return OK;
            }
        } else if (root->type() == NODE_EXPR_UNEG) {
            if (et->secrecDataType () == DATATYPE_BOOL) {
                root->setResultType (et);
                return OK;
            }
        }
        else if (root->type() == NODE_EXPR_UMINUS) {
            if (isNumericDataType(et->secrecDataType())
                    || et->secrecDataType() == DATATYPE_NUMERIC) {
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
        e->setResultType(TypeBasic::get(m_context, DATATYPE_BOOL));
    }

    return OK;
}

/*******************************************************************************
  TreeNodeExprArrayConstructor
*******************************************************************************/

TypeChecker::Status TreeNodeExprArrayConstructor::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprArrayConstructor * e) {
    if (e->haveResultType ()) {
        return OK;
    }

    TypeNonVoid* elemType = 0;
    BOOST_FOREACH (TreeNodeExpr& child, e->expressions ()) {
        child.setContextSecType (e->contextSecType ());
        child.setContextDataType (e->contextDataType ());
        child.setContextDimType (0);
        TCGUARD (visitExpr (&child));

        if (checkAndLogIfVoid (&child))
            return E_TYPE;

        TypeNonVoid* childType = static_cast<TypeNonVoid*>(child.resultType ());

        if (! childType->isScalar ()) {
            m_log.fatalInProc (e) << "Expecting scalar elements in array constructor at " << child.location () << ".";
            return E_TYPE;
        }

        assert (childType != 0);
        if (elemType == 0) {
            elemType = childType;
        }
        else {
            elemType = upperTypeNonVoid (getContext (), childType, elemType);
            if (elemType == 0) {
                m_log.fatalInProc (e) << "Array element of invalid type at " << child.location () << ".";
                return E_TYPE;
            }
        }
    }

    BOOST_FOREACH (TreeNodeExpr& child, e->expressions ()) {
        classifyIfNeeded (&child, elemType->secrecSecType ());
    }


    e->setResultType (TypeBasic::get (getContext (), elemType->secrecSecType (), elemType->secrecDataType (), 1));
    return OK;
}

void TreeNodeExprArrayConstructor::instantiateDataTypeV(Context & cxt, SecrecDataType dType) {
    resetDataType (cxt, dType);
    BOOST_FOREACH (TreeNodeExpr& child, expressions ()) {
        child.instantiateDataType (cxt, dType);
    }
}

/*******************************************************************************
  TreeNodeExprInt
*******************************************************************************/

TypeChecker::Status TreeNodeExprInt::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprInt * e) {
    if (! e->haveResultType()) {
        SecrecDataType dtype = DATATYPE_NUMERIC; /* default */
        if (e->haveContextDataType()) {
            dtype = dtypeDeclassify(e->contextDataType());
            if (! isNumericDataType(dtype)) {
                m_log.fatalInProc(e) << "Expecting numeric context at " << e->location() << '.';
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

TypeChecker::Status TreeNodeExprFloat::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprFloat * e) {
    if (!e->haveResultType()) {
        SecrecDataType dType = DATATYPE_NUMERIC; /* default */
        if (e->haveContextDataType()) {
            dType = dtypeDeclassify(e->contextDataType());
            switch (dType) {
            case DATATYPE_NUMERIC:
            case DATATYPE_FLOAT32:
            case DATATYPE_FLOAT64:
                break;
            default:
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

    root->setResultType(TypeBasic::get(getContext(),
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
    Symbol * s = findIdentifier (SYM_SYMBOL, id);

    if (s == 0) {
        // TODO: this is not the prettiest solution
        SymbolDimensionality* symDim = findIdentifier<SYM_DIM>(id);
        if (symDim == 0)
            return E_TYPE;

        s = ConstantInt::get (getContext (), DATATYPE_UINT64, symDim->dimType ());
    }

    TypeNonVoid * type = static_cast<TypeNonVoid *>(s->secrecType());
    e->setResultType(TypeBasic::get(m_context,
        type->secrecSecType (), type->secrecDataType (), type->secrecDimType ()));
    e->setValueSymbol (s);
    return OK;
}

/*******************************************************************************
  TreeNodeExprString
*******************************************************************************/

bool TreeNodeExprString::isConstant () const {
    BOOST_FOREACH (TreeNodeStringPart& part, parts ()) {
        if (! part.isConstant ())
            return false;
    }

    return true;
}

TypeChecker::Status TreeNodeExprString::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprString * e) {
    if (e->haveResultType())
        return OK;

    BOOST_FOREACH (TreeNodeStringPart& part, e->parts ()) {
        TCGUARD (visit (&part));
    }

    e->setResultType(TypeBasic::get(m_context, DATATYPE_STRING));
    return OK;
}

/*******************************************************************************
  TreeNodeStringPart
*******************************************************************************/

TypeChecker::Status TypeChecker::visit (TreeNodeStringPart * e) {
    return e->accept (*this);
}

/*******************************************************************************
  TreeNodeStringPartFragment
*******************************************************************************/


TypeChecker::Status TreeNodeStringPartFragment::accept (TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeStringPartFragment *) {
    return OK;
}

/*******************************************************************************
  TreeNodeStringPartIdentifier
*******************************************************************************/

StringRef TreeNodeStringPartIdentifier::staticValue () const {
    assert (isConstant ());
    return m_value->value ();
}

TypeChecker::Status TreeNodeStringPartIdentifier::accept (TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeStringPartIdentifier * p) {
    if (p->value () || p->secrecType ())
        return OK;

    const StringRef name = p->name ();
    SymbolDomain* symDom = m_st->find<SYM_DOMAIN>(name);
    SymbolDataType* symTy = m_st->find<SYM_TYPE>(name);
    SymbolSymbol* symSym = m_st->find<SYM_SYMBOL>(name);

    if (symDom == 0 && symTy == 0 && symSym == 0) {
        m_log.fatalInProc (p) << "Identifier \'" << name <<
                                 "\' at " << p->location () << " not in scope.";
        return E_TYPE;
    }

    if ((!!symDom + !!symTy + !!symSym) != 1) {
        m_log.fatalInProc (p) << "Ambiguous use of identifier \'" << name <<
                                 "\' at " << p->location () << ".";
        return E_TYPE;
    }

    if (symDom != 0) {
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
    if (symTy != 0) {
        p->setValue (ConstantString::get (getContext (),
            SecrecFundDataTypeToString (symTy->dataType ())));
    }
    else
    if (symSym != 0) {
        TypeNonVoid* ty = symSym->secrecType ();
        if (! canPrintValue (ty)) {
            m_log.fatalInProc(p)
                << "Unable to convert variable \"" << symSym->name () << "\" to string. "
                << "Got " << Type::PrettyPrint (ty) << " at " << p->location() << ". "
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

TypeChecker::Status TreeNodeExprTernary::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprTernary * root) {
    if (root->haveResultType())
        return OK;

    TreeNodeExpr * e1 = root->conditional();
    e1->setContextSecType(PublicSecType::get(getContext()));
    e1->setContextDataType(DATATYPE_BOOL);
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

    TCGUARD (visit(e->securityType()));
    e->setResultType(TypeBasic::get(getContext(), DATATYPE_UINT64));
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

    TypeContext suppliedContext;
    TreeNodeExpr * subExpr = e->expression();
    subExpr->setContext(e);
    BOOST_FOREACH (TreeNodeTypeF& node, e->types()) {
        TCGUARD (visit (&node));
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
    TCGUARD (visitExpr(subExpr));

    Type * ty = subExpr->resultType();
    if (ty->secrecSecType()->isPrivate() ||
            ty->secrecDataType() != DATATYPE_UINT8 ||
            ty->secrecDimType() != 1)
    {
        m_log.fatalInProc(e) << "Invalid argument. Expected public byte array, got "
                             << *ty << " at " << e->location() << '.';
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

TypeChecker::Status TreeNodeExprBytesFromString::accept(TypeChecker & tyChecker) {
    return tyChecker.visit(this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeExprBytesFromString * e) {
    if (e->haveResultType())
        return OK;

    TreeNodeExpr * subExpr = e->expression();
    subExpr->setContext(TypeBasic::get(getContext(), DATATYPE_STRING));
    TCGUARD (visitExpr(subExpr));

    Type * ty = subExpr->resultType();
    if (ty->secrecSecType()->isPrivate() ||
            ty->secrecDataType() != DATATYPE_STRING ||
            ty->secrecDimType() != 0)
    {
        m_log.fatalInProc(e) << "Invalid argument. Expected public string, got "
                             << *ty << " at " << e->location() << '.';
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
