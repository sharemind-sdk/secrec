#include "treenode.h"
#include "symboltable.h"
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

    // Indent:
    for (unsigned i = 0; i < startIndent; i++) {
        os << ' ';
    }

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
//                                                SecrecSecType otherSecType)
{
    TreeNode *&child = m_children.at(index);
    assert(dynamic_cast<TreeNodeExpr*>(child) != 0);
    if (!ty.isVoid ()) {
        if (isPrivate (ty.secrecSecType ()) &&
            isPublic (static_cast<TreeNodeExpr*>(child)->resultType().secrecSecType()))
        {
            TreeNodeExprClassify *ec = new TreeNodeExprClassify(child->location());
            ec->appendChild(child);
            ec->resetParent(this);
            child = ec;
        }
    }

    return static_cast<TreeNodeExpr*>(child);
}

bool TreeNodeExpr::checkAndLogIfVoid(CompileLog& log) {
    assert (haveResultType());
    if (resultType().isVoid()) {
        log.fatal() << "Subexpression has type void at "
                    << location();
        return true;
    }

    return false;
}

/**
 * type check indices for NODE_SUBSCRIPT
 * \arg node is expected to be of type NODE_SUBSCRIPT
 * \arg destDim will be set to the number of slices given
 * \return ICode::OK if indices are type correct, first occured error otherwise
 */
