#include "secrec/treenode.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>


namespace {

std::string xmlEncode(const std::string &input) {
    typedef std::string::const_iterator SCI;

    std::ostringstream os;

    for (SCI it = input.begin(); it != input.end(); it++) {
        unsigned char c = static_cast<unsigned char>(*it);
        switch (c) {
            case '&': os << "&amp;"; break;
            case '<': os << "&lt;"; break;
            case '>': os << "&gt;"; break;
            case '"': os << "&quot;"; break;
            case '\'': os << "&apos;"; break;
            default:
                if (c < 32 || c > 127) {
                    os << "&#" << static_cast<unsigned int>(c) << ';';
                } else {
                    os << c;
                }
                break;
        }
    }
    return os.str();
}

} // anonymous namespace

namespace SecreC {

/*******************************************************************************
  Class TreeNode
*******************************************************************************/

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
        case NODE_DECL_GLOBALS: return "DECL_GLOBALS";
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


/*******************************************************************************
  Class TreeNodeBool
*******************************************************************************/

std::string TreeNodeBool::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"bool:" << stringHelper() << "\"";
    return os.str();
}


/*******************************************************************************
  Class TreeNodeInt
*******************************************************************************/

std::string TreeNodeInt::stringHelper() const {
    std::ostringstream os;
    os << m_value;
    return os.str();
}

std::string TreeNodeInt::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"int:" << m_value << "\"";
    return os.str();
}


/*******************************************************************************
  Class TreeNodeUInt
*******************************************************************************/

std::string TreeNodeUInt::stringHelper() const {
    std::ostringstream os;
    os << m_value;
    return os.str();
}

std::string TreeNodeUInt::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"uint:" << m_value << "\"";
    return os.str();
}


/*******************************************************************************
  Class TreeNodeString
*******************************************************************************/

std::string TreeNodeString::stringHelper() const {
    std::ostringstream os;
    os << "\"" << m_value << "\"";
    return os.str();
}

std::string TreeNodeString::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"string:" << xmlEncode(m_value) << "\"";
    return os.str();
}

/*******************************************************************************
  Class TreeNodeIdentifier
*******************************************************************************/

std::string TreeNodeIdentifier::stringHelper() const {
    std::ostringstream os;
    os << "\"" << m_value << "\"";
    return os.str();
}

std::string TreeNodeIdentifier::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"string:" << xmlEncode(m_value) << "\"";
    return os.str();
}

/*******************************************************************************
  Class TreeNodeBasicType
*******************************************************************************/

std::string TreeNodeBasicType::stringHelper() const {
    std::ostringstream os;
    os << "\"" << BasicType::toString(m_secType, m_varType) << "\"";
    return os.str();
}

std::string TreeNodeBasicType::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"basic:" << BasicType::toString(m_secType, m_varType) << "\"";
    return os.str();
}


/*******************************************************************************
  Class TreeNodeArrayType
*******************************************************************************/

std::string TreeNodeArrayType::stringHelper() const {
    std::ostringstream os;
    os << m_value;
    return os.str();
}

std::string TreeNodeArrayType::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"array:" << m_value << "\"";
    return os.str();
}

} // namespace SecreC

/*******************************************************************************
  C interface for Yacc
*******************************************************************************/

extern "C" struct TreeNode *treenode_init(enum SecrecTreeNodeType type, YYLTYPE *loc) {
    return (TreeNode*) (new SecreC::TreeNode(type, *loc));
}

extern "C" void treenode_free(struct TreeNode *node) {
    delete ((SecreC::TreeNode*) node);
}

extern "C" enum SecrecTreeNodeType treenode_type(struct TreeNode *node) {
    return ((SecreC::TreeNode*) node)->type();
}

extern "C" unsigned treenode_numChildren(struct TreeNode *node) {
    return ((SecreC::TreeNode*) node)->children().size();
}

extern "C" struct TreeNode *treenode_childAt(struct TreeNode *node, unsigned index) {
    return (TreeNode*) ((SecreC::TreeNode*) node)->children().at(index).data();
}

extern "C" void treenode_appendChild(struct TreeNode *parent, struct TreeNode *child) {
    return ((SecreC::TreeNode*) parent)->appendChild((SecreC::TreeNode*) child);
}

extern "C" void treenode_prependChild(struct TreeNode *parent, struct TreeNode *child) {
    return ((SecreC::TreeNode*) parent)->prependChild((SecreC::TreeNode*) child);
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

extern "C" struct TreeNode *treenode_init_string(const char *value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeString(value, *loc);
}

extern "C" struct TreeNode *treenode_init_identifier(const char *value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeIdentifier(value, *loc);
}

extern "C" struct TreeNode *treenode_init_basictype(enum SecrecSecType secType,
                                                    enum SecrecVarType varType,
                                                    YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeBasicType(secType, varType, *loc);
}

extern "C" struct TreeNode *treenode_init_arraytype(unsigned value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeArrayType(value, *loc);
}
