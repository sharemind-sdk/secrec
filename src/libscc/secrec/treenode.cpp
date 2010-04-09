#include "secrec/treenode.h"

#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include "secrec/symboltable.h"
#include "misc.h"


namespace {

void patchList(std::vector<SecreC::Imop*> &list,
               SecreC::Imop *dest)
{
    typedef std::vector<SecreC::Imop*>::const_iterator IVCI;
    for (IVCI it(list.begin()); it != list.end(); it++) {
        (*it)->setJumpDest(dest);
    }
    list.clear();
}

template <class T>
inline void appendVectorToVector(std::vector<T> &dst,
                                 const std::vector<T> &src)
{
    dst.insert(dst.end(), src.begin(), src.end());
}

} // anonymous namespace

namespace SecreC {

TreeNode::TreeNode(Type type, const struct YYLTYPE &loc)
    : m_parent(0), m_function(0), m_type(type), m_location(loc)
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

TreeNodeFundef* TreeNode::containingFunction() {
    if (m_function != 0) return m_function;
    if (m_parent != 0) {
        return (m_function = m_parent->containingFunction());
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
        case NODE_FUNDEF: return "FUNDEF";
        case NODE_FUNDEFS: return "FUNDEFS";
        case NODE_PROGRAM: return "PROGRAM";

        case NODE_TYPETYPE: return "TYPETYPE";
        case NODE_TYPEVOID: return "TYPEVOID";
        case NODE_DATATYPE_F: return "DATATYPE_F";
        case NODE_DATATYPE_ARRAY: return "DATATYPE_ARRAY";
        case NODE_SECTYPE_F: return "SECTYPE_F";
        default: return "UNKNOWN";
    }
}

std::string TreeNode::toString(unsigned indent, unsigned startIndent)
        const
{
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

} // namespace SecreC

/********************************************************************************************
  C interface for Yacc
********************************************************************************************/

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
        case NODE_FUNDEF:
            return (TreeNode*) (new SecreC::TreeNodeFundef(*loc));
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
        case NODE_EXPR_RVARIABLE:
            return (TreeNode*) (new SecreC::TreeNodeExprRVariable(*loc));
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
        case NODE_STMT_FOR:
            return (TreeNode*) (new SecreC::TreeNodeStmtFor(*loc));
        case NODE_STMT_IF:
            return (TreeNode*) (new SecreC::TreeNodeStmtIf(*loc));
        case NODE_STMT_RETURN:
            return (TreeNode*) (new SecreC::TreeNodeStmtReturn(*loc));
        case NODE_STMT_WHILE:
            return (TreeNode*) (new SecreC::TreeNodeStmtWhile(*loc));
        case NODE_DECL:
            return (TreeNode*) (new SecreC::TreeNodeStmtDecl(*loc));
        case NODE_TYPETYPE:
            return (TreeNode*) (new SecreC::TreeNodeTypeType(*loc));
        case NODE_TYPEVOID:
            return (TreeNode*) (new SecreC::TreeNodeTypeVoid(*loc));
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
    return (TreeNode*) ((const SecreC::TreeNode*) node)->children().at(index);
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

    return (TreeNode*) new SecreC::TreeNodeExprBool(value, *loc);
}

extern "C" struct TreeNode *treenode_init_int(int value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeExprInt(value, *loc);
}

extern "C" struct TreeNode *treenode_init_uint(unsigned value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeExprUInt(value, *loc);
}

extern "C" struct TreeNode *treenode_init_string(const char *value,
                                                 YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeExprString(value, *loc);
}

extern "C" struct TreeNode *treenode_init_identifier(const char *value,
                                                     YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeIdentifier(value, *loc);
}

extern "C" struct TreeNode *treenode_init_secTypeF(
        enum SecrecSecType secType,
        YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeSecTypeF(secType, *loc);
}

extern "C" struct TreeNode *treenode_init_dataTypeF(
        enum SecrecDataType dataType,
        YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeDataTypeF(dataType, *loc);
}

extern "C" struct TreeNode *treenode_init_dataTypeArray(
        unsigned value,
        YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeDataTypeArray(value, *loc);
}

std::ostream &operator<<(std::ostream &out, const YYLTYPE &loc) {
    /// \todo filename
    out << "(" << loc.first_line << "," << loc.first_column
        << ")(" << loc.last_line << "," << loc.last_column << ")";
    return out;
}

