#include "treenode.h"

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

}

const char *TreeNode::NOCHILD = "__child_node_deleted__";

/*******************************************************************************
  C accessor functions:
*******************************************************************************/

extern "C" TreeNode *treenode_init(NodeType type, const struct YYLTYPE *loc) {
    assert(loc != 0);
    return new TreeNode(type, *loc);
}

extern "C" struct TreeNode *treenode_init_bool(unsigned value,
                                               const struct YYLTYPE *loc)
{
    assert(loc != 0);
    return new TreeNode(static_cast<bool>(value), *loc);
}

extern "C" struct TreeNode *treenode_init_int(int value,
                                              const struct YYLTYPE *loc)
{
    assert(loc != 0);
    return new TreeNode(value, *loc);
}

extern "C" struct TreeNode *treenode_init_uint(unsigned value,
                                               const struct YYLTYPE *loc)
{
    assert(loc != 0);
    return new TreeNode(value, *loc);
}

extern "C" struct TreeNode *treenode_init_string(const char *value,
                                                 const struct YYLTYPE *loc)
{
    assert(value != 0);
    assert(loc != 0);
    std::string s(value);
    return new TreeNode(s, *loc);
}

extern "C" struct TreeNode *treenode_init_type(enum SecrecSecType secType,
                                               enum SecrecType type,
                                               const struct YYLTYPE *loc)
{
    assert(loc != 0);
    return new TreeNode(secType, type, *loc);
}

extern "C" struct TreeNode *treenode_init_vtype(enum SecrecSecType secType,
                                                enum SecrecType type,
                                                unsigned dimensions,
                                                const struct YYLTYPE *loc)
{
    assert(dimensions > 0);
    assert(loc != 0);
    return new TreeNode(secType, type, dimensions, *loc);
}

extern "C" void treenode_free(struct TreeNode *node) {
    assert(node != 0);
    delete node;
}

extern "C" NodeType treenode_type(const struct TreeNode *node) {
    assert(node != 0);
    return node->type();
}

extern "C" const YYLTYPE *treenode_location(const struct TreeNode *node) {
    assert(node != 0);
    return &node->location();
}

extern "C" unsigned treenode_numChildren(const struct TreeNode *node) {
    assert(node != 0);
    return node->children().size();
}

extern "C" struct TreeNode *treenode_childAt(const struct TreeNode *node,
                                             unsigned index)
{
    assert(node != 0);
    assert(index < node->children().size());
    return node->children().at(index);
}

extern "C" unsigned treenode_constant(const struct TreeNode *node) {
    assert(node != 0);
    return node->constant();
}

extern "C" unsigned treenode_generated(const struct TreeNode *node) {
    assert(node != 0);
    return node->generated();
}

extern "C" void treenode_appendChild(struct TreeNode *node,
                                     struct TreeNode *child)
{
    assert(node != 0);
    assert(child != 0);
    node->appendChild(child);
}

extern "C" void treenode_setLocation(struct TreeNode *node,
                                     const struct YYLTYPE *loc)
{
    assert(node != 0);
    assert(loc != 0);
    node->setLocation(*loc);
}

extern "C" void treenode_setConstant(struct TreeNode *node, unsigned constant) {
    assert(node != 0);
    node->setConstant(constant);
}

extern "C" void treenode_setGenerated(struct TreeNode *node, unsigned generated)
{
    assert(node != 0);
    node->setGenerated(generated);
}

extern "C" unsigned treenode_value_bool(const struct TreeNode *node) {
    return node->valueBool();
}

extern "C" int treenode_value_int(const struct TreeNode *node) {
    assert(node != 0);
    return node->valueInt();
}

extern "C" unsigned treenode_value_uint(const struct TreeNode *node) {
    assert(node != 0);
    return node->valueUInt();
}

extern "C" const char *treenode_value_string(const struct TreeNode *node) {
    assert(node != 0);
    return node->valueString().c_str();
}

extern "C" enum SecrecSecType treenode_value_secType(
        const struct TreeNode *node)
{
    assert(node != 0);
    return node->valueSecType();
}

extern "C" enum SecrecType treenode_value_type(const struct TreeNode *node) {
    assert(node != 0);
    return node->valueType();
}

extern "C" unsigned treenode_value_dimensions(const struct TreeNode *node) {
    assert(node != 0);
    return node->valueDimensions();
}

extern "C" void treenode_setValue_bool(struct TreeNode *node, unsigned value) {
    assert(node != 0);
    node->setValue(static_cast<bool>(value));
}

extern "C" void treenode_setValue_int(struct TreeNode *node, int value) {
    assert(node != 0);
    node->setValue(value);
}