ICode::Status tyCheckIndices (TreeNode* node,
                              SymbolTable& st,
                              CompileLog& log,
                              unsigned& destDim) {
    TreeNode::ChildrenListConstIterator i, j, begin, end;
    assert (node->type() == NODE_SUBSCRIPT);
    begin = node->children().begin ();
    end = node->children().end ();
    destDim = 0;
    for (i = begin; i != end; ++ i) {
        TreeNode* tNode = *i;

        switch (tNode->type()) {
            case NODE_INDEX_INT:
                break;
            case NODE_INDEX_SLICE:
                ++ destDim;
                break;
            default:
                assert (false && "Reached an index that isn't int or a slice.");
                return ICode::E_TYPE;
        }

        for (j = tNode->children().begin(); j != tNode->children().end(); ++ j) {
            if ((*j)->type() == NODE_EXPR_NONE) {
                continue;
            }

            assert (dynamic_cast<TreeNodeExpr*>(*j) != 0);
            TreeNodeExpr* e = static_cast<TreeNodeExpr*>(*j);
            ICode::Status s = e->calculateResultType(st, log);
            if (s != ICode::OK) {
                return s;
            }

            if (e->checkAndLogIfVoid(log)) {
                return ICode::E_TYPE;
            }

            const TypeNonVoid* eTy = static_cast<const TypeNonVoid*>(&e->resultType());

            if (isPrivate (eTy->secrecSecType()) ||
                    eTy->secrecDataType() != DATATYPE_INT ||
                    eTy->secrecDimType() > 0) {
                log.fatal() << "Invalid type for index at " << e->location() << ". "
                            << "Expected public integer scalar, got " << *eTy << ".";
                return ICode::E_TYPE;
            }
        }
    }

    return ICode::OK;
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
        case NODE_EXPR_FREAD:
            return (TreeNode*) (new SecreC::TreeNodeExprFRead(*loc));
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

TreeNode *treenode_init_secTypeF(
        enum SecrecSecType secType,
        YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeSecTypeF(secType, *loc);
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
  TreeNodeExprAssign
*******************************************************************************/

ICode::Status TreeNodeExprAssign::calculateResultType(SymbolTable &st,
                                                      CompileLog &log)
{
    typedef DataTypeVar DTV;
    typedef TypeNonVoid TNV;

    if (haveResultType()) return ICode::OK;

    assert(children().size() == 2);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

    // Get symbol for l-value:
    TreeNode *e1 = children().at(0);
    assert(e1 != 0);
    assert(e1->type() == NODE_LVALUE);
    assert(e1->children().size() > 0 && e1->children().size() <= 2);
    assert(dynamic_cast<TreeNodeIdentifier*>(e1->children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(e1->children().at(0));
    SymbolSymbol *dest = id->getSymbol(st, log);
    if (dest == 0) return ICode::E_OTHER;

    // Calculate type of r-value:
    assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
    TreeNodeExpr *src = static_cast<TreeNodeExpr*>(children().at(1));
    ICode::Status s = src->calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Get types for destination and source:
    const SecreC::Type &destType = dest->secrecType();
    SecrecDimType destDim = dest->secrecType().secrecDimType();
    assert(destType.isVoid() == false);
    const SecreC::Type &srcType = src->resultType();
    SecrecDimType srcDim = src->resultType().secrecDimType();

    if (e1->children().size() == 2) {
        s = tyCheckIndices (e1->children().at(1), st, log, destDim);
        if (s != ICode::OK) {
            return ICode::E_TYPE;
        }
    }

    if (!(srcDim == destDim || srcDim == 0)) {
        log.fatal() << "Incompatible dimensionalities in assignemnt at "
                    << location() << ". "
                    << "Expected " << destDim << " got " << srcDim << ".";
        return ICode::E_TYPE;
    }

    // Check types:
    if (src->checkAndLogIfVoid(log)) return ICode::E_TYPE;
    if (!destType.canAssign(srcType)) {
        log.fatal() << "Invalid assignment from value of type " << srcType
                    << " to variable of type " << destType << ". At "
                    << location();

        return ICode::E_TYPE;
    }

    // Add implicit classify node if needed:
    classifyChildAtIfNeeded(1, destType);

    assert(dynamic_cast<const TNV*>(&destType) != 0);
    const TNV &destTypeNV = static_cast<const TNV&>(destType);
    assert(destTypeNV.dataType().kind() == DataType::VAR);
    assert(dynamic_cast<const DTV*>(&destTypeNV.dataType()) != 0);
    const DTV &ddtv = static_cast<const DTV&>(destTypeNV.dataType());
    setResultType(new TNV(ddtv.dataType()));
    return ICode::OK;
}

/******************************************************************
  TreeNodeExprCast
******************************************************************/

ICode::Status TreeNodeExprCast::calculateResultType(SymbolTable &st,
                                                    CompileLog &log)
{
    if (haveResultType ())
        return ICode::OK;

    assert (children ().size () == 2);
    assert (dynamic_cast<TreeNodeDataTypeF*>(children ().at (0)));
    assert (dynamic_cast<TreeNodeExpr*>(children ().at (1)));

    TreeNodeDataTypeF* dType = static_cast<TreeNodeDataTypeF*>(children ().at (0));
    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children ().at (1));
    ICode::Status s = e->calculateResultType (st, log);
    if (s != ICode::OK) {
        return s;
    }

    if (e->checkAndLogIfVoid (log)) {
        return ICode::E_TYPE;
    }

    const TypeNonVoid* eType = static_cast<const TypeNonVoid*>(&e->resultType ());

    /// \todo right now we only allow identity casts
    if (dType->dataType () != eType->secrecDataType ()) {
        log.error () << "Illegal type cast at " << location ();
        return ICode::E_TYPE;
    }

    setResultType (new TypeNonVoid (
                       eType->secrecSecType (),
                       dType->dataType (),
                       eType->secrecDimType ()));
    return ICode::OK;
}

/******************************************************************
  TreeNodeExprIndex
******************************************************************/

ICode::Status TreeNodeExprIndex::calculateResultType(SymbolTable &st,
                                                     CompileLog &log)
{
    typedef TypeNonVoid TNV;

    if (haveResultType()) return ICode::OK;

    assert (children().size() == 2);
    assert (dynamic_cast<TreeNodeExpr* >(children().at(0)));
    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;

    TNV const* eType = static_cast<TNV const*>(&e->resultType());
    SecrecDimType k = 0;
    SecrecDimType n = eType->secrecDimType();

    if (std::distance(children().at(1)->children().begin(),
                      children().at(1)->children().end()) != n) {
        log.fatal() << "Incorrent number of indices at"
                    << e->location();
        return ICode::E_TYPE;
    }

    s = tyCheckIndices (children().at(1), st, log, k);
    if (s != ICode::OK) {
        return s;
    }

    setResultType(new TNV(eType->secrecSecType(), eType->secrecDataType(), k));
    return ICode::OK;
}


/******************************************************************
  TreeNodeExprSize
******************************************************************/

ICode::Status TreeNodeExprSize::calculateResultType(SymbolTable &st,
                                                    CompileLog &log)
{
    typedef TypeNonVoid TNV;

    if (haveResultType()) return ICode::OK;

    assert (children().size() == 1);
    assert (dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;

    setResultType(new TNV (DATATYPE_INT));
    return ICode::OK;
}

/******************************************************************
  TreeNodeExprShape
******************************************************************/

ICode::Status TreeNodeExprShape::calculateResultType(SymbolTable &st,
                                                     CompileLog &log)
{
    typedef TypeNonVoid TNV;

    if (haveResultType()) return ICode::OK;

    assert (children().size() == 1);
    assert (dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;

    setResultType(new TNV (DATATYPE_INT, 1));
    return ICode::OK;
}

/******************************************************************
  TreeNodeExprCat
******************************************************************/

ICode::Status TreeNodeExprCat::calculateResultType(SymbolTable &st,
                                                   CompileLog &log)
{
    typedef TypeNonVoid TNV;

    if (haveResultType()) return ICode::OK;

    assert (children().size() == 3 ||
            children().size() == 2);

    // missing argument is interpreted as 0
    if (children().size() == 2) {
        TreeNode* e = new TreeNodeExprInt(0, location());
        appendChild(e);
    }

    TNV const* eTypes[2];

    // check that first subexpressions 2 are arrays and of equal dimensionalities
    for (int i = 0; i < 2; ++ i) {
        assert (dynamic_cast<TreeNodeExpr*>(children().at(i)) != 0);
        TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(i));
        ICode::Status s = e->calculateResultType(st, log);
        if (s != ICode::OK) return s;
        if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;

        eTypes[i] = static_cast<TNV const*>(&e->resultType());
        if (eTypes[i]->isScalar()) {
            log.fatal() << "Concatenation of scalar values at "
                        << e->location();
            return ICode::E_TYPE;
        }
    }

    if (eTypes[0]->secrecDataType() != eTypes[1]->secrecDataType()) {
        log.fatal() << "Data types mismatch at "
                    << children().at(0)->location() << " and "
                    << children().at(1)->location();
        return ICode::E_TYPE;
    }

    if (eTypes[0]->secrecDimType() != eTypes[1]->secrecDimType()) {
        log.fatal() << "Dimensionalities mismatch at "
                    << children().at(0)->location() << " and "
                    << children().at(1)->location();
        return ICode::E_TYPE;
    }

    // type checker actually allows for aribtrary expression here
    // but right now parser expects integer literals, this is OK
    assert (dynamic_cast<TreeNodeExpr*>(children().at(2)) != 0);
    TreeNodeExpr* e3 = static_cast<TreeNodeExpr*>(children().at(2));
    ICode::Status s = e3->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (e3->checkAndLogIfVoid(log)) return ICode::E_TYPE;

    TNV const* e3Type = static_cast<TNV const*>(&e3->resultType());
    if (!e3Type->isScalar() ||
        e3Type->secrecDataType() != DATATYPE_INT ||
        isPrivate (e3Type->secrecSecType())) {
        log.fatal() << "Expected public scalar integer at "
                    << children().at(2)->location()
                    << " got " << *e3Type;
        return ICode::E_TYPE;
    }

    setResultType(new TNV(
            upperSecType(eTypes[0]->secrecSecType(), eTypes[1]->secrecSecType()),
            eTypes[0]->secrecDataType(),
            eTypes[0]->secrecDimType()));
    return ICode::OK;
}


/******************************************************************
  TreeNodeExprReshape
******************************************************************/

ICode::Status TreeNodeExprReshape::calculateResultType(SymbolTable &st,
                                                       CompileLog &log)
{
    typedef TypeNonVoid TNV;

    if (haveResultType()) return ICode::OK;

    assert (children().size() >= 1);
    assert (dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;

    TNV const* eType = static_cast<TNV const*>(&e->resultType());
    SecrecDimType resDim = 0;
    for (size_t i = 1; i < children().size(); ++ i, ++ resDim) {
        assert (dynamic_cast<TreeNodeExpr*>(children().at(i)) != 0);
        TreeNodeExpr* ei = static_cast<TreeNodeExpr*>(children().at(i));
        s = ei->calculateResultType(st, log);
        if (s != ICode::OK) return s;

        if (ei->checkAndLogIfVoid(log)) return ICode::E_TYPE;
        TNV const* eiType = static_cast<TNV const*>(&ei->resultType());
        if (eiType->secrecDataType() != DATATYPE_INT ||
            isPrivate (eiType->secrecSecType()) ||
            !eiType->isScalar()) {
            log.fatal() << "Expected public integer scalar at "
                        << ei->location()
                        << " got " << eiType->toString();
            return ICode::E_TYPE;
        }
    }

    if (resDim == 0) {
        log.fatal() << "Conversion from non-scalar to scalar at "
                    << location();
        return ICode::E_TYPE;
    }

    setResultType(new TNV(eType->secrecSecType(), eType->secrecDataType(), resDim));
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprFRead
*******************************************************************************/

ICode::Status TreeNodeExprFRead::calculateResultType(SymbolTable &st,
                                                      CompileLog &log)
{
    typedef TypeNonVoid TNV;
    if (haveResultType()) return ICode::OK;
    assert (children().size() == 1);
    assert (dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;
    TypeNonVoid const* eType = static_cast<TypeNonVoid const*>(&e->resultType());

    if (!eType->isScalar() ||
         eType->secrecDataType() != DATATYPE_STRING ||
         isPrivate (eType->secrecSecType())) {
        log.fatal() << "fread expression at " << location() << " has to take public scalar string as argument, got "
                    << *eType;
        return ICode::E_TYPE;
    }

    setResultType(new TypeNonVoid(SECTYPE_PRIVATE, DATATYPE_INT, 2));

    return ICode::OK;
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

ICode::Status TreeNodeExprBinary::calculateResultType(SymbolTable &st,
                                                      CompileLog &log)
{
    typedef TypeNonVoid TNV;

    if (haveResultType()) return ICode::OK;

    assert(children().size() == 2);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

    const SecreC::Type *eType1, *eType2;

    {
        assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
        TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
        ICode::Status s = e1->calculateResultType(st, log);
        if (s != ICode::OK) return s;
        eType1 = &e1->resultType();
    }
    {
        assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
        TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
        ICode::Status s = e2->calculateResultType(st, log);
        if (s != ICode::OK) return s;
        eType2 = &e2->resultType();
    }

    /// \todo implement more expressions
    if (!eType1->isVoid() && !eType2->isVoid()
#ifndef NDEBUG
        && (assert(dynamic_cast<const TNV*>(eType1) != 0), true)
        && (assert(dynamic_cast<const TNV*>(eType2) != 0), true)
#endif
        && static_cast<const TNV*>(eType1)->kind() == TNV::BASIC
        && static_cast<const TNV*>(eType2)->kind() == TNV::BASIC)
    {
        // Add implicit classify nodes if needed:
        {
            TreeNodeExpr *e1 = classifyChildAtIfNeeded(0, *eType2);
            ICode::Status s = e1->calculateResultType(st, log);
            if (s != ICode::OK) return s;
            eType1 = &e1->resultType();
        }
        {
            TreeNodeExpr *e2 = classifyChildAtIfNeeded(1, *eType1);
            ICode::Status s = e2->calculateResultType(st, log);
            if (s != ICode::OK) return s;
            eType2 = &e2->resultType();
        }


        SecrecDataType d1 = eType1->secrecDataType();
        SecrecDataType d2 = eType2->secrecDataType();
        SecrecSecType s1 = eType1->secrecSecType();
        SecrecSecType s2 = eType2->secrecSecType();
        SecrecDimType n1 = eType1->secrecDimType();
        SecrecDimType n2 = eType2->secrecDimType();

        if (n1 == 0 || n2 == 0 || n1 == n2) {
            switch (type()) {
                case NODE_EXPR_BINARY_ADD:
                    if (d1 != d2) break;
                    if ((d1 & (DATATYPE_INT|DATATYPE_UINT|DATATYPE_STRING)) == 0x0)
                        break;

                    setResultType(new TNV(upperSecType(s1, s2), d1, upperDimType(n1, n2)));
                    return ICode::OK;
                case NODE_EXPR_BINARY_SUB:
                case NODE_EXPR_BINARY_MUL:
                case NODE_EXPR_BINARY_MOD:
                case NODE_EXPR_BINARY_DIV:
                    if (d1 != d2) break;
                    if ((d1 & (DATATYPE_INT|DATATYPE_UINT)) == 0x0) break;
                    setResultType(new TNV(upperSecType(s1, s2), d1, upperDimType(n1, n2)));
                    return ICode::OK;
                case NODE_EXPR_BINARY_EQ:
                case NODE_EXPR_BINARY_GE:
                case NODE_EXPR_BINARY_GT:
                case NODE_EXPR_BINARY_LE:
                case NODE_EXPR_BINARY_LT:
                case NODE_EXPR_BINARY_NE:
                    if (d1 != d2) break;
                    setResultType(new TNV(upperSecType(s1, s2), DATATYPE_BOOL, upperDimType(n1, n2)));
                    return ICode::OK;
                case NODE_EXPR_BINARY_LAND:
                case NODE_EXPR_BINARY_LOR:
                    if (d1 != DATATYPE_BOOL || d2 != DATATYPE_BOOL) break;
                    setResultType(new TNV(upperSecType(s1, s2), DATATYPE_BOOL, upperDimType(n1, n2)));
                    return ICode::OK;
                case NODE_EXPR_BINARY_MATRIXMUL:
                    log.fatal() << "Matrix multiplication not yet supported. At "
                                << location();
                    return ICode::E_NOT_IMPLEMENTED;
                default:
                    assert(false);
            }
        }
    }

    log.fatal() << "Invalid binary operation " << operatorString()
                << " between operands of type " << *eType1 << " and " << *eType2
                << " at " << location();

    return ICode::E_TYPE;
}

/*******************************************************************************
  TreeNodeExprBool
*******************************************************************************/

std::string TreeNodeExprBool::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"bool:" << stringHelper() << "\"";
    return os.str();
}

ICode::Status TreeNodeExprBool::calculateResultType(SymbolTable &, CompileLog &)
{
    if (haveResultType()) return ICode::OK;

    assert(children().empty());

    setResultType(new TypeNonVoid(DATATYPE_BOOL));
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprClassify
*******************************************************************************/

ICode::Status TreeNodeExprClassify::calculateResultType(SymbolTable &st,
                                                        CompileLog &log)
{
    if (haveResultType()) return ICode::OK;

    assert(children().size() == 1);

    assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;
    assert(isPublic (e->resultType().secrecSecType()));
    setResultType(new TypeNonVoid(SECTYPE_PRIVATE, e->resultType().secrecDataType(), e->resultType().secrecDimType()));
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprDeclassify
*******************************************************************************/

ICode::Status TreeNodeExprDeclassify::calculateResultType(SymbolTable &st,
                                                          CompileLog &log)
{
    if (haveResultType()) return ICode::OK;

    assert(children().size() == 1);

    assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;

    if (!e->resultType().isVoid()) {
        if (isPrivate (e->resultType().secrecSecType())) {
            setResultType(new TypeNonVoid(
                e->resultType().secrecDataType(),
                e->resultType().secrecDimType()));
            return ICode::OK;
        }
    }

    log.fatal() << "Argument of type " << e->resultType()
                << " passed to declassify operator at " << location();

    return ICode::E_TYPE;
}

/*******************************************************************************
  TreeNodeExprProcCall
*******************************************************************************/

ICode::Status TreeNodeExprProcCall::calculateResultType(SymbolTable &st,
                                                        CompileLog &log)
{
    typedef DataTypeProcedureVoid DTFV;
    typedef DataTypeProcedure DTF;
    typedef ChildrenListConstIterator CLCI;

    if (haveResultType()) return ICode::OK;

    // Get identifier:
    assert(!children().empty());
    assert(children().at(0)->type() == NODE_IDENTIFIER);
    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));

    // Check whether the function name is overridden by some variable name:
    Symbol *s = st.find(id->value());
    if (s != 0 && s->symbolType() != Symbol::PROCEDURE) {
        log.fatal() << "Identifier " << id->value() << " is not a function at "
                    << location();
        return ICode::E_TYPE;
    }

    DataTypeProcedureVoid dataType;
    std::vector<TreeNodeExpr*> arguments(children().size() - 1);

    // Calculate the vector of types of the arguments:
    int i = 0;
    for (CLCI it(children().begin() + 1); it != children().end(); it++, i++) {
        assert(((*it)->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(*it) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(*it);

        ICode::Status s = e->calculateResultType(st, log);
        if (s != ICode::OK) return s;
        if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;
        assert(dynamic_cast<const TypeNonVoid*>(&e->resultType()) != 0);
        const TypeNonVoid &t = static_cast<const TypeNonVoid&>(e->resultType());
        dataType.addParamType(t.dataType());
        arguments[i] = e;
    }

    // Search for the procedure by its name and the data types of its arguments:
    m_procedure = st.findGlobalProcedure(id->value(), dataType);
    if (m_procedure == 0) {
        log.fatal() << "No function with parameter data types of ("
                    << dataType.mangle() << ") found in scope at "
                    << location();
        return ICode::E_TYPE;
    }

    const TypeNonVoid &ft = m_procedure->decl()->procedureType();
    assert(ft.kind() == TypeNonVoid::PROCEDURE
           || ft.kind() == TypeNonVoid::PROCEDUREVOID);
    assert(dynamic_cast<const DTFV*>(&ft.dataType()) != 0);
    const DTFV &rstv = static_cast<const DTFV&>(ft.dataType());

    // Check security types of parameters:
    assert(rstv.paramTypes().size() == children().size() - 1);
    for (unsigned i = 0; i < rstv.paramTypes().size(); i++) {
        assert(rstv.paramTypes().at(i)->kind() == DataType::BASIC);
        assert(dynamic_cast<DataTypeBasic*>(rstv.paramTypes().at(i)) != 0);
        DataTypeBasic *need = static_cast<DataTypeBasic*>(rstv.paramTypes()[i]);

        assert(dataType.paramTypes().at(i)->kind() == DataType::BASIC);
        assert(dynamic_cast<DataTypeBasic*>(dataType.paramTypes().at(i)) != 0);
        DataTypeBasic *have = static_cast<DataTypeBasic*>(dataType.paramTypes()[i]);

        assert(need->secType() != SECTYPE_INVALID);
        assert(have->secType() != SECTYPE_INVALID);

        if (isPublic (need->secType()) && isPrivate (have->secType()))
        {
            log.fatal() << "Argument " << (i + 1) << " to function "
                << id->value() << " at " << arguments[i]->location()
                << " is expected to be of public type instead of private!";
            return ICode::E_TYPE;
        }

        if (need->dimType() != have->dimType()) {
            log.fatal() << "Argument " << (i + 1) << " to function "
                << id->value() << " at " << arguments[i]->location()
                << " has mismatching dimensionality!";
            return ICode::E_TYPE;
        }

        // Add implicit classify node if needed:
        classifyChildAtIfNeeded(i + 1, TypeNonVoid (*need));
    }

    // Set result type:
    if (ft.kind() == TypeNonVoid::PROCEDURE) {
        assert(dynamic_cast<const DTF*>(&ft.dataType()) != 0);
        const DTF &rdt = static_cast<const DTF&>(ft.dataType());
        setResultType(new TypeNonVoid(rdt.returnType()));
    } else {
        setResultType(new TypeVoid);
    }

    return ICode::OK;
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

ICode::Status TreeNodeExprInt::calculateResultType(SymbolTable &, CompileLog &)
{
    if (haveResultType()) return ICode::OK;

    assert(children().empty());

    setResultType(new TypeNonVoid(DATATYPE_INT));
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprRVariable
*******************************************************************************/

ICode::Status TreeNodeExprRVariable::calculateResultType(SymbolTable &st,
                                                         CompileLog &log)
{
    typedef DataTypeVar DTV;
    typedef TypeNonVoid TNV;

    if (haveResultType()) return ICode::OK;

    assert(children().size() == 1);
    assert(children().at(0)->type() == NODE_IDENTIFIER);

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));
    SymbolSymbol *s = id->getSymbol(st, log);
    if (s == 0) return ICode::E_OTHER;

    assert(s->secrecType().isVoid() == false);
    assert(dynamic_cast<const TNV*>(&s->secrecType()) != 0);
    const TNV *type = static_cast<const TNV*>(&s->secrecType());
    assert(type->dataType().kind() == DataType::VAR);
    assert(dynamic_cast<const DTV*>(&type->dataType()) != 0);
    setResultType(new TNV(static_cast<const DTV&>(type->dataType()).dataType())
                  );

    return ICode::OK;
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

ICode::Status TreeNodeExprString::calculateResultType(SymbolTable &,
                                                      CompileLog &)
{
    if (haveResultType()) return ICode::OK;

    assert(children().empty());

    setResultType(new TypeNonVoid(DATATYPE_STRING));
    return ICode::OK;
}


/*******************************************************************************
  TreeNodeExprTernary
*******************************************************************************/

ICode::Status TreeNodeExprTernary::calculateResultType(SymbolTable &st,
                                                       CompileLog &log)
{
    if (haveResultType()) return ICode::OK;

    assert(children().size() == 3);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(2)->type() & NODE_EXPR_MASK) != 0x0);

    assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e1->calculateResultType(st, log);
    if (e1->checkAndLogIfVoid(log)) return ICode::E_TYPE;
    if (s != ICode::OK) return s;

    assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->calculateResultType(st, log);
    if (s != ICode::OK) return s;

    assert(dynamic_cast<TreeNodeExpr*>(children().at(2)) != 0);
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2));
    s = e3->calculateResultType(st, log);
    if (s != ICode::OK) return s;

    const SecreC::Type &eType1 = e1->resultType();
    const SecreC::Type &eType2 = e2->resultType();
    const SecreC::Type &eType3 = e3->resultType();

    assert(dynamic_cast<const TypeNonVoid*>(&eType1) != 0);
    const TypeNonVoid &cType = static_cast<const TypeNonVoid&>(eType1);

    // check if conditional expression is of public boolean type
    if (cType.kind() != TypeNonVoid::BASIC
        || cType.dataType().kind() != DataType::BASIC
        || static_cast<const DataTypeBasic&>(cType.dataType()).dataType()
            != DATATYPE_BOOL
        || isPrivate (static_cast<const DataTypeBasic&>(cType.dataType()).secType()))
    {
        log.fatal() << "Conditional subexpression at " << e1->location()
                    << " of ternary expression has to be public boolean, got "
                    << cType;
        return ICode::E_TYPE;
    }

    // check the types of results
    if (eType2.isVoid() != eType3.isVoid()) {
        log.fatal() << "Subxpression at " << e2->location() << " is "
                    << (eType2.isVoid() ? "" : "not")
                    << " void while subexpression at " << e3->location()
                    << (eType3.isVoid() ? " is" : " isn't");
        return ICode::E_TYPE;

    }

    if (!eType2.isVoid()) {
        if (eType2.secrecDataType() != eType3.secrecDataType()) {
            log.fatal() << "Results of ternary expression  at "
                        << location()
                        << " have to be of same data types, got "
                        << eType2 << " and " << eType3;
            return ICode::E_TYPE;
        }

        SecrecDimType n1 = eType1.secrecDimType();
        SecrecDimType n2 = eType2.secrecDimType();
        SecrecDimType n3 = eType2.secrecDimType();

        if (n2 != n3) {
            log.fatal() << "Results of ternary expression at "
                        << location()
                        << " aren't of equal dimensionalities";
            return ICode::E_TYPE;
        }

        if (n1 != 0 && n1 != n2) {
            log.fatal() << "Conditional expression at "
                        << e1->location()
                        << " is non-scalar and doesn't match resulting subexpressions";
            return ICode::E_TYPE;
        }
    }

    // Add implicit classify nodes if needed:
    e2 = classifyChildAtIfNeeded(1, eType3);
    s = e2->calculateResultType(st, log);
    if (s != ICode::OK) return s;

    e3 = classifyChildAtIfNeeded(2, eType2);
    s = e3->calculateResultType(st, log);
    if (s != ICode::OK) return s;

    assert(e2->resultType() == e3->resultType());

    setResultType(e2->resultType().clone());
    return ICode::OK;
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

ICode::Status TreeNodeExprUInt::calculateResultType(SymbolTable &, CompileLog &)
{
    if (haveResultType()) return ICode::OK;

    assert(children().empty());

    setResultType(new TypeNonVoid(DATATYPE_UINT));
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprUnary
*******************************************************************************/

ICode::Status TreeNodeExprUnary::calculateResultType(SymbolTable &st,
                                                     CompileLog &log)
{
    typedef DataTypeBasic DTB;

    if (haveResultType()) return ICode::OK;

    assert(type() == NODE_EXPR_UMINUS || type() == NODE_EXPR_UNEG);
    assert(children().size() == 1);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);

    assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    const SecreC::Type &eType = e->resultType();

    /// \todo implement for matrixes also
    if (!eType.isVoid()
#ifndef NDEBUG
        && (assert(dynamic_cast<const TypeNonVoid*>(&eType) != 0), true)
#endif
        && static_cast<const TypeNonVoid&>(eType).kind() == TypeNonVoid::BASIC)
    {
        const TypeNonVoid &et = static_cast<const TypeNonVoid&>(eType);
        assert(dynamic_cast<const DTB*>(&et.dataType()) != 0);
        const DTB &bType = static_cast<const DTB&>(et.dataType());
        if (type() == NODE_EXPR_UNEG && bType.dataType() == DATATYPE_BOOL) {
            setResultType(et.clone());
            return ICode::OK;
        } else if (type() == NODE_EXPR_UMINUS) {
            if (bType.dataType() == DATATYPE_INT) {
                setResultType(et.clone());
                return ICode::OK;
            }
        }
    }

    log.fatal() << "Invalid expression of type (" << eType
                << ") given to unary "
                << (type() == NODE_EXPR_UNEG ? "negation" : "minus")
                << " operator at " << location();

    return ICode::E_TYPE;
}

/*******************************************************************************
  TreeNodeExprPostfix
*******************************************************************************/

ICode::Status TreeNodeExprPostfix::calculateResultType(SymbolTable &st,
                                                       CompileLog &log)
{
    if (haveResultType()) return ICode::OK;

    assert(type() == NODE_EXPR_POSTFIX_INC ||
           type() == NODE_EXPR_POSTFIX_DEC);
    assert(children().size() == 1);
    assert((children().at(0)->type() == NODE_LVALUE) != 0x0);
    TreeNode* lval = children().at(0);
    assert (lval->children ().size () <= 2);
    assert(dynamic_cast<TreeNodeIdentifier* >(lval->children ().at(0)) != 0);

    TreeNodeIdentifier *e = static_cast<TreeNodeIdentifier*>(lval->children ().at(0));
    const SecreC::Type &eType = e->getSymbol (st, log)->secrecType ();
    unsigned destDim = eType.secrecDimType ();
    if (lval->children ().size () == 2) {
        ICode::Status s = tyCheckIndices (lval->children ().at (1), st, log, destDim);
        if (s != ICode::OK) {
            return s;
        }
    }

    // check that argument is a variable
    if (e->type () == NODE_EXPR_RVARIABLE) {
        log.fatal() << "Postfix "
                    << (type() == NODE_EXPR_POSTFIX_INC ? "increment" : "decrement")
                    << " expects variable at " << location();
        return ICode::E_TYPE;
    }

    // increment or decrement of void
    if (eType.isVoid ()) {
        log.fatal() << "Invalid expression of void type given to postfix "
                    << (type() == NODE_EXPR_POSTFIX_INC ? "increment" : "decrement")
                    << " operator at " << location();
        return ICode::E_TYPE;
    }

    // check that we are operating on numeric types
    if (!isNumericDataType (eType.secrecDataType ())) {
        log.fatal() << "Postfix "
                    << (type() == NODE_EXPR_POSTFIX_INC ? "increment" : "decrement")
                    << " operator expects numeric type, given "
                    << eType << " at " << location();
        return ICode::E_TYPE;
    }

    setResultType (new TypeNonVoid (
                       eType.secrecSecType (),
                       eType.secrecDataType (),
                       destDim));
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeExprPrefix
*******************************************************************************/

ICode::Status TreeNodeExprPrefix::calculateResultType(SymbolTable &st,
                                                       CompileLog &log)
{
    if (haveResultType()) return ICode::OK;

    assert (type() == NODE_EXPR_PREFIX_INC || type() == NODE_EXPR_PREFIX_DEC);
    assert (children().size() == 1);

    TreeNode* lval = children().at(0);
    assert (lval->type() == NODE_LVALUE);
    assert (lval->children ().size () > 0 && lval->children().size() <= 2);
    assert (dynamic_cast<TreeNodeIdentifier* >(lval->children ().at(0)) != 0);

    TreeNodeIdentifier *e = static_cast<TreeNodeIdentifier*>(lval->children ().at(0));
    const SecreC::Type &eType = e->getSymbol (st, log)->secrecType ();
    unsigned destDim = eType.secrecDimType ();
    if (lval->children ().size () == 2) {
        ICode::Status s = tyCheckIndices (lval->children ().at (1), st, log, destDim);
        if (s != ICode::OK) {
            return s;
        }
    }

    // check that argument is a variable
    if (e->type () == NODE_EXPR_RVARIABLE) {
        log.fatal() << "Prefix "
                    << (type() == NODE_EXPR_PREFIX_INC ? "increment" : "decrement")
                    << " expects variable at " << location();
        return ICode::E_TYPE;
    }

    // increment or decrement of void
    if (eType.isVoid ()) {
        log.fatal() << "Invalid expression of void type given to prefix "
                    << (type() == NODE_EXPR_PREFIX_INC ? "increment" : "decrement")
                    << " operator at " << location();
        return ICode::E_TYPE;
    }

    // check that we are operating on numeric types
    if (!isNumericDataType (eType.secrecDataType ())) {
        log.fatal() << "Prefix "
                    << (type() == NODE_EXPR_PREFIX_INC ? "increment" : "decrement")
                    << " operator expects numeric type, given "
                    << eType << " at " << location();
        return ICode::E_TYPE;
    }

    setResultType (new TypeNonVoid (
                       eType.secrecSecType (),
                       eType.secrecDataType (),
                       destDim));
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeProcDef
*******************************************************************************/

const std::string &TreeNodeProcDef::procedureName() const {
    assert(children().size() >= 3);
    assert(dynamic_cast<const TreeNodeIdentifier*>(children().at(0)) != 0);
    return static_cast<const TreeNodeIdentifier*>(children().at(0))->value();
}

ICode::Status TreeNodeProcDef::calculateProcedureType(SymbolTable &stable,
                                                      CompileLog &log)
{
    typedef TypeNonVoid TNV;

    if (m_cachedType != 0) return ICode::OK;

    assert(dynamic_cast<TreeNodeType*>(children().at(1)) != 0);
    TreeNodeType *rt = static_cast<TreeNodeType*>(children().at(1));

    if (rt->type() == NODE_TYPEVOID) {
        assert(rt->secrecType().isVoid());

        DataTypeProcedureVoid dt;
        ICode::Status s = addParameters(dt, stable, log);
        if (s != ICode::OK) return s;
        m_cachedType = new TNV(dt);
    } else {
        assert(rt->type() == NODE_TYPETYPE);
        assert(!rt->secrecType().isVoid());
        assert(dynamic_cast<const TNV*>(&rt->secrecType()) != 0);
        const TNV &tt = static_cast<const TNV&>(rt->secrecType());
        assert(tt.dataType().kind() == DataType::BASIC);

        DataTypeProcedure dt(tt.dataType());
        ICode::Status s = addParameters(dt, stable, log);
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
    os << m_secType;
    return os.str();
}

std::string TreeNodeSecTypeF::xmlHelper() const {
    std::ostringstream os;
    os << "type=\"" << m_secType << "\"";
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
            ICode::Status s = e->calculateResultType(st, log);
            if (s != ICode::OK) return s;
            if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;
            if (   !e->resultType().secrecDataType() == DATATYPE_INT
                || !e->resultType().isScalar()
                || isPrivate (e->resultType().secrecSecType())) {
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

        if (isPrivate (e->resultType().secrecSecType()) &&
            isPublic (justType.secrecSecType()))
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

const SecreC::Type &TreeNodeTypeType::secrecType() const {
    typedef TreeNodeDataTypeF TNDT;
    typedef TreeNodeSecTypeF TNST;
    typedef TreeNodeDimTypeF TNDimT;

    assert(children().size() == 3);
    if (m_cachedType != 0) return *m_cachedType;

    assert(dynamic_cast<TNST*>(children().at(0)) != 0);
    TNST *st = static_cast<TNST*>(children().at(0));
    assert(dynamic_cast<TNDT*>(children().at(1)) != 0);
    TNDT *dt = static_cast<TNDT*>(children().at(1));
    assert(dynamic_cast<TNDimT*>(children().at(2)) != 0);
    TNDimT *dimt = static_cast<TNDimT*>(children().at(2));

    m_cachedType = new SecreC::TypeNonVoid(st->secType(), dt->dataType(), dimt->dimType());
    return *m_cachedType;
}

std::string TreeNodeTypeType::stringHelper() const {
    return secrecType().toString();
}

} // namespace SecreC