namespace SecreC {


/*******************************************************************************
  TreeNodeCodeable
*******************************************************************************/

void TreeNodeCodeable::patchBreakList(Imop *dest) {
    patchList(m_breakList, dest);
}

void TreeNodeCodeable::patchContinueList(Imop *dest) {
    patchList(m_continueList, dest);
}

void TreeNodeCodeable::patchNextList(Imop *dest) {
    patchList(m_nextList, dest);
}

void TreeNodeCodeable::addToBreakList(const std::vector<Imop*> &bl) {
    appendVectorToVector(m_breakList, bl);
}

void TreeNodeCodeable::addToContinueList(const std::vector<Imop*> &cl) {
    appendVectorToVector(m_continueList, cl);
}

void TreeNodeCodeable::addToNextList(const std::vector<Imop*> &nl) {
    appendVectorToVector(m_nextList, nl);
}

/*******************************************************************************
  TreeNodeStmtCompound
*******************************************************************************/

ICode::Status TreeNodeStmtCompound::generateCode(ICode::CodeList &code,
                                                 SymbolTable &st,
                                                 std::ostream &es)
{
    typedef ChildrenListConstIterator CLCI;

    TreeNodeStmt *last = 0;
    setResultFlags(TreeNodeStmt::FALLTHRU);

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert(dynamic_cast<TreeNodeStmt*>(*it) != 0);
        TreeNodeStmt *c = static_cast<TreeNodeStmt*>(*it);
        ICode::Status s = c->generateCode(code, st, es);
        if (s != ICode::OK) return s;

        assert(c->resultFlags() != 0x0);

        if (c->firstImop() == 0) {
            if (c->type() != NODE_DECL) {
                es << "Statement with no effect at " << c->location() << std::endl;
                return ICode::E_OTHER;
            }
        } else if (firstImop() == 0) {
            setFirstImop(c->firstImop());
        }

        addToBreakList(c->breakList());
        addToContinueList(c->continueList());
        if (last != 0) {
            // Static checking:
            if ((resultFlags() & TreeNodeStmt::FALLTHRU) == 0x0) {
                es << "Unreachable statement at " << c->location() << std::endl;
                return ICode::E_OTHER;
            } else {
                setResultFlags((resultFlags() & ~TreeNodeStmt::FALLTHRU) | c->resultFlags());
            }

            last->patchNextList(c->firstImop());
        } else {
            setResultFlags(c->resultFlags());
        }
        last = c;
    }
    if (last != 0) {
        addToNextList(last->nextList());
    }

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeDataTypeArray
*******************************************************************************/

const DataType &TreeNodeDataTypeArray::dataType() const {
    typedef TreeNodeDataType TNDT;

    assert(children().size() == 1);
    if (m_cachedType != 0) return *m_cachedType;

    assert(dynamic_cast<TNDT*>(children().at(0)) != 0);
    TNDT *t = static_cast<TNDT*>(children().at(0));

    m_cachedType = new SecreC::DataTypeArray(t->dataType(), m_dim);
    return *m_cachedType;
}

std::string TreeNodeDataTypeArray::stringHelper() const {
    std::ostringstream os;
    os << m_dim;
    return os.str();
}

std::string TreeNodeDataTypeArray::xmlHelper() const {
    std::ostringstream os;
    os << "dim=\"" << m_dim << "\"";
    return os.str();
}


/*******************************************************************************
  TreeNodeDataTypeF
*******************************************************************************/

std::string TreeNodeDataTypeF::stringHelper() const {
    std::ostringstream os;
    os << m_cachedType;
    return os.str();
}

std::string TreeNodeDataTypeF::xmlHelper() const {
    std::ostringstream os;
    os << "type=\"" << m_cachedType << "\"";
    return os.str();
}


/*******************************************************************************
  TreeNodeExpr
*******************************************************************************/

void TreeNodeExpr::patchTrueList(Imop *dest) {
    patchList(m_trueList, dest);
}

void TreeNodeExpr::patchFalseList(Imop *dest) {
    patchList(m_falseList, dest);
}

void TreeNodeExpr::patchNextList(Imop *dest) {
    patchList(m_nextList, dest);
}

void TreeNodeExpr::addToFalseList(const std::vector<Imop*> &fl) {
    appendVectorToVector(m_falseList, fl);
}

void TreeNodeExpr::addToTrueList(const std::vector<Imop*> &tl) {
    appendVectorToVector(m_trueList, tl);
}

void TreeNodeExpr::addToNextList(const std::vector<Imop*> &nl) {
    appendVectorToVector(m_nextList, nl);
}

/*******************************************************************************
  TreeNodeExprAssign
*******************************************************************************/

ICode::Status TreeNodeExprAssign::calculateResultType(SymbolTable &st,
                                                      std::ostream &es)
{
    if (resultType() != 0) return ICode::OK;

    assert(children().size() == 2);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));
    SymbolSymbol *dest = id->getSymbol(st, es);
    if (dest == 0) return ICode::E_OTHER;

    assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
    TreeNodeExpr *src = static_cast<TreeNodeExpr*>(children().at(1));
    ICode::Status s = src->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    const SecreC::Type &destType = dest->secrecType();
    assert(destType.isVoid() == false);
    const SecreC::Type *srcType = src->resultType();

    /// \todo implement more expressions
    assert(dynamic_cast<const TypeNonVoid*>(srcType) != 0);
    const TypeNonVoid* srcTypeNV = static_cast<const TypeNonVoid*>(srcType);
    assert(dynamic_cast<const TypeNonVoid*>(&destType) != 0);
    const TypeNonVoid* destTypeNV = static_cast<const TypeNonVoid*>(&destType);

    if (destTypeNV->kind() == TypeNonVoid::VAR
        && srcTypeNV->kind() == TypeNonVoid::BASIC)
    {
        assert(dynamic_cast<const DataTypeVar*>(&destTypeNV->dataType()) != 0);
        const DataTypeVar &destTypeVar = static_cast<const DataTypeVar&>(destTypeNV->dataType());

        assert(dynamic_cast<const DataTypeBasic*>(&srcTypeNV->dataType()) != 0);
        const DataTypeBasic &srcTypeBasic = static_cast<const DataTypeBasic&>(srcTypeNV->dataType());

        if (destTypeVar.equivalentTo(srcTypeBasic)) {
            assert(srcTypeNV->secType().kind() == SecType::BASIC);
            assert(dynamic_cast<const SecTypeBasic*>(&srcTypeNV->secType()) != 0);
            const SecTypeBasic &sst = static_cast<const SecTypeBasic&>(srcTypeNV->secType());

            assert(destTypeNV->secType().kind() == SecType::BASIC);
            assert(dynamic_cast<const SecTypeBasic*>(&destTypeNV->secType()) != 0);
            const SecTypeBasic &tst = static_cast<const SecTypeBasic&>(destTypeNV->secType());

            assert(sst.secType() != SECTYPE_INVALID);
            assert(tst.secType() != SECTYPE_INVALID);

            if (tst.secType() == SECTYPE_PUBLIC && sst.secType() == SECTYPE_PRIVATE) {
                es << "Invalid assignment of private value to public variable." << std::endl;
                return ICode::E_OTHER;
            }

            setResultType(srcType->clone());
            return ICode::OK;
        }
    }

    /// \todo Provide better error messages
    es << "Invalid assignment from value of type " << *srcTypeNV
       << " to variable of type " << *destTypeNV << ". At " << location() << std::endl;

    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprAssign::generateCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es,
                                               SymbolWithValue *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate code for child expressions:
    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *e1 = static_cast<TreeNodeIdentifier*>(children().at(0));
    Symbol *destSym = st.find(e1->value());
    assert(destSym->symbolType() == Symbol::SYMBOL);
    assert(dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol *destSymSym = static_cast<SymbolSymbol*>(destSym);

    assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));

    // Generate code for binary expression:
    switch (type()) {
        case NODE_EXPR_ASSIGN:
            s = e2->generateCode(code, st, es, destSymSym);
            if (s != ICode::OK) return s;
            break;
        case NODE_EXPR_ASSIGN_MUL: /* Fall through: */
        case NODE_EXPR_ASSIGN_DIV: /* Fall through: */
        case NODE_EXPR_ASSIGN_MOD: /* Fall through: */
        case NODE_EXPR_ASSIGN_ADD: /* Fall through: */
        case NODE_EXPR_ASSIGN_SUB: /* Fall through: */
        default:
            /// \todo Write better error message
            es << "This kind of assignement is not yet implemented. At " << location()
               << std::endl;
            return ICode::E_NOT_IMPLEMENTED;
    }

    setFirstImop(e2->firstImop());

    if (r != 0) {
        Imop *i = new Imop(Imop::ASSIGN);
        i->setDest(r);
        i->setArg1(destSymSym);
        code.push_back(i);
        patchFirstImop(i);
        e2->patchNextList(i);
    } else {
        setNextList(e2->nextList());
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprAssign::generateBoolCode(ICode::CodeList &, SymbolTable &st,
                                                   std::ostream &es)
{
    /// \todo Write assertion about return type

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    /// \todo Implement

    return ICode::E_NOT_IMPLEMENTED;
}


/*******************************************************************************
  TreeNodeExprBinary
*******************************************************************************/

ICode::Status TreeNodeExprBinary::calculateResultType(SymbolTable &st,
                                                      std::ostream &es)
{
    if (resultType() != 0) return ICode::OK;

    assert(children().size() == 2);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e1->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    const SecreC::Type *eType1 = static_cast<const TreeNodeExpr*>(e1)->resultType();
    const SecreC::Type *eType2 = static_cast<const TreeNodeExpr*>(e2)->resultType();

    /// \todo implement more expressions
    if (!eType1->isVoid() && !eType2->isVoid()
#ifndef NDEBUG
        && (assert(dynamic_cast<const TypeNonVoid*>(eType1) != 0), true)
        && (assert(dynamic_cast<const TypeNonVoid*>(eType2) != 0), true)
#endif
        && static_cast<const TypeNonVoid*>(eType1)->kind() == TypeNonVoid::BASIC
        && static_cast<const TypeNonVoid*>(eType2)->kind() == TypeNonVoid::BASIC)
    {
        const TypeNonVoid *et1 = static_cast<const TypeNonVoid*>(eType1);
        const TypeNonVoid *et2 = static_cast<const TypeNonVoid*>(eType2);
        assert(dynamic_cast<const DataTypeBasic*>(&et1->dataType()) != 0);
        assert(dynamic_cast<const SecTypeBasic*>(&et1->secType()) != 0);
        assert(dynamic_cast<const DataTypeBasic*>(&et2->dataType()) != 0);
        assert(dynamic_cast<const SecTypeBasic*>(&et2->secType()) != 0);
        SecrecDataType d1 = static_cast<const DataTypeBasic*>(&et1->dataType())->dataType();
        SecrecSecType s1 = static_cast<const SecTypeBasic*>(&et1->secType())->secType();
        SecrecDataType d2 = static_cast<const DataTypeBasic*>(&et2->dataType())->dataType();
        SecrecSecType s2 = static_cast<const SecTypeBasic*>(&et2->secType())->secType();

        switch (type()) {
            case NODE_EXPR_ADD:
                if (d1 != d2) break;
                if ((d1 & (DATATYPE_INT|DATATYPE_UINT|DATATYPE_STRING)) == 0x0) break;
                setResultType(new SecreC::TypeNonVoid(upperSecType(s1, s2), d1));
                return ICode::OK;
            case NODE_EXPR_SUB:
            case NODE_EXPR_MUL:
            case NODE_EXPR_MOD:
            case NODE_EXPR_DIV:
                if (d1 != d2) break;
                if ((d1 & (DATATYPE_INT|DATATYPE_UINT)) == 0x0) break;
                setResultType(new SecreC::TypeNonVoid(upperSecType(s1, s2), d1));
                return ICode::OK;
            case NODE_EXPR_EQ:
            case NODE_EXPR_GE:
            case NODE_EXPR_GT:
            case NODE_EXPR_LE:
            case NODE_EXPR_LT:
            case NODE_EXPR_NE:
                if (d1 != d2) break;
                setResultType(new SecreC::TypeNonVoid(upperSecType(s1, s2), DATATYPE_BOOL));
                return ICode::OK;
            case NODE_EXPR_LAND:
            case NODE_EXPR_LOR:
                if (d1 != DATATYPE_BOOL || d2 != DATATYPE_BOOL) break;
                setResultType(new SecreC::TypeNonVoid(upperSecType(s1, s2), DATATYPE_BOOL));
                return ICode::OK;
            default:
                /// \todo Write better error message
                es << "This kind of binary operation is not yet supported. At "
                   << location() << std::endl;
                return ICode::E_NOT_IMPLEMENTED;
        }
    }

    /// \todo Write better error message
    es << "Invalid binary operation at " << location() << std::endl;

    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprBinary::generateCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es,
                                               SymbolWithValue *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        SecreC::Type *rt = resultType();
        setResult(st.appendTemporary(*rt));
    } else {
        assert(r->secrecType().canAssign(*resultType()));
        setResult(r);
    }

    // Generate code for child expressions:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    setFirstImop(e1->firstImop());

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    patchFirstImop(e2->firstImop());

    // Generate code for binary expression:
    Imop *i;
    switch (type()) {
        case NODE_EXPR_ADD:  i = new Imop(Imop::ADD);  break;
        case NODE_EXPR_SUB:  i = new Imop(Imop::SUB);  break;
        case NODE_EXPR_MUL:  i = new Imop(Imop::MUL);  break;
        case NODE_EXPR_DIV:  i = new Imop(Imop::DIV);  break;
        case NODE_EXPR_MOD:  i = new Imop(Imop::MOD);  break;
        case NODE_EXPR_EQ:   i = new Imop(Imop::EQ);   break;
        case NODE_EXPR_GE:   i = new Imop(Imop::GE);   break;
        case NODE_EXPR_GT:   i = new Imop(Imop::GT);   break;
        case NODE_EXPR_LE:   i = new Imop(Imop::LE);   break;
        case NODE_EXPR_LT:   i = new Imop(Imop::LT);   break;
        case NODE_EXPR_NE:   i = new Imop(Imop::NE);   break;
        case NODE_EXPR_LAND: i = new Imop(Imop::LAND); break;
        case NODE_EXPR_LOR:  i = new Imop(Imop::LOR);  break;
        default:
            /// \todo Write better error message
            es << "Binary is not yet implemented. At " << location()
               << std::endl;
            return ICode::E_NOT_IMPLEMENTED;
    }

    i->setDest(result());
    i->setArg1(static_cast<const TreeNodeExpr*>(e1)->result());
    i->setArg2(static_cast<const TreeNodeExpr*>(e2)->result());
    code.push_back(i);
    patchFirstImop(i);

    // Patch next lists of child expressions:
    e1->patchNextList(e2->firstImop());
    e2->patchNextList(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprBinary::generateBoolCode(ICode::CodeList &code, SymbolTable &st,
                                                   std::ostream &es)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    assert(type() == NODE_EXPR_EQ
           || type() == NODE_EXPR_GE
           || type() == NODE_EXPR_GT
           || type() == NODE_EXPR_LAND
           || type() == NODE_EXPR_LE
           || type() == NODE_EXPR_LOR
           || type() == NODE_EXPR_LT
           || type() == NODE_EXPR_NE);

    if (type() == NODE_EXPR_LAND || type() == NODE_EXPR_LOR) {
        // Generate code for child expressions:
        TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
        s = e1->generateBoolCode(code, st, es);
        if (s != ICode::OK) return s;
        setFirstImop(e1->firstImop());

        TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
        s = e2->generateBoolCode(code, st, es);
        if (s != ICode::OK) return s;
        patchFirstImop(e2->firstImop());

        // Short circuit the code:
        if (type() == NODE_EXPR_LAND) {
            e1->patchTrueList(e2->firstImop());
            setFalseList(e1->falseList());

            setTrueList(e2->trueList());
            addToFalseList(e2->falseList());
        } else {
            assert(type() == NODE_EXPR_LOR);

            e1->patchFalseList(e2->firstImop());
            setTrueList(e1->trueList());

            setFalseList(e2->falseList());
            addToTrueList(e2->trueList());
        }

        return ICode::OK;
    }

    // Generate code for child expressions:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    setFirstImop(e1->firstImop());

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    patchFirstImop(e2->firstImop());

    Imop *tj;
    e1->patchNextList(e2->firstImop());
    e2->patchNextList(tj);

    switch (type()) {
        case NODE_EXPR_EQ: tj = new Imop(Imop::JE,  0); break;
        case NODE_EXPR_GE: tj = new Imop(Imop::JGE, 0); break;
        case NODE_EXPR_GT: tj = new Imop(Imop::JGT, 0); break;
        case NODE_EXPR_LE: tj = new Imop(Imop::JLE, 0); break;
        case NODE_EXPR_LT: tj = new Imop(Imop::JLT, 0); break;
        case NODE_EXPR_NE: tj = new Imop(Imop::JNE, 0); break;
        default:
            assert(false); // Shouldn't happen.
    }

    tj->setArg1(e1->result());
    tj->setArg2(e2->result());
    addToTrueList(tj);
    code.push_back(tj);
    patchFirstImop(tj);

    Imop *fj = new Imop(Imop::JUMP, 0);
    addToFalseList(fj);
    code.push_back(fj);
    return ICode::OK;
}


/*******************************************************************************
  TreeNodeExprBool
*******************************************************************************/

std::string TreeNodeExprBool::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"bool:" << stringHelper() << "\"";
    return os.str();
}

