#ifndef TREENODE_H
#define TREENODE_H

#include "parser.h"

#ifdef __cplusplus
#include <deque>
#include <string>
#endif /* #ifdef __cplusplus */


/*******************************************************************************
  class TreeNode
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

enum NodeType {
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
    NODE_TYPE,
    NODE_VTYPE,
    NODE_FUNDEF,
    NODE_FUNDEF_PARAM,
    NODE_FUNDEFS,
    NODE_PROGRAM
};

enum SecrecType { TYPE_VOID, TYPE_BOOL, TYPE_INT, TYPE_UINT, TYPE_STRING };
enum SecrecSecType { SECTYPE_PUBLIC, SECTYPE_PRIVATE };


struct TreeNode;

/* C accessors functions: */

struct TreeNode *treenode_init(enum NodeType type,
                               const struct YYLTYPE *loc);
struct TreeNode *treenode_init_bool(unsigned value,
                                    const struct YYLTYPE *loc);
struct TreeNode *treenode_init_int(int value,
                                   const struct YYLTYPE *loc);
struct TreeNode *treenode_init_uint(unsigned value,
                                    const struct YYLTYPE *loc);
struct TreeNode *treenode_init_string(const char *value,
                                      const struct YYLTYPE *loc);
struct TreeNode *treenode_init_type(enum SecrecSecType secType,
                                    enum SecrecType type,
                                    const struct YYLTYPE *loc);
struct TreeNode *treenode_init_vtype(enum SecrecSecType secType,
                                     enum SecrecType type,
                                     unsigned dimensions,
                                     const struct YYLTYPE *loc);
void treenode_free(struct TreeNode *node);

enum NodeType treenode_type(const struct TreeNode *node);
const struct YYLTYPE *treenode_location(const struct TreeNode *node);
unsigned treenode_numChildren(const struct TreeNode *node);
struct TreeNode *treenode_childAt(const struct TreeNode *node, unsigned index);
unsigned treenode_constant(const struct TreeNode *node);
unsigned treenode_generated(const struct TreeNode *node);
void treenode_appendChild(struct TreeNode *node, struct TreeNode *child);
void treenode_setLocation(struct TreeNode *node, const struct YYLTYPE *loc);
void treenode_setConstant(struct TreeNode *node, unsigned constant);
void treenode_setGenerated(struct TreeNode *node, unsigned generated);

unsigned treenode_value_bool(const struct TreeNode *node);
int treenode_value_int(const struct TreeNode *node);
unsigned treenode_value_uint(const struct TreeNode *node);
const char *treenode_value_string(const struct TreeNode *node);
enum SecrecSecType treenode_value_secType(const struct TreeNode *node);
enum SecrecType treenode_value_type(const struct TreeNode *node);
unsigned treenode_value_dimensions(const struct TreeNode *node);
void treenode_setValue_bool(struct TreeNode *node, unsigned value);
void treenode_setValue_int(struct TreeNode *node, int value);
void treenode_setValue_uint(struct TreeNode *node, unsigned value);
void treenode_setValue_string(struct TreeNode *node, const char *value);
void treenode_setValue_secType(struct TreeNode *node, enum SecrecSecType value);
void treenode_setValue_type(struct TreeNode *node, enum SecrecType value);
void treenode_setValue_dimensions(struct TreeNode *node, unsigned value);

void treenode_print(const struct TreeNode *node, FILE *stream, unsigned indentation);
void treenode_printXml(const struct TreeNode *node, FILE *stream);

#ifdef __cplusplus
} /* extern "C" */

/* class definition: */

class TreeNode {
    public: /* methods */
        explicit TreeNode(NodeType type, const YYLTYPE &loc);
        explicit TreeNode(bool value, const YYLTYPE &loc);
        explicit TreeNode(int value, const YYLTYPE &loc);
        explicit TreeNode(unsigned value, const YYLTYPE &loc);
        explicit TreeNode(const std::string &value, const YYLTYPE &loc);
        explicit TreeNode(SecrecSecType secType, SecrecType type,
                          const YYLTYPE &loc);
        explicit TreeNode(SecrecSecType secType, SecrecType type,
                          unsigned dimensions, const YYLTYPE &loc);
        virtual ~TreeNode();

        inline NodeType type() const;
        inline const std::deque<TreeNode*> &children() const;
        inline const YYLTYPE &location() const;
        inline bool constant() const;
        inline bool generated() const;

        void appendChild(TreeNode *child, bool reparent = true);
        void setLocation(const YYLTYPE &location);
        void setConstant(bool constant);
        void setGenerated(bool generated);

        inline bool valueBool() const;
        inline int valueInt() const;
        inline unsigned valueUInt() const;
        const std::string &valueString() const;
        inline SecrecSecType valueSecType() const;
        inline SecrecType valueType() const;
        inline unsigned valueDimensions() const;

        void setValue(bool value);
        void setValue(int value);
        void setValue(unsigned value);
        void setValue(const std::string &value);
        void setValue(SecrecSecType value);
        void setValue(SecrecType value);
        void setValueDimensions(unsigned value);

        std::string toString(unsigned indentation = 4,
                             unsigned startIndent = 0) const;
        std::string toXml(bool full = false) const;

        static const char *nodeTypeName(NodeType type);
        static const char *varTypeName(SecrecType type);
        static const char *secTypeName(SecrecSecType type);

    private: /* methods */
        void replaceChildren(TreeNode *child);

    public: /* static variables */
        static const char *NOCHILD;

    private: /* fields */
        TreeNode              *m_parent;
        const NodeType         m_type;
        std::deque<TreeNode*>  m_children;
        YYLTYPE                m_location;

        /* Flags: */
        bool                   m_constant;
        bool                   m_generated;

        /* Data: */
        union {
            std::string       *m_valueString;
            bool               m_valueBool;
            int                m_valueInt;
            unsigned int       m_valueUInt;
            struct {
                SecrecSecType secType;
                SecrecType    type;
                unsigned      dimensions;
            } m_valueType;
        };
};

inline NodeType TreeNode::type() const {
    return m_type;
}

inline const std::deque<TreeNode*> &TreeNode::children() const {
    return m_children;
}

inline const YYLTYPE &TreeNode::location() const {
    return m_location;
}

inline bool TreeNode::constant() const {
    return m_constant;
}

inline bool TreeNode::generated() const {
    return m_generated;
}

inline bool TreeNode::valueBool() const {
    return m_valueBool;
}

inline int TreeNode::valueInt() const {
    return m_valueInt;
}

inline unsigned TreeNode::valueUInt() const {
    return m_valueUInt;
}

inline SecrecSecType TreeNode::valueSecType() const {
    return m_valueType.secType;
}

inline SecrecType TreeNode::valueType() const {
    return m_valueType.type;
}

inline unsigned TreeNode::valueDimensions() const {
    return m_valueType.dimensions;
}

#endif /* #ifdef __cplusplus */

#endif /* #ifdef TREENODE_H */
