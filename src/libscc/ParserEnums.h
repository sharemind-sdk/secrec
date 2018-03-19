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

#ifndef SECREC_PARSER_ENUMS_H
#define SECREC_PARSER_ENUMS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned SecrecDimType; /**< (~ SecrecDimType(0)) reserved for undefined  */

#ifdef __cplusplus
namespace SecreC { using SecrecDimType = ::SecrecDimType; }
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
    DATATYPE_NUMERIC_FLOAT,
    DATATYPE_FLOAT32,
    DATATYPE_FLOAT64,
    NUM_DATATYPES
};

#ifdef O
    #error "Macro O must not be defined before including this header."
#endif

#define TYPE_ARG_LIST \
    O(TYPE_ARG_DATA_TYPE_CONST,   TypeArgDataTypeConst) \
    O(TYPE_ARG_DIM_TYPE_CONST,    TypeArgDimTypeConst) \
    O(TYPE_ARG_PUBLIC,            TypeArgPublic) \
    O(TYPE_ARG_TEMPLATE,          TypeArgTemplate) \
    O(TYPE_ARG_VAR,               TypeArgVar)

#define DATA_TYPE_LIST \
    O(DATATYPE_TEMPLATE_F,        DataTypeTemplateF) \
    O(DATATYPE_CONST_F,           DataTypeConstF) \
    O(DATATYPE_VAR_F,             DataTypeVarF)

#define DIM_TYPE_LIST \
    O(DIMTYPE_CONST_F,            DimTypeConstF) \
    O(DIMTYPE_VAR_F,              DimTypeVarF)

#define DATATYPE_LIST \
    DATA_TYPE_LIST \
    DIM_TYPE_LIST \
    O(SECTYPE_PRIVATE_F,          SecTypeF) \
    O(SECTYPE_PUBLIC_F,           SecTypeF) \
    O(TYPEVAR,                    TypeVarF)

#define EXPR_LIST \
    O(EXPR_ARRAY_CONSTRUCTOR,     ExprArrayConstructor) \
    O(EXPR_BINARY_ADD,            ExprBinary) \
    O(EXPR_BINARY_ASSIGN,         ExprAssign) \
    O(EXPR_BINARY_ASSIGN_ADD,     ExprAssign) \
    O(EXPR_BINARY_ASSIGN_AND,     ExprAssign) \
    O(EXPR_BINARY_ASSIGN_DIV,     ExprAssign) \
    O(EXPR_BINARY_ASSIGN_MOD,     ExprAssign) \
    O(EXPR_BINARY_ASSIGN_MUL,     ExprAssign) \
    O(EXPR_BINARY_ASSIGN_OR,      ExprAssign) \
    O(EXPR_BINARY_ASSIGN_SUB,     ExprAssign) \
    O(EXPR_BINARY_ASSIGN_XOR,     ExprAssign) \
    O(EXPR_BINARY_DIV,            ExprBinary) \
    O(EXPR_BINARY_EQ,             ExprBinary) \
    O(EXPR_BINARY_GE,             ExprBinary) \
    O(EXPR_BINARY_GT,             ExprBinary) \
    O(EXPR_BINARY_LAND,           ExprBinary) \
    O(EXPR_BINARY_LE,             ExprBinary) \
    O(EXPR_BINARY_LOR,            ExprBinary) \
    O(EXPR_BINARY_LT,             ExprBinary) \
    O(EXPR_BINARY_MATRIXMUL,      ExprBinary) \
    O(EXPR_BINARY_MOD,            ExprBinary) \
    O(EXPR_BINARY_MUL,            ExprBinary) \
    O(EXPR_BINARY_NE,             ExprBinary) \
    O(EXPR_BINARY_SHL,            ExprBinary) \
    O(EXPR_BINARY_SHR,            ExprBinary) \
    O(EXPR_BINARY_SUB,            ExprBinary) \
    O(EXPR_BITWISE_AND,           ExprBinary) \
    O(EXPR_BITWISE_OR,            ExprBinary) \
    O(EXPR_BITWISE_XOR,           ExprBinary) \
    O(EXPR_BYTES_FROM_STRING,     ExprBytesFromString) \
    O(EXPR_CAST,                  ExprCast) \
    O(EXPR_CAT,                   ExprCat) \
    O(EXPR_CLASSIFY,              ExprClassify) \
    O(EXPR_DECLASSIFY,            ExprDeclassify) \
    O(EXPR_DOMAINID,              ExprDomainID) \
    O(EXPR_INDEX,                 ExprIndex) \
    O(EXPR_NONE,                  ExprNone) \
    O(EXPR_POSTFIX_DEC,           ExprPostfix) \
    O(EXPR_POSTFIX_INC,           ExprPostfix) \
    O(EXPR_PREFIX_DEC,            ExprPrefix) \
    O(EXPR_PREFIX_INC,            ExprPrefix) \
    O(EXPR_PROCCALL,              ExprProcCall) \
    O(EXPR_RESHAPE,               ExprReshape) \
    O(EXPR_RVARIABLE,             ExprRVariable) \
    O(EXPR_SELECTION,             ExprSelection) \
    O(EXPR_SHAPE,                 ExprShape) \
    O(EXPR_SIZE,                  ExprSize) \
    O(EXPR_STRING_FROM_BYTES,     ExprStringFromBytes) \
    O(EXPR_STRLEN,                ExprStrlen) \
    O(EXPR_TERNIF,                ExprTernary) \
    O(EXPR_TOSTRING,              ExprToString) \
    O(EXPR_TYPE_QUAL,             ExprQualified) \
    O(EXPR_UINV,                  ExprUnary) \
    O(EXPR_UMINUS,                ExprUnary) \
    O(EXPR_UNEG,                  ExprUnary) \
    O(LITE_BOOL,                  ExprBool) \
    O(LITE_FLOAT,                 ExprFloat) \
    O(LITE_INT,                   ExprInt) \
    O(LITE_STRING,                ExprString)