ICode::Status TreeNodeExprBool::calculateResultType(SymbolTable &,
                                                    std::ostream &)
{
    if (resultType() != 0) return ICode::OK;

    assert(children().empty());

    setResultType(new SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_BOOL));
    return ICode::OK;
}

ICode::Status TreeNodeExprBool::generateCode(ICode::CodeList &code,
                                             SymbolTable &st,
                                             std::ostream &es,
                                             SymbolWithValue *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    SymbolConstantBool *sym = st.constantBool(m_value);
    if (r != 0) {
        Imop *i = new Imop(Imop::ASSIGN, r, sym);
        code.push_back(i);
        setFirstImop(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprBool::generateBoolCode(ICode::CodeList &code,
                                                 SymbolTable &st,
                                                 std::ostream &es)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    Imop *i = new Imop(Imop::JUMP, 0);
    setFirstImop(i);
    if (m_value) {
        addToTrueList(i);
    } else {
        addToFalseList(i);
    }
    code.push_back(i);
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

ICode::Status TreeNodeExprInt::calculateResultType(SymbolTable &,
                                                   std::ostream &)
{
    if (resultType() != 0) return ICode::OK;

    assert(children().empty());

    setResultType(new SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT));
    return ICode::OK;
}

ICode::Status TreeNodeExprInt::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es,
                                            SymbolWithValue *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    SymbolConstantInt *sym = st.constantInt(m_value);
    if (r != 0) {
        Imop *i = new Imop(Imop::ASSIGN, r, sym);
        setFirstImop(i);
        code.push_back(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprInt::generateBoolCode(ICode::CodeList &,
                                                SymbolTable &,
                                                std::ostream &es)
{
    es << "Integer as boolean expression not implemented. At "
       << location() << std::endl;
    return ICode::E_NOT_IMPLEMENTED;
}


/*******************************************************************************
  TreeNodeExprRVariable
*******************************************************************************/

ICode::Status TreeNodeExprRVariable::calculateResultType(SymbolTable &st,
                                                         std::ostream &es)
{
    if (resultType() != 0) return ICode::OK;

    assert(children().size() == 1);
    assert(children().at(0)->type() == NODE_IDENTIFIER);

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));
    SymbolSymbol *s = id->getSymbol(st, es);
    if (s == 0) return ICode::E_OTHER;

    assert(s->secrecType().isVoid() == false);
    assert(dynamic_cast<const TypeNonVoid*>(&s->secrecType()) != 0);
    const TypeNonVoid *type = static_cast<const TypeNonVoid*>(&s->secrecType());
    assert(type->dataType().kind() == DataType::VAR);
    assert(dynamic_cast<const DataTypeVar*>(&type->dataType()) != 0);
    setResultType(new TypeNonVoid(type->secType(), DataTypeBasic(static_cast<const DataTypeVar&>(type->dataType()))));

    return ICode::OK;
}

ICode::Status TreeNodeExprRVariable::generateCode(ICode::CodeList &code,
                                                  SymbolTable &st,
                                                  std::ostream &es,
                                                  SymbolWithValue *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));

    if (r == 0) {
        setResult(id->getSymbol(st, es));
    } else {
        assert(r->secrecType().canAssign(*resultType()));
        setResult(r);

        Imop *i = new Imop(Imop::ASSIGN, r, id->getSymbol(st, es));
        code.push_back(i);
        setFirstImop(i);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprRVariable::generateBoolCode(ICode::CodeList &code,
                                                      SymbolTable &st,
                                                      std::ostream &es)
{
    // Type check
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));

    Imop *i = new Imop(Imop::JT, 0, id->getSymbol(st, es));
    code.push_back(i);
    setFirstImop(i);
    addToTrueList(i);

    i = new Imop(Imop::JUMP, 0);
    code.push_back(i);
    addToFalseList(i);

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
                                                      std::ostream &)
{
    if (resultType() != 0) return ICode::OK;

    assert(children().empty());

    setResultType(new SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_STRING));
    return ICode::OK;
}

