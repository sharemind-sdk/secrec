#include "treenode.h"

#include <algorithm>
#include <sstream>
#include <boost/foreach.hpp>

#include "symboltable.h"
#include "typechecker.h"
#include "misc.h"
#include "context.h"

namespace {

using namespace SecreC;

template <class T >
T* childAt (const TreeNode* node, unsigned i) {
    assert (i < node->children ().size ());
    assert (dynamic_cast<T*>(node->children ().at (i)) != 0);
    return static_cast<T*>(node->children ().at (i));
}

TreeNodeStmt* statementAt (const TreeNode* node, unsigned i) {
    return childAt<TreeNodeStmt>(node, i);
}

TreeNodeExpr* expressionAt (const TreeNode* node, unsigned i) {
    return childAt<TreeNodeExpr>(node, i);
}

}

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
    BOOST_FOREACH (TreeNode* child, m_children) {
        assert (child != 0);
        if (child->m_parent == this) {
            delete child; // that's kinda sad
        }
    }
}

TreeNode* TreeNode::clone (TreeNode* parent) const {
    TreeNode* out = cloneV ();

    if (parent) {
        out->resetParent (parent);
    }

    BOOST_FOREACH (TreeNode* n, m_children) {
        out->m_children.push_back (n->clone (out));
    }

    return out;
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
        case NODE_LITE_STRING: return "STRING";
        case NODE_LITE_FLOAT: return "FLOAT";
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
        case NODE_EXPR_TOSTRING: return "TOSTRING";
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
        case NODE_VAR_INIT: return "VAR_INIT";
        case NODE_MODULE: return "MODULE";
        case NODE_IMPORT: return "IMPORT";
        default: return "UNKNOWN";
    }
}

