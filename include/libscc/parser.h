#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
namespace SecreC {
    class TreeNode;
    class TreeNodeProgram;
} // namespace SecreC
#define TYPE_TREENODE        SecreC::TreeNode*
#define TYPE_TREENODEPROGRAM SecreC::TreeNodeProgram*
extern "C" {
#else /* #ifdef __cplusplus */
#define TYPE_TREENODE        void*
#define TYPE_TREENODEPROGRAM void*
#endif /* #ifdef __cplusplus */

union YYSTYPE;
struct YYLTYPE;

enum SecrecTreeNodeType {
    /** Just a node for counting, internal use only. */
    NODE_INTERNAL_USE     = 0x0,

    NODE_IDENTIFIER       = 0x1,
    NODE_LITE_BOOL        = 0x2,
    NODE_LITE_INT         = 0x3,
    NODE_LITE_UINT        = 0x4,
    NODE_LITE_STRING      = 0x5,
        NODE_LITE_MASK    = 0xf,
    NODE_EXPR_NONE        = 0x010, /* void expression          */
    NODE_EXPR_CLASSIFY    = 0x020, /* classify(expr)           */
    NODE_EXPR_DECLASSIFY  = 0x030, /* declassify(expr)         */
    NODE_EXPR_PROCCALL    = 0x040, /* expr(), expr(expr, ...)  */
    NODE_EXPR_WILDCARD    = 0x050, /* expr[*]                  */
    NODE_EXPR_SUBSCRIPT   = 0x060, /* expr[expr]               */
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
    NODE_DECL_VSUFFIX   = 0x0c000,
    NODE_GLOBALS        = 0x0d000,
    NODE_PROCDEF        = 0x0e000,
    NODE_PROCDEFS       = 0x0f000,
    NODE_PROGRAM        = 0x10000,

    NODE_TYPETYPE       = 0x100000,
    NODE_TYPEVOID       = 0x200000,
    NODE_DATATYPE_F     = 0x300000,
    NODE_DATATYPE_ARRAY = 0x400000,
    NODE_SECTYPE_F      = 0x500000
};

enum SecrecSecType { SECTYPE_INVALID = 0x00, SECTYPE_PUBLIC = 0x01,
                     SECTYPE_PRIVATE = 0x02 };
enum SecrecDataType { DATATYPE_INVALID = 0x00, DATATYPE_BOOL = 0x01,
                      DATATYPE_INT = 0x02, DATATYPE_UINT = 0x04,
                      DATATYPE_STRING = 0x08 };

/**
    Parses SecreC from the standard input.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
*/
extern int sccparse(TYPE_TREENODEPROGRAM *result);

/**
    Parses SecreC from the given input.
    \param input pointer to the input stream.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
*/
extern int sccparse_file(FILE *input, TYPE_TREENODEPROGRAM *result);

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
extern int sccparse_mem(const void *buf, size_t size, TYPE_TREENODEPROGRAM *result);

union YYSTYPE {
    TYPE_TREENODE treenode;
    char *nothing;
    char *str;
};
typedef union YYSTYPE YYSTYPE;

#define YYLTYPE YYLTYPE
typedef struct YYLTYPE {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
    char *filename;
} YYLTYPE;
#define YYLTYPE_IS_TRIVIAL 1

#ifdef __cplusplus
} /* extern "C" */
#endif /* #ifdef __cplusplus */

#endif /* #ifdef PARSER_H */