ICode::Status TreeNodeExprString::generateCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es,
                                               SymbolWithValue *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    SymbolConstantString *sym = st.constantString(m_value);
    if (r != 0) {
        Imop *i = new Imop(Imop::ASSIGN, r, sym);
        setFirstImop(i);
        code.push_back(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprString::generateBoolCode(ICode::CodeList &,
                                                   SymbolTable &,
                                                   std::ostream &es)
{
    es << "String as boolean expression not implemented. At "
       << location() << std::endl;
    return ICode::E_NOT_IMPLEMENTED;
}


/*******************************************************************************
  TreeNodeExprTernary
*******************************************************************************/

ICode::Status TreeNodeExprTernary::calculateResultType(SymbolTable &st,
                                                       std::ostream &es)
{
    if (resultType() != 0) return ICode::OK;

    assert(children().size() == 3);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(2)->type() & NODE_EXPR_MASK) != 0x0);

    assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e1->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    assert(dynamic_cast<TreeNodeExpr*>(children().at(2)) != 0);
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2));
    s = e3->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    const SecreC::Type *eType1 = e1->resultType();
    const SecreC::Type *eType2 = e2->resultType();
    const SecreC::Type *eType3 = e3->resultType();

    if (!eType1->isVoid()) {
        assert(dynamic_cast<const TypeNonVoid*>(eType1) != 0);
        const TypeNonVoid *cType = static_cast<const TypeNonVoid*>(eType1);

        if (cType->kind() == TypeNonVoid::BASIC
            && cType->dataType().kind() == DataType::BASIC
            && cType->secType().kind() == SecType::BASIC
            && static_cast<const SecreC::DataTypeBasic&>(cType->dataType()).dataType() == DATATYPE_BOOL
            && static_cast<const SecreC::SecTypeBasic&>(cType->secType()).secType()== SECTYPE_PUBLIC
            && *eType2 == *eType3)
        {
            setResultType(eType2->clone());
            return ICode::OK;
        }
    }

    /// \todo Provide better error messages
    es << "Invalid ternary operation at " << location() << std::endl;

    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprTernary::generateCode(ICode::CodeList &code,
                                                SymbolTable &st,
                                                std::ostream &es,
                                                SymbolWithValue *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        SecreC::Type *rt = resultType();
        setResult(st.appendTemporary(*rt));
    } else {
        assert(r->secrecType() == *resultType());
        setResult(r);
    }

    // Generate code for boolean expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    s = e1->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for first value child expression:
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->generateCode(code, st, es, result());
    if (s != ICode::OK) return s;

    // Jump out of the ternary construct:
    Imop *j = new Imop(Imop::JUMP, 0);
    addToNextList(j);
    code.push_back(j);

    // Generate code for second value child expression:
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2));
    s = e3->generateCode(code, st, es, result());
    if (s != ICode::OK) return s;

    // Link boolean expression code to the rest of the code:
    e1->patchTrueList(e2->firstImop());
    e1->patchFalseList(e3->firstImop());

    // Handle next lists of value child expressions:
    addToNextList(e2->nextList());
    addToNextList(e3->nextList());

    return ICode::OK;
}