extern "C" void treenode_setValue_uint(struct TreeNode *node, unsigned value) {
    assert(node != 0);
    node->setValue(value);
}

extern "C" void treenode_setValue_string(struct TreeNode *node,
                                         const char *value)
{
    assert(node != 0);
    assert(value != 0);
    std::string s(value);
    node->setValue(s);
}

extern "C" void treenode_setValue_secType(struct TreeNode *node,
                                          enum SecrecSecType value)
{
    assert(node != 0);
    node->setValue(value);
}

extern "C" void treenode_setValue_type(struct TreeNode *node,
                                       enum SecrecType value)
{
    assert(node != 0);
    node->setValue(value);
}

extern "C" void treenode_setValue_dimensions(struct TreeNode *node,
                                             unsigned value)
{
    assert(node != 0);
    assert(value > 0);
    node->setValueDimensions(value);
}

extern "C" void treenode_print(const struct TreeNode *node, FILE *stream,
                               unsigned indentation)
{
    assert(node != 0);
    fprintf(stream, "%s\n", node->toString(indentation, 0).c_str());
}

extern "C" void treenode_printXml(const struct TreeNode *node, FILE *stream) {
    assert(node != 0);
    fprintf(stream, "%s\n", node->toXml(true).c_str());
}

/*******************************************************************************
  Class TreeNode
*******************************************************************************/

TreeNode::TreeNode(NodeType type, const struct YYLTYPE &loc)
    : m_parent(0), m_type(type), m_location(loc), m_constant(false),
    m_generated(false), m_valueString(0)
{
    // Intentionally empty
}

TreeNode::TreeNode(bool value, const struct YYLTYPE &loc)
    : m_parent(0), m_type(NODE_LITE_BOOL), m_location(loc), m_constant(false),
    m_generated(false), m_valueBool(value)
{
    // Intentionally empty
}

TreeNode::TreeNode(int value, const struct YYLTYPE &loc)
    : m_parent(0), m_type(NODE_LITE_INT), m_location(loc), m_constant(false),
    m_generated(false), m_valueInt(value)
{
    // Intentionally empty
}

TreeNode::TreeNode(unsigned value, const struct YYLTYPE &loc)
    : m_parent(0), m_type(NODE_LITE_UINT), m_location(loc), m_constant(false),
    m_generated(false), m_valueUInt(value)
{
    // Intentionally empty
}

TreeNode::TreeNode(const std::string &value, const struct YYLTYPE &loc)
    : m_parent(0), m_type(NODE_LITE_STRING), m_location(loc), m_constant(false),
    m_generated(false), m_valueString(new std::string(value))
{
    // Intentionally empty
}

TreeNode::TreeNode(SecrecSecType secType, SecrecType type, const YYLTYPE &loc)
    : m_parent(0), m_type(NODE_TYPE), m_location(loc), m_constant(false),
    m_generated(false)
{
    m_valueType.secType = secType;
    m_valueType.type = type;
}

TreeNode::TreeNode(SecrecSecType secType, SecrecType type, unsigned dimensions,
                   const YYLTYPE &loc)
    : m_parent(0), m_type(NODE_VTYPE), m_location(loc), m_constant(false),
    m_generated(false)
{
    assert(dimensions > 0);
    m_valueType.secType = secType;
    m_valueType.type = type;
    m_valueType.dimensions = dimensions;
}

TreeNode::~TreeNode() {
    // Delete all children:
    while (!m_children.empty()) {
        TreeNode *child = m_children.back();
        if (child != 0 && child->m_parent == this) {
            child->m_parent = 0;
            delete child;
        }
        m_children.pop_back();
        std::replace(m_children.begin(), m_children.end(),
                     child, static_cast<TreeNode*>(0));
    }

    switch (m_type) {
        case NODE_INTERNAL_USE:
        case NODE_IDENTIFIER:
        case NODE_LITE_STRING:
            if (m_valueString != 0) {
                delete m_valueString;
            }
            break;
        default:
            break;
    }

    if (m_parent != 0) {
        m_parent->replaceChildren(this);
    }
}

void TreeNode::appendChild(TreeNode *child, bool reparent) {
    assert(child != 0);
    m_children.push_back(child);
    if (reparent) {
        if (child->m_parent != 0) {
            child->m_parent->replaceChildren(child);
        }
        child->m_parent = this;
    }
}

void TreeNode::setLocation(const YYLTYPE &location) {
    m_location = location;
}

void TreeNode::setConstant(bool constant) {
    m_constant = constant;
}

void TreeNode::setGenerated(bool generated) {
    m_generated = generated;
}

