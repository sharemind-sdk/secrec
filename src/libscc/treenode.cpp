#include "treenode.h"
#include "symboltable.h"
#include "typechecker.h"
#include "misc.h"

#include <algorithm>
#include <sstream>

/**
 * NB!
 * All of the generateCode and generateBoolCode implementations
 * are found in codegen/ subdirectory
 */

namespace {

template <class T>
inline void appendVectorToVector(std::vector<T> &dst, const std::vector<T> &src)
{
    dst.insert(dst.end(), src.begin(), src.end());
}

} // anonymous namespace

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
        default: return "UNKNOWN";
    }
}

std::string TreeNode::toString(unsigned indent, unsigned startIndent) const {
    typedef ChildrenListConstIterator CLCI;

    std::ostringstream os;

    os << std::string (' ', startIndent);
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

TreeNodeExpr *TreeNode::classifyChildAtIfNeeded(int index, const Type& ty)
{
    TreeNode *&child = m_children.at(index);
    assert(dynamic_cast<TreeNodeExpr*>(child) != 0);
    if (!ty.isVoid ()) {
        if (ty.secrecSecType ().isPrivate () &&
            static_cast<TreeNodeExpr*>(child)->resultType().secrecSecType().isPublic ())
        {
            TreeNodeExprClassify *ec = new TreeNodeExprClassify(
                        static_cast<const PrivateSecType&>(ty.secrecSecType ()).domain (),
                        child->location());
            ec->appendChild(child);
            ec->resetParent(this);
            child = ec;
        }
    }

    return static_cast<TreeNodeExpr*>(child);
}

ICode::Status TreeNodeExpr::calculateResultType(SymbolTable &st, CompileLog &log) {
    TypeChecker tyChecker (st, log);
    return accept (tyChecker);
}

bool TreeNodeExpr::checkAndLogIfVoid(CompileLog& log) {
    assert (haveResultType());
    if (resultType().isVoid()) {
        log.fatal() << "Subexpression has type void at "
                    << location() << ".";
        return true;
    }

    return false;
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

ICode::Status TreeNodeProcDef::calculateProcedureType(SymbolTable &st,
                                                      CompileLog &log)
{
    typedef TypeNonVoid TNV;

    if (m_cachedType != 0) return ICode::OK;

    assert(dynamic_cast<TreeNodeType*>(children().at(1)) != 0);
    TreeNodeType *rt = static_cast<TreeNodeType*>(children().at(1));
    ICode::Status s = rt->calculateType (st, log);
    if (s != ICode::OK) return s;
    if (rt->type() == NODE_TYPEVOID) {
        assert(rt->secrecType().isVoid());

        DataTypeProcedureVoid dt;
        s = addParameters(dt, st, log);
        if (s != ICode::OK) return s;
        m_cachedType = new TNV(dt);
    } else {
        assert(rt->type() == NODE_TYPETYPE);
        assert(!rt->secrecType().isVoid());
        assert(dynamic_cast<const TNV*>(&rt->secrecType()) != 0);
        const TNV &tt = static_cast<const TNV&>(rt->secrecType());
        assert(tt.dataType().kind() == DataType::BASIC);

        DataTypeProcedure dt (tt.dataType());
        s = addParameters(dt, st, log);
        if (s != ICode::OK) return s;

        m_cachedType = new TNV(dt);
    }

    return ICode::OK;
}

ICode::Status TreeNodeProcDef::addParameters(DataTypeProcedureVoid &dt,
                                             SymbolTable &stable,
                                             CompileLog &log) const
{
    typedef DataTypeVar DTV;
    typedef ChildrenListConstIterator CLCI;

    if (children().size() <= 3) return ICode::OK;

    for (CLCI it(children().begin() + 3); it != children().end(); it++) {
        assert((*it)->type() == NODE_DECL);
        assert(dynamic_cast<TreeNodeStmtDecl*>(*it) != 0);
        TreeNodeStmtDecl *d = static_cast<TreeNodeStmtDecl*>(*it);

        ICode::Status s = d->calculateResultType(stable, log);
        if (s != ICode::OK) return s;

        const TypeNonVoid &pt = d->resultType();
        assert(pt.dataType().kind() == DataType::VAR);
        assert(dynamic_cast<const DTV*>(&pt.dataType()) != 0);
        dt.addParamType(static_cast<const DTV&>(pt.dataType()).dataType());
    }
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeIdentifier
*******************************************************************************/

SymbolSymbol *TreeNodeIdentifier::getSymbol(SymbolTable &st, CompileLog &log)
        const
{
    Symbol *s = st.find(m_value);
    if (s == 0) {
        log.fatal() << "Undefined symbol \"" << m_value << "\" at "
                    << location();
        return 0;
    }

    assert(s->symbolType() == Symbol::SYMBOL);
    assert(dynamic_cast<SymbolSymbol*>(s) != 0);
    return static_cast<SymbolSymbol*>(s);
}

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

ICode::Status TreeNodeStmtDecl::calculateResultType(SymbolTable &st, CompileLog &log) {
    typedef DataTypeBasic DTB;
    typedef TreeNodeType TNT;
    typedef TypeNonVoid TNV;

    if (m_type != 0) return ICode::OK;

    assert(children().size() >= 2);
    assert(dynamic_cast<TNT*>(children().at(1)) != 0);
    TNT *type = static_cast<TNT*>(children().at(1));
    ICode::Status s = type->calculateType (st, log);
    if (s != ICode::OK) return s;

    /// \todo Check here for overrides first if new symbol table is needed.

    // First we create the new symbol, but we don't add it to the symbol table:
    assert(type->secrecType().isVoid() == false);
    assert(dynamic_cast<const TNV*>(&type->secrecType()) != 0);
    const TNV &justType = static_cast<const TNV&>(type->secrecType());

    assert(justType.kind() == TNV::BASIC);
    assert(dynamic_cast<const DTB*>(&justType.dataType()) != 0);
    const DTB &dataType = static_cast<const DTB&>(justType.dataType());

    unsigned n = 0;
    if (children().size() > 2) {
        TreeNode::ChildrenListConstIterator
                ei = children().at(2)->children().begin(),
                ee = children().at(2)->children().end();
        for (; ei != ee; ++ ei, ++ n) {
            assert(((*ei)->type() & NODE_EXPR_MASK) != 0);
            assert(dynamic_cast<TreeNodeExpr*>(*ei) != 0);
            TreeNodeExpr* e = static_cast<TreeNodeExpr*>(*ei);
            s = e->calculateResultType(st, log);
            if (s != ICode::OK) return s;
            if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;
            if (   !e->resultType().secrecDataType() == DATATYPE_INT
                || !e->resultType().isScalar()
                || e->resultType().secrecSecType().isPrivate ()) {
                log.fatal() << "Expecting public unsigned integer scalar at "
                            << e->location();
                return ICode::E_TYPE;
            }
        }
    }

    if (n > 0 && n != justType.secrecDimType()) {
        log.fatal() << "Mismatching number of shape components in declaration at "
                    << location();
        return ICode::E_TYPE;
    }

    if (children().size() > 3) {
        TreeNode *t = children().at(3);
        assert((t->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(t) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        ICode::Status s = e->calculateResultType(st, log);
        if (s != ICode::OK) return s;

        assert(e->haveResultType());
        if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;

        if (n == 0 && e->resultType().isScalar() && justType.secrecDimType() > 0) {
            log.fatal() << "Defining array without shape annotation with scalar at "
                        << location();
            return ICode::E_TYPE;

        }

        if (e->resultType().secrecSecType().isPrivate () &&
            justType.secrecSecType().isPublic ())
        {
            log.fatal() << "Public variable is given a private initializer at "
                        << location();
            return ICode::E_TYPE;
        }

        if (e->resultType().secrecDataType() != justType.secrecDataType()) {
            log.fatal() << "Variable of type " << TNV(DataTypeVar(dataType))
                        << " is given an initializer expression of type "
                        << e->resultType() << " at " << location();
            return ICode::E_TYPE;
        }

        if (!e->resultType().isScalar() &&
            e->resultType().secrecDimType() != justType.secrecDimType()) {
            log.fatal() << "Variable of dimensionality " << dataType.secrecDimType()
                        << " is given an initializer expression of dimensionality "
                        << e->resultType().secrecDimType() << " at " << location();
            return ICode::E_TYPE;
        }
    }

    m_type = new TNV(DataTypeVar(dataType));

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeTypeType
*******************************************************************************/

ICode::Status TreeNodeTypeType::calculateType (SymbolTable& st, CompileLog& log) {
    typedef TreeNodeDataTypeF TNDT;
    typedef TreeNodeSecTypeF TNST;
    typedef TreeNodeDimTypeF TNDimT;

    if (m_cachedType != 0) {
        return ICode::OK;
    }

    assert(children().size() == 3);
    assert(dynamic_cast<TNST*>(children().at(0)) != 0);
    TreeNodeSecTypeF *sty = static_cast<TNST*>(children().at(0));
    SecurityType* secType = 0;
    if (sty->isPublic ()) {
        secType = new PublicSecType ();
    }
    else {
        TreeNodeIdentifier* id = static_cast<TreeNodeIdentifier*>(sty->children ().at (0));
        Symbol* sym = st.find (id->value ());
        if (sym == 0) {
            log.error () << "Symbol at " << sty->location () << " not declared!";
            return ICode::E_TYPE;
        }

        if (dynamic_cast<SymbolDomain*>(sym) == 0) {
            log.error () << "Mismatching symbol at " << sty->location ();
            return ICode::E_TYPE;
        }

        secType = new PrivateSecType (static_cast<SymbolDomain*>(sym));
    }

    assert(dynamic_cast<TNDT*>(children().at(1)) != 0);
    TNDT *dty = static_cast<TNDT*>(children().at(1));
    assert(dynamic_cast<TNDimT*>(children().at(2)) != 0);
    TNDimT *dimty = static_cast<TNDimT*>(children().at(2));
    m_cachedType = new SecreC::TypeNonVoid(*secType, dty->dataType(), dimty->dimType());
    delete secType;
    return ICode::OK;
}

std::string TreeNodeTypeType::stringHelper() const {
    return secrecType().toString();
}

} // namespace SecreC
