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
    NODE_INTERNAL_USE           = 0x000,
    NODE_IDENTIFIER             = 0x001,
    NODE_LITE_BOOL              = 0x002,
    NODE_LITE_FLOAT             = 0x003,
    NODE_LITE_INT               = 0x004,
    NODE_LITE_STRING            = 0x005,
        NODE_LITE_MASK          = 0x00f,
    NODE_EXPR_NONE              = 0x010, /* void expression          */
    NODE_EXPR_BINARY_ASSIGN     = 0x020, /* expr = expr              */
    NODE_EXPR_BINARY_ASSIGN_ADD = 0x030, /* expr += expr             */
    NODE_EXPR_BINARY_ASSIGN_AND = 0x040, /* expr &= expr             */
    NODE_EXPR_BINARY_ASSIGN_DIV = 0x050, /* expr /= expr             */
    NODE_EXPR_BINARY_ASSIGN_MOD = 0x060, /* expr %= expr             */
    NODE_EXPR_BINARY_ASSIGN_MUL = 0x070, /* expr *= expr             */
    NODE_EXPR_BINARY_ASSIGN_OR  = 0x080, /* expr |= expr             */
    NODE_EXPR_BINARY_ASSIGN_SUB = 0x090, /* expr -= expr             */
    NODE_EXPR_BINARY_ASSIGN_XOR = 0x0a0, /* expr ^= expr             */
    NODE_EXPR_BINARY_ADD        = 0x0b0, /* expr + expr              */
    NODE_EXPR_BINARY_DIV        = 0x0c0, /* expr / expr              */
    NODE_EXPR_BINARY_EQ         = 0x0d0, /* expr == expr             */
    NODE_EXPR_BINARY_GE         = 0x0e0, /* expr >= expr             */
    NODE_EXPR_BINARY_GT         = 0x0f0, /* expr > expr              */
    NODE_EXPR_BINARY_LAND       = 0x100, /* expr && expr             */
    NODE_EXPR_BINARY_LE         = 0x110, /* expr <= expr             */
    NODE_EXPR_BINARY_LOR        = 0x120, /* expr || expr             */
    NODE_EXPR_BINARY_LT         = 0x130, /* expr < expr              */
    NODE_EXPR_BINARY_MATRIXMUL  = 0x140, /* expr # expr              */
    NODE_EXPR_BINARY_MOD        = 0x150, /* expr % expr              */
    NODE_EXPR_BINARY_MUL        = 0x160, /* expr * expr              */
    NODE_EXPR_BINARY_NE         = 0x170, /* expr != expr             */
    NODE_EXPR_BINARY_SUB        = 0x180, /* expr - expr              */
    NODE_EXPR_BITWISE_AND       = 0x190, /* expr & expr              */
    NODE_EXPR_BITWISE_OR        = 0x1a0, /* expr | expr              */
    NODE_EXPR_BITWISE_XOR       = 0x1b0, /* expr ^ expr              */
    NODE_EXPR_BYTES_FROM_STRING = 0x1c0, /* __bytes_from_string      */
    NODE_EXPR_CAST              = 0x1d0, /* (type) expr              */
    NODE_EXPR_CAT               = 0x1e0, /* cat(expr, expr {, expr}) */
    NODE_EXPR_CLASSIFY          = 0x1f0, /* classify(expr)           */
    NODE_EXPR_DECLASSIFY        = 0x200, /* declassify(expr)         */
    NODE_EXPR_DOMAINID          = 0x210, /* __domainid (expr)        */
    NODE_EXPR_INDEX             = 0x220, /* expr[expr,...]           */
    NODE_EXPR_POSTFIX_DEC       = 0x230, /* expr --                  */
    NODE_EXPR_POSTFIX_INC       = 0x240, /* expr ++                  */
    NODE_EXPR_PREFIX_DEC        = 0x250, /* -- expr                  */
    NODE_EXPR_PREFIX_INC        = 0x260, /* ++ expr                  */
    NODE_EXPR_PROCCALL          = 0x270, /* expr(), expr(expr, ...)  */
    NODE_EXPR_RESHAPE           = 0x280, /* reshape(expr, ...        */
    NODE_EXPR_RVARIABLE         = 0x290, /* x (R-value)              */
    NODE_EXPR_SHAPE             = 0x2a0, /* shape(expr)              */
    NODE_EXPR_SIZE              = 0x2b0, /* size(expr)               */
    NODE_EXPR_STRING_FROM_BYTES = 0x2c0, /* __string_from_bytes      */
    NODE_EXPR_TERNIF            = 0x2d0, /* expr ? expr : expr       */
    NODE_EXPR_TOSTRING          = 0x2e0, /* tostring(expr)           */
    NODE_EXPR_TYPE_QUAL         = 0x2f0, /* expr :: type             */
    NODE_EXPR_UINV              = 0x300, /* ~expr                    */
    NODE_EXPR_UMINUS            = 0x310, /* -expr                    */
    NODE_EXPR_UNEG              = 0x320, /* !expr                    */
    NODE_EXPR_ARRAY_CONSTRUCTOR = 0x330, /* { e1, e2, ..., en }      */
    NODE_EXPR_BINARY_SHL        = 0x331, /* e1 << e2                 */
    NODE_EXPR_BINARY_SHR        = 0x332, /* e1 >> e2                 */
    NODE_EXPR_SELECTION         = 0x333, /* e.name                   */
        NODE_EXPR_MASK          = 0xfff, /* NB! Including literals.  */

    NODE_DECL                   = 0x01000,
    NODE_DIMENSIONS             = 0x02000,
    NODE_INDEX_INT              = 0x03000,
    NODE_INDEX_SLICE            = 0x04000,
    NODE_LVALUE                 = 0x05000,
    NODE_OPDEF                  = 0x06000,
    NODE_PROCDEF                = 0x07000,
    NODE_PROGRAM                = 0x08000,
    NODE_PUSH                   = 0x09000,
    NODE_PUSHCREF               = 0x0a000,
    NODE_PUSHREF                = 0x0b000,
    NODE_STMT_ASSERT            = 0x0c000,
    NODE_STMT_BREAK             = 0x0e000,
    NODE_STMT_COMPOUND          = 0x10000,
    NODE_STMT_CONTINUE          = 0x11000,
    NODE_STMT_DOWHILE           = 0x12000,
    NODE_STMT_EXPR              = 0x13000,
    NODE_STMT_FOR               = 0x14000,
    NODE_STMT_IF                = 0x15000,
    NODE_STMT_PRINT             = 0x16000,
    NODE_STMT_RETURN            = 0x17000,
    NODE_STMT_SYSCALL           = 0x18000,
    NODE_STMT_WHILE             = 0x19000,
    NODE_SUBSCRIPT              = 0x1a000,
    NODE_VAR_INIT               = 0x1b000,
    NODE_SYSCALL_RETURN         = 0x1c000,
    NODE_STRUCT_DECL            = 0x1d000,
    NODE_ATTRIBUTE              = 0x1e000,
    NODE_LVALUE_VARIABLE        = 0x1f000,
    NODE_LVALUE_SELECT          = 0x20000,
    NODE_LVALUE_INDEX           = 0x21000,

    NODE_DATATYPE_CONST_F       = 0x0100000,
    NODE_DATATYPE_VAR_F         = 0x0200000,
    NODE_DIMTYPE_CONST_F        = 0x0300000,
    NODE_DIMTYPE_VAR_F          = 0x0400000,
    NODE_DOMAIN                 = 0x0500000,
    NODE_IMPORT                 = 0x0600000,
    NODE_KIND                   = 0x0700000,
    NODE_MODULE                 = 0x0800000,
    NODE_SECTYPE_PUBLIC_F       = 0x0900000,
    NODE_SECTYPE_PRIVATE_F      = 0x0a00000,
    NODE_TEMPLATE_DECL          = 0x0b00000,
    NODE_TEMPLATE_DIM_QUANT     = 0x0c00000,
    NODE_TEMPLATE_DOMAIN_QUANT  = 0x0d00000,
    NODE_TEMPLATE_DATA_QUANT    = 0x0e00000,
    NODE_TYPETYPE               = 0x0f00000,
    NODE_TYPEVOID               = 0x1000000,
    NODE_TYPEVAR                = 0x2000000,
    NODE_TEMPLATE_STRUCT        = 0x3000000,
    NODE_STRING_PART_FRAGMENT   = 0x4000000,
    NODE_STRING_PART_IDENTIFIER = 0x5000000,

    NODE_TYPE_ARG_VAR             = 0x6000000,
    NODE_TYPE_ARG_TEMPLATE        = 0x7000000,
    NODE_TYPE_ARG_DATA_TYPE_CONST = 0x8000000,
    NODE_TYPE_ARG_DIM_TYPE_CONST  = 0x9000000,
    NODE_TYPE_ARG_PUBLIC          = 0xa000000
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