const std::string &TreeNode::valueString() const {
    assert(m_valueString != 0);
    return *m_valueString;
}

void TreeNode::setValue(bool value) {
    m_valueBool = value;
}

void TreeNode::setValue(int value) {
    m_valueInt = value;
}

void TreeNode::setValue(unsigned value) {
    m_valueUInt = value;
}

void TreeNode::setValue(const std::string &value) {
    if (m_valueString != 0) {
        *m_valueString = value;
    } else {
        m_valueString = new std::string(value);
    }
}

void TreeNode::setValue(SecrecSecType value) {
    m_valueType.secType = value;
}

void TreeNode::setValue(SecrecType value) {
    m_valueType.type = value;
}

void TreeNode::setValueDimensions(unsigned value) {
    assert(value > 0);
    m_valueType.dimensions = value;
}

const char *TreeNode::nodeTypeName(NodeType type) {
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
        case NODE_TYPE: return "TYPE";
        case NODE_VTYPE: return "VTYPE";
        case NODE_FUNDEF: return "FUNDEF";
        case NODE_FUNDEF_PARAM: return "FUNDEF_PARAM";
        case NODE_FUNDEFS: return "FUNDEFS";
        case NODE_PROGRAM: return "PROGRAM";
        default: return "UNKNOWN";
    }
}

const char *TreeNode::varTypeName(SecrecType type) {
    switch (type) {
        case TYPE_VOID: return "void";
        case TYPE_BOOL: return "bool";
        case TYPE_INT: return "int";
        case TYPE_UINT: return "unsigned int";
        case TYPE_STRING: return "string";
        default: return "<unknown>";
    }
}

const char *TreeNode::secTypeName(SecrecSecType type) {
    if (type == SECTYPE_PRIVATE) return "private";

    assert(type == SECTYPE_PUBLIC);
    return "public";
}

std::string TreeNode::toString(unsigned indent, unsigned startIndent)
        const
{
    std::ostringstream os;

    // Indent:
    for (unsigned i = 0; i < startIndent; i++) {
        os << ' ';
    }

    os << nodeTypeName(m_type);
    switch (m_type) {
        case NODE_IDENTIFIER:
        case NODE_LITE_STRING:
            os << " \"" << valueString() << "\"";
            break;
        case NODE_LITE_BOOL:
            os << ' ' << (m_valueBool ? "true" : "false");
            break;
        case NODE_LITE_INT:
            os << ' ' << m_valueInt;
            break;
        case NODE_LITE_UINT:
            os << ' ' << m_valueUInt;
            break;
        case NODE_TYPE:
            os << ' ' << secTypeName(valueSecType())
               << ' ' << varTypeName(valueType());
            break;
        case NODE_VTYPE:
            os << ' ' << secTypeName(valueSecType())
               << ' ' << varTypeName(valueType());
            for (unsigned i = 0; i < valueDimensions(); i++) {
                os << "[]";
            }
            break;
        default:
            break;
    }

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
    os << '<' << nodeTypeName(m_type);

    switch (m_type) {
        case NODE_IDENTIFIER:
        case NODE_LITE_STRING:
            os << " value=\"string:" << xmlEncode(valueString()) << '"';
            break;
        case NODE_LITE_BOOL:
            os << " value=\"bool:" << (m_valueBool ? "true" : "false") << '"';
            break;
        case NODE_LITE_INT:
            os << " value=\"int:" << m_valueInt << '"';
            break;
        case NODE_LITE_UINT:
            os << " value=\"uint:" << m_valueUInt << '"';
            break;
        case NODE_TYPE:
            os << " value=\"type:" << secTypeName(valueSecType()) << ' '
                                   << varTypeName(valueType()) << '"';
            break;
        case NODE_VTYPE:
            os << " value=\"vtype:" << secTypeName(valueSecType()) << ' '
                                    << varTypeName(valueType()) << ' '
                                    << valueDimensions() << '"';
            break;
        default:
            break;
    }

    if (m_children.empty()) {
        os << "/>";
    } else {
        os << '>';
        for (unsigned i = 0; i < m_children.size(); i++) {
            os << m_children.at(i)->toXml(false);
        }
        os << "</" << nodeTypeName(m_type) << '>';
    }
    return os.str();
}

void TreeNode::replaceChildren(TreeNode *child) {
    typedef std::deque<TreeNode*>::iterator TDI;

    for (TDI it = m_children.begin(); it != m_children.end(); it++) {
        if (*it == child) {
            *it = new TreeNode(NODE_INTERNAL_USE, m_location);
            (*it)->setValue(std::string(TreeNode::NOCHILD));
            (*it)->m_parent = this;
        }
    }
}