#define STMT_LIST \
    O(STMT_ASSERT,                StmtAssert) \
    O(STMT_BREAK,                 StmtBreak) \
    O(STMT_COMPOUND,              StmtCompound) \
    O(STMT_CONTINUE,              StmtContinue) \
    O(STMT_DOWHILE,               StmtDoWhile) \
    O(STMT_EXPR,                  StmtExpr) \
    O(STMT_FOR,                   StmtFor) \
    O(STMT_IF,                    StmtIf) \
    O(STMT_PRINT,                 StmtPrint) \
    O(STMT_RETURN,                StmtReturn) \
    O(STMT_SYSCALL,               StmtSyscall) \
    O(STMT_WHILE,                 StmtWhile)

#define LVALUE_LIST \
    O(LVALUE_INDEX,               LIndex) \
    O(LVALUE_SELECT,              LSelect) \
    O(LVALUE_VARIABLE,            LVariable)

#define QUANTIFIER_LIST \
    O(TEMPLATE_QUANTIFIER_DATA,   QuantifierData) \
    O(TEMPLATE_QUANTIFIER_DIM,    QuantifierDim) \
    O(TEMPLATE_QUANTIFIER_DOMAIN, QuantifierDomain)

#define STRING_PART_LIST \
    O(STRING_PART_FRAGMENT,       StringPartFragment) \
    O(STRING_PART_IDENTIFIER,     StringPartIdentifier)

#define MISC_LIST \
    O(ATTRIBUTE,                  Attribute) \
    O(KIND,                       Kind) \
    O(DOMAIN,                     Domain) \
    O(STRUCT_DECL,                StructDecl) \
    O(PROCDEF,                    ProcDef) \
    O(OPDEF,                      OpDef) \
    O(CASTDEF,                    CastDef) \
    O(TEMPLATE_DECL,              Template) \
    O(PROGRAM,                    Program) \
    O(IMPORT,                     Import) \
    O(MODULE,                     Module) \
    O(VAR_INIT,                   VarInit) \
    O(DECL,                       Decl) \
    O(TYPEVOID,                   TypeVoid) \
    O(TYPETYPE,                   TypeType) \
    O(IDENTIFIER,                 Identifier) \
    O(INDEX_SLICE,                IndexSlice) \
    O(INTERNAL_USE,               InternalUse) \
    O(DIMENSIONS,                 Dimensions) \
    O(SUBSCRIPT,                  Subscript) \
    O(INDEX_INT,                  IndexInt) \
    O(PUSHCREF,                   SyscallParam) \
    O(PUSHREF,                    SyscallParam) \
    O(PUSH,                       SyscallParam) \
    O(READONLY,                   SyscallParam) \
    O(SYSCALL_RETURN,             SyscallParam) \
    O(DATATYPE_DECL,              DataTypeDecl) \
    O(DATATYPE_DECL_PARAM_PUBLIC, DataTypeDeclParamPublic) \
    O(DATATYPE_DECL_PARAM_SIZE,   DataTypeDeclParamSize)

#define TREENODE_LIST \
    TYPE_ARG_LIST \
    DATATYPE_LIST \
    EXPR_LIST \
    STMT_LIST \
    LVALUE_LIST \
    QUANTIFIER_LIST \
    STRING_PART_LIST \
    MISC_LIST

enum SecrecTreeNodeType {
#define O(ENUM,CLASS) NODE_ ## ENUM ,
    TREENODE_LIST
#undef O
};

#ifdef __cplusplus
} /* extern "C" */
#endif /* #ifdef __cplusplus */

#endif /* SECREC_PARSER_ENUMS_H */
