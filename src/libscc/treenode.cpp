#include "treenode.h"

#include <algorithm>
#include <sstream>

#include "symboltable.h"
#include "typechecker.h"
#include "misc.h"
#include "context.h"


namespace SecreC {

TreeNode::TreeNode(SecrecTreeNodeType type, const struct YYLTYPE &loc)
    : m_parent (0)
    , m_procedure (0)
    , m_type (type)
    , m_location (loc)
{
    // Intentionally empty
}

TreeNode::~TreeNode() {
    typedef ChildrenListConstIterator CLCI;
    for (CLCI it(m_children.begin()); it != m_children.end(); it++) {
        assert((*it) != 0);
        if ((*it)->m_parent == this) {
            delete *it;
        }
    }
}

TreeNodeProcDef* TreeNode::containingProcedure() {
    if (m_procedure != 0) return m_procedure;
    if (m_parent != 0) {
        return (m_procedure = m_parent->containingProcedure());
    }
    return 0;
}

void TreeNode::appendChild(TreeNode *child) {
    assert(child != 0);
    m_children.push_back(child);
    child->resetParent(this);
}

void TreeNode::prependChild(TreeNode *child) {
    assert(child != 0);
    m_children.push_front(child);
    child->resetParent(this);
}

void TreeNode::setLocation(const YYLTYPE &location) {
    m_location = location;
}

const char *TreeNode::typeName(SecrecTreeNodeType type) {
    switch (type) {
        case NODE_INTERNAL_USE: return "INTERNAL_USE";

        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_LITE_BOOL: return "BOOL";
        case NODE_LITE_INT: return "INT";
        case NODE_LITE_UINT: return "UINT";
        case NODE_LITE_STRING: return "STRING";
        case NODE_EXPR_NONE: return "EXPR_NONE";
        case NODE_EXPR_CLASSIFY: return "EXPR_CLASSIFY";
        case NODE_EXPR_DECLASSIFY: return "EXPR_DECLASSIFY";
        case NODE_EXPR_PROCCALL: return "EXPR_PROCCALL";
        case NODE_EXPR_INDEX: return "EXPR_INDEX";
        case NODE_EXPR_UNEG: return "EXPR_UNEG";
        case NODE_EXPR_UMINUS: return "EXPR_UMINUS";
        case NODE_EXPR_POSTFIX_INC: return "EXPR_POSTFIX_INC";
        case NODE_EXPR_POSTFIX_DEC: return "EXPR_POSTFIX_DEC";
        case NODE_EXPR_PREFIX_INC: return "EXPR_PREFIX_INC";
        case NODE_EXPR_PREFIX_DEC: return "EXPR_PREFIX_DEC";
        case NODE_EXPR_CAST: return "EXPR_CAST";
        case NODE_EXPR_BINARY_MATRIXMUL: return "EXPR_MATRIXMUL";
        case NODE_EXPR_BINARY_MUL: return "EXPR_MUL";
        case NODE_EXPR_BINARY_DIV: return "EXPR_DIV";
        case NODE_EXPR_BINARY_MOD: return "EXPR_MOD";
        case NODE_EXPR_BINARY_ADD: return "EXPR_ADD";
        case NODE_EXPR_BINARY_SUB: return "EXPR_SUB";
        case NODE_EXPR_BINARY_EQ: return "EXPR_EQ";
        case NODE_EXPR_BINARY_NE: return "EXPR_NE";
        case NODE_EXPR_BINARY_LE: return "EXPR_LE";
        case NODE_EXPR_BINARY_GT: return "EXPR_GT";
        case NODE_EXPR_BINARY_GE: return "EXPR_GE";
        case NODE_EXPR_BINARY_LT: return "EXPR_LT";
        case NODE_EXPR_BINARY_LAND: return "EXPR_LAND";
        case NODE_EXPR_BINARY_LOR: return "EXPR_LOR";
        case NODE_EXPR_TERNIF: return "EXPR_TERNIF";
        case NODE_EXPR_ASSIGN_MUL: return "EXPR_ASSIGN_MUL";
        case NODE_EXPR_ASSIGN_DIV: return "EXPR_ASSIGN_DIV";
        case NODE_EXPR_ASSIGN_MOD: return "EXPR_ASSIGN_MOD";
        case NODE_EXPR_ASSIGN_ADD: return "EXPR_ASSIGN_ADD";
        case NODE_EXPR_ASSIGN_SUB: return "EXPR_ASSIGN_SUB";
        case NODE_EXPR_ASSIGN: return "EXPR_ASSIGN";
        case NODE_EXPR_RVARIABLE: return "RVARIABLE";
        case NODE_EXPR_SIZE: return "SIZE";
        case NODE_EXPR_SHAPE: return "SHAPE";
        case NODE_EXPR_CAT: return "CAT";
        case NODE_EXPR_RESHAPE: return "RESHAPE";
        case NODE_STMT_IF: return "STMT_IF";
        case NODE_STMT_FOR: return "STMT_FOR";
        case NODE_STMT_WHILE: return "STMT_WHILE";
        case NODE_STMT_DOWHILE: return "STMT_DOWHILE";
        case NODE_STMT_COMPOUND: return "STMT_COMPOUND";
        case NODE_STMT_RETURN: return "STMT_RETURN";
        case NODE_STMT_CONTINUE: return "STMT_CONTINUE";
        case NODE_STMT_BREAK: return "STMT_BREAK";
        case NODE_STMT_EXPR: return "STMT_EXPR";
        case NODE_STMT_ASSERT: return "STMT_ASSERT";
        case NODE_STMT_PRINT: return "NODE_STMT_PRINT";
        case NODE_DECL: return "DECL";
        case NODE_PROCDEF: return "PROCDEF";
        case NODE_PROGRAM: return "PROGRAM";
        case NODE_DIMENSIONS: return "DIMENSIONS";
        case NODE_INDEX_INT: return "NODE_INDEX_INT";
        case NODE_INDEX_SLICE: return "NODE_INDEX_SLICE";
        case NODE_LVALUE: return "NODE_LVALUE";
        case NODE_SUBSCRIPT: return "NODE_SUBSCRIPT";
        case NODE_TYPETYPE: return "TYPETYPE";
        case NODE_TYPEVOID: return "TYPEVOID";
        case NODE_DATATYPE_F: return "DATATYPE_F";
        case NODE_DIMTYPE_F: return "DIMTYPE_F";
        case NODE_SECTYPE_F: return "SECTYPE_F";
        case NODE_KIND: return "KIND";
        case NODE_DOMAIN: return "DOMAIN";
        case NODE_TEMPLATE_DECL: return "TEMPLATE_DECL";
        case NODE_TEMPLATE_QUANT: return "TEMPLATE_QUANT";
        case NODE_STMT_SYSCALL: return "STMT_SYSCALL";
        case NODE_STMT_PUSH: return "STMT_PUSH";
        case NODE_STMT_PUSHREF: return "STMT_PUSHREF";
        case NODE_STMT_PUSHCREF: return "STMT_PUSHCREF";
        case NODE_EXPR_DOMAINID: return "EXPR_DOMAINID";
        default: return "UNKNOWN";
    }
}

std::string TreeNode::toString(unsigned indent, unsigned startIndent) const {
    typedef ChildrenListConstIterator CLCI;

    std::ostringstream os;

    os << std::string (startIndent, ' ');
    os << typeName(m_type);

    const std::string sh(stringHelper());
    if (!sh.empty())
        os << ' ' << sh;

    os << " at " << location();

    for (CLCI it(m_children.begin()); it != m_children.end(); it++) {
        os << std::endl;
        assert((*it)->parent() == this);
        os << (*it)->toString(indent, startIndent + indent);
    }
    return os.str();
}

std::string TreeNode::toXml(bool full) const {
    typedef ChildrenListConstIterator CLCI;

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
        for (CLCI it(m_children.begin()); it != m_children.end(); it++) {
            assert((*it)->parent() == this);
            os << (*it)->toXml(false);
        }
        os << "</" << typeName(m_type) << '>';
    }
    return os.str();
}

