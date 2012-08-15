#ifndef SECREC_PARSER_H
#define SECREC_PARSER_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
namespace SecreC {
    class TreeNode;
    class TreeNodeModule;
} // namespace SecreC
#define TYPE_TREENODE        SecreC::TreeNode*
#define TYPE_TREENODEMODULE  SecreC::TreeNodeModule*
extern "C" {
#else /* #ifdef __cplusplus */
#define TYPE_TREENODE        void*
#define TYPE_TREENODEMODULE void*
#endif /* #ifdef __cplusplus */

union YYSTYPE;
struct YYLTYPE;

enum SecrecOperator {
    SCOP_NONE     = 0x00,
    SCOP_BIN_MUL  = 0x01,
    SCOP_BIN_DIV  = 0x02,
    SCOP_BIN_MOD  = 0x03,
    SCOP_BIN_ADD  = 0x04,
    SCOP_BIN_SUB  = 0x05,
    SCOP_BIN_EQ   = 0x06,
    SCOP_BIN_NE   = 0x07,
    SCOP_BIN_LE   = 0x08,
    SCOP_BIN_GT   = 0x09,
    SCOP_BIN_GE   = 0x0a,
    SCOP_BIN_LT   = 0x0b,
    SCOP_BIN_LAND = 0x0c,
    SCOP_BIN_LOR  = 0x0d,
    SCOP_UN_INV  = 0x10,
    SCOP_UN_NEG  = 0x20,
    SCOP_UN_MINUS = 0x30
};

enum SecrecTreeNodeType {
    /** Just a node for counting, internal use only. */
    NODE_INTERNAL_USE     = 0x0,

    NODE_IDENTIFIER       = 0x1,
    NODE_LITE_BOOL        = 0x2,
    NODE_LITE_INT         = 0x3,
    NODE_LITE_STRING      = 0x5,
    NODE_LITE_FLOAT       = 0x6,
        NODE_LITE_MASK    = 0xf,
    NODE_EXPR_NONE        = 0x010, /* void expression          */
    NODE_EXPR_CLASSIFY    = 0x020, /* classify(expr)           */
    NODE_EXPR_DECLASSIFY  = 0x030, /* declassify(expr)         */
    NODE_EXPR_PROCCALL    = 0x040, /* expr(), expr(expr, ...)  */
    NODE_EXPR_INDEX       = 0x050, /* expr[expr,...]           */
    NODE_EXPR_UINV        = 0x060, /* ~expr                    */
    NODE_EXPR_UNEG        = 0x070, /* !expr                    */
    NODE_EXPR_UMINUS      = 0x080, /* -expr                    */
    NODE_EXPR_CAST        = 0x090, /* (type) expr              */
    NODE_EXPR_BINARY_MATRIXMUL = 0x0a0, /* expr # expr              */
    NODE_EXPR_BINARY_MUL       = 0x0b0, /* expr * expr              */
    NODE_EXPR_BINARY_DIV       = 0x0c0, /* expr / expr              */
    NODE_EXPR_BINARY_MOD       = 0x0d0, /* expr % expr              */
    NODE_EXPR_BINARY_ADD       = 0x0e0, /* expr + expr              */
    NODE_EXPR_BINARY_SUB       = 0x0f0, /* expr - expr              */
    NODE_EXPR_BINARY_EQ        = 0x100, /* expr == expr             */
    NODE_EXPR_BINARY_NE        = 0x110, /* expr != expr             */
    NODE_EXPR_BINARY_LE        = 0x120, /* expr <= expr             */
    NODE_EXPR_BINARY_GT        = 0x130, /* expr > expr              */
    NODE_EXPR_BINARY_GE        = 0x140, /* expr >= expr             */
    NODE_EXPR_BINARY_LT        = 0x150, /* expr < expr              */
    NODE_EXPR_BINARY_LAND      = 0x160, /* expr && expr             */
    NODE_EXPR_BINARY_LOR       = 0x170, /* expr || expr             */
    NODE_EXPR_TERNIF      = 0x180, /* expr ? expr : expr       */
    NODE_EXPR_ASSIGN_MUL  = 0x190, /* expr *= expr             */
    NODE_EXPR_ASSIGN_DIV  = 0x1a0, /* expr /= expr             */
    NODE_EXPR_ASSIGN_MOD  = 0x1b0, /* expr %= expr             */
    NODE_EXPR_ASSIGN_ADD  = 0x1c0, /* expr += expr             */
    NODE_EXPR_ASSIGN_SUB  = 0x1d0, /* expr -= expr             */
    NODE_EXPR_ASSIGN      = 0x1e0, /* expr = expr              */
    NODE_EXPR_RVARIABLE   = 0x1f0, /* x (R-value)              */
    NODE_EXPR_SIZE        = 0x200, /* size(expr)               */
    NODE_EXPR_SHAPE       = 0x210, /* shape(expr)              */
    NODE_EXPR_CAT         = 0x220, /* cat(expr, expr {, expr}) */
    NODE_EXPR_RESHAPE     = 0x230, /* reshape(expr, ...        */
    NODE_EXPR_TOSTRING    = 0x240, /* tostring(expr)           */
    NODE_EXPR_POSTFIX_INC = 0x250, /* expr ++                  */
    NODE_EXPR_POSTFIX_DEC = 0x260, /* expr --                  */
    NODE_EXPR_PREFIX_INC  = 0x270, /* ++ expr                  */
    NODE_EXPR_PREFIX_DEC  = 0x280, /* -- expr                  */
    NODE_EXPR_DOMAINID    = 0x290, /* __domainid (expr)        */
    NODE_EXPR_TYPE_QUAL   = 0x2a0, /* expr :: type             */
    NODE_EXPR_STRING_FROM_BYTES = 0x2b0,
    NODE_EXPR_BYTES_FROM_STRING = 0x2c0,
        NODE_EXPR_MASK    = 0xfff, /* NB! Including literals.  */
    NODE_STMT_IF        = 0x01000,
    NODE_STMT_FOR       = 0x02000,
    NODE_STMT_WHILE     = 0x03000,
    NODE_STMT_DOWHILE   = 0x04000,
    NODE_STMT_COMPOUND  = 0x05000,
    NODE_STMT_RETURN    = 0x06000,
    NODE_STMT_CONTINUE  = 0x07000,
    NODE_STMT_BREAK     = 0x08000,
    NODE_STMT_EXPR      = 0x09000,
    NODE_STMT_ASSERT    = 0x0a000,
    NODE_DECL           = 0x0b000,
    NODE_OPDEF          = 0x0c000,
    NODE_PROCDEF        = 0x0e000,
    NODE_PROGRAM        = 0x10000,
    NODE_DIMENSIONS     = 0x11000,
    NODE_INDEX_INT      = 0x12000,
    NODE_INDEX_SLICE    = 0x13000,
    NODE_LVALUE         = 0x14000,
    NODE_SUBSCRIPT      = 0x15000,
    NODE_STMT_PRINT     = 0x16000,
    NODE_STMT_SYSCALL   = 0x17000,
    NODE_STMT_PUSH      = 0x18000,
    NODE_STMT_PUSHREF   = 0x19000,
    NODE_STMT_PUSHCREF  = 0x1a000,
    NODE_VAR_INIT       = 0x1b000,

    NODE_TYPETYPE       = 0x100000,
    NODE_TYPEVOID       = 0x200000,
    NODE_DATATYPE_F     = 0x300000,
    NODE_DIMTYPE_F      = 0x400000,
    NODE_SECTYPE_F      = 0x500000,
    NODE_DOMAIN         = 0x600000,
    NODE_KIND           = 0x700000,
    NODE_TEMPLATE_DECL  = 0x800000,
    NODE_TEMPLATE_QUANT = 0x900000,
    NODE_MODULE         = 0xa00000,
    NODE_IMPORT         = 0xb00000
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

typedef int SecrecDimType; /**< Dimensionality type is undefined if negative! */
#ifdef __cplusplus
namespace SecreC { typedef ::SecrecDimType SecrecDimType; }
#endif /* #ifdef __cplusplus */

/**
    Parses SecreC from the standard input.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
*/
extern int sccparse(TYPE_TREENODEMODULE *result);

/**
    Parses SecreC from the given input.
    \param input pointer to the input stream.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
*/
extern int sccparse_file(FILE *input, TYPE_TREENODEMODULE *result);

/**
    Parses SecreC from the given memory region.
    \param buf pointer to the start of the region.
    \param size size of the region in bytes.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
    \retval 3 Parsing failed because the input could not be read.
*/
extern int sccparse_mem(const void *buf, size_t size, TYPE_TREENODEMODULE *result);

union YYSTYPE {
    TYPE_TREENODE treenode;
    char *nothing;
    char *str;
    uint64_t integer_literal;
};
typedef union YYSTYPE YYSTYPE;

#define YYLTYPE YYLTYPE
typedef struct YYLTYPE {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
} YYLTYPE;
#define YYLTYPE_IS_TRIVIAL 1

#ifdef __cplusplus
} /* extern "C" */
#endif /* #ifdef __cplusplus */

#endif /* #ifdef PARSER_H */

