#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
namespace SecreC {
    class TreeNode;
} // namespace SecreC
#define TYPE_TREENODE SecreC::TreeNode
extern "C" {
#else /* #ifdef __cplusplus */
struct TreeNode;
#define TYPE_TREENODE struct TreeNode
#endif /* #ifdef __cplusplus */

union YYSTYPE;
struct YYLTYPE;

enum SecrecTreeNodeType {
    NODE_INTERNAL_USE,    /* Just a node for counting, internal use only. */

    NODE_IDENTIFIER,
    NODE_LITE_BOOL,
    NODE_LITE_INT,
    NODE_LITE_UINT,
    NODE_LITE_STRING,
    NODE_EXPR_NONE,       /* no expression in statements */
    NODE_EXPR_FUNCALL,    /* expr(), expr(expr, ...)  */
    NODE_EXPR_WILDCARD,   /* expr[*]                  */
    NODE_EXPR_SUBSCRIPT,  /* expr[expr]               */
    NODE_EXPR_UNEG,       /* !expr                    */
    NODE_EXPR_UMINUS,     /* -expr                    */
    NODE_EXPR_CAST,       /* (type) expr              */
    NODE_EXPR_MATRIXMUL,  /* expr # expr              */
    NODE_EXPR_MUL,        /* expr * expr              */
    NODE_EXPR_DIV,        /* expr / expr              */
    NODE_EXPR_MOD,        /* expr % expr              */
    NODE_EXPR_ADD,        /* expr + expr              */
    NODE_EXPR_SUB,        /* expr - expr              */
    NODE_EXPR_EQ,         /* expr == expr             */
    NODE_EXPR_NE,         /* expr != expr             */
    NODE_EXPR_LE,         /* expr <= expr             */
    NODE_EXPR_GT,         /* expr > expr              */
    NODE_EXPR_GE,         /* expr >= expr             */
    NODE_EXPR_LT,         /* expr < expr              */
    NODE_EXPR_LAND,       /* expr && expr             */
    NODE_EXPR_LOR,        /* expr || expr             */
    NODE_EXPR_TERNIF,     /* expr ? expr : expr       */
    NODE_EXPR_ASSIGN_MUL, /* expr *= expr             */
    NODE_EXPR_ASSIGN_DIV, /* expr /= expr             */
    NODE_EXPR_ASSIGN_MOD, /* expr %= expr             */
    NODE_EXPR_ASSIGN_ADD, /* expr += expr             */
    NODE_EXPR_ASSIGN_SUB, /* expr -= expr             */
    NODE_EXPR_ASSIGN,     /* expr = expr              */
    NODE_STMT_IF,
    NODE_STMT_FOR,
    NODE_STMT_WHILE,
    NODE_STMT_DOWHILE,
    NODE_STMT_COMPOUND,
    NODE_STMT_RETURN,
    NODE_STMT_CONTINUE,
    NODE_STMT_BREAK,
    NODE_STMT_EXPR,
    NODE_DECL,
    NODE_DECL_VSUFFIX,
    NODE_DECL_GLOBALS,
    NODE_BASICTYPE,
    NODE_ARRAYTYPE,
    NODE_FUNDEF,
    NODE_FUNDEF_PARAM,
    NODE_FUNDEFS,
    NODE_PROGRAM
};

enum SecrecSecType { SECTYPE_PUBLIC = 0x01, SECTYPE_PRIVATE = 0x02 };
enum SecrecVarType { VARTYPE_VOID = 0x01, VARTYPE_BOOL = 0x02, VARTYPE_INT = 0x04,
                     VARTYPE_UINT = 0x08, VARTYPE_STRING = 0x10 };

/**
    Parses SecreC from the standard input.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
*/
extern int sccparse(TYPE_TREENODE **result);

/**
    Parses SecreC from the given input.
    \param input pointer to the input stream.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
*/
extern int sccparse_file(FILE *input, TYPE_TREENODE **result);

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
extern int sccparse_mem(const void *buf, size_t size, TYPE_TREENODE **result);

union YYSTYPE {
    TYPE_TREENODE *treenode;
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