/*******************************************************************************
  C interface for Yacc
*******************************************************************************/

SecreC::TreeNode *treenode_init(enum SecrecTreeNodeType type,
                                const YYLTYPE *loc)
{
    switch (type) {
        case NODE_PROGRAM:
            return (TreeNode*) (new SecreC::TreeNodeProgram(*loc));
        case NODE_PROCDEF:
            return (TreeNode*) (new SecreC::TreeNodeProcDef(*loc));
        case NODE_EXPR_UNEG:     /* Fall through: */
        case NODE_EXPR_UMINUS:
            return (TreeNode*) (new SecreC::TreeNodeExprUnary(type, *loc));
        case NODE_EXPR_POSTFIX_INC: /* Fall through: */
        case NODE_EXPR_POSTFIX_DEC:
            return (TreeNode*) (new SecreC::TreeNodeExprPostfix(type, *loc));
        case NODE_EXPR_PREFIX_INC: /* Fall through: */
        case NODE_EXPR_PREFIX_DEC:
            return (TreeNode*) (new SecreC::TreeNodeExprPrefix(type, *loc));
        case NODE_EXPR_BINARY_MATRIXMUL:  /* Fall through: */
        case NODE_EXPR_BINARY_MUL:        /* Fall through: */
        case NODE_EXPR_BINARY_DIV:        /* Fall through: */
        case NODE_EXPR_BINARY_MOD:        /* Fall through: */
        case NODE_EXPR_BINARY_ADD:        /* Fall through: */
        case NODE_EXPR_BINARY_SUB:        /* Fall through: */
        case NODE_EXPR_BINARY_EQ:         /* Fall through: */
        case NODE_EXPR_BINARY_NE:         /* Fall through: */
        case NODE_EXPR_BINARY_LE:         /* Fall through: */
        case NODE_EXPR_BINARY_GT:         /* Fall through: */
        case NODE_EXPR_BINARY_GE:         /* Fall through: */
        case NODE_EXPR_BINARY_LT:         /* Fall through: */
        case NODE_EXPR_BINARY_LAND:       /* Fall through: */
        case NODE_EXPR_BINARY_LOR:        /* Fall through: */
            return (TreeNode*) (new SecreC::TreeNodeExprBinary(type, *loc));
        case NODE_EXPR_TERNIF:
            return (TreeNode*) (new SecreC::TreeNodeExprTernary(*loc));
        case NODE_EXPR_ASSIGN_MUL: /* Fall through: */
        case NODE_EXPR_ASSIGN_DIV: /* Fall through: */
        case NODE_EXPR_ASSIGN_MOD: /* Fall through: */
        case NODE_EXPR_ASSIGN_ADD: /* Fall through: */
        case NODE_EXPR_ASSIGN_SUB: /* Fall through: */
        case NODE_EXPR_ASSIGN:
            return (TreeNode*) (new SecreC::TreeNodeExprAssign(type, *loc));
        case NODE_EXPR_DECLASSIFY:
            return (TreeNode*) (new SecreC::TreeNodeExprDeclassify(*loc));
        case NODE_EXPR_PROCCALL:
            return (TreeNode*) (new SecreC::TreeNodeExprProcCall(*loc));
        case NODE_EXPR_RVARIABLE:
            return (TreeNode*) (new SecreC::TreeNodeExprRVariable(*loc));
        case NODE_EXPR_INDEX:
            return (TreeNode*) (new SecreC::TreeNodeExprIndex(*loc));
        case NODE_EXPR_SIZE:
            return (TreeNode*) (new SecreC::TreeNodeExprSize(*loc));
        case NODE_EXPR_SHAPE:
            return (TreeNode*) (new SecreC::TreeNodeExprShape(*loc));
        case NODE_EXPR_CAT:
            return (TreeNode*) (new SecreC::TreeNodeExprCat(*loc));
        case NODE_EXPR_RESHAPE:
            return (TreeNode*) (new SecreC::TreeNodeExprReshape(*loc));
        case NODE_STMT_BREAK:
            return (TreeNode*) (new SecreC::TreeNodeStmtBreak(*loc));
        case NODE_STMT_CONTINUE:
            return (TreeNode*) (new SecreC::TreeNodeStmtContinue(*loc));
        case NODE_STMT_COMPOUND:
            return (TreeNode*) (new SecreC::TreeNodeStmtCompound(*loc));
        case NODE_STMT_DOWHILE:
            return (TreeNode*) (new SecreC::TreeNodeStmtDoWhile(*loc));
        case NODE_STMT_EXPR:
            return (TreeNode*) (new SecreC::TreeNodeStmtExpr(*loc));
        case NODE_STMT_ASSERT:
            return (TreeNode*) (new SecreC::TreeNodeStmtAssert(*loc));
        case NODE_STMT_FOR:
            return (TreeNode*) (new SecreC::TreeNodeStmtFor(*loc));
        case NODE_STMT_IF:
            return (TreeNode*) (new SecreC::TreeNodeStmtIf(*loc));
        case NODE_STMT_RETURN:
            return (TreeNode*) (new SecreC::TreeNodeStmtReturn(*loc));
        case NODE_STMT_WHILE:
            return (TreeNode*) (new SecreC::TreeNodeStmtWhile(*loc));
        case NODE_STMT_PRINT:
            return (TreeNode*) (new SecreC::TreeNodeStmtPrint(*loc));
        case NODE_STMT_SYSCALL:
            return (TreeNode*) (new SecreC::TreeNodeStmtSyscall(*loc));
        case NODE_STMT_PUSH:
            return (TreeNode*) (new SecreC::TreeNodeStmtPush(*loc));
        case NODE_STMT_PUSHREF:
            return (TreeNode*) (new SecreC::TreeNodeStmtPushRef(*loc, false));
        case NODE_STMT_PUSHCREF:
            return (TreeNode*) (new SecreC::TreeNodeStmtPushRef(*loc, true));
        case NODE_EXPR_DOMAINID:
            return (TreeNode*) (new SecreC::TreeNodeExprDomainID(*loc));
        case NODE_DECL:
            return (TreeNode*) (new SecreC::TreeNodeStmtDecl(*loc));
        case NODE_TYPETYPE:
            return (TreeNode*) (new SecreC::TreeNodeTypeType(*loc));
        case NODE_TYPEVOID:
            return (TreeNode*) (new SecreC::TreeNodeTypeVoid(*loc));
        case NODE_EXPR_CAST:
            return (TreeNode*) (new SecreC::TreeNodeExprCast (*loc));
        case NODE_KIND:
            return (TreeNode*) (new SecreC::TreeNodeKind (*loc));
        case NODE_DOMAIN:
            return (TreeNode*) (new SecreC::TreeNodeDomain (*loc));
        case NODE_TEMPLATE_QUANT:
            return (TreeNode*) (new SecreC::TreeNodeQuantifier (*loc));
        case NODE_TEMPLATE_DECL:
            return (TreeNode*) (new SecreC::TreeNodeTemplate (*loc));
        default:
            assert(type != NODE_IDENTIFIER);
            assert((type & NODE_LITE_MASK) == 0x0);
            return (TreeNode*) (new SecreC::TreeNode(type, *loc));
    }
}

