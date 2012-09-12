#include "treenode.h"

#include <algorithm>
#include <boost/foreach.hpp>
#include <sstream>

#include "context.h"
#include "misc.h"
#include "symbol.h"
#include "symboltable.h"
#include "treenode_c.h"
#include "typechecker.h"

namespace SecreC {

namespace /* anonymous */ {

template <class T >
T * childAt(const TreeNode * node, size_t i) {
    assert(i < node->children().size());
    assert(dynamic_cast<T *>(node->children().at(i)) != 0);
    return static_cast<T *>(node->children().at(i));
}

template <class T >
T *& childPtrRefAt(TreeNode * node, size_t i) {
    assert(i < node->children().size());
    assert(dynamic_cast<T *>(node->children().at(i)) != 0);
    return reinterpret_cast<T *&>(node->children().at(i));
}

TreeNodeStmt * statementAt(const TreeNode * node, size_t i) {
    return childAt<TreeNodeStmt>(node, i);
}

TreeNodeExpr * expressionAt(const TreeNode * node, size_t i) {
    return childAt<TreeNodeExpr>(node, i);
}

TreeNodeExpr *& expressionPtrRefAt(TreeNode * node, size_t i) {
    return childPtrRefAt<TreeNodeExpr>(node, i);
}

} // namespace anonymous

/*******************************************************************************
  TreeNode
*******************************************************************************/

TreeNode::Location::FilenameCache TreeNode::Location::m_filenameCache;


TreeNode::TreeNode(SecrecTreeNodeType type, const Location & loc)
    : m_parent(0)
    , m_procedure(0)
    , m_type(type)
    , m_location(loc)
{
    // Intentionally empty
}

TreeNode::~TreeNode() {
    BOOST_FOREACH (TreeNode * child, m_children) {
        assert(child != 0);
        if (child->m_parent == this) {
            delete child; // that's kinda sad
        }
    }
}

TreeNode * TreeNode::clone(TreeNode * parent) const {
    TreeNode * out = cloneV();

    if (parent) {
        out->resetParent(parent);
    }

    BOOST_FOREACH(const TreeNode * n, m_children) {
        out->m_children.push_back(n->clone(out));
    }

    return out;
}

TreeNodeProcDef * TreeNode::containingProcedure() const {
    if (m_procedure != 0) return m_procedure;
    if (m_parent != 0) {
        return (m_procedure = m_parent->containingProcedure());
    }
    return 0;
}

void TreeNode::appendChild(TreeNode * child) {
    assert(child != 0);
    m_children.push_back(child);
    child->resetParent(this);
}

void TreeNode::prependChild(TreeNode * child) {
    assert(child != 0);
    m_children.push_front(child);
    child->resetParent(this);
}

void TreeNode::setLocation(const TreeNode::Location & location) {
    m_location = location;
}

#define CASE_NODE_NAME(NAME) case NODE_ ## NAME : do { return #NAME; } while (0)
const char *TreeNode::typeName(SecrecTreeNodeType type) {
    switch (type) {
    CASE_NODE_NAME(INTERNAL_USE);
    CASE_NODE_NAME(DATATYPE_F);
    CASE_NODE_NAME(DECL);
    CASE_NODE_NAME(DIMENSIONS);
    CASE_NODE_NAME(DIMTYPE_F);
    CASE_NODE_NAME(DOMAIN);
    CASE_NODE_NAME(EXPR_BINARY_ASSIGN);
    CASE_NODE_NAME(EXPR_BINARY_ASSIGN_ADD);
    CASE_NODE_NAME(EXPR_BINARY_ASSIGN_AND);
    CASE_NODE_NAME(EXPR_BINARY_ASSIGN_DIV);
    CASE_NODE_NAME(EXPR_BINARY_ASSIGN_MOD);
    CASE_NODE_NAME(EXPR_BINARY_ASSIGN_MUL);
    CASE_NODE_NAME(EXPR_BINARY_ASSIGN_OR);
    CASE_NODE_NAME(EXPR_BINARY_ASSIGN_SUB);
    CASE_NODE_NAME(EXPR_BINARY_ASSIGN_XOR);
    CASE_NODE_NAME(EXPR_BINARY_ADD);
    CASE_NODE_NAME(EXPR_BINARY_DIV);
    CASE_NODE_NAME(EXPR_BINARY_EQ);
    CASE_NODE_NAME(EXPR_BINARY_GE);
    CASE_NODE_NAME(EXPR_BINARY_GT);
    CASE_NODE_NAME(EXPR_BINARY_LAND);
    CASE_NODE_NAME(EXPR_BINARY_LOR);
    CASE_NODE_NAME(EXPR_BINARY_LE);
    CASE_NODE_NAME(EXPR_BINARY_LT);
    CASE_NODE_NAME(EXPR_BINARY_MATRIXMUL);
    CASE_NODE_NAME(EXPR_BINARY_MOD);
    CASE_NODE_NAME(EXPR_BINARY_MUL);
    CASE_NODE_NAME(EXPR_BINARY_NE);
    CASE_NODE_NAME(EXPR_BINARY_SUB);
    CASE_NODE_NAME(EXPR_BITWISE_AND);
    CASE_NODE_NAME(EXPR_BITWISE_OR);
    CASE_NODE_NAME(EXPR_BITWISE_XOR);
    CASE_NODE_NAME(EXPR_BYTES_FROM_STRING);
    CASE_NODE_NAME(EXPR_CAST);
    CASE_NODE_NAME(EXPR_CAT);
    CASE_NODE_NAME(EXPR_CLASSIFY);
    CASE_NODE_NAME(EXPR_DECLASSIFY);
    CASE_NODE_NAME(EXPR_DOMAINID);
    CASE_NODE_NAME(EXPR_INDEX);
    CASE_NODE_NAME(EXPR_NONE);
    CASE_NODE_NAME(EXPR_POSTFIX_DEC);
    CASE_NODE_NAME(EXPR_POSTFIX_INC);
    CASE_NODE_NAME(EXPR_PREFIX_DEC);
    CASE_NODE_NAME(EXPR_PREFIX_INC);
    CASE_NODE_NAME(EXPR_PROCCALL);
    CASE_NODE_NAME(EXPR_RESHAPE);
    CASE_NODE_NAME(EXPR_RVARIABLE);
    CASE_NODE_NAME(EXPR_SHAPE);
    CASE_NODE_NAME(EXPR_SIZE);
    CASE_NODE_NAME(EXPR_STRING_FROM_BYTES);
    CASE_NODE_NAME(EXPR_TERNIF);
    CASE_NODE_NAME(EXPR_TOSTRING);
    CASE_NODE_NAME(EXPR_TYPE_QUAL);
    CASE_NODE_NAME(EXPR_UINV);
    CASE_NODE_NAME(EXPR_UMINUS);
    CASE_NODE_NAME(EXPR_UNEG);
    CASE_NODE_NAME(IDENTIFIER);
    CASE_NODE_NAME(IMPORT);
    CASE_NODE_NAME(INDEX_INT);
    CASE_NODE_NAME(INDEX_SLICE);
    CASE_NODE_NAME(KIND);
    CASE_NODE_NAME(LITE_BOOL);
    CASE_NODE_NAME(LITE_FLOAT);
    CASE_NODE_NAME(LITE_INT);
    CASE_NODE_NAME(LITE_STRING);
    CASE_NODE_NAME(LVALUE);
    CASE_NODE_NAME(MODULE);
    CASE_NODE_NAME(PROCDEF);
    CASE_NODE_NAME(PROGRAM);
    CASE_NODE_NAME(PUSH);
    CASE_NODE_NAME(PUSHCREF);
    CASE_NODE_NAME(PUSHREF);
    CASE_NODE_NAME(SECTYPE_F);
    CASE_NODE_NAME(STMT_ASSERT);
    CASE_NODE_NAME(STMT_BREAK);
    CASE_NODE_NAME(STMT_COMPOUND);
    CASE_NODE_NAME(STMT_CONTINUE);
    CASE_NODE_NAME(STMT_DOWHILE);
    CASE_NODE_NAME(STMT_EXPR);
    CASE_NODE_NAME(STMT_FOR);
    CASE_NODE_NAME(STMT_IF);
    CASE_NODE_NAME(STMT_PRINT);
    CASE_NODE_NAME(STMT_RETURN);
    CASE_NODE_NAME(STMT_SYSCALL);
    CASE_NODE_NAME(STMT_WHILE);
    CASE_NODE_NAME(SUBSCRIPT);
    CASE_NODE_NAME(TEMPLATE_DECL);
    CASE_NODE_NAME(TEMPLATE_QUANT);
    CASE_NODE_NAME(TYPETYPE);
    CASE_NODE_NAME(TYPEVOID);
    CASE_NODE_NAME(VAR_INIT);
    default: return "UNKNOWN";
    }
}

#undef CASE_NODE_NAME

std::string TreeNode::toString(unsigned indent, unsigned startIndent) const {
    std::ostringstream os;

    os << std::string(startIndent, ' ');
    os << typeName(m_type);

    const std::string sh(stringHelper());
    if (!sh.empty())
        os << ' ' << sh;

    os << " at " << location();

    BOOST_FOREACH(TreeNode* child, m_children) {
        os << std::endl;
        assert(child->parent() == this);
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
        BOOST_FOREACH (TreeNode * child, m_children) {
            assert(child->parent() == this);
            os << child->toXml(false);
        }
        os << "</" << typeName(m_type) << '>';
    }
    return os.str();
}

/*******************************************************************************
  TreeNodeSecTypeF
*******************************************************************************/

void TreeNodeSecTypeF::setCachedType(SecurityType * ty) {
    assert(m_cachedType == 0);
    assert(ty != 0);
    m_cachedType = ty;
}

TreeNodeIdentifier * TreeNodeSecTypeF::identifier() const {
    assert(children().size() == 1 &&
           "Called on public type?");
    return childAt<TreeNodeIdentifier>(this, 0);
}

std::string TreeNodeSecTypeF::stringHelper() const {
    std::ostringstream os;
    if (m_isPublic)
        os << "public";
    else
        os << static_cast<TreeNodeIdentifier *>(children().at(0))->value();
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
    os << "type=\"" << SecrecFundDataTypeToString(m_dataType) << "\"";
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

SecreC::Type * TreeNodeType::secrecType() const {
    assert(m_cachedType != 0);
    return m_cachedType;
}

TreeNodeSecTypeF * TreeNodeType::secType() const {
    assert(children().size() == 3);
    return childAt<TreeNodeSecTypeF>(this, 0);
}

TreeNodeDataTypeF * TreeNodeType::dataType() const {
    assert(children().size() == 3);
    return childAt<TreeNodeDataTypeF>(this, 1);
}

TreeNodeDimTypeF * TreeNodeType::dimType() const {
    assert(children().size() == 3);
    return childAt<TreeNodeDimTypeF>(this, 2);
}

bool TreeNodeType::isNonVoid() const {
    assert(children().size() == 3 || children().empty());
    return ! children().empty();
}

std::string TreeNodeType::typeString() const {
    if (m_cachedType)
        return m_cachedType->toNormalString();

    if (!isNonVoid())
        return "void";

    std::ostringstream oss;
    TreeNodeSecTypeF * const st = secType();
    if (!st->isPublic())
        oss << st->identifier()->value() << ' ';

    oss << SecrecFundDataTypeToString(dataType()->dataType());
    if (dimType()->dimType() > 0)
        oss << "[[" << dimType()->dimType() << "]]";

    return oss.str();
}

/*******************************************************************************
  OverloadableOperator
*******************************************************************************/

std::string OverloadableOperator::operatorName() const {
    std::ostringstream os;
    os << "__operator" << getOperator();
    return os.str();
}

/*******************************************************************************
  TreeNodeExpr
*******************************************************************************/

bool TreeNodeExpr::havePublicBoolType() const {
    assert(m_resultType != 0);
    return m_resultType->secrecDataType() == DATATYPE_BOOL
           && m_resultType->secrecSecType()->isPublic()
           && m_resultType->isScalar();
}

SecreC::Type * TreeNodeExpr::resultType() const {
    assert(m_resultType != 0);
    return m_resultType;
}

void TreeNodeExpr::setResultType(SecreC::Type * type) {
    assert(m_resultType == 0);
    m_resultType = type;
}

void TreeNodeExpr::resetDataType(Context & cxt, SecrecDataType dType) {
    assert(dynamic_cast<TypeNonVoid *>(m_resultType) != 0);
    m_resultType = TypeNonVoid::get(cxt,
            m_resultType->secrecSecType(),
            dType,
            m_resultType->secrecDimType());
}

/*******************************************************************************
  TreeNodeExprUnary
*******************************************************************************/

TreeNodeExpr * TreeNodeExprUnary::expression() const {
    return expressionAt(this, 0);
}


SecrecOperator TreeNodeExprUnary::getOperatorV() const {
    switch (type()) {
        case NODE_EXPR_UINV:   return SCOP_UN_INV;
        case NODE_EXPR_UNEG:   return SCOP_UN_NEG;
        case NODE_EXPR_UMINUS: return SCOP_UN_MINUS;
        default:               return SCOP_NONE;
    }
}

/*******************************************************************************
  OverloadableOperator
*******************************************************************************/

SecrecOperator OverloadableOperator::getOperator() const {
    if (m_operator == SCOP_NONE) { // unlikeley
        m_operator = getOperatorV();
        assert((m_operator != SCOP_NONE) &&
               "getOperatorV returned undefined operator.");
    }

    return m_operator;
}

/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/

TreeNodeExpr * TreeNodeExprBinary::leftExpression() const {
    assert(children().size() == 2);
    return expressionAt(this, 0);
}

TreeNodeExpr * TreeNodeExprBinary::rightExpression() const {
    assert(children().size() == 2);
    return expressionAt(this, 1);
}

TreeNodeExpr *& TreeNodeExprBinary::leftExpressionPtrRef() {
    assert(children().size() == 2u);
    return expressionPtrRefAt(this, 0u);
}

TreeNodeExpr *& TreeNodeExprBinary::rightExpressionPtrRef() {
    assert(children().size() == 2u);
    return expressionPtrRefAt(this, 1u);
}

const char *TreeNodeExprBinary::operatorString() const {
    switch (getOperator()) {
    case SCOP_BIN_ADD:  return "+";
    case SCOP_BIN_BAND: return "&";
    case SCOP_BIN_BOR:  return "|";
    case SCOP_BIN_DIV:  return "/";
    case SCOP_BIN_EQ:   return "==";
    case SCOP_BIN_GE:   return ">=";
    case SCOP_BIN_GT:   return ">";
    case SCOP_BIN_LAND: return "&&";
    case SCOP_BIN_LE:   return "<=";
    case SCOP_BIN_LOR:  return "||";
    case SCOP_BIN_LT:   return "<";
    case SCOP_BIN_MOD:  return "%";
    case SCOP_BIN_MUL:  return "*";
    case SCOP_BIN_NE:   return "!=";
    case SCOP_BIN_SUB:  return "-";
    case SCOP_BIN_XOR:  return "^";
    default:
        assert(false); // shouldn't happen
    }

    return "?";
}

SecrecOperator TreeNodeExprBinary::getOperatorV() const {
    switch (type()) {
    case NODE_EXPR_BINARY_ADD:  return SCOP_BIN_ADD;
    case NODE_EXPR_BINARY_DIV:  return SCOP_BIN_DIV;
    case NODE_EXPR_BINARY_EQ:   return SCOP_BIN_EQ;
    case NODE_EXPR_BINARY_GE:   return SCOP_BIN_GE;
    case NODE_EXPR_BINARY_GT:   return SCOP_BIN_GT;
    case NODE_EXPR_BINARY_LAND: return SCOP_BIN_LAND;
    case NODE_EXPR_BINARY_LE:   return SCOP_BIN_LE;
    case NODE_EXPR_BINARY_LOR:  return SCOP_BIN_LOR;
    case NODE_EXPR_BINARY_LT:   return SCOP_BIN_LT;
    case NODE_EXPR_BINARY_MOD:  return SCOP_BIN_MOD;
    case NODE_EXPR_BINARY_MUL:  return SCOP_BIN_MUL;
    case NODE_EXPR_BINARY_NE:   return SCOP_BIN_NE;
    case NODE_EXPR_BINARY_SUB:  return SCOP_BIN_SUB;
    case NODE_EXPR_BITWISE_AND: return SCOP_BIN_BAND;
    case NODE_EXPR_BITWISE_OR:  return SCOP_BIN_BOR;
    case NODE_EXPR_BITWISE_XOR: return SCOP_BIN_XOR;
    default:
        assert(false); // shouldn't happen
    }

    return SCOP_NONE;
}

/*******************************************************************************
  TreeNodeExprClassify
*******************************************************************************/

TreeNodeExpr * TreeNodeExprClassify::expression() const {
    assert(children().size() == 1);
    return expressionAt(this, 0);
}

TreeNode * TreeNodeExprClassify::cloneV() const {
    assert(false && "ICE: Classify nodes are created during type checking and "
           "it's assumed that procedures are cloned before type checking is performed.");
    return new TreeNodeExprClassify(m_contextSecType, m_location);
}

/*******************************************************************************
  TreeNodeExprDeclassify
*******************************************************************************/

TreeNodeExpr * TreeNodeExprDeclassify::expression() const {
    assert(children().size() == 1);
    return expressionAt(this, 0);
}

/*******************************************************************************
  TreeNodeExprProcCall
*******************************************************************************/

TreeNodeIdentifier * TreeNodeExprProcCall::procName() const {
    assert(!children().empty());
    return childAt<TreeNodeIdentifier>(this, 0);
}

TreeNode::ChildrenListConstRange TreeNodeExprProcCall::paramRange() const {
    assert(!children().empty());
    return std::make_pair(++ children().begin(),
                             children().end());
}

/*******************************************************************************
  TreeNodeExprRVariable
*******************************************************************************/

TreeNodeIdentifier * TreeNodeExprRVariable::identifier() const {
    assert(children().size() == 1);
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

TreeNodeExpr *& TreeNodeExprTernary::trueBranchPtrRef() {
    assert(children().size() == 3u);
    return expressionPtrRefAt(this, 1u);
}

TreeNodeExpr *& TreeNodeExprTernary::falseBranchPtrRef() {
    assert(children().size() == 3u);
    return expressionPtrRefAt(this, 2u);
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

TreeNodeExpr *& TreeNodeExprAssign::rightHandSidePtrRef() {
    assert(children().size() == 2u);
    return expressionPtrRefAt(this, 1u);
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

TreeNodeExpr *& TreeNodeExprCat::leftExpressionPtrRef() {
    assert(children().size() == 3u || children().size() == 2u);
    return expressionPtrRefAt(this, 0u);
}
TreeNodeExpr *& TreeNodeExprCat::rightExpressionPtrRef() {
    assert(children().size() == 3u || children().size() == 2u);
    return expressionPtrRefAt(this, 1u);
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

const std::string TreeNodeProcDef::printableSignature() const {
    std::ostringstream oss;
    if (m_procSymbol) {
        m_procSymbol->print(oss);
    } else {
        oss << returnType()->typeString() << ' '
            << procedureName() << '(';
        size_t i = 0u;
        BOOST_FOREACH (const TreeNode * n, paramRange()) {
            if (i > 0u)
                oss << ", ";
            i++;

            assert (dynamic_cast<const TreeNodeStmtDecl*>(n) != 0);
            const TreeNodeStmtDecl * const d = static_cast<const TreeNodeStmtDecl*>(n);
            oss << d->varType()->typeString() << ' ' << d->variableName();
        }
        oss << ')';
    }
    return oss.str();
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
  TreeNodeExprStringFromBytes
*******************************************************************************/

TreeNodeExpr* TreeNodeExprStringFromBytes::expression () const {
    assert (children ().size () == 1);
    return expressionAt (this, 0);
}

/*******************************************************************************
  TreeNodeExprBytesFromString
*******************************************************************************/

TreeNodeExpr* TreeNodeExprBytesFromString::expression () const {
    assert (children ().size () == 1);
    return expressionAt (this, 0);
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

TreeNodeExpr *& TreeNodeVarInit::rightHandSidePtrRef() {
    assert(children().size() > 2u && children().size() <= 3u);
    return expressionPtrRefAt(this, 2u);
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

TreeNodeExpr *& TreeNodeStmtReturn::expressionPtrRef() {
    assert(!children().empty());
    return expressionPtrRefAt(this, 0u);
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

TreeNodeExprString* TreeNodeStmtSyscall::name () const {
    assert (children ().size () >= 1);
    return childAt<TreeNodeExprString>(this, 0);
}

TreeNode::ChildrenListConstRange TreeNodeStmtSyscall::paramRange () const {
    assert (children ().size () >= 1);
    return std::make_pair (begin () + 1, end ());
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

TreeNodeModule::~TreeNodeModule() {
    BOOST_FOREACH (TreeNodeProcDef* instance, m_generatedInstances) {
        delete instance;
    }
}

bool TreeNodeModule::hasName() const {
   return children().size() == 2;
}

std::string TreeNodeModule::name() const {
    assert(children().size() == 1 || children().size() == 2);

    if (hasName()) {
        return childAt<TreeNodeIdentifier>(this, 1)->value();
    }

    return "";
}

TreeNodeProgram * TreeNodeModule::program() const {
    assert(children().size() == 1 || children().size() == 2);
    return childAt<TreeNodeProgram>(this, 0);
}

/*******************************************************************************
  TreeNodeImport
*******************************************************************************/

const std::string & TreeNodeImport::name() const {
    assert(children().size() == 1);
    return childAt<TreeNodeIdentifier>(this, 0)->value();
}

} // namespace SecreC

/*******************************************************************************
  C interface for Yacc
*******************************************************************************/

TreeNode * treenode_init(enum SecrecTreeNodeType type, const YYLTYPE * loc) {
    switch (type) {
    case NODE_PROGRAM:
        return (TreeNode *)(new SecreC::TreeNodeProgram(*loc));
    case NODE_PROCDEF:
        return (TreeNode *)(new SecreC::TreeNodeProcDef(*loc));
    case NODE_EXPR_UINV:     /* Fall through: */
    case NODE_EXPR_UNEG:     /* Fall through: */
    case NODE_EXPR_UMINUS:
        return (TreeNode *)(new SecreC::TreeNodeExprUnary(type, *loc));
    case NODE_EXPR_POSTFIX_INC: /* Fall through: */
    case NODE_EXPR_POSTFIX_DEC:
        return (TreeNode *)(new SecreC::TreeNodeExprPostfix(type, *loc));
    case NODE_EXPR_PREFIX_INC: /* Fall through: */
    case NODE_EXPR_PREFIX_DEC:
        return (TreeNode *)(new SecreC::TreeNodeExprPrefix(type, *loc));
    case NODE_EXPR_BINARY_ADD:        /* Fall through: */
    case NODE_EXPR_BINARY_DIV:        /* Fall through: */
    case NODE_EXPR_BINARY_EQ:         /* Fall through: */
    case NODE_EXPR_BINARY_GE:         /* Fall through: */
    case NODE_EXPR_BINARY_GT:         /* Fall through: */
    case NODE_EXPR_BINARY_LAND:       /* Fall through: */
    case NODE_EXPR_BINARY_LE:         /* Fall through: */
    case NODE_EXPR_BINARY_LOR:        /* Fall through: */
    case NODE_EXPR_BINARY_LT:         /* Fall through: */
    case NODE_EXPR_BINARY_MATRIXMUL:  /* Fall through: */
    case NODE_EXPR_BINARY_MOD:        /* Fall through: */
    case NODE_EXPR_BINARY_MUL:        /* Fall through: */
    case NODE_EXPR_BINARY_NE:         /* Fall through: */
    case NODE_EXPR_BINARY_SUB:        /* Fall through: */
    case NODE_EXPR_BITWISE_AND:       /* Fall through: */
    case NODE_EXPR_BITWISE_OR:        /* Fall through: */
    case NODE_EXPR_BITWISE_XOR:       /* Fall through: */
        return (TreeNode *)(new SecreC::TreeNodeExprBinary(type, *loc));
    case NODE_EXPR_TERNIF:
        return (TreeNode *)(new SecreC::TreeNodeExprTernary(*loc));
    case NODE_EXPR_BINARY_ASSIGN_ADD: /* Fall through: */
    case NODE_EXPR_BINARY_ASSIGN_AND: /* Fall through: */
    case NODE_EXPR_BINARY_ASSIGN_DIV: /* Fall through: */
    case NODE_EXPR_BINARY_ASSIGN_MOD: /* Fall through: */
    case NODE_EXPR_BINARY_ASSIGN_MUL: /* Fall through: */
    case NODE_EXPR_BINARY_ASSIGN_OR:  /* Fall through: */
    case NODE_EXPR_BINARY_ASSIGN_SUB: /* Fall through: */
    case NODE_EXPR_BINARY_ASSIGN_XOR: /* Fall through: */
    case NODE_EXPR_BINARY_ASSIGN:
        return (TreeNode *)(new SecreC::TreeNodeExprAssign(type, *loc));
    case NODE_EXPR_DECLASSIFY:
        return (TreeNode *)(new SecreC::TreeNodeExprDeclassify(*loc));
    case NODE_EXPR_PROCCALL:
        return (TreeNode *)(new SecreC::TreeNodeExprProcCall(*loc));
    case NODE_EXPR_RVARIABLE:
        return (TreeNode *)(new SecreC::TreeNodeExprRVariable(*loc));
    case NODE_EXPR_INDEX:
        return (TreeNode *)(new SecreC::TreeNodeExprIndex(*loc));
    case NODE_EXPR_SIZE:
        return (TreeNode *)(new SecreC::TreeNodeExprSize(*loc));
    case NODE_EXPR_SHAPE:
        return (TreeNode *)(new SecreC::TreeNodeExprShape(*loc));
    case NODE_EXPR_CAT:
        return (TreeNode *)(new SecreC::TreeNodeExprCat(*loc));
    case NODE_EXPR_RESHAPE:
        return (TreeNode *)(new SecreC::TreeNodeExprReshape(*loc));
    case NODE_EXPR_TOSTRING:
        return (TreeNode *)(new SecreC::TreeNodeExprToString(*loc));
    case NODE_EXPR_STRING_FROM_BYTES:
        return (TreeNode *)(new SecreC::TreeNodeExprStringFromBytes(*loc));
    case NODE_EXPR_BYTES_FROM_STRING:
        return (TreeNode *)(new SecreC::TreeNodeExprBytesFromString(*loc));
    case NODE_STMT_BREAK:
        return (TreeNode *)(new SecreC::TreeNodeStmtBreak(*loc));
    case NODE_STMT_CONTINUE:
        return (TreeNode *)(new SecreC::TreeNodeStmtContinue(*loc));
    case NODE_STMT_COMPOUND:
        return (TreeNode *)(new SecreC::TreeNodeStmtCompound(*loc));
    case NODE_STMT_DOWHILE:
        return (TreeNode *)(new SecreC::TreeNodeStmtDoWhile(*loc));
    case NODE_STMT_EXPR:
        return (TreeNode *)(new SecreC::TreeNodeStmtExpr(*loc));
    case NODE_STMT_ASSERT:
        return (TreeNode *)(new SecreC::TreeNodeStmtAssert(*loc));
    case NODE_STMT_FOR:
        return (TreeNode *)(new SecreC::TreeNodeStmtFor(*loc));
    case NODE_STMT_IF:
        return (TreeNode *)(new SecreC::TreeNodeStmtIf(*loc));
    case NODE_STMT_RETURN:
        return (TreeNode *)(new SecreC::TreeNodeStmtReturn(*loc));
    case NODE_STMT_WHILE:
        return (TreeNode *)(new SecreC::TreeNodeStmtWhile(*loc));
    case NODE_STMT_PRINT:
        return (TreeNode *)(new SecreC::TreeNodeStmtPrint(*loc));
    case NODE_STMT_SYSCALL:
        return (TreeNode *)(new SecreC::TreeNodeStmtSyscall(*loc));
    case NODE_EXPR_DOMAINID:
        return (TreeNode *)(new SecreC::TreeNodeExprDomainID(*loc));
    case NODE_EXPR_TYPE_QUAL:
        return (TreeNode *)(new SecreC::TreeNodeExprQualified(*loc));
    case NODE_DECL:
        return (TreeNode *)(new SecreC::TreeNodeStmtDecl(*loc));
    case NODE_TYPETYPE:
        return (TreeNode *)(new SecreC::TreeNodeTypeType(*loc));
    case NODE_TYPEVOID:
        return (TreeNode *)(new SecreC::TreeNodeTypeVoid(*loc));
    case NODE_EXPR_CAST:
        return (TreeNode *)(new SecreC::TreeNodeExprCast(*loc));
    case NODE_KIND:
        return (TreeNode *)(new SecreC::TreeNodeKind(*loc));
    case NODE_DOMAIN:
        return (TreeNode *)(new SecreC::TreeNodeDomain(*loc));
    case NODE_TEMPLATE_QUANT:
        return (TreeNode *)(new SecreC::TreeNodeQuantifier(*loc));
    case NODE_TEMPLATE_DECL:
        return (TreeNode *)(new SecreC::TreeNodeTemplate(*loc));
    case NODE_VAR_INIT:
        return (TreeNode *)(new SecreC::TreeNodeVarInit(*loc));
    case NODE_MODULE:
        return (TreeNode *)(new SecreC::TreeNodeModule(*loc));
    case NODE_IMPORT:
        return (TreeNode *)(new SecreC::TreeNodeImport(*loc));
    default:
        assert(type != NODE_IDENTIFIER);
        assert((type & NODE_LITE_MASK) == 0x0);
        return (TreeNode *)(new SecreC::TreeNode(type, *loc));
    }
}

void treenode_free(TreeNode * node) {
    delete(SecreC::TreeNode *) node;
}

enum SecrecTreeNodeType treenode_type(TreeNode * node) {
    return ((const SecreC::TreeNode *) node)->type();
}

const YYLTYPE treenode_location(const TreeNode * node) {
    return ((const SecreC::TreeNode *) node)->location().toYYLTYPE();
}

unsigned treenode_numChildren(const TreeNode * node) {
    return ((const SecreC::TreeNode *) node)->children().size();
}

TreeNode * treenode_childAt(const TreeNode * node, unsigned index) {
    return (TreeNode *)((const SecreC::TreeNode *) node)->children().at(index);
}

void treenode_appendChild(TreeNode * parent, TreeNode * child) {
    return ((SecreC::TreeNode *) parent)->appendChild((SecreC::TreeNode *) child);
}

void treenode_prependChild(TreeNode * parent, TreeNode * child) {
    typedef SecreC::TreeNode TN;
    return ((TN *) parent)->prependChild((TN *) child);
}

void treenode_setLocation(TreeNode * node, YYLTYPE * loc) {
    return ((SecreC::TreeNode *) node)->setLocation(*loc);
}

void treenode_moveChildren(TreeNode * cfrom, TreeNode * cto) {
    SecreC::TreeNode * from = (SecreC::TreeNode *) cfrom;
    SecreC::TreeNode * to = (SecreC::TreeNode *) cto;
    BOOST_FOREACH (SecreC::TreeNode * child, from->children()) {
        to->appendChild(child);
    }

    from->children().clear();
}

TreeNode * treenode_init_bool(unsigned value, YYLTYPE * loc) {
    return (TreeNode *) new SecreC::TreeNodeExprBool(value, *loc);
}

TreeNode * treenode_init_int(int value, YYLTYPE * loc) {
    return (TreeNode *) new SecreC::TreeNodeExprInt(value, *loc);
}

TreeNode * treenode_init_string(const char * value, YYLTYPE * loc) {
    return (TreeNode *) new SecreC::TreeNodeExprString(value, *loc);
}

TreeNode * treenode_init_float(const char * value, YYLTYPE * loc) {
    return (TreeNode *) new SecreC::TreeNodeExprFloat(value, *loc);
}

TreeNode * treenode_init_identifier(const char * value, YYLTYPE * loc) {
    return (TreeNode *) new SecreC::TreeNodeIdentifier(value, *loc);
}

TreeNode * treenode_init_publicSecTypeF(YYLTYPE * loc) {
    return (TreeNode *) new SecreC::TreeNodeSecTypeF(true, *loc);
}

TreeNode * treenode_init_privateSecTypeF(YYLTYPE * loc) {
    return (TreeNode *) new SecreC::TreeNodeSecTypeF(false, *loc);
}

TreeNode * treenode_init_dataTypeF(enum SecrecDataType dataType, YYLTYPE * loc) {
    return (TreeNode *) new SecreC::TreeNodeDataTypeF(dataType, *loc);
}

TreeNode * treenode_init_dimTypeF(unsigned dimType, YYLTYPE * loc) {
    return (TreeNode *) new SecreC::TreeNodeDimTypeF(dimType, *loc);
}

TreeNode * treenode_init_opdef(enum SecrecOperator op, YYLTYPE * loc) {
    TreeNode * node = (TreeNode *) new SecreC::TreeNodeOpDef(op, *loc);
    std::ostringstream os;
    os << "__operator" << op;
    treenode_appendChild(node, treenode_init_identifier(os.str().c_str(), loc));
    return node;
}
