/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_PARSER_ENUMS_H
#define SECREC_PARSER_ENUMS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned SecrecDimType; /**< (~ SecrecDimType(0)) reserved for undefined  */

#ifdef __cplusplus
namespace SecreC { typedef ::SecrecDimType SecrecDimType; }
#endif /* #ifdef __cplusplus */

enum SecrecOperator {
    SCOP_NONE     = 0x000,
    SCOP_BIN_ADD  = 0x001,
    SCOP_BIN_BAND = 0x002,
    SCOP_BIN_BOR  = 0x003,
    SCOP_BIN_DIV  = 0x004,
    SCOP_BIN_EQ   = 0x005,
    SCOP_BIN_GE   = 0x006,
    SCOP_BIN_GT   = 0x007,
    SCOP_BIN_LAND = 0x008,
    SCOP_BIN_LE   = 0x009,
    SCOP_BIN_LOR  = 0x00a,
    SCOP_BIN_LT   = 0x00b,
    SCOP_BIN_MOD  = 0x00c,
    SCOP_BIN_MUL  = 0x00d,
    SCOP_BIN_NE   = 0x00e,
    SCOP_BIN_SUB  = 0x00f,
    SCOP_BIN_XOR  = 0x010,
    SCOP_BIN_SHL  = 0x011,
    SCOP_BIN_SHR  = 0x012,
    SCOP_UN_INV   = 0x100,
    SCOP_UN_NEG   = 0x200,
    SCOP_UN_MINUS = 0x300
};

enum SecrecTreeNodeType {
    /** Just a node for counting, internal use only. */
    NODE_ATTRIBUTE,
    NODE_DATATYPE_CONST_F,
    NODE_DATATYPE_TEMPLATE_F,
    NODE_DATATYPE_VAR_F,
    NODE_DECL,
    NODE_DIMENSIONS,
    NODE_DIMTYPE_CONST_F,
    NODE_DIMTYPE_VAR_F,
    NODE_DOMAIN,

    NODE_EXPR_ARRAY_CONSTRUCTOR, /* { e1, e2, ..., en }      */
    NODE_EXPR_BINARY_ADD,        /* expr + expr              */
    NODE_EXPR_BINARY_ASSIGN_ADD, /* expr += expr             */
    NODE_EXPR_BINARY_ASSIGN_AND, /* expr &= expr             */
    NODE_EXPR_BINARY_ASSIGN_DIV, /* expr /= expr             */
    NODE_EXPR_BINARY_ASSIGN,     /* expr = expr              */
    NODE_EXPR_BINARY_ASSIGN_MOD, /* expr %= expr             */
    NODE_EXPR_BINARY_ASSIGN_MUL, /* expr *= expr             */
    NODE_EXPR_BINARY_ASSIGN_OR,  /* expr |= expr             */
    NODE_EXPR_BINARY_ASSIGN_SUB, /* expr -= expr             */
    NODE_EXPR_BINARY_ASSIGN_XOR, /* expr ^= expr             */
    NODE_EXPR_BINARY_DIV,        /* expr / expr              */
    NODE_EXPR_BINARY_EQ,         /* expr == expr             */
    NODE_EXPR_BINARY_GE,         /* expr >= expr             */
    NODE_EXPR_BINARY_GT,         /* expr > expr              */
    NODE_EXPR_BINARY_LAND,       /* expr && expr             */
    NODE_EXPR_BINARY_LE,         /* expr <= expr             */
    NODE_EXPR_BINARY_LOR,        /* expr || expr             */
    NODE_EXPR_BINARY_LT,         /* expr < expr              */
    NODE_EXPR_BINARY_MATRIXMUL,  /* expr # expr              */
    NODE_EXPR_BINARY_MOD,        /* expr % expr              */
    NODE_EXPR_BINARY_MUL,        /* expr * expr              */
    NODE_EXPR_BINARY_NE,         /* expr != expr             */
    NODE_EXPR_BINARY_SHL,        /* e1 << e2                 */
    NODE_EXPR_BINARY_SHR,        /* e1 >> e2                 */
    NODE_EXPR_BINARY_SUB,        /* expr - expr              */
    NODE_EXPR_BITWISE_AND,       /* expr & expr              */
    NODE_EXPR_BITWISE_OR,        /* expr | expr              */
    NODE_EXPR_BITWISE_XOR,       /* expr ^ expr              */
    NODE_EXPR_BYTES_FROM_STRING, /* __bytes_from_string      */
    NODE_EXPR_CAST,              /* (type) expr              */
    NODE_EXPR_CAT,               /* cat(expr, expr {, expr}) */
    NODE_EXPR_CLASSIFY,          /* classify(expr)           */
    NODE_EXPR_DECLASSIFY,        /* declassify(expr)         */
    NODE_EXPR_DOMAINID,          /* __domainid (expr)        */
    NODE_EXPR_INDEX,             /* expr[expr,...]           */
    NODE_EXPR_NONE,              /* void expression          */
    NODE_EXPR_POSTFIX_DEC,       /* expr --                  */
    NODE_EXPR_POSTFIX_INC,       /* expr ++                  */
    NODE_EXPR_PREFIX_DEC,        /* -- expr                  */
    NODE_EXPR_PREFIX_INC,        /* ++ expr                  */
    NODE_EXPR_PROCCALL,          /* expr(), expr(expr, ...)  */
    NODE_EXPR_RESHAPE,           /* reshape(expr, ...        */
    NODE_EXPR_RVARIABLE,         /* x (R-value)              */
    NODE_EXPR_SELECTION,         /* e.name                   */
    NODE_EXPR_SHAPE,             /* shape(expr)              */
    NODE_EXPR_SIZE,              /* size(expr)               */
    NODE_EXPR_STRING_FROM_BYTES, /* __string_from_bytes      */
    NODE_EXPR_TERNIF,            /* expr ? expr : expr       */
    NODE_EXPR_TOSTRING,          /* tostring(expr)           */
    NODE_EXPR_TYPE_QUAL,         /* expr :: type             */
    NODE_EXPR_UINV,              /* ~expr                    */
    NODE_EXPR_UMINUS,            /* -expr                    */
    NODE_EXPR_UNEG,              /* !expr                    */