void treenode_free(TreeNode *node) {
    delete ((SecreC::TreeNode*) node);
}

enum SecrecTreeNodeType treenode_type(TreeNode *node) {
    return ((const SecreC::TreeNode*) node)->type();
}

const YYLTYPE *treenode_location(const TreeNode *node) {
    return &((const SecreC::TreeNode*) node)->location();
}

unsigned treenode_numChildren(const TreeNode *node) {
    return ((const SecreC::TreeNode*) node)->children().size();
}

TreeNode *treenode_childAt(const TreeNode *node,
                                             unsigned index)
{
    return (TreeNode*) ((const SecreC::TreeNode*) node)->children().at(index);
}

void treenode_appendChild(TreeNode *parent,
                                     TreeNode *child)
{
    return ((SecreC::TreeNode*) parent)->appendChild((SecreC::TreeNode*) child);
}

void treenode_prependChild(TreeNode *parent,
                                      TreeNode *child)
{
    typedef SecreC::TreeNode TN;
    return ((TN*) parent)->prependChild((TN*) child);
}

void treenode_setLocation(TreeNode *node, YYLTYPE *loc) {
    return ((SecreC::TreeNode*) node)->setLocation(*loc);
}

TreeNode *treenode_init_bool(unsigned value, YYLTYPE *loc) {

    return (TreeNode*) new SecreC::TreeNodeExprBool(value, *loc);
}

