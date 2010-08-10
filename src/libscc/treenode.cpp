#include "treenode.h"
#include <algorithm>
#include <sstream>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include "symboltable.h"
#include "misc.h"


namespace {

void patchList(std::vector<SecreC::Imop*> &list, SecreC::Imop *dest) {
    typedef std::vector<SecreC::Imop*>::const_iterator IVCI;
    for (IVCI it(list.begin()); it != list.end(); it++) {
        (*it)->setJumpDest(dest);
    }
    list.clear();
}

template <class T>
inline void appendVectorToVector(std::vector<T> &dst, const std::vector<T> &src)
{
    dst.insert(dst.end(), src.begin(), src.end());
}

} // anonymous namespace

namespace SecreC {

TreeNode::TreeNode(Type type, const struct YYLTYPE &loc)
    : m_parent(0), m_procedure(0), m_type(type), m_location(loc)
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

const char *TreeNode::typeName(Type type) {
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
        case NODE_EXPR_WILDCARD: return "EXPR_WILDCARD";
        case NODE_EXPR_SUBSCRIPT: return "EXPR_SUBSCRIPT";
        case NODE_EXPR_UNEG: return "EXPR_UNEG";
        case NODE_EXPR_UMINUS: return "EXPR_UMINUS";
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
        case NODE_DECL: return "DECL";
        case NODE_DECL_VSUFFIX: return "DECL_VSUFFIX";
        case NODE_GLOBALS: return "DECL_GLOBALS";
        case NODE_PROCDEF: return "PROCDEF";
        case NODE_PROCDEFS: return "PROCDEFS";
        case NODE_PROGRAM: return "PROGRAM";

        case NODE_TYPETYPE: return "TYPETYPE";
        case NODE_TYPEVOID: return "TYPEVOID";
        case NODE_DATATYPE_F: return "DATATYPE_F";
        case NODE_DATATYPE_ARRAY: return "DATATYPE_ARRAY";
        case NODE_SECTYPE_F: return "SECTYPE_F";
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

TreeNodeExpr *TreeNode::classifyChildAtIfNeeded(int index,
                                                SecrecSecType otherSecType)
{
    TreeNode *&child = m_children.at(index);
    assert(dynamic_cast<TreeNodeExpr*>(child) != 0);
    if (otherSecType == SECTYPE_PRIVATE
        && static_cast<TreeNodeExpr*>(child)->resultType().secrecSecType() == SECTYPE_PUBLIC)
    {
        TreeNodeExprClassify *ec = new TreeNodeExprClassify(child->location());
        ec->appendChild(child);
        ec->resetParent(this);
        child = ec;
    }
    return static_cast<TreeNodeExpr*>(child);
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
        case NODE_PROCDEFS:
            return (TreeNode*) (new SecreC::TreeNodeProcDefs(*loc));
        case NODE_PROCDEF:
            return (TreeNode*) (new SecreC::TreeNodeProcDef(*loc));
        case NODE_EXPR_WILDCARD: /* Fall through: */
        case NODE_EXPR_UNEG:     /* Fall through: */
        case NODE_EXPR_UMINUS:
            return (TreeNode*) (new SecreC::TreeNodeExprUnary(type, *loc));
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
        case NODE_DECL:
            return (TreeNode*) (new SecreC::TreeNodeStmtDecl(*loc));
        case NODE_TYPETYPE:
            return (TreeNode*) (new SecreC::TreeNodeTypeType(*loc));
        case NODE_TYPEVOID:
            return (TreeNode*) (new SecreC::TreeNodeTypeVoid(*loc));
        case NODE_EXPR_SUBSCRIPT:
            /// \todo
        case NODE_EXPR_CAST:
            /// \todo
        case NODE_DECL_VSUFFIX:
            /// \todo
        default:
            assert(type != NODE_IDENTIFIER);
            assert((type & NODE_LITE_MASK) == 0x0);
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
        struct TreeNode *itemType,
        unsigned value,
        YYLTYPE *loc)
{
    typedef SecreC::TreeNodeDataTypeArray ATN;
    ATN *t = new ATN(value, *loc);
    t->appendChild((SecreC::TreeNode*) itemType);
    return (TreeNode*) t;
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

ICode::Status TreeNodeStmtCompound::generateCode(ICodeList &code,
                                                 SymbolTable &st,
                                                 CompileLog &log)
{
    typedef ChildrenListConstIterator CLCI;

    TreeNodeStmt *last = 0;
    setResultFlags(TreeNodeStmt::FALLTHRU);

    SymbolTable& innerScope = *st.newScope();

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert(dynamic_cast<TreeNodeStmt*>(*it) != 0);
        TreeNodeStmt *c = static_cast<TreeNodeStmt*>(*it);
        ICode::Status s = c->generateCode(code, innerScope, log);
        if (s != ICode::OK) return s;

        assert(c->resultFlags() != 0x0);

        if (c->firstImop() == 0) {
            if (c->type() != NODE_DECL) {
                log.fatal() << "Statement with no effect at " << c->location();
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
                log.fatal() << "Unreachable statement at " << c->location();
                return ICode::E_OTHER;
            } else {
                setResultFlags((resultFlags() & ~TreeNodeStmt::FALLTHRU)
                               | c->resultFlags());
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
                                                      CompileLog &log)
{
    typedef DataTypeVar DTV;
    typedef TypeNonVoid TNV;

    if (haveResultType()) return ICode::OK;

    assert(children().size() == 2);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

    // Get symbol for l-value:
    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));
    SymbolSymbol *dest = id->getSymbol(st, log);
    if (dest == 0) return ICode::E_OTHER;

    // Calculate type of r-value:
    assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
    TreeNodeExpr *src = static_cast<TreeNodeExpr*>(children().at(1));
    ICode::Status s = src->calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Get types for destination and source:
    const SecreC::Type &destType = dest->secrecType();
    assert(destType.isVoid() == false);
    const SecreC::Type &srcType = src->resultType();

    // Check types:
    if (srcType.isVoid()) {
        log.fatal() << "Subexpression has type void at " << src->location();
        return ICode::E_TYPE;
    }
    if (!destType.canAssign(srcType)) {
        log.fatal() << "Invalid assignment from value of type " << srcType
                    << " to variable of type " << destType << ". At "
                    << location();

        return ICode::E_TYPE;
    }

    // Add implicit classify node if needed:
    classifyChildAtIfNeeded(1, destType.secrecSecType());

    assert(dynamic_cast<const TNV*>(&destType) != 0);
    const TNV &destTypeNV = static_cast<const TNV&>(destType);
    assert(destTypeNV.dataType().kind() == DataType::VAR);
    assert(dynamic_cast<const DTV*>(&destTypeNV.dataType()) != 0);
    const DTV &ddtv = static_cast<const DTV&>(destTypeNV.dataType());
    setResultType(new TNV(destTypeNV.secType(), ddtv.dataType()));
    return ICode::OK;
}

ICode::Status TreeNodeExprAssign::generateCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log,
                                               Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
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

    // Generate code for assignment
    if (type() == NODE_EXPR_ASSIGN) {
        // Simple assignment

        // Generate code for righthand expression:
        s = e2->generateCode(code, st, log, destSymSym);
        if (s != ICode::OK) return s;
        setFirstImop(e2->firstImop());

        if (r != 0) {
            Imop *i = new Imop(this, Imop::ASSIGN, r, destSymSym);
            code.push_imop(i);
            patchFirstImop(i);
            e2->patchNextList(i);
            setResult(r);
        } else {
            setResult(destSymSym);
            setNextList(e2->nextList());
        }
    } else {
        // Arithmetic assignments

        // Generate code for righthand expression:
        s = e2->generateCode(code, st, log);
        if (s != ICode::OK) return s;
        setFirstImop(e2->firstImop());

        Imop::Type iType;
        switch (type()) {
            case NODE_EXPR_ASSIGN_MUL: iType = Imop::MUL; break;
            case NODE_EXPR_ASSIGN_DIV: iType = Imop::DIV; break;
            case NODE_EXPR_ASSIGN_MOD: iType = Imop::MOD; break;
            case NODE_EXPR_ASSIGN_ADD: iType = Imop::ADD; break;
            case NODE_EXPR_ASSIGN_SUB: iType = Imop::SUB; break;
            default:
                assert(false); // shouldn't happen
        }

        Imop *i = new Imop(this, iType, destSymSym, destSymSym, e2->result());
        code.push_imop(i);
        patchFirstImop(i);
        e2->patchNextList(i);

        if (r != 0) {
            i = new Imop(this, Imop::ASSIGN, r, destSymSym);
            code.push_imop(i);
            setResult(r);
        } else {
            setResult(destSymSym);
        }
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprAssign::generateBoolCode(ICodeList &code,
                                                   SymbolTable &st,
                                                   CompileLog &log)
{
    assert(havePublicBoolType());

    ICode::Status s = generateCode(code, st, log);
    if (s != ICode::OK) return s;

    Imop *i = new Imop(this, Imop::JT, 0, result());
    code.push_imop(i);
    patchFirstImop(i);
    addToTrueList(i);

    i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    addToFalseList(i);

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
            TreeNodeExpr *e1 = classifyChildAtIfNeeded(0, eType2->secrecSecType());
            ICode::Status s = e1->calculateResultType(st, log);
            if (s != ICode::OK) return s;
            eType1 = &e1->resultType();
        }
        {
            TreeNodeExpr *e2 = classifyChildAtIfNeeded(1, eType1->secrecSecType());
            ICode::Status s = e2->calculateResultType(st, log);
            if (s != ICode::OK) return s;
            eType2 = &e2->resultType();
        }


        SecrecDataType d1 = static_cast<const DataTypeBasic&>(eType1->tnvDataType()).dataType();
        SecrecDataType d2 = static_cast<const DataTypeBasic&>(eType2->tnvDataType()).dataType();
        SecrecSecType s1 = eType1->secrecSecType();
        SecrecSecType s2 = eType2->secrecSecType();

        switch (type()) {
            case NODE_EXPR_BINARY_ADD:
                if (d1 != d2) break;
                if ((d1 & (DATATYPE_INT|DATATYPE_UINT|DATATYPE_STRING)) == 0x0)
                    break;

                setResultType(new TNV(upperSecType(s1, s2), d1));
                return ICode::OK;
            case NODE_EXPR_BINARY_SUB:
            case NODE_EXPR_BINARY_MUL:
            case NODE_EXPR_BINARY_MOD:
            case NODE_EXPR_BINARY_DIV:
                if (d1 != d2) break;
                if ((d1 & (DATATYPE_INT|DATATYPE_UINT)) == 0x0) break;
                setResultType(new TNV(upperSecType(s1, s2), d1));
                return ICode::OK;
            case NODE_EXPR_BINARY_EQ:
            case NODE_EXPR_BINARY_GE:
            case NODE_EXPR_BINARY_GT:
            case NODE_EXPR_BINARY_LE:
            case NODE_EXPR_BINARY_LT:
            case NODE_EXPR_BINARY_NE:
                if (d1 != d2) break;
                setResultType(new TNV(upperSecType(s1, s2), DATATYPE_BOOL));
                return ICode::OK;
            case NODE_EXPR_BINARY_LAND:
            case NODE_EXPR_BINARY_LOR:
                if (d1 != DATATYPE_BOOL || d2 != DATATYPE_BOOL) break;
                setResultType(new TNV(upperSecType(s1, s2), DATATYPE_BOOL));
                return ICode::OK;
            case NODE_EXPR_BINARY_MATRIXMUL:
                log.fatal() << "Matrix multiplication not yet supported. At "
                            << location();
                return ICode::E_NOT_IMPLEMENTED;
            default:
                assert(false);
        }
    }

    log.fatal() << "Invalid binary operation " << operatorString()
                << " between operands of type " << *eType1 << " and " << *eType2
                << " at " << location();

    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprBinary::generateCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log,
                                               Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the binary expression, if needed:
    if (r == 0) {
        if (!resultType().isVoid()) {
            assert(dynamic_cast<const TypeNonVoid*>(&resultType()) != 0);
            setResult(st.appendTemporary(static_cast<const TypeNonVoid&>(resultType())));
        }
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));

    /*
      If first sub-expression is public, then generate short-circuit code for
      logical && and logical ||.
    */
    if (e1->resultType().secrecSecType() == SECTYPE_PUBLIC
        && (type() == NODE_EXPR_BINARY_LAND || type() == NODE_EXPR_BINARY_LOR))
    {
        // Generate code for first child expression:
        /**
          \note The short-circuit code generated here is not the exactly the
                same as in the semantics paper. Namely to optimize some more, we
                immediately assign the result of the first operator instead of
                assigning first to a temporary.
        */
        s = e1->generateCode(code, st, log, result());
        if (s != ICode::OK) return s;
        setFirstImop(e1->firstImop());

        Imop *j = new Imop(this,
                           type() == NODE_EXPR_BINARY_LAND ? Imop::JF : Imop::JT,
                           0,
                           result());
        code.push_imop(j);
        addToNextList(j);
        e1->patchNextList(j);

        // Generate code for second child expression:
        s = e2->generateCode(code, st, log, result());
        if (s != ICode::OK) return s;
        patchFirstImop(e2->firstImop());

        return ICode::OK;
    }

    // Generate code for first child expression:
    s = e1->generateCode(code, st, log);
    if (s != ICode::OK) return s;
    setFirstImop(e1->firstImop());

    // Generate code for second child expression:
    s = e2->generateCode(code, st, log);
    if (s != ICode::OK) return s;
    patchFirstImop(e2->firstImop());

    // Generate code for binary expression:
    Imop *i;
    switch (type()) {
        case NODE_EXPR_BINARY_ADD:  i = new Imop(this, Imop::ADD);  break;
        case NODE_EXPR_BINARY_SUB:  i = new Imop(this, Imop::SUB);  break;
        case NODE_EXPR_BINARY_MUL:  i = new Imop(this, Imop::MUL);  break;
        case NODE_EXPR_BINARY_DIV:  i = new Imop(this, Imop::DIV);  break;
        case NODE_EXPR_BINARY_MOD:  i = new Imop(this, Imop::MOD);  break;
        case NODE_EXPR_BINARY_EQ:   i = new Imop(this, Imop::EQ);   break;
        case NODE_EXPR_BINARY_GE:   i = new Imop(this, Imop::GE);   break;
        case NODE_EXPR_BINARY_GT:   i = new Imop(this, Imop::GT);   break;
        case NODE_EXPR_BINARY_LE:   i = new Imop(this, Imop::LE);   break;
        case NODE_EXPR_BINARY_LT:   i = new Imop(this, Imop::LT);   break;
        case NODE_EXPR_BINARY_NE:   i = new Imop(this, Imop::NE);   break;
        case NODE_EXPR_BINARY_LAND: i = new Imop(this, Imop::LAND); break;
        case NODE_EXPR_BINARY_LOR:  i = new Imop(this, Imop::LOR);  break;
        default:
            log.fatal() << "Binary " << operatorString()
                        << " not yet implemented. At " << location();
            return ICode::E_NOT_IMPLEMENTED;
    }

    i->setDest(result());
    i->setArg1(static_cast<const TreeNodeExpr*>(e1)->result());
    i->setArg2(static_cast<const TreeNodeExpr*>(e2)->result());
    code.push_imop(i);
    patchFirstImop(i);

    // Patch next lists of child expressions:
    e1->patchNextList(e2->firstImop());
    e2->patchNextList(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprBinary::generateBoolCode(ICodeList &code,
                                                   SymbolTable &st,
                                                   CompileLog &log)
{
    typedef TypeNonVoid TNV;

    assert(havePublicBoolType());

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));

    switch (type()) {
        case NODE_EXPR_BINARY_LAND: // fall through
        case NODE_EXPR_BINARY_LOR:
            assert(!e1->resultType().isVoid());
            assert(dynamic_cast<const TNV*>(&e1->resultType()) != 0);

            /*
              If first sub-expression is public, then generate short-circuit
              code for logical && and logical ||.
            */
            if (static_cast<const TNV&>(e1->resultType()).secType()
                    == SecTypeBasic(SECTYPE_PUBLIC))
            {
                // Generate code for first child expression:
                ICode::Status s = e1->generateBoolCode(code, st, log);
                if (s != ICode::OK) return s;
                setFirstImop(e1->firstImop());

                // Generate code for second child expression:
                s = e2->generateBoolCode(code, st, log);
                if (s != ICode::OK) return s;
                patchFirstImop(e2->firstImop());

                // Short circuit the code:
                if (type() == NODE_EXPR_BINARY_LAND) {
                    e1->patchTrueList(e2->firstImop());
                    setFalseList(e1->falseList());

                    setTrueList(e2->trueList());
                    addToFalseList(e2->falseList());
                } else {
                    assert(type() == NODE_EXPR_BINARY_LOR);

                    e1->patchFalseList(e2->firstImop());
                    setTrueList(e1->trueList());

                    setFalseList(e2->falseList());
                    addToTrueList(e2->trueList());
                }
            } else {
                ICode::Status s = generateCode(code, st, log);
                if (s != ICode::OK) return s;

                Imop *j1, *j2;
                if (type() == NODE_EXPR_BINARY_LAND) {
                    j1 = new Imop(this, Imop::JF, 0);
                    patchFirstImop(j1);
                    addToFalseList(j1);

                    j2 = new Imop(this, Imop::JUMP, 0);
                    addToTrueList(j2);
                } else {
                    assert(type() == NODE_EXPR_BINARY_LOR);

                    j1 = new Imop(this, Imop::JT, 0);
                    patchFirstImop(j1);
                    addToTrueList(j1);

                    j2 = new Imop(this, Imop::JUMP, 0);
                    addToFalseList(j2);
                }
                j1->setArg1(result());
                code.push_imop(j1);
                code.push_imop(j2);
                patchNextList(j1);
            }
            break;
        case NODE_EXPR_BINARY_EQ:   // fall through
        case NODE_EXPR_BINARY_GE:   // fall through
        case NODE_EXPR_BINARY_GT:   // fall through
        case NODE_EXPR_BINARY_LE:   // fall through
        case NODE_EXPR_BINARY_LT:   // fall through
        case NODE_EXPR_BINARY_NE:   // fall through
        {
            // Generate code for first child expression:
            ICode::Status s = e1->generateCode(code, st, log);
            if (s != ICode::OK) return s;
            setFirstImop(e1->firstImop());

            // Generate code for second child expression:
            s = e2->generateCode(code, st, log);
            if (s != ICode::OK) return s;
            patchFirstImop(e2->firstImop());

            Imop::Type cType;
            switch (type()) {
                case NODE_EXPR_BINARY_EQ: cType = Imop::JE; break;
                case NODE_EXPR_BINARY_GE: cType = Imop::JGE; break;
                case NODE_EXPR_BINARY_GT: cType = Imop::JGT; break;
                case NODE_EXPR_BINARY_LE: cType = Imop::JLE; break;
                case NODE_EXPR_BINARY_LT: cType = Imop::JLT; break;
                case NODE_EXPR_BINARY_NE: cType = Imop::JNE; break;
                default:
                    assert(false); // Shouldn't happen.
            }

            Imop *tj = new Imop(this, cType, 0);
            tj->setArg1(e1->result());
            tj->setArg2(e2->result());
            code.push_imop(tj);
            addToTrueList(tj);
            patchFirstImop(tj);

            e1->patchNextList(e2->firstImop());
            e2->patchNextList(tj);

            Imop *fj = new Imop(this, Imop::JUMP, 0);
            addToFalseList(fj);
            code.push_imop(fj);
            break;
        }
        default:
            assert(false);
            break;
    }
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

ICode::Status TreeNodeExprBool::calculateResultType(SymbolTable &, CompileLog &)
{
    if (haveResultType()) return ICode::OK;

    assert(children().empty());

    setResultType(new SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_BOOL));
    return ICode::OK;
}

ICode::Status TreeNodeExprBool::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log,
                                             Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    SymbolConstantBool *sym = st.constantBool(m_value);
    if (r != 0) {
        Imop *i = new Imop(this, Imop::ASSIGN, r, sym);
        code.push_imop(i);
        setFirstImop(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprBool::generateBoolCode(ICodeList &code,
                                                 SymbolTable &,
                                                 CompileLog &)
{
    assert(havePublicBoolType());

    Imop *i = new Imop(this, Imop::JUMP, 0);
    setFirstImop(i);
    if (m_value) {
        addToTrueList(i);
    } else {
        addToFalseList(i);
    }
    code.push_imop(i);
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

    if (e->resultType().isVoid()) {
        log.fatal() << "Call to a void procedure used as a subexpression at "
                    << location();
        return ICode::E_TYPE;
    }

    assert(e->resultType().secrecSecType() == SECTYPE_PUBLIC);
    setResultType(new TypeNonVoid(SECTYPE_PRIVATE, e->resultType().tnvDataType()));
    return ICode::OK;
}

ICode::Status TreeNodeExprClassify::generateCode(ICodeList &code,
                                                   SymbolTable &st,
                                                   CompileLog &log,
                                                   Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the classification, if needed:
    if (r == 0) {
        if (!resultType().isVoid()) {
            assert(dynamic_cast<const TypeNonVoid*>(&resultType()) != 0);
            setResult(st.appendTemporary(static_cast<const TypeNonVoid&>(resultType())));
        }
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = e->generateCode(code, st, log);
    if (s != ICode::OK) return s;
    setFirstImop(e->firstImop());

    Imop *i = new Imop(this,
                       Imop::CLASSIFY,
                       result(),
                       static_cast<const TreeNodeExpr*>(e)->result());
    code.push_imop(i);
    patchFirstImop(i);

    // Patch next list of child expression:
    e->patchNextList(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprClassify::generateBoolCode(ICodeList &, SymbolTable &,
                                                     CompileLog &)
{
    assert(false); // This method shouldn't be called.
    return ICode::E_NOT_IMPLEMENTED;
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
        if (e->resultType().secrecSecType() == SECTYPE_PRIVATE) {
            setResultType(new TypeNonVoid(SECTYPE_PUBLIC, e->resultType().tnvDataType()));
            return ICode::OK;
        }
    }

    log.fatal() << "Argument of type " << e->resultType()
                << " passed to declassify operator at " << location();

    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprDeclassify::generateCode(ICodeList &code,
                                                   SymbolTable &st,
                                                   CompileLog &log,
                                                   Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the declassification, if needed:
    if (r == 0) {
        if (!resultType().isVoid()) {
            assert(dynamic_cast<const TypeNonVoid*>(&resultType()) != 0);
            setResult(st.appendTemporary(static_cast<const TypeNonVoid&>(resultType())));
        }
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = e->generateCode(code, st, log);
    if (s != ICode::OK) return s;
    setFirstImop(e->firstImop());

    Imop *i = new Imop(this, Imop::DECLASSIFY, result(), static_cast<const TreeNodeExpr*>(e)->result());
    code.push_imop(i);
    patchFirstImop(i);

    // Patch next list of child expression:
    e->patchNextList(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprDeclassify::generateBoolCode(ICodeList &code,
                                                       SymbolTable &st,
                                                       CompileLog &log)
{
    assert(havePublicBoolType());

    ICode::Status s = generateCode(code, st, log);
    if (s != ICode::OK) return s;

    Imop *i = new Imop(this, Imop::JT, 0, result());
    code.push_imop(i);
    patchFirstImop(i);
    addToTrueList(i);

    i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    addToFalseList(i);

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeExprProcCall
*******************************************************************************/

ICode::Status TreeNodeExprProcCall::calculateResultType(SymbolTable &st,
                                                        CompileLog &log)
{
    typedef SecTypeProcedureVoid STFV;
    typedef DataTypeProcedureVoid DTFV;
    typedef SecTypeProcedure STF;
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

    SecTypeProcedureVoid secType;
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

        if (e->resultType().isVoid()) {
            log.fatal() << "Can't pass argument of void type to function at "
                        << e->location();
            return ICode::E_TYPE;
        }
        assert(dynamic_cast<const TypeNonVoid*>(&e->resultType()) != 0);
        const TypeNonVoid &t = static_cast<const TypeNonVoid&>(e->resultType());
        secType.addParamType(t.secType());
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
    assert(dynamic_cast<const STFV*>(&ft.secType()) != 0);
    const STFV &rstv = static_cast<const STFV&>(ft.secType());

    // Check security types of parameters:
    assert(rstv.paramTypes().size() == children().size() - 1);
    for (unsigned i = 0; i < rstv.paramTypes().size(); i++) {
        assert(rstv.paramTypes().at(i)->kind() == SecType::BASIC);
        assert(dynamic_cast<SecTypeBasic*>(rstv.paramTypes().at(i)) != 0);
        SecTypeBasic *need = static_cast<SecTypeBasic*>(rstv.paramTypes()[i]);

        assert(secType.paramTypes().at(i)->kind() == SecType::BASIC);
        assert(dynamic_cast<SecTypeBasic*>(secType.paramTypes().at(i)) != 0);
        SecTypeBasic *have = static_cast<SecTypeBasic*>(secType.paramTypes()[i]);

        assert(need->secType() != SECTYPE_INVALID);
        assert(have->secType() != SECTYPE_INVALID);

        if (need->secType() == SECTYPE_PUBLIC
            && have->secType() == SECTYPE_PRIVATE)
        {
            log.fatal() << "Argument " << (i + 1) << " to function "
                << id->value() << " at " << arguments[i]->location()
                << " is expected to be of public type instead of private!";
            return ICode::E_TYPE;
        }

        // Add implicit classify node if needed:
        classifyChildAtIfNeeded(i + 1, need->secType());
    }

    // Set result type:
    if (ft.kind() == TypeNonVoid::PROCEDURE) {
        assert(dynamic_cast<const STF*>(&rstv) != 0);
        const STF &rst = static_cast<const STF&>(rstv);
        assert(dynamic_cast<const DTF*>(&ft.dataType()) != 0);
        const DTF &rdt = static_cast<const DTF&>(ft.dataType());
        setResultType(new TypeNonVoid(rst.returnSecType(), rdt.returnType()));
    } else {
        setResultType(new TypeVoid);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprProcCall::generateCode(ICodeList &code,
                                                 SymbolTable &st,
                                                 CompileLog &log,
                                                 Symbol *r)
{
    typedef ChildrenListConstIterator CLCI;

    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    std::stack<Symbol*> resultList;

    // Initialize arguments
    TreeNodeExpr *last = 0;
    for (CLCI it(children().begin() + 1); it != children().end(); it++) {
        assert(((*it)->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(*it) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(*it);
        ICode::Status s = e->generateCode(code, st, log);
        if (s != ICode::OK) return s;

        if (e->firstImop() != 0) {
            if (last != 0) {
                last->patchNextList(e->firstImop());
            } else {
                setFirstImop(e->firstImop());
            }
            last = e;
        }

        assert(e->result() != 0);
        resultList.push(e->result());
    }

    // Add them as arguments in a backward manner:
    while (!resultList.empty()) {
        Imop *i = new Imop(this, Imop::PUSHPARAM);
        i->setArg1(resultList.top());
        code.push_imop(i);

        if (last != 0) {
            last->patchNextList(i);
            last = 0;
        } else {
            patchFirstImop(i);
        }

        resultList.pop();
    }

    // Do function call
    Imop *i = new Imop(this, Imop::CALL);
    Imop *c = new Imop(this, Imop::RETCLEAN);
    if (r == 0) {
        // Generate temporary for the result of the unary expression
        if (!resultType().isVoid()) {
            assert(dynamic_cast<const TypeNonVoid*>(&resultType()) != 0);
            SymbolTemporary *t = st.appendTemporary(static_cast<const TypeNonVoid&>(resultType()));
            setResult(t);
            i->setDest(t);
        }
    } else {
        i->setDest(r);
    }
    i->setCallDest(m_procedure, c);
    code.push_imop(i);
    patchFirstImop(i);
    code.push_imop(c);

    return ICode::OK;
}

ICode::Status TreeNodeExprProcCall::generateBoolCode(ICodeList &code,
                                                     SymbolTable &st,
                                                     CompileLog &log)
{
    assert(havePublicBoolType());

    ICode::Status s = generateCode(code, st, log);
    if (s != ICode::OK) return s;

    Imop *i = new Imop(this, Imop::JT, 0, result());
    code.push_imop(i);
    addToTrueList(i);

    i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    addToFalseList(i);

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

    setResultType(new SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT));
    return ICode::OK;
}

ICode::Status TreeNodeExprInt::generateCode(ICodeList &code, SymbolTable &st,
                                            CompileLog &log, Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    SymbolConstantInt *sym = st.constantInt(m_value);
    if (r != 0) {
        Imop *i = new Imop(this, Imop::ASSIGN, r, sym);
        setFirstImop(i);
        code.push_imop(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprInt::generateBoolCode(ICodeList &, SymbolTable &,
                                                CompileLog &)
{
    assert(false); // This method shouldn't be called.
    return ICode::E_NOT_IMPLEMENTED;
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
    setResultType(new TNV(type->secType(),
                          static_cast<const DTV&>(type->dataType()).dataType())
                  );

    return ICode::OK;
}

ICode::Status TreeNodeExprRVariable::generateCode(ICodeList &code,
                                                  SymbolTable &st,
                                                  CompileLog &log,
                                                  Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));

    if (r == 0) {
        setResult(id->getSymbol(st, log));
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);

        Imop *i = new Imop(this, Imop::ASSIGN, r, id->getSymbol(st, log));
        code.push_imop(i);
        setFirstImop(i);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprRVariable::generateBoolCode(ICodeList &code,
                                                      SymbolTable &st,
                                                      CompileLog &log)
{
    assert(havePublicBoolType());

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));

    Imop *i = new Imop(this, Imop::JT, 0, id->getSymbol(st, log));
    code.push_imop(i);
    setFirstImop(i);
    addToTrueList(i);

    i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
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
                                                      CompileLog &)
{
    if (haveResultType()) return ICode::OK;

    assert(children().empty());

    setResultType(new SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_STRING));
    return ICode::OK;
}

ICode::Status TreeNodeExprString::generateCode(ICodeList &code, SymbolTable &st,
                                               CompileLog &log,
                                               Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    SymbolConstantString *sym = st.constantString(m_value);
    if (r != 0) {
        Imop *i = new Imop(this, Imop::ASSIGN, r, sym);
        setFirstImop(i);
        code.push_imop(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprString::generateBoolCode(ICodeList &, SymbolTable &,
                                                   CompileLog &)
{
    assert(false); // This method shouldn't be called.
    return ICode::E_NOT_IMPLEMENTED;
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

    if (!eType1.isVoid()) {
        assert(dynamic_cast<const TypeNonVoid*>(&eType1) != 0);
        const TypeNonVoid &cType = static_cast<const TypeNonVoid&>(eType1);

        if (cType.kind() == TypeNonVoid::BASIC
            && cType.dataType().kind() == DataType::BASIC
            && cType.secType().kind() == SecType::BASIC
            && static_cast<const DataTypeBasic&>(cType.dataType()).dataType()
                   == DATATYPE_BOOL
            && static_cast<const SecTypeBasic&>(cType.secType()).secType()
                   == SECTYPE_PUBLIC
            && eType2 == eType3)
        {
            // Add implicit classify nodes if needed:
            e2 = classifyChildAtIfNeeded(1, eType3.secrecSecType());
            s = e2->calculateResultType(st, log);
            if (s != ICode::OK) return s;

            e3 = classifyChildAtIfNeeded(2, eType2.secrecSecType());
            s = e3->calculateResultType(st, log);
            if (s != ICode::OK) return s;

            assert(e2->resultType() == e3->resultType());

            setResultType(e2->resultType().clone());
            return ICode::OK;
        }
    }

    /// \todo Provide better error messages
    log.fatal() << "Invalid ternary operation at " << location();

    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprTernary::generateCode(ICodeList &code,
                                                SymbolTable &st,
                                                CompileLog &log,
                                                Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the ternary expression, if needed:
    if (r == 0) {
        if (!resultType().isVoid()) {
            assert(dynamic_cast<const TypeNonVoid*>(&resultType()) != 0);
            setResult(st.appendTemporary(static_cast<const TypeNonVoid&>(resultType())));
        }
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    // Generate code for boolean expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    s = e1->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;
    setFirstImop(e1->firstImop());

    // Generate code for first value child expression:
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->generateCode(code, st, log, result());
    if (s != ICode::OK) return s;

    // Jump out of the ternary construct:
    Imop *j = new Imop(this, Imop::JUMP, 0);
    addToNextList(j);
    code.push_imop(j);

    // Generate code for second value child expression:
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2));
    s = e3->generateCode(code, st, log, result());
    if (s != ICode::OK) return s;

    // Link boolean expression code to the rest of the code:
    e1->patchTrueList(e2->firstImop());
    e1->patchFalseList(e3->firstImop());

    // Handle next lists of value child expressions:
    addToNextList(e2->nextList());
    addToNextList(e3->nextList());

    return ICode::OK;
}

ICode::Status TreeNodeExprTernary::generateBoolCode(ICodeList &code,
                                                    SymbolTable &st,
                                                    CompileLog &log)
{
    assert(havePublicBoolType());

    // Generate code for boolean expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e1->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;

    // Generate code for first value child expression:
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = e2->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;

    // Generate code for second value child expression:
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2));
    s = e3->generateCode(code, st, log);
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

ICode::Status TreeNodeExprUInt::calculateResultType(SymbolTable &, CompileLog &)
{
    if (haveResultType()) return ICode::OK;

    assert(children().empty());

    setResultType(new SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT));
    return ICode::OK;
}

ICode::Status TreeNodeExprUInt::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log,
                                             Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    SymbolConstantUInt *sym = st.constantUInt(m_value);
    if (r != 0) {
        Imop *i = new Imop(this, Imop::ASSIGN, r, sym);
        setFirstImop(i);
        code.push_imop(i);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprUInt::generateBoolCode(ICodeList &, SymbolTable &,
                                                 CompileLog &)
{
    assert(false); // This method shouldn't be called.
    return ICode::E_NOT_IMPLEMENTED;
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

ICode::Status TreeNodeExprUnary::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        if (!resultType().isVoid()) {
            assert(dynamic_cast<const TypeNonVoid*>(&resultType()) != 0);
            setResult(st.appendTemporary(static_cast<const TypeNonVoid&>(resultType())));
        }
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = e->generateCode(code, st, log);
    if (s != ICode::OK) return s;

    setFirstImop(e->firstImop());

    // Generate code for unary expression:
    /// \todo implement for matrixes also
    Imop *i = new Imop(this,
                       type() == NODE_EXPR_UNEG ? Imop::UNEG : Imop::UMINUS,
                       result(),
                       static_cast<const TreeNodeExpr*>(e)->result());
    code.push_imop(i);
    patchFirstImop(i);

    // Patch next list of child expression:
    e->patchNextList(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprUnary::generateBoolCode(ICodeList &code,
                                                  SymbolTable &st,
                                                  CompileLog &log)
{
    assert(havePublicBoolType());
    assert(type() == NODE_EXPR_UNEG);

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;

    addToFalseList(e->trueList());
    addToTrueList(e->falseList());
    setFirstImop(e->firstImop());
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
    typedef SecTypeBasic STB;
    typedef TypeNonVoid TNV;

    if (m_cachedType != 0) return ICode::OK;

    assert(dynamic_cast<TreeNodeType*>(children().at(1)) != 0);
    TreeNodeType *rt = static_cast<TreeNodeType*>(children().at(1));

    if (rt->type() == NODE_TYPEVOID) {
        assert(rt->secrecType().isVoid());

        SecTypeProcedureVoid st;
        DataTypeProcedureVoid dt;

        ICode::Status s = addParameters(st, dt, stable, log);
        if (s != ICode::OK) return s;

        m_cachedType = new TNV(st, dt);
    } else {
        assert(rt->type() == NODE_TYPETYPE);
        assert(!rt->secrecType().isVoid());
        assert(dynamic_cast<const TNV*>(&rt->secrecType()) != 0);
        const TNV &tt = static_cast<const TNV&>(rt->secrecType());

        assert(tt.secType().kind() == SecType::BASIC);
        assert(tt.dataType().kind() == DataType::BASIC
               || tt.dataType().kind() == DataType::ARRAY);

        assert(dynamic_cast<const STB*>(&tt.secType()) != 0);
        const STB &tts = static_cast<const STB&>(tt.secType());

        SecTypeProcedure st(tts.secType());
        DataTypeProcedure dt(tt.dataType());

        ICode::Status s = addParameters(st, dt, stable, log);
        if (s != ICode::OK) return s;

        m_cachedType = new TNV(st, dt);
    }

    return ICode::OK;
}

ICode::Status TreeNodeProcDef::generateCode(ICodeList &code, SymbolTable &st,
                                           CompileLog &log)
{
    typedef TreeNodeIdentifier TNI;
    typedef TypeNonVoid TNV;
    typedef ChildrenListConstIterator CLCI;

    assert(children().size() >= 3);
    assert(dynamic_cast<const TNI*>(children().at(0)) != 0);
    const TNI *id = static_cast<const TNI*>(children().at(0));

    ICode::Status s = calculateProcedureType(st, log);
    if (s != ICode::OK) return s;

    std::ostringstream os;
    os << "Start of function: " << id->value();
    setFirstImop(code.push_comment(os.str()));
    os.str("");

    // Add to symbol table:
    /// \todo Check whether already defined
    SymbolProcedure *ns = st.appendProcedure(*this);

    // Generate local scope:
    SymbolTable &localScope = *st.newScope();
    if (children().size() > 3) {
        for (CLCI it(children().begin() + 3); it != children().end(); it++) {
            assert((*it)->type() == NODE_DECL);
            assert(dynamic_cast<TreeNodeStmtDecl*>(*it) != 0);
            TreeNodeStmtDecl *paramDecl = static_cast<TreeNodeStmtDecl*>(*it);
            paramDecl->setProcParam(true);
            ICode::Status s = paramDecl->generateCode(code, localScope, log);
            if (s != ICode::OK) return s;
        }
    }

    // Generate code for function body:
    assert(dynamic_cast<TreeNodeStmt*>(children().at(2)) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(children().at(2));
    s = body->generateCode(code, localScope, log);
    if (s != ICode::OK) return s;
    assert(body->resultFlags() != 0x0);
    assert((body->resultFlags() & ~TreeNodeStmt::MASK) == 0);

    // Static checking:
    assert(ns->secrecType().isVoid() == false);
    assert(dynamic_cast<const TNV*>(&ns->secrecType()) != 0);
    const TNV &fType = static_cast<const TNV&>(ns->secrecType());
    if (fType.kind() == TNV::PROCEDURE) {
        if (body->resultFlags() != TreeNodeStmt::RETURN) {
            if ((body->resultFlags() & TreeNodeStmt::BREAK) != 0x0) {
                log.fatal() << "Function at " << location()
                    << " contains a break statement outside of any loop!";
                return ICode::E_OTHER;
            } else if ((body->resultFlags() & TreeNodeStmt::CONTINUE) != 0x0) {
                log.fatal() << "Function at " << location()
                    << " contains a continue statement outside of any loop!";
                return ICode::E_OTHER;
            } else {
                assert((body->resultFlags() & TreeNodeStmt::FALLTHRU) != 0x0);
                log.fatal() << "Function at " << location()
                            << " does not always return a value!";
                return ICode::E_OTHER;
            }
        }
        assert((body->resultFlags() & TreeNodeStmt::RETURN) != 0x0);
    } else {
        assert(fType.kind() == TNV::PROCEDUREVOID);
        if (body->resultFlags() != TreeNodeStmt::RETURN) {
            if ((body->resultFlags() & TreeNodeStmt::BREAK) != 0x0) {
                log.fatal() << "Function at " << location()
                    << " contains a break statement outside of any loop!";
                return ICode::E_OTHER;
            } else if ((body->resultFlags() & TreeNodeStmt::CONTINUE) != 0x0) {
                log.fatal() << "Function at " << location()
                    << " contains a continue statement outside of any loop!";
                return ICode::E_OTHER;
            }
            assert(fType.kind() == TNV::PROCEDUREVOID);
            Imop *i = new Imop(this, Imop::RETURNVOID);
            i->setReturnDestFirstImop(firstImop());
            body->patchNextList(i);
            code.push_imop(i);
        }
    }

    assert(body->breakList().empty());
    assert(body->continueList().empty());
    assert(body->nextList().empty());

    os << "End of function: " << id->value();
    code.push_comment(os.str());
    return ICode::OK;
}

ICode::Status TreeNodeProcDef::addParameters(SecTypeProcedureVoid &st,
                                             DataTypeProcedureVoid &dt,
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
        st.addParamType(pt.secType());
        assert(pt.dataType().kind() == DataType::VAR);
        assert(dynamic_cast<const DTV*>(&pt.dataType()) != 0);
        dt.addParamType(static_cast<const DTV&>(pt.dataType()).dataType());
    }
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeProcDefs
*******************************************************************************/

ICode::Status TreeNodeProcDefs::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log)
{
    typedef ChildrenListConstIterator CLCI;

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_PROCDEF);
        assert(dynamic_cast<TreeNodeProcDef*>(*it) != 0);
        TreeNodeProcDef *procdef = static_cast<TreeNodeProcDef*>(*it);

        // Generate code:
        ICode::Status s = procdef->generateCode(code, st, log);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeGlobals
*******************************************************************************/

ICode::Status TreeNodeGlobals::generateCode(ICodeList &code, SymbolTable &st,
                                            CompileLog &log)
{
    typedef ChildrenListConstIterator CLCI;

    TreeNodeStmtDecl *last = 0;
    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_DECL);
        assert(dynamic_cast<TreeNodeStmtDecl*>(*it) != 0);
        TreeNodeStmtDecl *decl = static_cast<TreeNodeStmtDecl*>(*it);
        decl->setGlobal();
        ICode::Status s = decl->generateCode(code, st, log);
        if (s != ICode::OK) return s;

        if (decl->firstImop() != 0) {
            if (last == 0) {
                setFirstImop(decl->firstImop());
            } else {
                last->patchNextList(decl->firstImop());
            }
            last = decl;
        } else {
            assert(decl->nextList().empty());
        }
    }
    if (last != 0) {
        addToNextList(last->nextList());
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
  TreeNodeProgram
*******************************************************************************/

ICode::Status TreeNodeProgram::generateCode(ICodeList &code, SymbolTable &st,
                                            CompileLog &log)
{
    typedef SymbolProcedure SP;

    if (children().empty()) {
        log.fatal() << "Program is empty";
        return ICode::E_EMPTY_PROGRAM;
    }

    assert(children().size() < 3);

    TreeNodeProcDefs *ps;

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

        ICode::Status s = gs->generateCode(code, st, log);
        if (s != ICode::OK) return s;
        code.push_comment("End of global declarations.");

        assert(children().at(1)->type() == NODE_PROCDEFS);
        assert(dynamic_cast<TreeNodeProcDefs*>(children().at(1)) != 0);
        ps = static_cast<TreeNodeProcDefs*>(children().at(1));
    } else {
        assert(children().at(0)->type() == NODE_PROCDEFS);
        assert(dynamic_cast<TreeNodeProcDefs*>(children().at(0)) != 0);
        ps = static_cast<TreeNodeProcDefs*>(children().at(0));
    }

    // Insert main call into the beginning of the program:
    Imop *mainCall = new Imop(this, Imop::CALL, 0, 0, 0);
    Imop *retClean = new Imop(this, Imop::RETCLEAN);
    code.push_imop(mainCall);
    code.push_imop(retClean);
    code.push_imop(new Imop(this, Imop::END));

    // Handle functions:
    ICode::Status s = ps->generateCode(code, st, log);
    if (s != ICode::OK) return s;

    // Check for "void main()":
    SP *mainProc = st.findGlobalProcedure("main", DataTypeProcedureVoid());
    if (mainProc == 0) {
        log.fatal() << "No function \"void main()\" found!";
        return ICode::E_NO_MAIN;
    }

    // Bind call to main(), i.e. mainCall:
    mainCall->setCallDest(mainProc, retClean);

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

ICode::Status TreeNodeStmtBreak::generateCode(ICodeList &code, SymbolTable &,
                                              CompileLog &)
{
    Imop *i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    setFirstImop(i);
    addToBreakList(i);
    setResultFlags(TreeNodeStmt::BREAK);

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtContinue
*******************************************************************************/

ICode::Status TreeNodeStmtContinue::generateCode(ICodeList &code, SymbolTable &,
                                                 CompileLog &)
{
    Imop *i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    setFirstImop(i);
    addToContinueList(i);
    setResultFlags(TreeNodeStmt::CONTINUE);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtDecl
*******************************************************************************/

const std::string &TreeNodeStmtDecl::variableName() const {
    typedef TreeNodeIdentifier TNI;

    assert(children().size() > 0 && children().size() <= 3);
    assert(children().at(0)->type() == NODE_IDENTIFIER);

    assert(dynamic_cast<TNI*>(children().at(0)) != 0);
    return static_cast<TNI*>(children().at(0))->value();
}

ICode::Status TreeNodeStmtDecl::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log)
{
    assert(children().size() > 0 && children().size() <= 3);

    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Initialize the new symbol (for initializer target)
    SymbolSymbol *ns = new SymbolSymbol(resultType());
    ns->setScopeType(m_global ? SymbolSymbol::GLOBAL : SymbolSymbol::LOCAL);
    ns->setName(variableName());

    if (m_procParam) {
        Imop *i = new Imop(this, Imop::POPPARAM, ns);
        code.push_imop(i);
        setFirstImop(i);
    } else if (children().size() > 2) {
        /*
          An initializer expression is present, lets generate code
          for the initializer:
        */
        TreeNode *t = children().at(2);
        assert((t->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(t) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        ICode::Status s = e->generateCode(code, st, log, ns);
        if (s != ICode::OK) {
            delete ns;
            return s;
        }
        setFirstImop(e->firstImop());
        addToNextList(e->nextList());
    } else {
        // Otherwise assign the default value to the symbol:
        Imop *i = new Imop(this, Imop::ASSIGN, ns);
        code.push_imop(i);
        setFirstImop(i);

        typedef DataTypeBasic DTB;
        typedef DataTypeVar DTV;
        assert(m_type->dataType().kind() == DataType::VAR);
        assert(dynamic_cast<const DTV*>(&m_type->dataType()) != 0);
        const DTV &dtv(static_cast<const DTV&>(m_type->dataType()));
        assert(dtv.dataType().kind() == DataType::BASIC);
        assert(dynamic_cast<const DTB*>(&dtv.dataType()) != 0);
        const DTB &dtb(static_cast<const DTB&>(dtv.dataType()));

        Symbol *def;
        switch (dtb.dataType()) {
            case DATATYPE_BOOL:
                def = st.constantBool(false);
                break;
            case DATATYPE_INT:
                def = st.constantInt(0);
                break;
            case DATATYPE_UINT:
                def = st.constantUInt(0);
                break;
            case DATATYPE_STRING:
                def = st.constantString("");
                break;
            default:
                assert(false); // Shouldn't happen!
        }
        i->setArg1(def);
    }

    // Add the symbol to the symbol table for use in later expressions:
    st.appendSymbol(ns);

    setResultFlags(TreeNodeStmt::FALLTHRU);
    return ICode::OK;
}

ICode::Status TreeNodeStmtDecl::calculateResultType(SymbolTable &st, CompileLog &log) {
    typedef DataTypeBasic DTB;
    typedef SecTypeBasic STB;
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

    assert(dynamic_cast<const STB*>(&justType.secType()) != 0);
    const STB &secType = static_cast<const STB&>(justType.secType());


    if (children().size() > 2) {
        TreeNode *t = children().at(2);
        assert((t->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(t) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        ICode::Status s = e->calculateResultType(st, log);
        if (s != ICode::OK) return s;

        assert(e->haveResultType());
        if (e->resultType().isVoid()) {
            log.fatal() << "Initializer expression is a void expression at "
                        << e->location();
            return ICode::E_TYPE;
        }

        if (e->resultType().secrecSecType() == SECTYPE_PRIVATE
            && secType.secType() == SECTYPE_PUBLIC)
        {
            log.fatal() << "Public variable is given a private initializer at "
                        << e->location();
            return ICode::E_TYPE;
        }
        if (e->resultType().secrecDataType() != dataType.secrecDataType()) {
            log.fatal() << "Variable of type " << TNV(secType, DataTypeVar(dataType))
                        << " is given an initializer expression of type "
                        << e->resultType() << " at " << location();
            return ICode::E_TYPE;
        }
    }

    m_type = new TNV(secType, DataTypeVar(dataType));

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtDoWhile
*******************************************************************************/

ICode::Status TreeNodeStmtDoWhile::generateCode(ICodeList &code,
                                                SymbolTable &st,
                                                CompileLog &log)
{
    assert(children().size() == 2);
    TreeNode *c0 = children().at(0);
    TreeNode *c1 = children().at(1);
    assert((c1->type() & NODE_EXPR_MASK) != 0);

    // Loop body:
    SymbolTable &innerScope = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(c0) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(c0);
    ICode::Status s = body->generateCode(code, innerScope, log);
    if (s != ICode::OK) return s;

    // Static checking:
    if (body->firstImop() == 0) {
        log.fatal() << "Empty loop body at " << body->location();
        return ICode::E_OTHER;
    }
    assert(body->resultFlags() != 0x0);
    if ((body->resultFlags()
         & (TreeNodeStmt::FALLTHRU | TreeNodeStmt::CONTINUE)) == 0x0)
    {
        log.fatal() << "Do-while loop at " << location() << " wont loop!";
        return ICode::E_OTHER;
    }
    setResultFlags((body->resultFlags()
                    & ~(TreeNodeStmt::BREAK | TreeNodeStmt::CONTINUE))
                   | TreeNodeStmt::FALLTHRU);

    // Conditional expression:
    assert(c1->type() != NODE_EXPR_NONE);
    assert(dynamic_cast<TreeNodeExpr*>(c1) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c1);
    s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in if statement must be of "
                       "type public bool in " << e->location();
        return ICode::E_TYPE;
    }
    s = e->generateBoolCode(code, st, log);
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

ICode::Status TreeNodeStmtExpr::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log)
{
    assert(children().size() == 1);
    assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->generateCode(code, st, log);
    if (s != ICode::OK) return s;

    setFirstImop(e->firstImop());
    setNextList(e->nextList());
    setResultFlags(TreeNodeStmt::FALLTHRU);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtExpr
*******************************************************************************/


ICode::Status TreeNodeStmtAssert::generateCode(ICodeList &code, SymbolTable &st,
                                               CompileLog &log)
{
    assert(children().size() == 1);

    TreeNode *c0 = children().at(0);

    // Type check the expression
    assert((c0->type() & NODE_EXPR_MASK) != 0);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in assert statement must be of "
                       "type public bool in " << e->location();
        return ICode::E_TYPE;
    }

    // Generate code for conditional expression:
    s = e->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);
    setFirstImop(e->firstImop());

    std::ostringstream os;
    os << "assert failed at " << location();
    Imop *i = new Imop(this, Imop::ERROR, 0, (Symbol*) new std::string(os.str()));
    code.push_imop(i);

    e->patchNextList(i);
    e->patchFalseList(i);
    addToNextList(e->trueList());

    setResultFlags(FALLTHRU);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtFor
*******************************************************************************/

ICode::Status TreeNodeStmtFor::generateCode(ICodeList &code, SymbolTable &st,
                                            CompileLog &log)
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
        ICode::Status s = e0->generateCode(code, st, log);
        if (s != ICode::OK) return s;
        setFirstImop(e0->firstImop());
    }

    // Conditional expression:
    TreeNodeExpr *e1 = 0;
    if (children().at(1)->type() != NODE_EXPR_NONE) {
        assert(dynamic_cast<TreeNodeExpr*>(c1) != 0);
        e1 = static_cast<TreeNodeExpr*>(c1);
        ICode::Status s = e1->calculateResultType(st, log);
        if (s != ICode::OK) return s;
        if (!e1->havePublicBoolType()) {
            log.fatal() << "Conditional expression in if statement must be of "
                           "type public bool in " << e1->location();
            return ICode::E_TYPE;
        }
        s = e1->generateBoolCode(code, st, log);
        if (s != ICode::OK) return s;
        patchFirstImop(e1->firstImop());
        addToNextList(e1->falseList());
    }

    // Loop body:
    SymbolTable &innerScope = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(c3) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(c3);
    ICode::Status s = body->generateCode(code, innerScope, log);
    if (s != ICode::OK) return s;
    patchFirstImop(body->firstImop());
    if (e1 != 0) e1->patchTrueList(body->firstImop());
    addToNextList(body->breakList());

    // Iteration expression:
    TreeNodeExpr *e2 = 0;
    if (children().at(2)->type() != NODE_EXPR_NONE) {
        assert(dynamic_cast<TreeNodeExpr*>(c2) != 0);
        e2 = static_cast<TreeNodeExpr*>(c2);
        ICode::Status s = e2->generateCode(code, st, log);
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
    if ((body->resultFlags()
        & (TreeNodeStmt::FALLTHRU | TreeNodeStmt::CONTINUE)) == 0x0)
    {
        log.fatal() << "For loop at " << location() << " wont loop!";
        return ICode::E_OTHER;
    }
    if (e1 == 0 && ((body->resultFlags()
        & (TreeNodeStmt::BREAK | TreeNodeStmt::RETURN)) == 0x0))
    {
        log.fatal() << "For loop at " << location() << " is clearly infinite!";
        return ICode::E_OTHER;
    }
    setResultFlags((body->resultFlags()
                    & ~(TreeNodeStmt::BREAK | TreeNodeStmt::CONTINUE))
                   | TreeNodeStmt::FALLTHRU);

    // Next iteration jump:
    Imop *j = new Imop(this, Imop::JUMP, 0);
    if (e1 != 0) {
        j->setJumpDest(e2->firstImop());
    } else {
        j->setJumpDest(body->firstImop());
    }
    code.push_imop(j);

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtIf
*******************************************************************************/

ICode::Status TreeNodeStmtIf::generateCode(ICodeList &code, SymbolTable &st,
                                           CompileLog &log)
{
    assert(children().size() == 2 || children().size() == 3);
    TreeNode *c0 = children().at(0);

    // Type check the expression
    assert((c0->type() & NODE_EXPR_MASK) != 0);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in if statement must be of "
                       "type public bool in " << e->location();
        return ICode::E_TYPE;
    }

    // Generate code for conditional expression:
    s = e->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);
    setFirstImop(e->firstImop());

    // Generate code for first branch:
    SymbolTable &innerScope1 = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(children().at(1)) != 0);
    TreeNodeStmt *s1 = static_cast<TreeNodeStmt*>(children().at(1));
    s = s1->generateCode(code, innerScope1, log);
    if (s != ICode::OK) return s;

    if (s1->firstImop() != 0) {
        e->patchTrueList(s1->firstImop());
        addToNextList(s1->nextList());
        addToBreakList(s1->breakList());
        addToContinueList(s1->continueList());
        assert(s1->resultFlags() != 0x0);
    } else {
        addToNextList(e->trueList());
    }

    if (children().size() == 2) {
        addToNextList(e->falseList());
        setResultFlags(s1->resultFlags() | TreeNodeStmt::FALLTHRU);
    } else {
        // Generate jump out of first branch, if needed:
        if ((s1->resultFlags() & TreeNodeStmt::FALLTHRU) != 0x0) {
            Imop *i = new Imop(this, Imop::JUMP, 0);
            code.push_imop(i);
            addToNextList(i);
        }

        // Generate code for second branch:
        SymbolTable &innerScope2 = *st.newScope();
        assert(dynamic_cast<TreeNodeStmt*>(children().at(2)) != 0);
        TreeNodeStmt *s2 = static_cast<TreeNodeStmt*>(children().at(2));
        s = s2->generateCode(code, innerScope2, log);
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

ICode::Status TreeNodeStmtReturn::generateCode(ICodeList &code, SymbolTable &st,
                                               CompileLog &log)
{
    if (children().empty()) {
        if (containingProcedure()->procedureType().kind()
            == TypeNonVoid::PROCEDURE)
        {
            log.fatal() << "Cannot return from non-void function without value "
                           "at " << location();
            return ICode::E_OTHER;
        }
        assert(containingProcedure()->procedureType().kind()
               == TypeNonVoid::PROCEDUREVOID);

        Imop *i = new Imop(this, Imop::RETURNVOID);
        i->setReturnDestFirstImop(containingProcedure()->firstImop());
        code.push_imop(i);
        setFirstImop(i);
    } else {
        assert(children().size() == 1);
        if (containingProcedure()->procedureType().kind()
            == TypeNonVoid::PROCEDUREVOID)
        {
            log.fatal() << "Cannot return value from void function at"
                        << location();
            return ICode::E_OTHER;
        }
        assert(containingProcedure()->procedureType().kind()
               == TypeNonVoid::PROCEDURE);

        assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
        ICode::Status s = e->calculateResultType(st, log);
        if (s != ICode::OK) return s;
        if (!containingProcedure()->procedureType().canAssign(e->resultType()))
        {
            log.fatal() << "Cannot return value of type " << e->resultType()
                        << " from function with type "
                        << containingProcedure()->procedureType() << ". At"
                        << location();
            return ICode::E_OTHER;
        }
        // Add implicit classify node if needed:
        e = classifyChildAtIfNeeded(0, containingProcedure()->procedureType().secrecSecType());

        s = e->generateCode(code, st, log);
        if (s != ICode::OK) return s;
        setFirstImop(e->firstImop());

        Imop *i = new Imop(this, Imop::RETURN);
        i->setArg1(e->result());
        i->setReturnDestFirstImop(containingProcedure()->firstImop());
        code.push_imop(i);
        patchFirstImop(i);
    }
    setResultFlags(TreeNodeStmt::RETURN);

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtWhile
*******************************************************************************/

ICode::Status TreeNodeStmtWhile::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log)
{
    assert(children().size() == 2);
    TreeNode *c0 = children().at(0);
    TreeNode *c1 = children().at(1);
    assert((c0->type() & NODE_EXPR_MASK) != 0);

    // Conditional expression:
    assert(c0->type() != NODE_EXPR_NONE);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in while statement must be of "
                       "type public bool in " << e->location();
        return ICode::E_TYPE;
    }
    s = e->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);

    // Loop body:
    SymbolTable &innerScope = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(c1) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(c1);
    s = body->generateCode(code, innerScope, log);
    if (s != ICode::OK) return s;

    // Static checking:
    if (body->firstImop() == 0) {
        log.fatal() << "Empty loop body at " << body->location();
        return ICode::E_OTHER;
    }
    assert(body->resultFlags() != 0x0);
    if ((body->resultFlags()
        & (TreeNodeStmt::FALLTHRU | TreeNodeStmt::CONTINUE)) == 0x0)
    {
        log.fatal() << "While loop at " << location() << " wont loop!";
        return ICode::E_OTHER;
    }
    setResultFlags((body->resultFlags()
                    & ~(TreeNodeStmt::BREAK | TreeNodeStmt::CONTINUE))
                   | TreeNodeStmt::FALLTHRU);

    Imop *i = new Imop(this, Imop::JUMP);
    code.push_imop(i);
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