    NODE_IDENTIFIER,
    NODE_IMPORT,
    NODE_INDEX_INT,
    NODE_INDEX_SLICE,
    NODE_INTERNAL_USE,
    NODE_KIND,
    NODE_LITE_BOOL,
    NODE_LITE_FLOAT,
    NODE_LITE_INT,
    NODE_LITE_STRING,

    NODE_LVALUE_INDEX,
    NODE_LVALUE_SELECT,
    NODE_LVALUE_VARIABLE,

    NODE_MODULE,
    NODE_OPDEF,
    NODE_PROCDEF,
    NODE_PROGRAM,
    NODE_PUSH,
    NODE_PUSHCREF,
    NODE_PUSHREF,
    NODE_SECTYPE_PRIVATE_F,
    NODE_SECTYPE_PUBLIC_F,

    NODE_STMT_ASSERT,
    NODE_STMT_BREAK,
    NODE_STMT_COMPOUND,
    NODE_STMT_CONTINUE,
    NODE_STMT_DOWHILE,
    NODE_STMT_EXPR,
    NODE_STMT_FOR,
    NODE_STMT_IF,
    NODE_STMT_PRINT,
    NODE_STMT_RETURN,
    NODE_STMT_SYSCALL,
    NODE_STMT_WHILE,

    NODE_STRING_PART_FRAGMENT,
    NODE_STRING_PART_IDENTIFIER,

    NODE_STRUCT_DECL,
    NODE_SUBSCRIPT,
    NODE_SYSCALL_RETURN,

    NODE_TEMPLATE_DECL,

    NODE_TEMPLATE_QUANTIFIER_DATA,
    NODE_TEMPLATE_QUANTIFIER_DIM,
    NODE_TEMPLATE_QUANTIFIER_DOMAIN,

    NODE_TYPE_ARG_DATA_TYPE_CONST,
    NODE_TYPE_ARG_DIM_TYPE_CONST,
    NODE_TYPE_ARG_PUBLIC,
    NODE_TYPE_ARG_TEMPLATE,
    NODE_TYPE_ARG_VAR,

    NODE_TYPETYPE,
    NODE_TYPEVAR,
    NODE_TYPEVOID,
    NODE_VAR_INIT
};

enum SecrecDataType {
    DATATYPE_UNDEFINED = 0x00,
    DATATYPE_UNIT,
    DATATYPE_BOOL,
    DATATYPE_STRING,
    DATATYPE_NUMERIC,
    DATATYPE_INT8,
    DATATYPE_INT16,
    DATATYPE_INT32,
    DATATYPE_INT64,
    DATATYPE_UINT8,
    DATATYPE_UINT16,
    DATATYPE_UINT32,
    DATATYPE_UINT64,
    DATATYPE_XOR_UINT8,
    DATATYPE_XOR_UINT16,
    DATATYPE_XOR_UINT32,
    DATATYPE_XOR_UINT64,
    DATATYPE_FLOAT32,
    DATATYPE_FLOAT64,
    NUM_DATATYPES
};

#ifdef __cplusplus
} /* extern "C" */
#endif /* #ifdef __cplusplus */

#endif /* SECREC_PARSER_ENUMS_H */