TreeNode *treenode_init_int(int value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeExprInt(value, *loc);
}

TreeNode *treenode_init_uint(unsigned value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeExprUInt(value, *loc);
}

TreeNode *treenode_init_string(const char *value,
                                                 YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeExprString(value, *loc);
}

TreeNode *treenode_init_identifier(const char *value,
                                                     YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeIdentifier(value, *loc);
}

TreeNode *treenode_init_publicSecTypeF (YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeSecTypeF(true, *loc);
}

TreeNode *treenode_init_privateSecTypeF(YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeSecTypeF (false, *loc);
}

TreeNode *treenode_init_dataTypeF(
        enum SecrecDataType dataType,
        YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeDataTypeF(dataType, *loc);
}

TreeNode *treenode_init_dimTypeF(
        unsigned dimType,
        YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeDimTypeF(dimType, *loc);
}

/*******************************************************************************
  TreeNodeDataTypeF
*******************************************************************************/

std::string TreeNodeDataTypeF::stringHelper() const {
    std::ostringstream os;
    os << m_dataType;
    return os.str();
}

std::string TreeNodeDataTypeF::xmlHelper() const {
    std::ostringstream os;
    os << "type=\"" << m_dataType << "\"";
    return os.str();
}

/*******************************************************************************
  TreeNodeDimTypeF
*******************************************************************************/

