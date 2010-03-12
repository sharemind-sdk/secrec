#include "secrec/treenode.h"

#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include "secrec/symboltable.h"
#include "secrec/treenodebool.h"
#include "secrec/treenodeexprunary.h"
#include "secrec/treenodeexprbinary.h"
#include "secrec/treenodeexprternary.h"
#include "secrec/treenodefundefs.h"
#include "secrec/treenodeglobals.h"
#include "secrec/treenodeidentifier.h"
#include "secrec/treenodeint.h"
#include "secrec/treenodeprogram.h"
#include "secrec/treenodestring.h"
#include "secrec/treenodetypebasic.h"
#include "secrec/treenodetypearray.h"
#include "secrec/treenodeuint.h"


namespace SecreC {

TreeNode::TreeNode(Type type, const struct YYLTYPE &loc)
    : m_parent(0), m_type(type), m_location(loc)
{
    // Intentionally empty
}

void TreeNode::appendChild(TreeNode *child, bool reparent) {
    assert(child != 0);
    m_children.push_back(child);
    if (reparent) {
        child->m_parent = this;
    }
}

void TreeNode::prependChild(TreeNode *child, bool reparent) {
    assert(child != 0);
    m_children.push_front(child);
    if (reparent) {
        child->m_parent = this;
    }
}

void TreeNode::setLocation(const YYLTYPE &location) {
    m_location = location;
}

const char *TreeNode::typeName(Type type) {
    switch (type) {
        case NODE_INTERNAL_USE: return "INTERNAL_USE";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_LITE_BOOL: return "BOOL";
        case NODE_LITE_INT: return "INT";
        case NODE_LITE_UINT: return "UINT";
        case NODE_LITE_STRING: return "STRING";
        case NODE_EXPR_NONE: return "EXPR_NONE";
        case NODE_EXPR_FUNCALL: return "EXPR_FUNCALL";
        case NODE_EXPR_WILDCARD: return "EXPR_WILDCARD";
        case NODE_EXPR_SUBSCRIPT: return "EXPR_SUBSCRIPT";
        case NODE_EXPR_UNEG: return "EXPR_UNEG";
        case NODE_EXPR_UMINUS: return "EXPR_UMINUS";
        case NODE_EXPR_CAST: return "EXPR_CAST";
        case NODE_EXPR_MATRIXMUL: return "EXPR_MATRIXMUL";
        case NODE_EXPR_MUL: return "EXPR_MUL";
        case NODE_EXPR_DIV: return "EXPR_DIV";
        case NODE_EXPR_MOD: return "EXPR_MOD";
        case NODE_EXPR_ADD: return "EXPR_ADD";
        case NODE_EXPR_SUB: return "EXPR_SUB";
        case NODE_EXPR_EQ: return "EXPR_EQ";
        case NODE_EXPR_NE: return "EXPR_NE";
        case NODE_EXPR_LE: return "EXPR_LE";
        case NODE_EXPR_GT: return "EXPR_GT";
        case NODE_EXPR_GE: return "EXPR_GE";
        case NODE_EXPR_LT: return "EXPR_LT";
        case NODE_EXPR_LAND: return "EXPR_LAND";
        case NODE_EXPR_LOR: return "EXPR_LOR";
        case NODE_EXPR_TERNIF: return "EXPR_TERNIF";
        case NODE_EXPR_ASSIGN_MUL: return "EXPR_ASSIGN_MUL";
        case NODE_EXPR_ASSIGN_DIV: return "EXPR_ASSIGN_DIV";
        case NODE_EXPR_ASSIGN_MOD: return "EXPR_ASSIGN_MOD";
        case NODE_EXPR_ASSIGN_ADD: return "EXPR_ASSIGN_ADD";
        case NODE_EXPR_ASSIGN_SUB: return "EXPR_ASSIGN_SUB";
        case NODE_EXPR_ASSIGN: return "EXPR_ASSIGN";
        case NODE_EXPR_LVARIABLE: return "LVARIABLE";
        case NODE_EXPR_RVARIABLE: return "RVARIABLE";
        case NODE_STMT_IF: return "STMT_IF";
        case NODE_STMT_FOR: return "STMT_FOR";
        case NODE_STMT_WHILE: return "STMT_WHILE";
        case NODE_STMT_DOWHILE: return "STMT_DOWHILE";
        case NODE_STMT_COMPOUND: return "STMT_COMPOUND";
        case NODE_STMT_RETURN: return "STMT_RETURN";
        case NODE_STMT_CONTINUE: return "STMT_CONTINUE";
        case NODE_STMT_BREAK: return "STMT_BREAK";
        case NODE_STMT_EXPR: return "STMT_EXPR";
        case NODE_DECL: return "DECL";
        case NODE_DECL_VSUFFIX: return "DECL_VSUFFIX";
        case NODE_GLOBALS: return "DECL_GLOBALS";
        case NODE_BASICTYPE:
        case NODE_ARRAYTYPE: return "TYPE";
        case NODE_FUNDEF: return "FUNDEF";
        case NODE_FUNDEF_PARAM: return "FUNDEF_PARAM";
        case NODE_FUNDEFS: return "FUNDEFS";
        case NODE_PROGRAM: return "PROGRAM";
        default: return "UNKNOWN";
    }
}

std::string TreeNode::toString(unsigned indent, unsigned startIndent)
        const
{
    std::ostringstream os;

    // Indent:
    for (unsigned i = 0; i < startIndent; i++) {
        os << ' ';
    }

    os << typeName(m_type);

    const std::string sh(stringHelper());
    if (!sh.empty())
        os << ' ' << sh;

    for (unsigned i = 0; i < m_children.size(); i++) {
        os << std::endl;
        os << m_children.at(i)->toString(indent, startIndent + indent);
    }
    return os.str();
}

std::string TreeNode::toXml(bool full) const {
    std::ostringstream os;
    if (full) {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    }
    os << '<' << typeName(m_type);

    const std::string xh(xmlHelper());
    if (!xh.empty())
        os << ' ' << xh;

    if (m_children.empty()) {
        os << "/>";
    } else {
        os << '>';
        for (unsigned i = 0; i < m_children.size(); i++) {
            os << m_children.at(i)->toXml(false);
        }
        os << "</" << typeName(m_type) << '>';
    }
    return os.str();
}

} // namespace SecreC