std::string TreeNode::toString(unsigned indent, unsigned startIndent) const {
    std::ostringstream os;

    os << std::string (startIndent, ' ');
    os << typeName(m_type);

    const std::string sh(stringHelper());
    if (!sh.empty())
        os << ' ' << sh;

    os << " at " << location();

    BOOST_FOREACH (TreeNode* child, m_children) {
        os << std::endl;
        assert (child->parent() == this);
        os << child->toString(indent, startIndent + indent);
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
        BOOST_FOREACH (TreeNode* child, m_children) {
            assert(child->parent() == this);
            os << child->toXml(false);
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
        case NODE_EXPR_TOSTRING:
            return (TreeNode*) (new SecreC::TreeNodeExprToString(*loc));
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
        case NODE_EXPR_TYPE_QUAL:
            return (TreeNode*) (new SecreC::TreeNodeExprQualified (*loc));
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
        case NODE_VAR_INIT:
            return (TreeNode*) (new SecreC::TreeNodeVarInit (*loc));
        case NODE_MODULE:
            return (TreeNode*) (new SecreC::TreeNodeModule (*loc));
        case NODE_IMPORT:
            return (TreeNode*) (new SecreC::TreeNodeImport (*loc));
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

void treenode_moveChildren (TreeNode* from, TreeNode* to) {
    BOOST_FOREACH (TreeNode* child, from->children ()) {
        to->appendChild (child);
    }

    from->children ().clear ();
}

TreeNode *treenode_init_bool(unsigned value, YYLTYPE *loc) {

    return (TreeNode*) new SecreC::TreeNodeExprBool(value, *loc);
}

TreeNode *treenode_init_int(int value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeExprInt(value, *loc);
}

TreeNode *treenode_init_string(const char *value,
                                                 YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeExprString(value, *loc);
}

TreeNode *treenode_init_float(const char *value,
                                                 YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeExprFloat(value, *loc);
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

TreeNode *treenode_init_opdef(enum SecrecOperator op, YYLTYPE *loc) {
    SecreC::TreeNode* node = new SecreC::TreeNodeOpDef (op, *loc);
    std::ostringstream os;
    os << "__operator" << op;
    treenode_appendChild (node, SecreC::treenode_init_identifier (os.str ().c_str (), loc));
    return node;
}

/*******************************************************************************
  TreeNodeSecTypeF
*******************************************************************************/

void TreeNodeSecTypeF::setCachedType (SecurityType* ty) {
    assert (m_cachedType == 0);
    assert (ty != 0);
    m_cachedType = ty;
}

TreeNodeIdentifier* TreeNodeSecTypeF::identifier () const {
    assert (children ().size () == 1 &&
            "Called on public type?");
    return childAt<TreeNodeIdentifier>(this, 0);
}

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
  TreeNodeType
*******************************************************************************/

SecreC::Type* TreeNodeType::secrecType () const {
    assert (m_cachedType != 0);
    return m_cachedType;
}

TreeNodeSecTypeF* TreeNodeType::secType () const {
    assert (children().size() == 3);
    return childAt<TreeNodeSecTypeF>(this, 0);
}

TreeNodeDataTypeF* TreeNodeType::dataType () const {
    assert (children().size() == 3);
    return childAt<TreeNodeDataTypeF>(this, 1);
}

TreeNodeDimTypeF* TreeNodeType::dimType () const {
    assert (children().size() == 3);
    return childAt<TreeNodeDimTypeF>(this, 2);
}

bool TreeNodeType::isNonVoid () const {
    assert (children ().size () == 3 || children ().empty ());
    return ! children ().empty ();
}

/*******************************************************************************
  OverloadableOperator
*******************************************************************************/

std::string OverloadableOperator::operatorName () const {
    std::ostringstream os;
    os << "__operator" << getOperator ();
    return os.str ();
}

/*******************************************************************************
  TreeNodeExpr
*******************************************************************************/

bool TreeNodeExpr::havePublicBoolType() const {
    assert(m_resultType != 0);
    return m_resultType->secrecDataType() == DATATYPE_BOOL
            && m_resultType->secrecSecType()->isPublic ()
            && m_resultType->isScalar();
}

SecreC::Type* TreeNodeExpr::resultType() const {
    assert(m_resultType != 0);
    return m_resultType;
}

void TreeNodeExpr::setResultType (SecreC::Type *type) {
    assert(m_resultType == 0);
    m_resultType = type;
}

/*******************************************************************************
  TreeNodeExprUnary
*******************************************************************************/

TreeNodeExpr* TreeNodeExprUnary::expression () const {
    return expressionAt (this, 0);
}


SecrecOperator TreeNodeExprUnary::getOperatorV () const {
    switch (type ()) {
        case NODE_EXPR_UNEG: return SCOP_UN_NEG;
        case NODE_EXPR_UMINUS: return SCOP_UN_MINUS;
        default: return SCOP_NONE;
    }
}

/*******************************************************************************
  OverloadableOperator
*******************************************************************************/

SecrecOperator OverloadableOperator::getOperator () const {
    if (m_operator == SCOP_NONE) { // unlikeley
        m_operator = getOperatorV ();
        assert ((m_operator != SCOP_NONE) &&
                "getOperatorV returned undefined operator.");
    }

    return m_operator;
}

/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/

TreeNodeExpr* TreeNodeExprBinary::leftExpression () const {
    assert (children ().size () == 2);
    return expressionAt (this, 0);
}

TreeNodeExpr* TreeNodeExprBinary::rightExpression () const {
    assert (children ().size () == 2);
    return expressionAt (this, 1);
}

const char *TreeNodeExprBinary::operatorString() const {
    switch (getOperator ()) {
        case SCOP_BIN_ADD:  return "+";
        case SCOP_BIN_SUB:  return "-";
        case SCOP_BIN_MUL:  return "*";
        case SCOP_BIN_MOD:  return "%";
        case SCOP_BIN_DIV:  return "/";
        case SCOP_BIN_EQ:   return "==";
        case SCOP_BIN_GE:   return ">=";
        case SCOP_BIN_GT:   return ">";
        case SCOP_BIN_LE:   return "<=";
        case SCOP_BIN_LT:   return "<";
        case SCOP_BIN_NE:   return "!=";
        case SCOP_BIN_LAND: return "&&";
        case SCOP_BIN_LOR:  return "||";
        default:
            assert(false); // shouldn't happen
    }

    return "?";
}

SecrecOperator TreeNodeExprBinary::getOperatorV () const {
    switch (type ()) {
        case NODE_EXPR_BINARY_ADD:  return SCOP_BIN_ADD;
        case NODE_EXPR_BINARY_SUB:  return SCOP_BIN_SUB;
        case NODE_EXPR_BINARY_MUL:  return SCOP_BIN_MUL;
        case NODE_EXPR_BINARY_MOD:  return SCOP_BIN_MOD;
        case NODE_EXPR_BINARY_DIV:  return SCOP_BIN_DIV;
        case NODE_EXPR_BINARY_EQ:   return SCOP_BIN_EQ;
        case NODE_EXPR_BINARY_GE:   return SCOP_BIN_GE;
        case NODE_EXPR_BINARY_GT:   return SCOP_BIN_GT;
        case NODE_EXPR_BINARY_LE:   return SCOP_BIN_LE;
        case NODE_EXPR_BINARY_LT:   return SCOP_BIN_LT;
        case NODE_EXPR_BINARY_NE:   return SCOP_BIN_NE;
        case NODE_EXPR_BINARY_LAND: return SCOP_BIN_LAND;
        case NODE_EXPR_BINARY_LOR:  return SCOP_BIN_LOR;
        default:
            assert (false); // shouldn't happen
    }

    return SCOP_NONE;
}

/*******************************************************************************
  TreeNodeExprClassify
*******************************************************************************/

TreeNodeExpr* TreeNodeExprClassify::expression () const {
    assert (children ().size () == 1);
    return expressionAt (this, 0);
}

TreeNode* TreeNodeExprClassify::cloneV () const {
    assert (false && "ICE: Classify nodes are created during type checking and "
            "it's assumed that procedures are cloned before type checking is performed.");
    return new TreeNodeExprClassify (m_contextSecType, m_location);
}

/*******************************************************************************
  TreeNodeExprDeclassify
*******************************************************************************/

TreeNodeExpr* TreeNodeExprDeclassify::expression () const {
    assert (children ().size () == 1);
    return expressionAt (this, 0);
}

/*******************************************************************************
  TreeNodeExprProcCall
*******************************************************************************/

TreeNodeIdentifier* TreeNodeExprProcCall::procName () const {
    assert (!children ().empty ());
    return childAt<TreeNodeIdentifier>(this, 0);
}

TreeNode::ChildrenListConstRange TreeNodeExprProcCall::paramRange () const {
    assert (!children ().empty ());
    return std::make_pair (++ children ().begin (),
                              children ().end ());
}

/*******************************************************************************
  TreeNodeExprRVariable
*******************************************************************************/

TreeNodeIdentifier* TreeNodeExprRVariable::identifier () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeIdentifier>(this, 0);
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
  TreeNodeExprFloat
*******************************************************************************/

std::string TreeNodeExprFloat::stringHelper() const {
    std::ostringstream os;
    os << m_value ;
    return os.str();
}

std::string TreeNodeExprFloat::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"double:" << m_value << "\"";
    return os.str ();
}

/*******************************************************************************
  TreeNodeExprTernary
*******************************************************************************/

TreeNodeExpr* TreeNodeExprTernary::conditional () const {
    assert(children().size() == 3);
    return expressionAt (this, 0);
}

TreeNodeExpr* TreeNodeExprTernary::trueBranch () const {
    assert(children().size() == 3);
    return expressionAt (this, 1);
}

TreeNodeExpr* TreeNodeExprTernary::falseBranch () const {
    assert(children().size() == 3);
    return expressionAt (this, 2);
}

/*******************************************************************************
  TreeNodeExprAssign
*******************************************************************************/

TreeNode* TreeNodeExprAssign::slice () const {
    assert (children ().size () == 2);
    TreeNode *e1 = children().at(0);
    if (e1->children().size() == 2) {
        return e1->children ().at (1);
    }

    return 0;
}

TreeNodeIdentifier* TreeNodeExprAssign::identifier () const {
    assert (children ().size () == 2);
    TreeNode *e1 = children().at(0);
    assert(e1 != 0);
    assert(e1->type() == NODE_LVALUE);
    assert(e1->children().size() > 0 && e1->children().size() <= 2);
    return childAt<TreeNodeIdentifier>(e1, 0);
}

TreeNodeExpr* TreeNodeExprAssign::rightHandSide () const {
    assert(children().size() == 2);
    return expressionAt (this, 1);
}

/*******************************************************************************
  TreeNodeExprCast
*******************************************************************************/

TreeNodeExpr* TreeNodeExprCast::expression () const {
    assert (children ().size () == 2);
    return expressionAt (this, 1);
}

TreeNodeDataTypeF* TreeNodeExprCast::dataType () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeDataTypeF>(this, 0);
}

/*******************************************************************************
  TreeNodeExprIndex
*******************************************************************************/

TreeNodeExpr* TreeNodeExprIndex::expression () const {
    assert (children().size() == 2);
    return expressionAt (this, 0);
}

TreeNode* TreeNodeExprIndex::indices () const {
    assert (children().size() == 2);
    return children ().at (1);
}

/*******************************************************************************
  TreeNodeExprSize
*******************************************************************************/

TreeNodeExpr* TreeNodeExprSize::expression () const {
    assert (children().size() == 1);
    return expressionAt (this, 0);
}

/*******************************************************************************
  TreeNodeExprShape
*******************************************************************************/

TreeNodeExpr* TreeNodeExprShape::expression () const {
    assert (children().size() == 1);
    return expressionAt (this, 0);
}

/*******************************************************************************
  TreeNodeExprReshape
*******************************************************************************/

TreeNodeExpr* TreeNodeExprReshape::reshapee () const {
    assert (children ().size() >= 1);
    return expressionAt (this, 0);
}

TreeNode::ChildrenListConstRange TreeNodeExprReshape::dimensions () {
    return std::make_pair (++ m_children.begin (),
                              m_children.end ());
}

/*******************************************************************************
  TreeNodeExprToString
*******************************************************************************/

TreeNodeExpr* TreeNodeExprToString::expression () const {
    assert (children ().size() >= 1);
    return expressionAt (this, 0);
}

/*******************************************************************************
  TreeNodeExprCat
*******************************************************************************/

TreeNodeExpr* TreeNodeExprCat::leftExpression () const {
    assert (children().size() == 3 ||
            children().size() == 2);
    return expressionAt (this, 0);
}
TreeNodeExpr* TreeNodeExprCat::rightExpression () const {
    assert (children().size() == 3 ||
            children().size() == 2);
    return expressionAt (this, 1);
}

TreeNodeExprInt* TreeNodeExprCat::dimensionality () const {
    if (children ().size () == 3) {
        return childAt<TreeNodeExprInt>(this, 2);
    }

    return 0;
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
  TreeNodeProcDef
*******************************************************************************/

const std::string &TreeNodeProcDef::procedureName() const {
    return identifier ()->value ();
}

TreeNodeIdentifier* TreeNodeProcDef::identifier () const {
    assert (children().size() >= 3);
    return childAt<TreeNodeIdentifier>(this, 0);
}

TreeNodeType* TreeNodeProcDef::returnType () const {
    assert (children().size() >= 3);
    return childAt<TreeNodeType>(this, 1);
}

TreeNodeStmt* TreeNodeProcDef::body () const {
    assert (children().size() >= 3);
    return childAt<TreeNodeStmt>(this, 2);
}

TreeNode::ChildrenListConstRange TreeNodeProcDef::paramRange () const {
    return std::make_pair (paramBegin (), paramEnd ());
}

TreeNode::ChildrenListConstIterator TreeNodeProcDef::paramBegin () const {
    assert (children ().size () > 2);
    return children ().begin () + 3;
}

TreeNode::ChildrenListConstIterator TreeNodeProcDef::paramEnd () const {
    return children ().end ();
}

/*******************************************************************************
  TreeNodeQuantifier
*******************************************************************************/

TreeNodeIdentifier* TreeNodeQuantifier::domain () {
    assert (children ().size () == 1 || children ().size () == 2);
    return childAt<TreeNodeIdentifier>(this, 0);
}

// will equal to zero, if kind not specified
TreeNodeIdentifier* TreeNodeQuantifier::kind () {
    assert (children ().size () == 1 || children ().size () == 2);
    if (children ().size () == 2) {
        return childAt<TreeNodeIdentifier>(this, 1);
    }

    return 0;
}

/*******************************************************************************
  TreeNodeTemplate
*******************************************************************************/

TreeNodeProcDef* TreeNodeTemplate::body () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeProcDef>(this, 1);
}

TreeNode::ChildrenList& TreeNodeTemplate::quantifiers () const {
    assert (children ().size () == 2);
    return children ().at (0)->children ();
}

/*******************************************************************************
  TreeNodeExprDomainID
*******************************************************************************/

TreeNodeSecTypeF* TreeNodeExprDomainID::securityType () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeSecTypeF>(this, 0);
}

/*******************************************************************************
  TreeNodeExprQualified
*******************************************************************************/

TreeNodeExpr* TreeNodeExprQualified::expression () const {
    assert (children ().size () >= 2);
    return expressionAt (this, 0);
}

TreeNode::ChildrenListConstRange TreeNodeExprQualified::types () const {
    assert (children ().size () >= 2);
    return std::make_pair (++ children ().begin (), children ().end ());
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
  TreeNodeStmtDecl
*******************************************************************************/

TreeNodeType* TreeNodeStmtDecl::varType () const {
    assert (children().size () >= 2);
    return childAt<TreeNodeType>(this, 0);
}

TreeNodeVarInit* TreeNodeStmtDecl::initializer () const {
    assert (children().size () >= 2);
    return childAt<TreeNodeVarInit>(this, 1);
}

TreeNode::ChildrenListConstRange TreeNodeStmtDecl::initializers () const {
    assert (children().size () >= 2);
    return std::make_pair (++ m_children.begin (),
                              m_children.end ());
}

TreeNode* TreeNodeVarInit::shape () const {
    assert(children().size() > 0 && children().size() <= 3);
    if (children().size() > 1) {
        return children ().at (1);
    }

    return 0;
}

TreeNodeExpr* TreeNodeVarInit::rightHandSide () const {
    assert(children().size() > 0 && children().size() <= 3);
    if (children ().size () > 2) {
        return expressionAt (this, 2);
    }

    return 0;
}

const std::string &TreeNodeVarInit::variableName() const {
    assert(children().size() > 0 && children().size() <= 3);
    return childAt<TreeNodeIdentifier>(this, 0)->value ();
}

/*******************************************************************************
  TreeNodeStmtDoWhile
*******************************************************************************/

TreeNodeExpr* TreeNodeStmtDoWhile::conditional () const {
    assert (children ().size () == 2);
    return expressionAt (this, 1);
}

TreeNodeStmt* TreeNodeStmtDoWhile::body () const {
    assert (children ().size () == 2);
    return statementAt (this, 0);
}

/*******************************************************************************
  TreeNodeStmtExpr
*******************************************************************************/

TreeNodeExpr* TreeNodeStmtExpr::expression () const {
    assert (children ().size () == 1);
    return expressionAt (this, 0);
}

/*******************************************************************************
  TreeNodeStmtAssert
*******************************************************************************/

TreeNodeExpr* TreeNodeStmtAssert::expression () const {
    assert (children ().size () == 1);
    return expressionAt (this, 0);
}

/*******************************************************************************
  TreeNodeStmtFor
*******************************************************************************/

TreeNode* TreeNodeStmtFor::initializer () const {
    assert (children ().size () == 4);
    if (children ().at (0)->type () != NODE_EXPR_NONE) {
        return children ().at (0);
    }

    return 0;
}

TreeNodeExpr* TreeNodeStmtFor::conditional () const {
    assert (children ().size () == 4);
    if (children ().at (1)->type () != NODE_EXPR_NONE) {
        return expressionAt (this, 1);
    }

    return 0;
}

TreeNodeExpr* TreeNodeStmtFor::iteratorExpr () const {
    assert (children ().size () == 4);
    if (children ().at (2)->type () != NODE_EXPR_NONE) {
        return expressionAt (this, 2);
    }

    return 0;
}

TreeNodeStmt* TreeNodeStmtFor::body () const {
    assert (children ().size () == 4);
    return statementAt (this, 3);
}

/*******************************************************************************
  TreeNodeStmtIf
*******************************************************************************/

TreeNodeExpr* TreeNodeStmtIf::conditional () const {
    assert (children ().size () == 2 || children ().size () == 3);
    return expressionAt (this, 0);
}

TreeNodeStmt* TreeNodeStmtIf::trueBranch () const {
    assert (children ().size () == 2 || children ().size () == 3);
    return statementAt (this, 1);
}

TreeNodeStmt* TreeNodeStmtIf::falseBranch () const {
    assert (children ().size () == 2 || children ().size () == 3);
    if (children ().size () == 3) {
        return statementAt (this, 2);
    }

    return 0;
}

/*******************************************************************************
  TreeNodeStmtReturn
*******************************************************************************/

TreeNodeExpr* TreeNodeStmtReturn::expression () const {
    if (!children ().empty ()) {
        return expressionAt (this, 0);
    }

    return 0;
}

/*******************************************************************************
  TreeNodeStmtWhile
*******************************************************************************/

TreeNodeExpr* TreeNodeStmtWhile::conditional () const {
    assert (children ().size () == 2);
    return expressionAt (this, 0);
}

TreeNodeStmt* TreeNodeStmtWhile::body () const {
    assert (children ().size () == 2);
    return statementAt (this, 1);
}

/*******************************************************************************
  TreeNodeStmtPrint
*******************************************************************************/

TreeNode::ChildrenList& TreeNodeStmtPrint::expressions () {
    assert (children ().size () == 1);
    return children ().at (0)->children ();
}

/*******************************************************************************
  TreeNodeStmtSyscall
*******************************************************************************/

TreeNodeExprString* TreeNodeStmtSyscall::expression () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeExprString>(this, 0);
}

/*******************************************************************************
  TreeNodeStmtPush
*******************************************************************************/

TreeNodeExpr* TreeNodeStmtPush::expression () const {
    assert (children ().size () == 1);
    return expressionAt (this, 0);
}

/*******************************************************************************
  TreeNodeStmtPushRef
*******************************************************************************/

TreeNodeIdentifier* TreeNodeStmtPushRef::identifier () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeIdentifier>(this, 0);
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
  TreeNodeModule
*******************************************************************************/

bool TreeNodeModule::hasName () const {
    return children ().size () == 2;
}

std::string TreeNodeModule::name () const {
    assert (children ().size () == 1 || children ().size () == 2);

    if (hasName ()) {
        return childAt<TreeNodeIdentifier>(this, 1)->value ();
    }

    return "";
}

TreeNodeProgram* TreeNodeModule::program () const {
    assert (children ().size () == 1 || children ().size () == 2);
    return childAt<TreeNodeProgram>(this, 0);
}

/*******************************************************************************
  TreeNodeImport
*******************************************************************************/

const std::string& TreeNodeImport::name () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeIdentifier>(this, 0)->value ();
}


} // namespace SecreC