ICode::Status TreeNodeExprTernary::generateBoolCode(ICode::CodeList &code,
                                                    SymbolTable &st,
                                                    std::ostream &es)
{
    /// \todo Write assertion that were have good return type

    // Type check
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate code for boolean expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    s = e1->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for first value child expression:
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for second value child expression:
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2));
    s = e3->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Link conditional expression code to the rest of the code:
    e1->patchTrueList(e2->firstImop());
    e1->patchFalseList(e3->firstImop());

    addToTrueList(e2->trueList());
    addToTrueList(e3->trueList());
    addToFalseList(e2->falseList());
    addToFalseList(e3->falseList());

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

ICode::Status TreeNodeExprUInt::calculateResultType(SymbolTable &,
                                                    std::ostream &)
{
    if (resultType() != 0) return ICode::OK;

    assert(children().empty());

    setResultType(new SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT));
    return ICode::OK;
}

ICode::Status TreeNodeExprUInt::generateCode(ICode::CodeList &code,
                                             SymbolTable &st,
                                             std::ostream &es,
                                             SymbolWithValue *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    SymbolConstantUInt *sym = st.constantUInt(m_value);
    if (r != 0) {
        Imop *i = new Imop(Imop::ASSIGN, r, sym);
        setFirstImop(i);
        code.push_back(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprUInt::generateBoolCode(ICode::CodeList &,
                                                 SymbolTable &,
                                                 std::ostream &es)
{
    es << "Unsigned int as boolean expression not implemented. At "
       << location() << std::endl;
    return ICode::E_NOT_IMPLEMENTED;
}


/*******************************************************************************
  TreeNodeExprUnary
*******************************************************************************/

ICode::Status TreeNodeExprUnary::calculateResultType(SymbolTable &st,
                                                     std::ostream &es)
{
    if (resultType() != 0) return ICode::OK;

    assert(type() == NODE_EXPR_UMINUS || type() == NODE_EXPR_UNEG);
    assert(children().size() == 1);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);

    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *eType = static_cast<const TreeNodeExpr*>(e)->resultType();

    /// \todo implement for matrixes also
    if (!eType->isVoid()
#ifndef NDEBUG
        && (assert(dynamic_cast<const TypeNonVoid*>(eType) != 0), true)
#endif
        && static_cast<const TypeNonVoid*>(eType)->kind() == TypeNonVoid::BASIC)
    {
        const TypeNonVoid *et = static_cast<const TypeNonVoid*>(eType);
        assert(dynamic_cast<const DataTypeBasic*>(&et->dataType()) != 0);
        const DataTypeBasic &bType = static_cast<const DataTypeBasic&>(et->dataType());
        if (type() == NODE_EXPR_UNEG && bType.dataType() == DATATYPE_BOOL) {
            setResultType(et->clone());
            return ICode::OK;
        } else if (type() == NODE_EXPR_UMINUS) {
            if (bType.dataType() == DATATYPE_INT) {
                setResultType(et->clone());
                return ICode::OK;
            }
        }
    }

    es << "Invalid expression of type (" << *eType << ") given to unary "
       << (type() == NODE_EXPR_UNEG ? "negation" : "minus")
       << "operator at " << location() << std::endl;

    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprUnary::generateCode(ICode::CodeList &code,
                                              SymbolTable &st,
                                              std::ostream &es,
                                              SymbolWithValue *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        SecreC::Type *rt = resultType();
        setResult(st.appendTemporary(*rt));
    } else {
        assert(r->secrecType().canAssign(*resultType()));
        setResult(r);
    }

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = e->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    setFirstImop(e->firstImop());

    // Generate code for unary expression:
    /// \todo implement for matrixes also
    Imop *i = new Imop(type() == NODE_EXPR_UNEG ? Imop::UNEG : Imop::UMINUS);
    i->setDest(result());
    i->setArg1(static_cast<const TreeNodeExpr*>(e)->result());
    code.push_back(i);
    patchFirstImop(i);

    // Patch next list of child expression:
    e->patchNextList(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprUnary::generateBoolCode(ICode::CodeList &code, SymbolTable &st,
                                                  std::ostream &es)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    assert(type() == NODE_EXPR_UNEG);

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = e->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    addToFalseList(e->trueList());
    addToTrueList(e->falseList());
    setFirstImop(e->firstImop());
    return ICode::OK;
}


/*******************************************************************************
  TreeNodeFundef
*******************************************************************************/

const SecreC::TypeNonVoid &TreeNodeFundef::functionType() const {
    if (m_cachedType != 0) return *m_cachedType;

    assert(dynamic_cast<TreeNodeType*>(children().at(1)) != 0);
    TreeNodeType *rt = static_cast<TreeNodeType*>(children().at(1));

    if (rt->type() == NODE_TYPEVOID) {
        assert(rt->secrecType().isVoid());

        SecTypeFunctionVoid *st = new SecTypeFunctionVoid();
        DataTypeFunctionVoid *dt = new DataTypeFunctionVoid();

        /// \todo Add parameters

        m_cachedType = new SecreC::TypeNonVoid(*st, *dt);

        delete st;
        delete dt;
    } else {
        assert(rt->type() == NODE_TYPETYPE);
        assert(!rt->secrecType().isVoid());
        assert(dynamic_cast<const TypeNonVoid*>(&rt->secrecType()) != 0);
        const TypeNonVoid &tt = static_cast<const TypeNonVoid&>(rt->secrecType());

        assert(tt.secType().kind() == SecType::BASIC);
        assert(tt.dataType().kind() == DataType::BASIC || tt.dataType().kind() == DataType::ARRAY);

        assert(dynamic_cast<const SecTypeBasic*>(&tt.secType()) != 0);
        const SecTypeBasic &tts = static_cast<const SecTypeBasic&>(tt.secType());

        SecTypeFunction *st = new SecTypeFunction(tts.secType());
        DataTypeFunction *dt = new DataTypeFunction(tt.dataType());

        /// \todo Add parameters

        m_cachedType = new SecreC::TypeNonVoid(*st, *dt);

        delete st;
        delete dt;
    }

    return *m_cachedType;
}

ICode::Status TreeNodeFundef::generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es)
{
    typedef ChildrenListConstIterator CLCI;

    assert(children().size() >= 3);
    assert(dynamic_cast<const TreeNodeIdentifier*>(children().at(0)) != 0);
    const TreeNodeIdentifier *id = static_cast<const TreeNodeIdentifier*>(children().at(0));

    std::ostringstream os;
    os << "Start of function: " << id->value();
    code.push_comment(os.str());
    os.str("");

    // Add to symbol table:
    SymbolFunction *ns = new SymbolFunction(this);
    ns->setName(id->value());
    st.appendGlobalSymbol(ns);

    // Generate local scope:
    SymbolTable &localScope = *st.newScope();
    if (children().size() > 3) {
        for (CLCI it(children().begin() + 3); it != children().end(); it++) {
            assert((*it)->type() == NODE_DECL);
            assert(dynamic_cast<TreeNodeStmtDecl*>(*it) != 0);
            ICode::Status s = static_cast<TreeNodeStmtDecl*>(*it)->generateCode(code, localScope, es);
            if (s != ICode::OK) return s;
        }
    }

    // Generate code for function body:
    assert(dynamic_cast<TreeNodeStmt*>(children().at(2)) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(children().at(2));
    ICode::Status s = body->generateCode(code, localScope, es);
    if (s != ICode::OK) return s;
    assert(body->resultFlags() != 0x0);
    assert((body->resultFlags() & ~TreeNodeStmt::MASK) == 0);

    // Static checking:
    assert(ns->secrecType().isVoid() == false);
    assert(dynamic_cast<const TypeNonVoid*>(&ns->secrecType()) != 0);
    const TypeNonVoid &fType = static_cast<const TypeNonVoid&>(ns->secrecType());
    if (fType.kind() == TypeNonVoid::FUNCTION) {
        if (body->resultFlags() != TreeNodeStmt::RETURN) {
            if ((body->resultFlags() & TreeNodeStmt::BREAK) != 0x0) {
                es << "Function at " << location() << " contains a break statement outside of any loop!" << std::endl;
                return ICode::E_OTHER;
            } else if ((body->resultFlags() & TreeNodeStmt::CONTINUE) != 0x0) {
                es << "Function at " << location() << " contains a continue statement outside of any loop!" << std::endl;
                return ICode::E_OTHER;
            } else {
                assert((body->resultFlags() & TreeNodeStmt::FALLTHRU) != 0x0);
                es << "Function at " << location() << " does not always return a value!" << std::endl;
                return ICode::E_OTHER;
            }
        }
        assert((body->resultFlags() & TreeNodeStmt::RETURN) != 0x0);
    } else {
        assert(fType.kind() == TypeNonVoid::FUNCTIONVOID);
        if (body->resultFlags() != TreeNodeStmt::RETURN) {
            if ((body->resultFlags() & TreeNodeStmt::BREAK) != 0x0) {
                es << "Function at " << location() << " contains a break statement outside of any loop!" << std::endl;
                return ICode::E_OTHER;
            } else if ((body->resultFlags() & TreeNodeStmt::CONTINUE) != 0x0) {
                es << "Function at " << location() << " contains a continue statement outside of any loop!" << std::endl;
                return ICode::E_OTHER;
            }
            assert(fType.kind() == TypeNonVoid::FUNCTIONVOID);
            Imop *i = new Imop(Imop::RETURNVOID);
            body->patchNextList(i);
            code.push_back(i);
        }
    }

    assert(body->breakList().empty());
    assert(body->continueList().empty());
    assert(body->nextList().empty());

    os << "End of function: " << id->value();
    code.push_comment(os.str());
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeFundefs
*******************************************************************************/

ICode::Status TreeNodeFundefs::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    typedef ChildrenListConstIterator CLCI;

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_FUNDEF);
        assert(dynamic_cast<TreeNodeFundef*>(*it) != 0);
        TreeNodeFundef *fundef = static_cast<TreeNodeFundef*>(*it);

        // Generate code:
        ICode::Status s = fundef->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeGlobals
*******************************************************************************/

ICode::Status TreeNodeGlobals::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    typedef ChildrenListConstIterator CLCI;

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_DECL);
        assert(dynamic_cast<TreeNodeStmtDecl*>(*it) != 0);
        TreeNodeStmtDecl *decl = static_cast<TreeNodeStmtDecl*>(*it);
        decl->setGlobal();
        ICode::Status s = decl->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}


/*******************************************************************************
  TreeNodeIdentifier
*******************************************************************************/

SymbolSymbol *TreeNodeIdentifier::getSymbol(SymbolTable &st, std::ostream &es) const {
    Symbol *s = st.find(m_value);
    if (s == 0) {
        es << "Undefined symbol \"" << m_value << "\" at " << location()
           << std::endl;
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
  TreeNodeProgram
*******************************************************************************/

ICode::Status TreeNodeProgram::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    if (children().empty()) {
        es << "Program is empty" << std::endl;
        return ICode::E_EMPTY_PROGRAM;
    }

    assert(children().size() < 3);

    TreeNodeFundefs *fs;

    /**
      \todo In contrast with grammar we don't allow mixed declarations of
            variables and functions in global scope.
    */

    // Handle global declarations:
    if (children().size() == 2) {
        code.push_comment("Start of global declarations:");
        assert(children().at(0)->type() == NODE_GLOBALS);
        assert(dynamic_cast<TreeNodeGlobals*>(children().at(0)) != 0);
        TreeNodeGlobals *gs = static_cast<TreeNodeGlobals*>(children().at(0));

        ICode::Status s = gs->generateCode(code, st, es);
        if (s != ICode::OK) return s;
        code.push_comment("End of global declarations.");

        assert(children().at(1)->type() == NODE_FUNDEFS);
        assert(dynamic_cast<TreeNodeFundefs*>(children().at(1)) != 0);
        fs = static_cast<TreeNodeFundefs*>(children().at(1));
    } else {
        assert(children().at(0)->type() == NODE_FUNDEFS);
        assert(dynamic_cast<TreeNodeFundefs*>(children().at(0)) != 0);
        fs = static_cast<TreeNodeFundefs*>(children().at(0));
    }

    // Insert main call into the beginning of the program:
    Imop *mainCall = new Imop(Imop::FUNCALL, 0, 0);
    code.push_back(mainCall);
    code.push_back(new Imop(Imop::END));

    // Handle functions:
    ICode::Status s = fs->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Check for "void main()":
    Symbol *mainFun = st.findGlobal("main");
    if (mainFun == 0 || mainFun->symbolType() != Symbol::FUNCTION) {
        es << "No function \"void main()\" found!" << std::endl;
        return ICode::E_NO_MAIN;
    }

    // Bind call to main(), i.e. mainCall:
    mainCall->setArg1(mainFun);

    return ICode::OK;
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
  TreeNodeStmtBreak
*******************************************************************************/

ICode::Status TreeNodeStmtBreak::generateCode(ICode::CodeList &code,
                                              SymbolTable &,
                                              std::ostream &)
{
    Imop *i = new Imop(Imop::JUMP, 0);
    code.push_back(i);
    setFirstImop(i);
    addToBreakList(i);
    setResultFlags(TreeNodeStmt::BREAK);

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtContinue
*******************************************************************************/

ICode::Status TreeNodeStmtContinue::generateCode(ICode::CodeList &code,
                                                 SymbolTable &,
                                                 std::ostream &)
{
    Imop *i = new Imop(Imop::JUMP, 0);
    code.push_back(i);
    setFirstImop(i);
    addToContinueList(i);
    setResultFlags(TreeNodeStmt::CONTINUE);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtDecl
*******************************************************************************/

ICode::Status TreeNodeStmtDecl::generateCode(ICode::CodeList &code,
                                             SymbolTable &st,
                                             std::ostream &es)
{
    typedef TreeNodeIdentifier TNI;
    typedef TreeNodeType       TNT;

    assert(children().size() > 0 && children().size() <= 3);
    assert(children().at(0)->type() == NODE_IDENTIFIER);

    assert(dynamic_cast<TNI*>(children().at(0)) != 0);
    TNI *id   = static_cast<TNI*>(children().at(0));
    assert(dynamic_cast<TNT*>(children().at(1)) != 0);
    TNT *type = static_cast<TNT*>(children().at(1));

    /// \todo Check here for overrides first if new symbol table is needed.

    // First we create the new symbol, but we don't add it to the symbol table:
    assert(type->secrecType().isVoid() == false);
    assert(dynamic_cast<const TypeNonVoid*>(&type->secrecType()) != 0);
    const TypeNonVoid &justType = static_cast<const TypeNonVoid&>(type->secrecType());

    assert(justType.kind() == TypeNonVoid::BASIC);
    assert(dynamic_cast<const DataTypeBasic*>(&justType.dataType()) != 0);
    const DataTypeBasic &dataType = static_cast<const DataTypeBasic&>(justType.dataType());

    assert(dynamic_cast<const SecTypeBasic*>(&justType.secType()) != 0);
    const SecTypeBasic &secType = static_cast<const SecTypeBasic&>(justType.secType());

    SymbolSymbol *ns = new SymbolSymbol(TypeNonVoid(secType, DataTypeVar(dataType)), this);
    ns->setScopeType(m_global ? SymbolSymbol::GLOBAL : SymbolSymbol::LOCAL);
    ns->setName(id->value());

    // Secondly we generate code for the initializer:
    if (children().size() > 2) {
        TreeNode *t = children().at(2);
        assert((t->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(t) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        ICode::Status s = e->generateCode(code, st, es, ns);
        if (s != ICode::OK) return s;
    }

    // Thirdly we add the symbol to the symbol table for use in expressions:
    st.appendSymbol(ns);

    setResultFlags(TreeNodeStmt::FALLTHRU);
    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtDoWhile
*******************************************************************************/

ICode::Status TreeNodeStmtDoWhile::generateCode(ICode::CodeList &code,
                                                SymbolTable &st,
                                                std::ostream &es)
{
    assert(children().size() == 2);
    TreeNode *c0 = children().at(0);
    TreeNode *c1 = children().at(1);
    assert((c1->type() & NODE_EXPR_MASK) != 0);

    // Loop body:
    SymbolTable &innerScope = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(c0) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(c0);
    ICode::Status s = body->generateCode(code, innerScope, es);
    if (s != ICode::OK) return s;

    // Static checking:
    if (body->firstImop() == 0) {
        es << "Empty loop body at " << body->location() << std::endl;
        return ICode::E_OTHER;
    }
    assert(body->resultFlags() != 0x0);
    if ((body->resultFlags() & (TreeNodeStmt::FALLTHRU | TreeNodeStmt::CONTINUE)) == 0x0) {
        es << "Do-while loop at " << location() << " wont loop!" << std::endl;
        return ICode::E_OTHER;
    }
    setResultFlags((body->resultFlags() & ~(TreeNodeStmt::BREAK | TreeNodeStmt::CONTINUE)) | TreeNodeStmt::FALLTHRU);

    // Conditional expression:
    assert(c1->type() != NODE_EXPR_NONE);
    assert(dynamic_cast<TreeNodeExpr*>(c1) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c1);
    s = e->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);

    // Patch jump lists:
    setFirstImop(body->firstImop());
    setNextList(body->breakList());
    e->patchTrueList(body->firstImop());
    addToNextList(e->falseList());
    body->patchContinueList(e->firstImop());
    body->patchNextList(e->firstImop());

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtExpr
*******************************************************************************/

ICode::Status TreeNodeStmtExpr::generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es)
{
    assert(children().size() == 1);
    assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    setFirstImop(e->firstImop());
    setNextList(e->nextList());
    setResultFlags(TreeNodeStmt::FALLTHRU);

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtFor
*******************************************************************************/

ICode::Status TreeNodeStmtFor::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    assert(children().size() == 4);
    TreeNode *c0 = children().at(0);
    TreeNode *c1 = children().at(1);
    TreeNode *c2 = children().at(2);
    TreeNode *c3 = children().at(3);
    assert((c0->type() & NODE_EXPR_MASK) != 0);
    assert((c1->type() & NODE_EXPR_MASK) != 0);
    assert((c2->type() & NODE_EXPR_MASK) != 0);

    // Initialization expression:
    TreeNodeExpr *e0 = 0;
    if (children().at(0)->type() != NODE_EXPR_NONE) {
        assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
        e0 = static_cast<TreeNodeExpr*>(c0);
        ICode::Status s = e0->generateCode(code, st, es);
        if (s != ICode::OK) return s;
        setFirstImop(e0->firstImop());
    }

    // Conditional expression:
    TreeNodeExpr *e1 = 0;
    if (children().at(1)->type() != NODE_EXPR_NONE) {
        assert(dynamic_cast<TreeNodeExpr*>(c1) != 0);
        e1 = static_cast<TreeNodeExpr*>(c1);
        ICode::Status s = e1->generateBoolCode(code, st, es);
        if (s != ICode::OK) return s;
        patchFirstImop(e1->firstImop());
        addToNextList(e1->falseList());
    }

    // Loop body:
    SymbolTable &innerScope = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(c3) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(c3);
    ICode::Status s = body->generateCode(code, innerScope, es);
    if (s != ICode::OK) return s;
    patchFirstImop(body->firstImop());
    if (e1 != 0) e1->patchTrueList(body->firstImop());
    addToNextList(body->breakList());

    // Iteration expression:
    TreeNodeExpr *e2 = 0;
    if (children().at(2)->type() != NODE_EXPR_NONE) {
        assert(dynamic_cast<TreeNodeExpr*>(c2) != 0);
        e2 = static_cast<TreeNodeExpr*>(c2);
        ICode::Status s = e2->generateCode(code, st, es);
        if (s != ICode::OK) return s;
        body->patchContinueList(e2->firstImop());
        body->patchNextList(e2->firstImop());
    } else {
        if (e1 != 0) {
            body->patchContinueList(e1->firstImop());
            body->patchNextList(e1->firstImop());
        } else {
            body->patchContinueList(body->firstImop());
            body->patchNextList(body->firstImop());
        }
    }

    // Static checking:
    assert(body->resultFlags() != 0x0);
    if ((body->resultFlags() & (TreeNodeStmt::FALLTHRU | TreeNodeStmt::CONTINUE)) == 0x0) {
        es << "For loop at " << location() << " wont loop!" << std::endl;
        return ICode::E_OTHER;
    }
    if (e1 == 0 && ((body->resultFlags() & (TreeNodeStmt::BREAK | TreeNodeStmt::RETURN)) == 0x0)) {
        es << "For loop at " << location() << " is clearly infinite!" << std::endl;
        return ICode::E_OTHER;
    }
    setResultFlags((body->resultFlags() & ~(TreeNodeStmt::BREAK | TreeNodeStmt::CONTINUE)) | TreeNodeStmt::FALLTHRU);

    // Next iteration jump:
    Imop *j = new Imop(Imop::JUMP, 0);
    if (e1 != 0) {
        j->setJumpDest(e2->firstImop());
    } else {
        j->setJumpDest(body->firstImop());
    }
    code.push_back(j);

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtIf
*******************************************************************************/

ICode::Status TreeNodeStmtIf::generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es)
{
    assert(children().size() == 2 || children().size() == 3);
    TreeNode *c0 = children().at(0);

    // Type check the expression
    assert((c0->type() & NODE_EXPR_MASK) != 0);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);
    ICode::Status s = e->calculateResultType(st, es);
    if (s != ICode::OK) return s;
    if (e->resultType()->isVoid()) {
        es << "Conditional expression of the if statement can't have a void "
              "type. At " << e->location() << std::endl;
        return ICode::E_TYPE;
    }
    assert(dynamic_cast<TypeNonVoid*>(e->resultType()) != 0);
    TypeNonVoid *eType = static_cast<TypeNonVoid*>(e->resultType());
    if (eType->kind() != TypeNonVoid::BASIC) {
        assert(eType->secType().kind() == SecType::BASIC);
        assert(dynamic_cast<const SecTypeBasic*>(&eType->secType()) != 0);
        const SecTypeBasic &st = static_cast<const SecTypeBasic&>(eType->secType());

        assert(eType->dataType().kind() == DataType::BASIC);
        assert(dynamic_cast<const DataTypeBasic*>(&eType->dataType()) != 0);
        const DataTypeBasic &dt = static_cast<const DataTypeBasic&>(eType->dataType());

        if (st.secType() != SECTYPE_PUBLIC || dt.dataType() != DATATYPE_BOOL) {
            es << "Conditional expression in if statement must be of type "
                  "public bool." << std::endl;
            return ICode::E_TYPE;
        }
    }

    // Generate code for conditional expression:
    s = e->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);
    setFirstImop(e->firstImop());

    // Generate code for first branch:
    SymbolTable &innerScope1 = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(children().at(1)) != 0);
    TreeNodeStmt *s1 = static_cast<TreeNodeStmt*>(children().at(1));
    s = s1->generateCode(code, innerScope1, es);
    if (s != ICode::OK) return s;


    e->patchTrueList(s1->firstImop());
    addToNextList(s1->nextList());
    addToBreakList(s1->breakList());
    addToContinueList(s1->continueList());
    assert(s1->resultFlags() != 0x0);

    if (children().size() == 2) {
        addToNextList(e->falseList());
        setResultFlags(s1->resultFlags() | TreeNodeStmt::FALLTHRU);
    } else {
        // Generate jump out of first branch:
        Imop *i = new Imop(Imop::JUMP, 0);
        code.push_back(i);
        addToNextList(i);

        // Generate code for second branch:
        SymbolTable &innerScope2 = *st.newScope();
        assert(dynamic_cast<TreeNodeStmt*>(children().at(2)) != 0);
        TreeNodeStmt *s2 = static_cast<TreeNodeStmt*>(children().at(2));
        s = s2->generateCode(code, innerScope2, es);
        if (s != ICode::OK) return s;

        e->patchFalseList(s2->firstImop());
        addToNextList(s2->nextList());
        addToBreakList(s2->breakList());
        addToContinueList(s2->continueList());
        assert(s2->resultFlags() != 0x0);
        setResultFlags(s1->resultFlags() | s2->resultFlags());
    }

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtReturn
*******************************************************************************/

ICode::Status TreeNodeStmtReturn::generateCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es)
{
    if (children().empty()) {
        if (containingFunction()->functionType().kind() == TypeNonVoid::FUNCTION) {
            es << "Cannot return from non-void function without value at " << location() << std::endl;
            return ICode::E_OTHER;
        }
        assert(containingFunction()->functionType().kind() == TypeNonVoid::FUNCTIONVOID);

        Imop *i = new Imop(Imop::RETURNVOID);
        code.push_back(i);
        setFirstImop(i);
    } else {
        assert(children().size() == 1);
        if (containingFunction()->functionType().kind() == TypeNonVoid::FUNCTIONVOID) {
            es << "Cannot return value from void function at" << location() << std::endl;
            return ICode::E_OTHER;
        }
        assert(containingFunction()->functionType().kind() == TypeNonVoid::FUNCTION);

        assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
        ICode::Status s = e->calculateResultType(st, es);
        if (s != ICode::OK) return s;
        if (!containingFunction()->functionType().canAssign(*e->resultType())) {
            es << "Cannot return value of type " << *e->resultType()
               << " from function with type "
               << containingFunction()->functionType() << ". At" << location()
               << std::endl;
            return ICode::E_OTHER;
        }

        s = e->generateCode(code, st, es);
        if (s != ICode::OK) return s;
        setFirstImop(e->firstImop());

        Imop *i = new Imop(Imop::RETURN);
        code.push_back(i);
        i->setArg1(e->result());
        patchFirstImop(i);
    }
    setResultFlags(TreeNodeStmt::RETURN);

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtWhile
*******************************************************************************/

ICode::Status TreeNodeStmtWhile::generateCode(ICode::CodeList &code,
                                              SymbolTable &st,
                                              std::ostream &es)
{
    assert(children().size() == 2);
    TreeNode *c0 = children().at(0);
    TreeNode *c1 = children().at(1);
    assert((c0->type() & NODE_EXPR_MASK) != 0);

    // Conditional expression:
    assert(c0->type() != NODE_EXPR_NONE);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);
    ICode::Status s = e->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);

    // Loop body:
    SymbolTable &innerScope = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(c1) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(c1);
    s = body->generateCode(code, innerScope, es);
    if (s != ICode::OK) return s;

    // Static checking:
    if (body->firstImop() == 0) {
        es << "Empty loop body at " << body->location() << std::endl;
        return ICode::E_OTHER;
    }
    assert(body->resultFlags() != 0x0);
    if ((body->resultFlags() & (TreeNodeStmt::FALLTHRU | TreeNodeStmt::CONTINUE)) == 0x0) {
        es << "While loop at " << location() << " wont loop!" << std::endl;
        return ICode::E_OTHER;
    }
    setResultFlags((body->resultFlags() & ~(TreeNodeStmt::BREAK | TreeNodeStmt::CONTINUE)) | TreeNodeStmt::FALLTHRU);

    Imop *i = new Imop(Imop::JUMP);
    code.push_back(i);
    i->setJumpDest(e->firstImop());

    // Patch jump lists:
    setFirstImop(e->firstImop());
    e->patchTrueList(body->firstImop());
    setNextList(e->falseList());
    addToBreakList(body->breakList());
    body->patchContinueList(e->firstImop());
    body->patchNextList(e->firstImop());

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeTypeType
*******************************************************************************/

const SecreC::Type &TreeNodeTypeType::secrecType() const {
    typedef TreeNodeDataType TNDT;
    typedef TreeNodeSecTypeF TNST;

    assert(children().size() == 2);
    if (m_cachedType != 0) return *m_cachedType;

    assert(dynamic_cast<TNST*>(children().at(0)) != 0);
    TNST *st = static_cast<TNST*>(children().at(0));
    assert(dynamic_cast<TNDT*>(children().at(1)) != 0);
    TNDT *dt = static_cast<TNDT*>(children().at(1));

    m_cachedType = new SecreC::TypeNonVoid(st->secType(), dt->dataType());
    return *m_cachedType;
}

std::string TreeNodeTypeType::stringHelper() const {
    return secrecType().toString();
}

} // namespace SecreC