/*******************************************************************************
  C interface for Yacc
*******************************************************************************/

extern "C" struct TreeNode *treenode_init(enum SecrecTreeNodeType type,
                                          const YYLTYPE *loc)
{
    switch (type) {
        case NODE_PROGRAM:
            return (TreeNode*) (new SecreC::TreeNodeProgram(*loc));
        case NODE_GLOBALS:
            return (TreeNode*) (new SecreC::TreeNodeGlobals(*loc));
        case NODE_FUNDEFS:
            return (TreeNode*) (new SecreC::TreeNodeFundefs(*loc));
        case NODE_EXPR_WILDCARD: /* Fall through: */
        case NODE_EXPR_UNEG:     /* Fall through: */
        case NODE_EXPR_UMINUS:
            return (TreeNode*) (new SecreC::TreeNodeExprUnary(type, *loc));
        case NODE_EXPR_MATRIXMUL:  /* Fall through: */
        case NODE_EXPR_MUL:        /* Fall through: */
        case NODE_EXPR_DIV:        /* Fall through: */
        case NODE_EXPR_MOD:        /* Fall through: */
        case NODE_EXPR_ADD:        /* Fall through: */
        case NODE_EXPR_SUB:        /* Fall through: */
        case NODE_EXPR_EQ:         /* Fall through: */
        case NODE_EXPR_NE:         /* Fall through: */
        case NODE_EXPR_LE:         /* Fall through: */
        case NODE_EXPR_GT:         /* Fall through: */
        case NODE_EXPR_GE:         /* Fall through: */
        case NODE_EXPR_LT:         /* Fall through: */
        case NODE_EXPR_LAND:       /* Fall through: */
        case NODE_EXPR_LOR:        /* Fall through: */
        case NODE_EXPR_ASSIGN_MUL: /* Fall through: */
        case NODE_EXPR_ASSIGN_DIV: /* Fall through: */
        case NODE_EXPR_ASSIGN_MOD: /* Fall through: */
        case NODE_EXPR_ASSIGN_ADD: /* Fall through: */
        case NODE_EXPR_ASSIGN_SUB: /* Fall through: */
        case NODE_EXPR_ASSIGN:
            return (TreeNode*) (new SecreC::TreeNodeExprBinary(type, *loc));
        case NODE_EXPR_TERNIF:
            return (TreeNode*) (new SecreC::TreeNodeExprTernary(*loc));
        default:
            return (TreeNode*) (new SecreC::TreeNode(type, *loc));
    }
}

extern "C" void treenode_free(struct TreeNode *node) {
    delete ((SecreC::TreeNode*) node);
}

extern "C" enum SecrecTreeNodeType treenode_type(const struct TreeNode *node) {
    return ((const SecreC::TreeNode*) node)->type();
}

extern "C" const YYLTYPE *treenode_location(const struct TreeNode *node) {
    return &((const SecreC::TreeNode*) node)->location();
}

extern "C" unsigned treenode_numChildren(const struct TreeNode *node) {
    return ((const SecreC::TreeNode*) node)->children().size();
}

extern "C" struct TreeNode *treenode_childAt(const struct TreeNode *node,
                                             unsigned index)
{
    return (TreeNode*) ((const SecreC::TreeNode*) node)->children().at(index).data();
}

extern "C" void treenode_appendChild(struct TreeNode *parent,
                                     struct TreeNode *child)
{
    return ((SecreC::TreeNode*) parent)->appendChild((SecreC::TreeNode*) child);
}

extern "C" void treenode_prependChild(struct TreeNode *parent,
                                      struct TreeNode *child)
{
    typedef SecreC::TreeNode TN;
    return ((TN*) parent)->prependChild((TN*) child);
}

extern "C" void treenode_setLocation(struct TreeNode *node, YYLTYPE *loc) {
    return ((SecreC::TreeNode*) node)->setLocation(*loc);
}

extern "C" struct TreeNode *treenode_init_bool(unsigned value, YYLTYPE *loc) {

    return (TreeNode*) new SecreC::TreeNodeBool(value, *loc);
}

extern "C" struct TreeNode *treenode_init_int(int value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeInt(value, *loc);
}

extern "C" struct TreeNode *treenode_init_uint(unsigned value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeUInt(value, *loc);
}

extern "C" struct TreeNode *treenode_init_string(const char *value,
                                                 YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeString(value, *loc);
}

extern "C" struct TreeNode *treenode_init_identifier(const char *value,
                                                     YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeIdentifier(value, *loc);
}

extern "C" struct TreeNode *treenode_init_basictype(enum SecrecSecType secType,
                                                    enum SecrecVarType varType,
                                                    YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeTypeBasic(secType, varType, *loc);
}

extern "C" struct TreeNode *treenode_init_arraytype(unsigned value,
                                                    YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeTypeArray(value, *loc);
}

std::ostream &operator<<(std::ostream &out, const YYLTYPE &loc) {
    /// \todo filename
    out << "(" << loc.first_line << "," << loc.first_column
        << ")(" << loc.last_line << "," << loc.last_column << ")";
    return out;
}