std::string TreeNodeDimTypeF::stringHelper() const {
    std::ostringstream os;
    os << m_dimType;
    return os.str();
}

std::string TreeNodeDimTypeF::xmlHelper() const {
    std::ostringstream os;
    os << "dim=\"" << m_dimType << "\"";
    return os.str();
}

/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/

const char *TreeNodeExprBinary::operatorString() const {
    switch (type()) {
        case NODE_EXPR_BINARY_ADD:  return "+";
        case NODE_EXPR_BINARY_SUB:  return "-";
        case NODE_EXPR_BINARY_MUL:  return "*";
        case NODE_EXPR_BINARY_MOD:  return "%";
        case NODE_EXPR_BINARY_DIV:  return "/";
        case NODE_EXPR_BINARY_EQ:   return "==";
        case NODE_EXPR_BINARY_GE:   return ">=";
        case NODE_EXPR_BINARY_GT:   return ">";
        case NODE_EXPR_BINARY_LE:   return "<=";
        case NODE_EXPR_BINARY_LT:   return "<";
        case NODE_EXPR_BINARY_NE:   return "!=";
        case NODE_EXPR_BINARY_LAND: return "&&";
        case NODE_EXPR_BINARY_LOR:  return "||";
        case NODE_EXPR_BINARY_MATRIXMUL: return "#";
        default:
            assert(false); // shouldn't happen
    }

    return "?";
}

/*******************************************************************************
  TreeNodeExprBool
*******************************************************************************/

std::string TreeNodeExprBool::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"bool:" << stringHelper() << "\"";
    return os.str();
}

/*******************************************************************************
  TreeNodeExprString
*******************************************************************************/

std::string TreeNodeExprString::stringHelper() const {
    std::ostringstream os;
    os << "\"" << m_value << "\"";
    return os.str();
}

std::string TreeNodeExprString::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"string:" << xmlEncode(m_value) << "\"";
    return os.str();
}
/*******************************************************************************
  TreeNodeExprInt
*******************************************************************************/

std::string TreeNodeExprInt::stringHelper() const {
    std::ostringstream os;
    os << m_value;
    return os.str();
}

std::string TreeNodeExprInt::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"int:" << m_value << "\"";
    return os.str();
}

/*******************************************************************************
  TreeNodeExprUInt
*******************************************************************************/

std::string TreeNodeExprUInt::stringHelper() const {
    std::ostringstream os;
    os << m_value;
    return os.str();
}

std::string TreeNodeExprUInt::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"uint:" << m_value << "\"";
    return os.str();
}

/*******************************************************************************
  TreeNodeProcDef
*******************************************************************************/

const std::string &TreeNodeProcDef::procedureName() const {
    assert(children().size() >= 3);
    assert(dynamic_cast<const TreeNodeIdentifier*>(children().at(0)) != 0);
    return static_cast<const TreeNodeIdentifier*>(children().at(0))->value();
}

/*******************************************************************************
  TreeNodeIdentifier
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
  TreeNodeSecTypeF
*******************************************************************************/

std::string TreeNodeSecTypeF::stringHelper() const {
    std::ostringstream os;
    if (m_isPublic)
        os << "public";
    else
        os << static_cast<TreeNodeIdentifier*>(children ().at (0))->value ();
    return os.str();
}

std::string TreeNodeSecTypeF::xmlHelper() const {
    std::ostringstream os;
    os << "type=\"" << stringHelper () << "\"";
    return os.str();
}

/*******************************************************************************
  TreeNodeStmtDecl
*******************************************************************************/

const std::string &TreeNodeStmtDecl::variableName() const {
    typedef TreeNodeIdentifier TNI;

    assert(children().size() > 0 && children().size() <= 4);
    assert(children().at(0)->type() == NODE_IDENTIFIER);

    assert(dynamic_cast<TNI*>(children().at(0)) != 0);
    return static_cast<TNI*>(children().at(0))->value();
}

/*******************************************************************************
  TreeNodeTypeType
*******************************************************************************/

std::string TreeNodeTypeType::stringHelper() const {
    if (m_cachedType != 0)
        return secrecType()->toString();
    return "";
}

/*******************************************************************************
  TreeNodeStmt
*******************************************************************************/

SecurityType* TreeNodeStmt::returnSecurityType () {
    return containingProcedure ()->procedureType ()->secrecSecType ();
}

} // namespace SecreC
