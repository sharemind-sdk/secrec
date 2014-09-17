/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_VISITOR_H
#define SECREC_VISITOR_H

#include "TreeNodeFwd.h"
#include "ParserEnums.h"

/*
 * Here we provide some dispatch callbacks for various abstract syntax tree
 * nodes.  These are intended to be used by AST visitor classes. For each
 * purely virtual class we have a method that dispatches the call to derived
 * classes.
 */

#define DISPATCH(ENUM,CLASS) \
    case NODE_##ENUM: \
        return visitor.visit##CLASS (static_cast<TreeNode##CLASS *>(t), std::forward<Args>(args)...);

#define DEFAULT(MSG) \
    default: \
        assert (false && (MSG)); \
        return typename V::result_type ();

#define DISPATCHSTMT(ENUM,CLASS) DISPATCH(STMT_ ## ENUM, Stmt ## CLASS)

#define DISPATCHEXPR(ENUM,CLASS) DISPATCH(EXPR_ ## ENUM, Expr ## CLASS)

#define DISPATCHLITE(ENUM,CLASS) DISPATCH(LITE_ ## ENUM, Expr ## CLASS)

namespace SecreC {

/**
 * \brief TreeNodeTypeArg visitor
 */
template <typename V, typename... Args>
inline typename V::result_type dispatchTypeArg (V& visitor, TreeNodeTypeArg* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(TYPE_ARG_DATA_TYPE_CONST, TypeArgDataTypeConst)
    DISPATCH(TYPE_ARG_DIM_TYPE_CONST, TypeArgDimTypeConst)
    DISPATCH(TYPE_ARG_PUBLIC, TypeArgPublic)
    DISPATCH(TYPE_ARG_TEMPLATE, TypeArgTemplate)
    DISPATCH(TYPE_ARG_VAR, TypeArgVar)
    DEFAULT("Unexpected TreeNodeTypeArg!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchDataTypeF (V& visitor, TreeNodeDataTypeF* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(DATATYPE_TEMPLATE_F, DataTypeTemplateF)
    DISPATCH(DATATYPE_CONST_F, DataTypeConstF)
    DISPATCH(DATATYPE_VAR_F, DataTypeVarF)
    DEFAULT("Unexpected TreeNodeDataTypeF!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchDimTypeF (V& visitor, TreeNodeDimTypeF* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(DIMTYPE_CONST_F, DimTypeConstF)
    DISPATCH(DIMTYPE_VAR_F, DimTypeVarF)
    DEFAULT("Unexpected TreeNodeDataTypeF!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchTypeF (V& visitor, TreeNodeTypeF* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(DATATYPE_CONST_F, DataTypeConstF)
    DISPATCH(DATATYPE_TEMPLATE_F, DataTypeTemplateF)
    DISPATCH(DATATYPE_VAR_F, DataTypeVarF)
    DISPATCH(DIMTYPE_CONST_F, DimTypeConstF)
    DISPATCH(DIMTYPE_VAR_F, DimTypeVarF)
    DISPATCH(SECTYPE_PRIVATE_F, SecTypeF)
    DISPATCH(SECTYPE_PUBLIC_F, SecTypeF)
    DISPATCH(TYPEVAR, TypeVarF)
    DEFAULT("Unexpected TreeNodeTypeF!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchExpr (V& visitor, TreeNodeExpr* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCHEXPR(ARRAY_CONSTRUCTOR, ArrayConstructor)
    DISPATCHEXPR(BINARY_ASSIGN_ADD, Assign)
    DISPATCHEXPR(BINARY_ASSIGN_AND, Assign)
    DISPATCHEXPR(BINARY_ASSIGN, Assign)
    DISPATCHEXPR(BINARY_ASSIGN_DIV, Assign)
    DISPATCHEXPR(BINARY_ASSIGN_MOD, Assign)
    DISPATCHEXPR(BINARY_ASSIGN_MUL, Assign)
    DISPATCHEXPR(BINARY_ASSIGN_OR, Assign)
    DISPATCHEXPR(BINARY_ASSIGN_SUB, Assign)
    DISPATCHEXPR(BINARY_ASSIGN_XOR, Assign)
    DISPATCHEXPR(BINARY_ADD, Binary)
    DISPATCHEXPR(BINARY_DIV, Binary)
    DISPATCHEXPR(BINARY_EQ, Binary)
    DISPATCHEXPR(BINARY_GE, Binary)
    DISPATCHEXPR(BINARY_GT, Binary)
    DISPATCHEXPR(BINARY_LAND, Binary)
    DISPATCHEXPR(BINARY_LE, Binary)
    DISPATCHEXPR(BINARY_LOR, Binary)
    DISPATCHEXPR(BINARY_LT, Binary)
    DISPATCHEXPR(BINARY_MATRIXMUL, Binary)
    DISPATCHEXPR(BINARY_MOD, Binary)
    DISPATCHEXPR(BINARY_MUL, Binary)
    DISPATCHEXPR(BINARY_NE, Binary)
    DISPATCHEXPR(BINARY_SHL, Binary)
    DISPATCHEXPR(BINARY_SHR, Binary)
    DISPATCHEXPR(BINARY_SUB, Binary)
    DISPATCHEXPR(BITWISE_AND, Binary)
    DISPATCHEXPR(BITWISE_OR, Binary)
    DISPATCHEXPR(BITWISE_XOR, Binary)
    DISPATCHEXPR(BYTES_FROM_STRING, BytesFromString)
    DISPATCHEXPR(CAST, Cast)
    DISPATCHEXPR(CAT, Cat)
    DISPATCHEXPR(CLASSIFY, Classify)
    DISPATCHEXPR(DECLASSIFY, Declassify)
    DISPATCHEXPR(DOMAINID, DomainID)
    DISPATCHEXPR(INDEX, Index)
    DISPATCHEXPR(NONE, None)
    DISPATCHEXPR(POSTFIX_DEC, Postfix)
    DISPATCHEXPR(POSTFIX_INC, Postfix)
    DISPATCHEXPR(PREFIX_DEC, Prefix)
    DISPATCHEXPR(PREFIX_INC, Prefix)
    DISPATCHEXPR(PROCCALL, ProcCall)
    DISPATCHEXPR(RESHAPE, Reshape)
    DISPATCHEXPR(RVARIABLE, RVariable)
    DISPATCHEXPR(SELECTION, Selection)
    DISPATCHEXPR(SHAPE, Shape)
    DISPATCHEXPR(SIZE, Size)
    DISPATCHEXPR(STRING_FROM_BYTES, StringFromBytes)
    DISPATCHEXPR(TERNIF, Ternary)
    DISPATCHEXPR(TOSTRING, ToString)
    DISPATCHEXPR(TYPE_QUAL, Qualified)
    DISPATCHEXPR(UINV, Unary)
    DISPATCHEXPR(UMINUS, Unary)
    DISPATCHEXPR(UNEG, Unary)
    DISPATCHLITE(BOOL, Bool)
    DISPATCHLITE(FLOAT, Float)
    DISPATCHLITE(INT, Int)
    DISPATCHLITE(STRING, String)
    DEFAULT("Unexpected TreeNodeExpr!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchStmt (V& visitor, TreeNodeStmt* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCHSTMT(ASSERT, Assert)
    DISPATCHSTMT(BREAK, Break)
    DISPATCHSTMT(COMPOUND, Compound)
    DISPATCHSTMT(CONTINUE, Continue)
    DISPATCHSTMT(DOWHILE, DoWhile)
    DISPATCHSTMT(EXPR, Expr)
    DISPATCHSTMT(FOR, For)
    DISPATCHSTMT(IF, If)
    DISPATCHSTMT(PRINT, Print)
    DISPATCHSTMT(RETURN, Return)
    DISPATCHSTMT(SYSCALL, Syscall)
    DISPATCHSTMT(WHILE, While)
    DEFAULT("Unexpected TreeNodeStmt!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchLValue(V& visitor, TreeNodeLValue* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(LVALUE_INDEX, LIndex)
    DISPATCH(LVALUE_SELECT, LSelect)
    DISPATCH(LVALUE_VARIABLE, LVariable)
    DEFAULT("Unexpected TreeNodeStmt!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchQuantifier (V& visitor, TreeNodeQuantifier* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(TEMPLATE_QUANTIFIER_DATA, QuantifierData)
    DISPATCH(TEMPLATE_QUANTIFIER_DIM, QuantifierDim)
    DISPATCH(TEMPLATE_QUANTIFIER_DOMAIN, QuantifierDomain)
    DEFAULT("Unexpected TreeNodeQuantifier!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchStringPart (V& visitor, TreeNodeStringPart* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(STRING_PART_FRAGMENT, StringPartFragment)
    DISPATCH(STRING_PART_IDENTIFIER, StringPartIdentifier)
    DEFAULT("Unexpected TreeNodeStringPart!")
    }
}

#undef DISPATCHSTMT
#undef DISPATCHLITE
#undef DISPATCHEXPR
#undef DISPATCH
#undef DEFAULT

} /* namespace SecreC { */

#endif /* SECREC_VISITOR_H */
