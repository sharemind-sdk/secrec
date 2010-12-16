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
        case NODE_EXPR_INDEX: return "EXPR_INDEX";
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
        case NODE_DECL_VSUFFIX: return "DECL_VSUFFIX";
        case NODE_GLOBALS: return "DECL_GLOBALS";
        case NODE_PROCDEF: return "PROCDEF";
        case NODE_PROCDEFS: return "PROCDEFS";
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

bool TreeNodeExpr::checkAndLogIfVoid(CompileLog& log) {
    assert (haveResultType());
    if (resultType().isVoid()) {
        log.fatal() << "Subexpression has type void at "
                    << location();
        return true;
    }

    return false;
}

// figure out how to compute that less often
ICode::Status TreeNodeExpr::computeSize (ICodeList& code, SymbolTable& st) {
    assert (haveResultType() &&
            "ICE: TreeNodeExpr::computeSize called on expression without type.");
    assert (result() != 0 &&
            "ICE: TreeNodeExpr::computeSize called on expression with non-void type but no result.");

    if (resultType().isVoid()) return ICode::OK;
    if (resultType().isScalar()) return ICode::OK;

    assert ((result()->getSizeSym() != 0) &&
            "ICE: TreeNodeExpr::computeSize called on expression without size symbol.");


    Symbol* size = result()->getSizeSym();
    Imop* i = new Imop (this, Imop::ASSIGN, size, st.constantInt(1));
    code.push_imop(i);
    patchFirstImop(i);
    patchNextList(i);
    prevPatchNextList(i);

    Symbol::dim_iterator
            di = result()->dim_begin(),
            de = result()->dim_end();
    for (; di != de; ++ di) {
        assert (*di != 0 &&
                "ICE: TreeNodeExpr::computeSize called on expression with corrupt shape.");
        Imop* i = new Imop (this, Imop::MUL, size, size, *di);
        code.push_imop(i);
    }

    return ICode::OK;
}

void TreeNodeExpr::copyShapeFrom(Symbol* sym, ICodeList &code) {
    assert (haveResultType());
    assert (sym != 0);

    if (resultType().isScalar()) {
        return;
    }

    Symbol::dim_iterator
            di = sym->dim_begin(),
            de = sym->dim_end();
    Symbol::dim_iterator
            dj = result()->dim_begin();
    for (; di != de; ++ di, ++ dj) {
        Imop* i = new Imop(this, Imop::ASSIGN, *dj, *di);
        code.push_imop(i);
        patchFirstImop(i);
        patchNextList(i);
        prevPatchNextList(i);
    }

    Symbol* sl = result()->getSizeSym();
    Symbol* sr = sym->getSizeSym();
    Imop* i = new Imop(this, Imop::ASSIGN, sl, sr);
    code.push_imop(i);
    patchFirstImop(i);
    patchNextList(i);
    prevPatchNextList(i);
}

void TreeNodeExpr::generateResultSymbol(SymbolTable& st) {
    assert (haveResultType());
    if (!resultType().isVoid()) {
        assert(dynamic_cast<const TypeNonVoid*>(&resultType()) != 0);
        Symbol* sym = st.appendTemporary(static_cast<const TypeNonVoid&>(resultType()));
        setResult(sym);
        for (unsigned i = 0; i < resultType().secrecDimType(); ++ i) {
            sym->setDim(i, st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0)));
        }

        sym->setSizeSym(st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0)));
    }
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

extern "C" struct TreeNode *treenode_init_dimTypeF(
        unsigned dimType,
        YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeDimTypeF(dimType, *loc);
}

namespace SecreC {

/*******************************************************************************
  TreeNodeBase
*******************************************************************************/

void TreeNodeBase::patchNextList(Imop *dest) {
    patchList(m_nextList, dest);
}

void TreeNodeBase::addToNextList(const std::vector<Imop*> &nl) {
    appendVectorToVector(m_nextList, nl);
}

ICode::Status TreeNodeBase::generateSubexprCode (TreeNodeExpr* e, ICodeList& code, SymbolTable& st, CompileLog& log, Symbol* r) {
    ICode::Status s = e->generateCode(code, st, log, r);
    if (s != ICode::OK) return s;
    if (e->firstImop() != 0) {
        Imop* i = e->firstImop();
        patchFirstImop(i);
        patchNextList(i);
        if (prevSubexpr() != 0)
            prevSubexpr()->patchNextList(i);
        setPrevSubexpr(e);
    }

    return ICode::OK;
}

void TreeNodeBase::prevPatchNextList (Imop* i) {
    if (prevSubexpr())
        prevSubexpr()->patchNextList(i);
}

/*******************************************************************************
  TreeNodeCodeable
*******************************************************************************/

void TreeNodeCodeable::patchBreakList(Imop *dest) {
    patchList(m_breakList, dest);
}

void TreeNodeCodeable::patchContinueList(Imop *dest) {
    patchList(m_continueList, dest);
}

void TreeNodeCodeable::addToBreakList(const std::vector<Imop*> &bl) {
    appendVectorToVector(m_breakList, bl);
}

void TreeNodeCodeable::addToContinueList(const std::vector<Imop*> &cl) {
    appendVectorToVector(m_continueList, cl);
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
  TreeNodeExpr
*******************************************************************************/

void TreeNodeExpr::patchTrueList(Imop *dest) {
    patchList(m_trueList, dest);
}

void TreeNodeExpr::patchFalseList(Imop *dest) {
    patchList(m_falseList, dest);
}

void TreeNodeExpr::addToFalseList(const std::vector<Imop*> &fl) {
    appendVectorToVector(m_falseList, fl);
}

void TreeNodeExpr::addToTrueList(const std::vector<Imop*> &tl) {
    appendVectorToVector(m_trueList, tl);
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

    // typecheck the indices if given
    if (e1->children().size() == 2) {
        destDim = 0;
        TreeNode* t = e1->children().at(1);
        assert (t->type() == NODE_SUBSCRIPT);
        TreeNode::ChildrenListConstIterator
                i = t->children().begin(),
                end = t->children().end();
        for (; i != end; ++ i) {
            switch ((*i)->type()) {
                case NODE_INDEX_INT: break;
                case NODE_INDEX_SLICE: destDim += 1; break;
                default: assert (false && "Reached an index that isn't int or a slice.");
            }
        }
    }

    if (!(srcDim == destDim || srcDim == 0)) {
        log.fatal() << "Incompatible dimensionalities in assignemnt at "
                    << location();
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
    classifyChildAtIfNeeded(1, destType.secrecSecType());

    assert(dynamic_cast<const TNV*>(&destType) != 0);
    const TNV &destTypeNV = static_cast<const TNV&>(destType);
    assert(destTypeNV.dataType().kind() == DataType::VAR);
    assert(dynamic_cast<const DTV*>(&destTypeNV.dataType()) != 0);
    const DTV &ddtv = static_cast<const DTV&>(destTypeNV.dataType());
    setResultType(new TNV(ddtv.dataType()));
    return ICode::OK;
}

/// \todo this and TreeNodeExprIndex have a lot of duplicate code, figure out how to fix that
ICode::Status TreeNodeExprAssign::generateCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log,
                                               Symbol *r)
{
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV; // symbol pair vector

    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate code for child expressions:
    TreeNode *lval = children().at(0);
    assert (lval != 0);
    assert(dynamic_cast<TreeNodeIdentifier*>(lval->children().at(0)) != 0);
    TreeNodeIdentifier *e1 = static_cast<TreeNodeIdentifier*>(lval->children().at(0));
    Symbol *destSym = st.find(e1->value());
    assert(destSym->symbolType() == Symbol::SYMBOL);
    assert(dynamic_cast<SymbolSymbol*>(destSym) != 0);
    SymbolSymbol *destSymSym = static_cast<SymbolSymbol*>(destSym);

    // Generate code for righthand side:
    assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = generateSubexprCode(e2, code, st, log);
    if (s != ICode::OK) return s;

    // x[e1,...,ek] = e
    if (lval->children().size() == 2) {
        SPV spv;
        std::vector<unsigned > slices;

        { // evaluate indices
            TreeNode::ChildrenListConstIterator
                    it = lval->children().at(1)->children().begin(),
                    it_end = lval->children().at(1)->children().end();
            for (unsigned count = 0; it != it_end; ++ it, ++ count) {
                TreeNode* t = *it;
                Symbol* r_lo = 0;
                Symbol* r_hi = 0;

                // lower bound
                TreeNode* t_lo = t->children().at(0);
                if (t_lo->type() == NODE_EXPR_NONE) {
                    r_lo = st.constantInt(0);
                }
                else {
                    TreeNodeExpr* e_lo = static_cast<TreeNodeExpr*>(t_lo);
                    s = generateSubexprCode(e_lo, code, st, log);
                    if (s != ICode::OK) return s;
                    r_lo = e_lo->result();
                }

                // upper bound
                if (t->type() == NODE_INDEX_SLICE) {
                    TreeNode* t_hi = t->children().at(1);
                    if (t_hi->type() == NODE_EXPR_NONE) {
                        r_hi = destSym->getDim(count);
                    }
                    else {
                        TreeNodeExpr* e_hi = static_cast<TreeNodeExpr*>(t_hi);
                        s = generateSubexprCode(e_hi, code, st, log);
                        if (s != ICode::OK) return s;
                        r_hi = e_hi->result();
                    }

                    slices.push_back(count);
                }
                else {
                    // if there is no upper bound then make one up
                    r_hi = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
                    Imop* i = new Imop(this, Imop::ADD, r_hi, r_lo, st.constantInt(1));
                    code.push_imop(i);
                    patchFirstImop(i);
                    prevPatchNextList(i);
                }

                spv.push_back(std::make_pair(r_lo, r_hi));
            }
        }

        // 2. check that indices are legal

        {
            std::stringstream ss;
            ss << "Index out of bounds at " << location() << ".";
            Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
            Imop* err = new Imop(this, Imop::ERROR, (Symbol*) 0,
                                 (Symbol*) new std::string(ss.str()));

            // check lower indices
            Symbol::dim_iterator di = destSym->dim_begin();
            for (SPV::const_iterator it(spv.begin()); it != spv.end(); ++ it, ++ di) {
                code.push_comment("checking s_lo");
                Symbol* s_lo = it->first;
                Imop* i = new Imop(this, Imop::JGE, (Symbol*) 0, s_lo, *di);
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i);
                i->setJumpDest(err);
            }

            // check upper indices
            for (std::vector<unsigned>::const_iterator it(slices.begin()); it != slices.end(); ++ it) {
                code.push_comment("checking s_hi");
                unsigned k = *it;
                Symbol* s_hi = spv[k].second;
                Symbol* d = destSym->getDim(k);
                Imop* i = new Imop(this, Imop::JGT, (Symbol*) 0, s_hi, d);
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i);
                i->setJumpDest(err);

                Symbol* s_lo = spv[k].first;
                i = new Imop(this, Imop::JGE, (Symbol*) 0, s_lo, s_hi);
                code.push_imop(i);
                i->setJumpDest(err);
            }

            // check that rhs has correct dimensions
            if (!e2->resultType().isScalar()) {
                assert (e2->resultType().secrecDimType() == slices.size());
                for (unsigned k = 0; k < e2->resultType().secrecDimType(); ++ k) {
                    Symbol* tsym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
                    Imop* i = new Imop(this, Imop::SUB, tsym, spv[slices[k]].second, spv[slices[k]].first);
                    code.push_imop(i);
                    patchFirstImop(i);
                    prevPatchNextList(i);

                    i = new Imop(this, Imop::JNE, (Symbol*) 0, tsym, e2->result()->getDim(k));
                    code.push_imop(i);
                    i->setJumpDest(err);
                }
            }

            code.push_imop(jmp);
            patchFirstImop(jmp);
            prevPatchNextList(jmp);
            addToNextList(jmp); /// \todo not sure about that...

            code.push_imop(err);
        }

        // 3. initialize stride: s[0] = 1; ...; s[n+1] = s[n] * d[n];
        // - Note that after this point next lists and first imop are patched!
        // - size of 'x' is stored as last element of the stride
        std::vector<Symbol* > stride;
        {
            code.push_comment("Computing stride:");
            Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
            Imop* i = new Imop(this, Imop::ASSIGN, sym, st.constantUInt(1));
            stride.push_back(sym);
            code.push_imop(i);
            patchFirstImop(i);
            patchNextList(i);
            prevPatchNextList(i);

            Symbol* prev = sym;
            for (Symbol::dim_iterator it(destSym->dim_begin()); it != destSym->dim_end(); ++ it) {
                sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
                i = new Imop(this, Imop::MUL, sym, prev, *it);
                stride.push_back(sym);
                code.push_imop(i);
                prev = sym;
            }
        }

        // 4. initialze running indices
        std::vector<Symbol* > indices;
        for (SPV::iterator it(spv.begin()); it != spv.end(); ++ it) {
            Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
            indices.push_back(sym);
        }

        // 6. initialze symbols for offsets and temporary results
        Symbol* offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
        Symbol* old_offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
        Symbol* tmp_result2 = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));

        // 7. start
        std::stack<Imop*> jump_stack;
        {
            SPV::iterator
                    spv_it = spv.begin(),
                    spv_it_end = spv.end();
            std::vector<Symbol* >::iterator
                    it_it = indices.begin();

            // offset = 0
            Imop* i = new Imop(this, Imop::ASSIGN, offset, st.constantUInt(0));
            code.push_imop(i);
            patchNextList(i);

            for (; spv_it != spv_it_end; ++ spv_it, ++ it_it) {
                Symbol* i_lo = spv_it->first;
                Symbol* i_hi = spv_it->second;
                Symbol* idx  = *it_it;

                // i = i_lo;
                i = new Imop(this, Imop::ASSIGN, idx, i_lo);
                code.push_imop(i);

                // L1: IF (i >= i_hi) GOTO O1;
                Imop* l1 = new Imop(this, Imop::JGE, (Symbol*) 0, idx, i_hi);
                code.push_imop(l1);
                jump_stack.push(l1);
            }
        }

        // 8. compute offset for RHS
        {
            // old_ffset = 0
            Imop* i = new Imop(this, Imop::ASSIGN, old_offset, st.constantUInt(0));
            code.push_imop(i);

            std::vector<Symbol* >::iterator stride_it = stride.begin();
            std::vector<Symbol* >::iterator it_end = indices.end();
            std::vector<Symbol* >::iterator it_it = indices.begin();
            for (; it_it != it_end; ++ stride_it, ++ it_it) {
                // tmp_result2 = s[k] * idx[k]
                i = new Imop(this, Imop::MUL, tmp_result2, *stride_it, *it_it);
                code.push_imop(i);

                // old_offset = old_offset + tmp_result2
                i = new Imop(this, Imop::ADD, old_offset, old_offset, tmp_result2);
                code.push_imop(i);
            }
        }

        // 9. load and store
        {
            if (type() == NODE_EXPR_ASSIGN) {
                if (!e2->resultType().isScalar()) {
                    Symbol* t1 = st.appendTemporary(TypeNonVoid(resultType().secrecSecType(), resultType().secrecDataType(), 0));

                    Imop* i = new Imop(this, Imop::LOAD, t1, e2->result(), offset);
                    code.push_imop(i);

                    i = new Imop(this, Imop::STORE, destSymSym, old_offset, t1);
                    code.push_imop(i);
                }
                else {
                    Imop* i = new Imop(this, Imop::STORE, destSymSym, old_offset, e2->result());
                    code.push_imop(i);
                }
            }
            else {
                Symbol* t1 = st.appendTemporary(TypeNonVoid(resultType().secrecSecType(), resultType().secrecDataType(), 0));
                Symbol* t2 = st.appendTemporary(TypeNonVoid(resultType().secrecSecType(), resultType().secrecDataType(), 0));
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

                Imop* i = new Imop(this, Imop::LOAD, t1, destSymSym, old_offset);
                code.push_imop(i);

                if (!e2->resultType().isScalar()) {
                    i = new Imop(this, Imop::LOAD, t2, e2->result(), offset);
                    code.push_imop(i);

                    i = new Imop(this, iType, t1, t1, t2);
                    code.push_imop(i);

                    i = new Imop(this, Imop::STORE, destSymSym, old_offset, t1);
                    code.push_imop(i);
                }
                else {
                    i = new Imop(this, iType, t1, t1, e2->result());
                    code.push_imop(i);

                    i = new Imop(this, Imop::STORE, destSymSym, old_offset, t1);
                    code.push_imop(i);
                }
            }

            // offset = offset + 1
            Imop* i = new Imop(this, Imop::ADD, offset, offset, st.constantUInt(1));
            code.push_imop(i);
       }

       // 9. loop exit
       {
            code.push_comment("Tail of indexing loop:");
            std::vector<Symbol* >::reverse_iterator
                    rit = indices.rbegin(),
                    rit_end = indices.rend();
            Imop* prev_jump = 0;
            for (; rit != rit_end; ++ rit) {
                Symbol* idx = *rit;

                // i = i + 1
                Imop* i = new Imop(this, Imop::ADD, idx, idx, st.constantUInt(1));
                code.push_imop(i);
                if (prev_jump != 0) prev_jump->setJumpDest(i);

                // GOTO L1;
                i = new Imop(this, Imop::JUMP, (Symbol*) 0);
                code.push_imop(i);
                i->setJumpDest(jump_stack.top());
                prev_jump = jump_stack.top();
                jump_stack.pop();

                // O1:
            }

            if (prev_jump != 0) addToNextList(prev_jump);
        }

        return ICode::OK;
    }

    // Generate code for regular x = e assignment
    if (type() == NODE_EXPR_ASSIGN) {

        if (r != 0)
            setResult(r);
         else
            setResult(destSymSym);

        if (!e2->resultType().isScalar()) {
            copyShapeFrom(e2->result(), code);
        }

        Imop *i = 0;
        if (resultType().isScalar())
            i = new Imop(this, Imop::ASSIGN, result(), e2->result());
        else {
            Imop::Type iType = e2->resultType().isScalar() ? Imop::FILL : Imop::ASSIGN;
            i = new Imop(this, iType, result(), e2->result(), result()->getSizeSym());
        }
        code.push_imop(i);
        patchFirstImop(i);
        e2->patchNextList(i);
        setNextList(e2->nextList());
    } else {
        // Arithmetic assignments

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

        Imop *i = 0;
        if (resultType().isScalar()) {
            i = new Imop(this, iType, destSymSym, destSymSym, e2->result());
        }
        else {
            Symbol* tmp = st.appendTemporary(static_cast<TypeNonVoid const&>(resultType()));
            i = new Imop(this, Imop::FILL, tmp, e2->result(), destSym->getSizeSym());
            code.push_imop(i);
            patchFirstImop(i);
            e2->patchNextList(i);

            i = new Imop(this, iType, destSymSym, destSymSym, tmp, destSym->getSizeSym());
        }

        code.push_imop(i);
        patchFirstImop(i);
        e2->patchNextList(i);

        if (r != 0) {
            i = 0;
            if (resultType().isScalar()) {
                i = new Imop(this, Imop::ASSIGN, r, destSymSym);
            }
            else {
                i = new Imop(this, Imop::ASSIGN, r, destSymSym, destSym->getSizeSym());
            }
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
    TreeNode::ChildrenListConstIterator
            i = children().at(1)->children().begin(),
            iend = children().at(1)->children().end();

    if (std::distance(i, iend) != n) {
        log.fatal() << "Incorrent number of indices at"
                    << e->location();
        return ICode::E_TYPE;
    }

    // for all indices
    for (; i != iend; ++ i) {
        TreeNode::Type type = (*i)->type();
        switch (type) {
            case NODE_INDEX_INT: break;
            case NODE_INDEX_SLICE: k += 1; break;
            default: assert (false && "Index that isn't int or slice.");
        }

        TreeNode::ChildrenListConstIterator
                j = (*i)->children().begin(),
                jend = (*i)->children().end();
        // check both INT and SLICE cases at the same time
        // we iterate the subexpressions like: [e1,e2,e3:e4,...]
        for (; j != jend; ++ j) {
            if ((*j)->type() == NODE_EXPR_NONE) continue;
            assert (dynamic_cast<TreeNodeExpr*>(*j) != 0);
            TreeNodeExpr* ej = static_cast<TreeNodeExpr*>(*j);
            s = ej->calculateResultType(st, log);
            if (s != ICode::OK) return s;
            if (ej->checkAndLogIfVoid(log)) return ICode::E_TYPE;

            TNV const* ejType = static_cast<TNV const*>(&ej->resultType());

            if (ejType->secrecSecType() != SECTYPE_PUBLIC) {
                log.fatal() << "Index of non-public type at " << ej->location();
                return ICode::E_TYPE;
            }

            if (ejType->secrecDataType() != DATATYPE_INT) {
                log.fatal() << "Index of non-integer type at " << ej->location();
                return ICode::E_TYPE;
            }

            if (ejType->secrecDimType() != 0) {
                log.fatal() << "Indexof non-scalar at " << ej->location();
                return ICode::E_TYPE;
            }
        }
    }

    setResultType(new TNV(eType->secrecSecType(), eType->secrecDataType(), k));
    return ICode::OK;
}

ICode::Status TreeNodeExprIndex::generateCode(ICodeList &code,
                                              SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    typedef TreeNode::ChildrenListConstIterator CLCI; // children list const iterator
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV; // symbol pair vector

    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;
    bool isScalar = resultType().isScalar();
    SPV spv; // store symbols corresponding to upper and lower indices
    std::vector<int > slices; // which of the indices are slices

    // 0. Generate temporary for the result of the binary expression, if needed:
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    // 1. evaluate subexpressions

    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;
    Symbol* x = e->result();

    {
        CLCI it = children().at(1)->children().begin();
        CLCI it_end = children().at(1)->children().end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            TreeNode* t = *it;
            Symbol* r_lo = 0;
            Symbol* r_hi = 0;

            // lower bound
            TreeNode* t_lo = t->children().at(0);
            if (t_lo->type() == NODE_EXPR_NONE) {
                r_lo = st.constantInt(0);
            }
            else {
                TreeNodeExpr* e_lo = static_cast<TreeNodeExpr*>(t_lo);
                s = generateSubexprCode(e_lo, code, st, log);
                if (s != ICode::OK) return s;
                r_lo = e_lo->result();
            }

            // upper bound
            if (t->type() == NODE_INDEX_SLICE) {
                TreeNode* t_hi = t->children().at(1);
                if (t_hi->type() == NODE_EXPR_NONE) {
                    r_hi = x->getDim(count);
                }
                else {
                    TreeNodeExpr* e_hi = static_cast<TreeNodeExpr*>(t_hi);
                    s = generateSubexprCode(e_hi, code, st, log);
                    if (s != ICode::OK) return s;
                    r_hi = e_hi->result();
                }

                slices.push_back(count);
            }
            else {
                // if there is no upper bound then make one up
                r_hi = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
                Imop* i = new Imop(this, Imop::ADD, r_hi, r_lo, st.constantInt(1));
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i);
            }

            spv.push_back(std::make_pair(r_lo, r_hi));
        }
    }

    // 2. check that indices are legal
    {
        code.push_comment("Validating indices:");

        std::stringstream ss;
        ss << "Index out of bounds at " << location() << ".";
        Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
        Imop* err = new Imop(this, Imop::ERROR, (Symbol*) 0,
                             (Symbol*) new std::string(ss.str()));

        Symbol::dim_iterator dit = x->dim_begin();
        for (SPV::iterator it(spv.begin()); it != spv.end(); ++ it, ++ dit) {
            Symbol* s_lo = it->first;
            Symbol* s_hi = it->second;
            Symbol* d = *dit;
            // o <= s_lo <= s_hi <= d

            code.push_comment("checking 0 <= s_lo <= s_hi <= d");

            Imop* i = new Imop(this, Imop::JGT, (Symbol*) 0, st.constantInt(0), s_lo);
            code.push_imop(i);
            patchFirstImop(i);
            prevPatchNextList(i);
            i->setJumpDest(err);

            i = new Imop(this, Imop::JGT, (Symbol*) 0, s_lo, s_hi);
            code.push_imop(i);
            i->setJumpDest(err);

            i = new Imop(this, Imop::JGT, (Symbol*) 0, s_hi, d);
            code.push_imop(i);
            i->setJumpDest(err);
        }

        code.push_imop(jmp);
        patchFirstImop(jmp);
        prevPatchNextList(jmp);
        addToNextList(jmp); /// \todo not sure about that...

        code.push_imop(err);
    }

    // 5. compute resulting shape
    {
        code.push_comment("Computing shape:");
        std::vector<int>::const_iterator
                it = slices.begin(),
                it_end = slices.end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            int k = *it;
            Symbol* sym = result()->getDim(count);
            assert (sym != 0);
            Imop* i = new Imop(this, Imop::SUB, sym, spv[k].second, spv[k].first);
            code.push_imop(i);
            patchNextList(i);
            prevPatchNextList(i);
        }

        s = computeSize(code, st);
        if (s != ICode::OK) return s;
    }

    // r = FILL def size
    if (!isScalar) {
        Imop* i = new Imop(this, Imop::FILL, result(), (Symbol*) 0, result()->getSizeSym());
        switch (resultType().secrecDataType()) {
            case DATATYPE_BOOL: i->setArg1(st.constantBool(false)); break;
            case DATATYPE_INT: i->setArg1(st.constantInt(0)); break;
            case DATATYPE_UINT: i->setArg1(st.constantUInt(0)); break;
            case DATATYPE_STRING: i->setArg1(st.constantString("")); break;
            default: assert (false);
        }

        code.push_imop(i);
    }


    // 4. initialze required temporary symbols
    std::vector<Symbol* > indices;
    for (SPV::iterator it(spv.begin()); it != spv.end(); ++ it) {
        Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
        indices.push_back(sym);
    }

    Symbol* offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
    Symbol* tmp_result = st.appendTemporary(TypeNonVoid(e->resultType().secrecSecType(), e->resultType().secrecDataType(), 0));
    Symbol* tmp_result2 = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));

    // 3. initialize strides: s[0] = 1; ...; s[n+1] = s[n] * d[n];
    std::vector<Symbol* > strides [2];
    //code.push_imop(new Imop(this, Imop::PRINT, (Symbol*) 0, st.constantString("<STRIDES ")));
    for (unsigned j = 0; j < 2; ++ j){
        code.push_comment("Computing stride:");
        Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
        Imop* i = new Imop(this, Imop::ASSIGN, sym, st.constantUInt(1));
        strides[j].push_back(sym);
        code.push_imop(i);
        patchFirstImop(i);
        patchNextList(i);
        prevPatchNextList(i);

        Symbol* prevSym = sym;
        Symbol* resSym = 0;
        if (j == 0) resSym = x;
        if (j == 1) resSym = result();
        for (Symbol::dim_iterator it(resSym->dim_begin()); it != resSym->dim_end(); ++ it) {
            sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
            i = new Imop(this, Imop::MUL, sym, prevSym, *it);
            strides[j].push_back(sym);
            code.push_imop(i);
            prevSym = sym;
        }
    }
    //code.push_imop(new Imop(this, Imop::PRINT, (Symbol*) 0, st.constantString(">")));


    // 7. start
    std::stack<Imop*> jump_stack;
    {
        SPV::iterator spv_it = spv.begin();
        SPV::iterator spv_it_end = spv.end();
        std::vector<Symbol* >::iterator it_it = indices.begin();
        code.push_comment("Head of indexing loop:");

        for (; spv_it != spv_it_end; ++ spv_it, ++ it_it) {
            Symbol* i_lo = spv_it->first;
            Symbol* i_hi = spv_it->second;
            Symbol* idx  = *it_it;

            // i = i_lo;
            Imop* i = new Imop(this, Imop::ASSIGN, idx, i_lo);
            code.push_imop(i);

            // L1: IF (i >= i_hi) GOTO O1;
            Imop* l1 = new Imop(this, Imop::JGE, (Symbol*) 0, idx, i_hi);
            code.push_imop(l1);
            jump_stack.push(l1);
        }
    }

    // 8. compute offset for RHS
    {
        // old_ffset = 0
        code.push_comment("Compute offset:");
        Imop* i = new Imop(this, Imop::ASSIGN, offset, st.constantUInt(0));
        code.push_imop(i);


        std::vector<Symbol* >::iterator stride_it = strides[0].begin();
        std::vector<Symbol* >::iterator it_end = indices.end();
        std::vector<Symbol* >::iterator it_it = indices.begin();
        for (; it_it != it_end; ++ stride_it, ++ it_it) {
            // tmp_result2 = s[k] * idx[k]
            i = new Imop(this, Imop::MUL, tmp_result2, *stride_it, *it_it);
            code.push_imop(i);

            // old_offset = old_offset + tmp_result2
            i = new Imop(this, Imop::ADD, offset, offset, tmp_result2);
            code.push_imop(i);
        }
    }

    // 9. load and store
    {
        code.push_comment("Load and store:");

        // tmp = x[old_offset] or r = x[old_offset] if scalar
        Imop* i = new Imop(this, Imop::LOAD, (isScalar ? result() : tmp_result), x, offset);
        code.push_imop(i);

        //code.push_imop(new Imop(this, Imop::PRINT, (Symbol*) 0, st.constantString(">")));

        // r[offset] = tmp is not scalar
        if (!isScalar) {
            i = new Imop(this, Imop::ASSIGN, offset, st.constantInt(0));
            code.push_imop(i);

            unsigned count = 0;
            for (std::vector<int >::iterator it(slices.begin()); it != slices.end(); ++ it, ++ count) {
                unsigned k = *it;
                Symbol* idx = indices.at(k);

                i = new Imop(this, Imop::SUB, tmp_result2, idx, spv[k].first);
                code.push_imop(i);

                i = new Imop(this, Imop::MUL, tmp_result2, tmp_result2, strides[1].at(count));
                code.push_imop(i);

                i = new Imop(this, Imop::ADD, offset, offset, tmp_result2);
                code.push_imop(i);
            }

            i = new Imop(this, Imop::STORE, result(), offset, tmp_result);
            code.push_imop(i);
        }
    }

    // 9. loop exit
    {
        code.push_comment("Tail of indexing loop:");
        std::vector<Symbol* >::reverse_iterator
                rit = indices.rbegin(),
                rit_end = indices.rend();
        Imop* prevJump = 0;
        for (; rit != rit_end; ++ rit) {
            Symbol* idx = *rit;

            // i = i + 1
            Imop* i = new Imop(this, Imop::ADD, idx, idx, st.constantUInt(1));
            code.push_imop(i);
            if (prevJump != 0) prevJump->setJumpDest(i);

            // GOTO L1;
            i = new Imop(this, Imop::JUMP, (Symbol*) 0);
            code.push_imop(i);
            i->setJumpDest(jump_stack.top());
            prevJump = jump_stack.top();
            jump_stack.pop();

            // O1:
        }

        if (prevJump != 0) addToNextList(prevJump);
    }

    return ICode::OK;
}

/// \todo generate specialized code here
ICode::Status TreeNodeExprIndex::generateBoolCode(ICodeList &code,
                                                  SymbolTable &st,
                                                  CompileLog &log)
{
    assert(havePublicBoolType());

    ICode::Status s = generateCode(code, st, log);
    if (s != ICode::OK) return s;

    Imop *i = new Imop(this, Imop::JT, 0, result());
    code.push_imop(i);
    patchFirstImop(i);
    patchNextList(i);
    addToTrueList(i);

    i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    addToFalseList(i);

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

    setResultType(new TNV(SECTYPE_PUBLIC, DATATYPE_INT, 0));
    return ICode::OK;
}

ICode::Status TreeNodeExprSize::generateCode(ICodeList &code,
                                             SymbolTable &st,
                                             CompileLog &log,
                                             Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    Symbol* size = e->resultType().isScalar() ? st.constantInt(1) : e->result()->getSizeSym();

    if (r != 0) {
        assert (r->secrecType().canAssign(resultType()));
        Imop* i = new Imop(this, Imop::ASSIGN, r, size);
        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i);
        setResult(r);
    }
    else {
        addToNextList(e->nextList());
        setResult(size);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprSize::generateBoolCode(ICodeList &,
                                                  SymbolTable &,
                                                  CompileLog &)
{
    assert (false && "TreeNodeExprSize::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
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

    setResultType(new TNV(SECTYPE_PUBLIC, DATATYPE_INT, 1));
    return ICode::OK;
}

ICode::Status TreeNodeExprShape::generateCode(ICodeList &code,
                                              SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;
    Symbol* n = st.constantInt(e->resultType().secrecDimType());
    Imop* i = 0;

    if (e->resultType().isScalar())
        i = new Imop(this, Imop::ASSIGN, result(), st.constantInt(0));
    else
        i = new Imop(this, Imop::FILL, result(), st.constantInt(0), n);
    code.push_imop(i);
    prevPatchNextList(i);

    i = new Imop(this, Imop::ASSIGN, result()->getDim(0), n);
    code.push_imop(i);
    patchFirstImop(i);

    Symbol::dim_iterator
            dti = e->result()->dim_begin(),
            dte = e->result()->dim_end();
    for (unsigned count = 0; dti != dte; ++ dti, ++ count) {
        Imop* i = new Imop(this, Imop::STORE, result(), st.constantInt(count), *dti);
        code.push_imop(i);
        patchFirstImop(i);
    }

    computeSize(code, st);

    return ICode::OK;
}

ICode::Status TreeNodeExprShape::generateBoolCode(ICodeList &,
                                                  SymbolTable &,
                                                  CompileLog &)
{
    assert (false && "TreeNodeExprShape::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
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
        e3Type->secrecSecType() != SECTYPE_PUBLIC) {
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

ICode::Status TreeNodeExprCat::generateCode(ICodeList &code,
                                            SymbolTable &st,
                                            CompileLog &log,
                                            Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary result if needed:
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    TreeNodeExpr* e1 = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e1, code, st, log);
    if (s != ICode::OK) return s;

    TreeNodeExpr* e2 = static_cast<TreeNodeExpr*>(children().at(1));
    s = generateSubexprCode(e2, code, st, log);
    if (s != ICode::OK) return s;

    unsigned k = static_cast<TreeNodeExprInt*>(children().at(2))->value();
    unsigned n = resultType().secrecDimType();

    // Compute resulting shape and perform sanity check:
    std::stringstream ss;
    ss << "Different sized dimensions in concat at " << location() << ".";
    Imop* err = new Imop(this, Imop::ERROR, (Symbol*) 0,
                         (Symbol*) new std::string(ss.str()));
    for (unsigned it = 0; it < resultType().secrecDimType(); ++ it) {
        Symbol* s1 = e1->result()->getDim(it);
        Symbol* s2 = e2->result()->getDim(it);
        if (it == k) {
            Imop* i = new Imop(this, Imop::ADD, result()->getDim(it), s1, s2);
            code.push_imop(i);
            patchFirstImop(i);
            prevPatchNextList(i);
        }
        else {
            Imop* i = new Imop(this, Imop::JNE, (Symbol*) 0, s1, s2);
            code.push_imop(i);
            patchFirstImop(i);
            i->setJumpDest(err);
            prevPatchNextList(i);

            i = new Imop(this, Imop::ASSIGN, result()->getDim(it), s1);
            code.push_imop(i);
        }
    }

    Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
    code.push_imop(jmp);
    patchFirstImop(jmp);
    prevPatchNextList(jmp);
    addToNextList(jmp);

    code.push_imop(err);

    // Initialize strides:
    std::vector<Symbol* > strides [3];
    {
        Imop* i = 0;
        code.push_comment("Computing strides:");
        for (unsigned j = 0; j < 3; ++ j) {
            strides[j].reserve(n);
            for (unsigned it = 0; it < n; ++ it)
                strides[j].push_back(st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0)));

            i = new Imop(this, Imop::ASSIGN, strides[j][0], st.constantInt(1));
            code.push_imop(i);
            patchNextList(i);

            for (unsigned it = 1; it < n; ++ it) {
                Symbol* sym = 0;
                if (j == 0) sym = e1->result()->getDim(it - 1);
                if (j == 1) sym = e2->result()->getDim(it - 1);
                if (j == 2) sym = result()->getDim(it - 1);
                i = new Imop(this, Imop::MUL, strides[j][it], strides[j][it - 1], sym);
                code.push_imop(i);
            }
        }
    }

    // Symbols for running indices:
    std::vector<Symbol* > indices;
    for (unsigned it = 0; it < n; ++ it) {
        Symbol* sym = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
        indices.push_back(sym);
    }

    // Compute size and allocate resulting array:

    s = computeSize(code, st);
    if (s != ICode::OK) return s;

    Imop* i = new Imop(this, Imop::FILL, result(), (Symbol*) 0, result()->getSizeSym());
    switch (resultType().secrecDataType()) {
        case DATATYPE_BOOL: i->setArg1(st.constantBool(false)); break;
        case DATATYPE_INT: i->setArg1(st.constantInt(0)); break;
        case DATATYPE_UINT: i->setArg1(st.constantUInt(0)); break;
        case DATATYPE_STRING: i->setArg1(st.constantString("")); break;
        default: assert (false);
    }

    code.push_imop(i);

    // Loop:
    std::stack<Imop*> imop_stack;
    {
        std::vector<Symbol*>::iterator
                it = indices.begin(),
                it_end = indices.end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            Symbol* idx = *it;

            // i = 0
            i = new Imop(this, Imop::ASSIGN, idx, st.constantInt(0));
            code.push_imop(i);

            // L1: IF (i >= i_hi) GOTO O1;
            Imop* l1 = new Imop(this, Imop::JGE, (Symbol*) 0, idx, result()->getDim(count));
            code.push_imop(l1);
            imop_stack.push(l1);
        }
    }

    Symbol* offset = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT));
    Symbol* tmp_int = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT));

    // j = 0 (right hand side index)
    i = new Imop(this, Imop::ASSIGN, offset, st.constantInt(0));
    code.push_imop(i);

    // IF (i_k >= d_k) GOTO T1;
    i = new Imop(this, Imop::JGE, (Symbol*) 0, indices[k], e1->result()->getDim(k));
    code.push_imop(i);
    addToNextList(i);

    { // compute j if i < d (for e1)
        code.push_comment("START LHS offset:");
        std::vector<Symbol* >::iterator
                it = strides[0].begin(),
                it_end = strides[0].end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            Imop* i = new Imop(this, Imop::MUL, tmp_int, *it, indices[count]);
            code.push_imop(i);

            i = new Imop(this, Imop::ADD, offset, offset, tmp_int);
            code.push_imop(i);
        }

        code.push_comment("END LHS offset:");
    }

    // t = x[j]
    Symbol* tmp_elem = st.appendTemporary(TypeNonVoid(resultType().secrecSecType(), resultType().secrecDataType()));
    i = new Imop(this, Imop::LOAD, tmp_elem, e1->result(), offset);
    code.push_imop(i);

    // jump out
    Imop* jump_out = new Imop(this, Imop::JUMP, (Symbol*) 0);
    code.push_imop(jump_out);

    { // compute j if i >= d (for e2)
        code.push_comment("START RHS offset:");
        std::vector<Symbol* >::iterator
                it = strides[1].begin(),
                it_end = strides[1].end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            if (count == k) {
                i = new Imop(this, Imop::SUB, tmp_int, indices[count], e1->result()->getDim(k));
                code.push_imop(i);
                patchNextList(i);

                i = new Imop(this, Imop::MUL, tmp_int, *it, tmp_int);
                code.push_imop(i);
            }
            else {
                i = new Imop(this, Imop::MUL, tmp_int, *it, indices[count]);
                code.push_imop(i);
                patchNextList(i);
            }

            i = new Imop(this, Imop::ADD, offset, offset, tmp_int);
            code.push_imop(i);
        }
        code.push_comment("END RHS offset.");
    }

    // t = y[j]
    i = new Imop(this, Imop::LOAD, tmp_elem, e2->result(), offset);
    code.push_imop(i);
    patchNextList(i);

    // out: r[i] = t
    i = new Imop(this, Imop::ASSIGN, offset, st.constantInt(0));
    code.push_imop(i);
    jump_out->setJumpDest(i);

    { // compute j if i < d (for e1)
        code.push_comment("START LHS offset:");
        std::vector<Symbol* >::iterator
                it = strides[2].begin(),
                it_end = strides[2].end();
        for (unsigned count = 0; it != it_end; ++ it, ++ count) {
            Imop* i = new Imop(this, Imop::MUL, tmp_int, *it, indices[count]);
            code.push_imop(i);

            i = new Imop(this, Imop::ADD, offset, offset, tmp_int);
            code.push_imop(i);
        }

        code.push_comment("END LHS offset:");
    }

    i = new Imop(this, Imop::STORE, result(), offset, tmp_elem);
    code.push_imop(i);


    // i = i + 1
    //i = new Imop(this, Imop::ADD, offset, offset, st.constantUInt(1));
    //code.push_imop(i);

    // loop exit
    {
        code.push_comment("Tail of indexing loop:");
        std::vector<Symbol* >::reverse_iterator
                rit = indices.rbegin(),
                rit_end = indices.rend();
        Imop* prevJump = 0;
        for (; rit != rit_end; ++ rit) {
            Symbol* idx = *rit;

            // i = i + 1
            Imop* i = new Imop(this, Imop::ADD, idx, idx, st.constantUInt(1));
            code.push_imop(i);
            if (prevJump != 0) prevJump->setJumpDest(i);

            // GOTO L1;
            i = new Imop(this, Imop::JUMP, (Symbol*) 0);
            code.push_imop(i);
            i->setJumpDest(imop_stack.top());
            prevJump = imop_stack.top();
            imop_stack.pop();

            // O1:
        }

        if (prevJump != 0) addToNextList(prevJump);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprCat::generateBoolCode(ICodeList &,
                                                SymbolTable &,
                                                CompileLog &)
{
    assert (false && "TreeNodeExprCat::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
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
            eiType->secrecSecType() != SECTYPE_PUBLIC ||
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

ICode::Status TreeNodeExprReshape::generateCode(ICodeList &code,
                                                SymbolTable &st,
                                                CompileLog &log,
                                                Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate temporary result if needed:
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    // Evaluate subexpressions:
    TreeNodeExpr* e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    {
        TreeNode::ChildrenListConstIterator it = children().begin() + 1;
        Symbol::dim_iterator dim_it = result()->dim_begin();
        for (; it != children().end(); ++ it, ++ dim_it) {
            TreeNodeExpr* ei = static_cast<TreeNodeExpr*>(*it);
            s = generateSubexprCode(ei, code, st, log, *dim_it);
            if (s != ICode::OK) return s;
        }
    }

    // Compute new size:
    Symbol* s_new = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC,DATATYPE_INT,0));
    Imop* i = new Imop(this, Imop::ASSIGN, s_new, st.constantInt(1));
    code.push_imop(i);
    prevPatchNextList(i);
    patchFirstImop(i);
    Symbol::dim_iterator
            dim_it = result()->dim_begin(),
            dim_end = result()->dim_end();
    for (; dim_it != dim_end; ++ dim_it) {
        Imop* i = new Imop(this, Imop::MUL, s_new, s_new, *dim_it);
        code.push_imop(i);
    }

    Symbol* rhs = e->result();

    if (!e->resultType().isScalar()) {
        // Check that new and old sizes are equal:
        Imop* jmp = new Imop (this, Imop::JE, (Symbol*) 0, rhs->getSizeSym(), s_new);
        code.push_imop(jmp);
        addToNextList(jmp);

        Imop* err = new Imop (this, Imop::ERROR, (Symbol*) 0,
                          (Symbol*) new std::string("ERROR: Mismatching sizes in reshape!"));
        code.push_imop(err);

    }
    else {
        // Convert scalar to constant array:
        rhs = st.appendTemporary(TypeNonVoid(e->resultType().secrecSecType(),
                                             e->resultType().secrecDataType(),
                                             resultType().secrecDimType()));
        Imop* i = new Imop(this, Imop::FILL, rhs, e->result(), s_new);
        code.push_imop(i);
    }

    // Copy result:
    i = new Imop(this, Imop::ASSIGN, result(), rhs, s_new);
    code.push_imop(i);
    patchNextList(i);

    i = new Imop(this, Imop::ASSIGN, result()->getSizeSym(), s_new);
    code.push_imop(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprReshape::generateBoolCode(ICodeList &,
                                                    SymbolTable &,
                                                    CompileLog &)
{
    assert (false && "TreeNodeExprReshape::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
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
         eType->secrecSecType() != SECTYPE_PUBLIC) {
        log.fatal() << "fread expression at " << location() << " has to take public scalar string as argument, got "
                    << *eType;
        return ICode::E_TYPE;
    }

    setResultType(new TypeNonVoid(SECTYPE_PRIVATE, DATATYPE_INT, 2));

    return ICode::OK;
}

ICode::Status TreeNodeExprFRead::generateCode(ICodeList &code,
                                              SymbolTable &st,
                                              CompileLog &log,
                                              Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Generate code for child expression
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the classification, if needed:
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
    }

    Imop* i = 0;

    i = new Imop(this, Imop::FREAD, (Symbol*) 0, e->result());
    code.push_imop(i);
    patchFirstImop(i);
    prevPatchNextList(i);

    i = new Imop(this, Imop::POP, result()->getDim(1));
    code.push_imop(i);

    i = new Imop(this, Imop::POP, result()->getDim(0));
    code.push_imop(i);

    computeSize(code, st);

    i = new Imop(this, Imop::FILL, result(), st.constantInt(0), result()->getSizeSym());
    code.push_imop(i);

    i = new Imop(this, Imop::POP, result(), result()->getSizeSym());
    code.push_imop(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprFRead::generateBoolCode(ICodeList &,
                                                    SymbolTable &,
                                                    CompileLog &)
{
    assert (false && "TreeNodeExprFRead::generateBoolCode called.");
    return ICode::E_NOT_IMPLEMENTED;
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

ICode::Status TreeNodeExprBinary::generateCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log,
                                               Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));

    /*
      If first sub-expression is public, then generate short-circuit code for
      logical && and logical ||.
    */
    if (e1->resultType().secrecSecType() == SECTYPE_PUBLIC
        && e1->resultType().isScalar()
        && e2->resultType().isScalar()
        && (type() == NODE_EXPR_BINARY_LAND || type() == NODE_EXPR_BINARY_LOR))
    {
        if (r == 0) {
            generateResultSymbol(st);
        } else {
            assert(r->secrecType().canAssign(resultType()));
            setResult(r);
        }

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
    s = generateSubexprCode(e1, code, st, log);
    if (s != ICode::OK) return s;
    Symbol* e1result = e1->result();

    // Generate code for second child expression:
    s = generateSubexprCode(e2, code, st, log);
    if (s != ICode::OK) return s;
    Symbol* e2result = e2->result();

    // Implicitly convert scalar to array if needed:
    Imop* jmp = 0;
    if (e1->resultType().secrecDimType() > e2->resultType().secrecDimType()) {
        e2result = st.appendTemporary(static_cast<TypeNonVoid const&>(e1->resultType()));
        e2result->inheritShape(e1result);
        Imop* i = new Imop(this, Imop::FILL, e2result, e2->result(), e1result->getSizeSym());
        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i);
    }
    else
    if (e2->resultType().secrecDimType() > e1->resultType().secrecDimType()) {
        e1result = st.appendTemporary(static_cast<TypeNonVoid const&>(e2->resultType()));
        e1result->inheritShape(e2result);
        Imop* i = new Imop(this, Imop::FILL, e1result, e1->result(), e2result->getSizeSym());
        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i);
    }
    else {
        Imop* err = new Imop(this, Imop::ERROR, (Symbol*) 0,
                             (Symbol*) new std::string("Mismaching shapes in addition!"));
        Symbol::dim_iterator
                di = e1result->dim_begin(),
                dj = e2result->dim_begin(),
                de = e1result->dim_end();
        for (; di != de; ++ di, ++ dj) {
            Imop* i = new Imop(this, Imop::JNE, (Symbol*) 0, *di, *dj);
            i->setJumpDest(err);
            patchFirstImop(i);
            prevPatchNextList(i);
            code.push_imop(i);
        }

        jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
        code.push_imop(jmp);
        patchFirstImop(jmp);
        prevPatchNextList(jmp);
        addToNextList(jmp);

        code.push_imop(err);
    }

    // Generate temporary for the result of the binary expression, if needed:
    if (r == 0) {
        generateResultSymbol(st);
        result()->inheritShape(e1result);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(e1result, code);
    }

    // Generate code for binary expression:
    Imop *i;
    Imop::Type cType;
    switch (type()) {
        case NODE_EXPR_BINARY_ADD:  cType = Imop::ADD;  break;
        case NODE_EXPR_BINARY_SUB:  cType = Imop::SUB;  break;
        case NODE_EXPR_BINARY_MUL:  cType = Imop::MUL;  break;
        case NODE_EXPR_BINARY_DIV:  cType = Imop::DIV;  break;
        case NODE_EXPR_BINARY_MOD:  cType = Imop::MOD;  break;
        case NODE_EXPR_BINARY_EQ:   cType = Imop::EQ;   break;
        case NODE_EXPR_BINARY_GE:   cType = Imop::GE;   break;
        case NODE_EXPR_BINARY_GT:   cType = Imop::GT;   break;
        case NODE_EXPR_BINARY_LE:   cType = Imop::LE;   break;
        case NODE_EXPR_BINARY_LT:   cType = Imop::LT;   break;
        case NODE_EXPR_BINARY_NE:   cType = Imop::NE;   break;
        case NODE_EXPR_BINARY_LAND: cType = Imop::LAND; break;
        case NODE_EXPR_BINARY_LOR:  cType = Imop::LOR;  break;
        default:
            log.fatal() << "Binary " << operatorString()
                        << " not yet implemented. At " << location();
            return ICode::E_NOT_IMPLEMENTED;
    }

    if (!resultType().isScalar()) {
        i = new Imop(this, cType, result(), e1result, e2result, result()->getSizeSym());
    }
    else {
        i = new Imop(this, cType, result(), e1result, e2result);
    }

    code.push_imop(i);
    patchFirstImop(i);
    patchNextList(i);
    prevPatchNextList(i);

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
            if (static_cast<const TNV&>(e1->resultType()).secrecSecType()
                    == SECTYPE_PUBLIC)
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

            Imop *tj = new Imop(this, cType, (Symbol*) 0,
                                e1->result(), e2->result());
            code.push_imop(tj);
            addToTrueList(tj);
            patchFirstImop(tj);

            if (e2->firstImop()) {
                e1->patchNextList(e2->firstImop());
                e2->patchNextList(tj);
            }
            else {
                e1->patchNextList(tj);
            }

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
    if (e->checkAndLogIfVoid(log)) return ICode::E_TYPE;
    assert(e->resultType().secrecSecType() == SECTYPE_PUBLIC);
    setResultType(new TypeNonVoid(SECTYPE_PRIVATE, e->resultType().secrecDataType()));
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

    // Generate code for child expression
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the classification, if needed:
    if (r == 0) {
        generateResultSymbol(st);
        result()->inheritShape(e->result());
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(e->result(), code);
    }

    Imop *i = 0;
    if (resultType().isScalar()) {
        i = new Imop(this, Imop::CLASSIFY,
                       result(), e->result());
    }
    else {
        i = new Imop(this, Imop::CLASSIFY,
                       result(), e->result(), result()->getSizeSym());
    }

    code.push_imop(i);
    patchFirstImop(i);
    prevPatchNextList(i);

    return ICode::OK;
}

ICode::Status TreeNodeExprClassify::generateBoolCode(ICodeList &, SymbolTable &,
                                                     CompileLog &)
{
    assert(false &&
           "ICE: TreeNodeExprClassify::generateBoolCode called.");
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
            setResultType(new TypeNonVoid(SECTYPE_PUBLIC, e->resultType().secrecDataType(),
                                                          e->resultType().secrecDimType()));
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

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the declassification, if needed:
    if (r == 0) {
        generateResultSymbol(st);
        result()->inheritShape(e->result());
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(e->result(), code);
    }

    Imop *i = 0;
    if (resultType().isScalar()) {
        i = new Imop(this, Imop::DECLASSIFY,
                     result(), e->result());
    }
    else {
        i = new Imop(this, Imop::DECLASSIFY,
                     result(), e->result(), result()->getSizeSym());
    }

    code.push_imop(i);
    patchFirstImop(i);
    prevPatchNextList(i);

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

        if (need->secType() == SECTYPE_PUBLIC
            && have->secType() == SECTYPE_PRIVATE)
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
        classifyChildAtIfNeeded(i + 1, need->secType());
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

ICode::Status TreeNodeExprProcCall::generateCode(ICodeList &code,
                                                 SymbolTable &st,
                                                 CompileLog &log,
                                                 Symbol *r)
{
    typedef ChildrenListConstIterator CLCI;

    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // generate temporary result, if needed
    if (r == 0) {
        generateResultSymbol(st);
    } else {
        setResult(r);
    }

    std::stack<TreeNodeExpr*> resultList;

    // Initialize arguments
    for (CLCI it(children().begin() + 1); it != children().end(); it++) {
        assert(((*it)->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(*it) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(*it);
        s = generateSubexprCode(e, code, st, log);
        if (s != ICode::OK) return s;

        assert(e->result() != 0);
        resultList.push(e);
    }

    // Add them as arguments in a backward manner:
    while (!resultList.empty()) {
        TreeNodeExpr* e = resultList.top();
        Symbol* sym = e->result();
        Imop *i = 0;
        if (!e->resultType().isScalar()) {
            i = new Imop(this, Imop::PUSH, 0, sym, sym->getSizeSym());
        }
        else {
            i = new Imop(this, Imop::PUSH, 0, sym);
        }

        code.push_imop(i);     
        patchFirstImop(i);
        prevPatchNextList(i);

        // push shape in reverse order
        Symbol::dim_reverese_iterator
                di = sym->dim_rbegin(),
                de = sym->dim_rend();
        for (; di != de; ++ di) {
            Imop* i = new Imop(this, Imop::PUSH, (Symbol*) 0, *di);
            code.push_imop(i);
        }

        resultList.pop();
    }

    // Do function call
    Imop *i = new Imop(this, Imop::CALL, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
    Imop *c = new Imop(this, Imop::RETCLEAN, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
    i->setCallDest(m_procedure, c);
    code.push_imop(i);
    code.push_imop(c);
    patchFirstImop(i);

    // pop shape, recompute size, and pop data
    if (!resultType().isVoid()) {
        Symbol::dim_iterator
                di = result()->dim_begin(),
                de = result()->dim_end();
        for (; di != de; ++ di) {
            Imop* i = new Imop(this, Imop::POP, *di);
            code.push_imop(i);
        }

        computeSize(code, st);
        if (resultType().isScalar())
            i = new Imop(this, Imop::POP, result());
        else
            i = new Imop(this, Imop::POP, result(), result()->getSizeSym());
        code.push_imop(i);
    }

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
        setResult(r);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprInt::generateBoolCode(ICodeList &, SymbolTable &,
                                                CompileLog &)
{
    assert(false &&
           "ICE: TreeNodeExprInt::generateBoolCode called.");
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
    setResultType(new TNV(static_cast<const DTV&>(type->dataType()).dataType())
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

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0));
    Symbol* src = id->getSymbol(st, log);

    if (r == 0) {
        setResult(src);
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(src, code);

        Imop *i = 0;
        if (!resultType().isScalar()) {
            i = new Imop(this, Imop::ASSIGN, r, src, src->getSizeSym());
        }
        else {
            i = new Imop(this, Imop::ASSIGN, r, src);
        }
        code.push_imop(i);
        patchFirstImop(i);
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
    assert(false && "TreeNodeExprString::generateBoolCode called."); // This method shouldn't be called.
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
        //|| cType.secType().kind() != SecType::BASIC
        || static_cast<const DataTypeBasic&>(cType.dataType()).dataType()
            != DATATYPE_BOOL
        || static_cast<const DataTypeBasic&>(cType.dataType()).secType()
            != SECTYPE_PUBLIC)
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

ICode::Status TreeNodeExprTernary::generateCode(ICodeList &code,
                                                SymbolTable &st,
                                                CompileLog &log,
                                                Symbol *r)
{
    // Type check:
    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0));
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1));
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2));

    if (e1->havePublicBoolType()) {

        // Generate temporary for the result of the ternary expression, if needed:
        if (r == 0) {
            generateResultSymbol(st);
        } else {
            assert(r->secrecType().canAssign(resultType()));
            setResult(r);
        }

        // Generate code for boolean expression:
        s = e1->generateBoolCode(code, st, log);
        if (s != ICode::OK) return s;
        setFirstImop(e1->firstImop());

        // Generate code for first value child expression:
        s = e2->generateCode(code, st, log, result());
        if (s != ICode::OK) return s;

        // Jump out of the ternary construct:
        Imop *j = new Imop(this, Imop::JUMP, 0);
        addToNextList(j);
        code.push_imop(j);

        // Generate code for second value child expression:
        s = e3->generateCode(code, st, log, result());
        if (s != ICode::OK) return s;

        // Link boolean expression code to the rest of the code:
        e1->patchTrueList(e2->firstImop());
        e1->patchFalseList(e3->firstImop());

        // Handle next lists of value child expressions:
        addToNextList(e2->nextList());
        addToNextList(e3->nextList());
    }
    else {
        // Evaluate subexpressions:
        s = generateSubexprCode(e1, code, st, log);
        if (s != ICode::OK) return s;

        s = generateSubexprCode(e2, code, st, log);
        if (s != ICode::OK) return s;

        s = generateSubexprCode(e3, code, st, log);
        if (s != ICode::OK) return s;

        // Generate temporary for the result of the ternary expression, if needed:
        if (r == 0) {
            generateResultSymbol(st);
            result()->inheritShape(e1->result());
        } else {
            assert(r->secrecType().canAssign(resultType()));
            setResult(r);
            copyShapeFrom(e1->result(), code);
        }

        // check that shapes match
        Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
        Imop* err = new Imop(this, Imop::ERROR, (Symbol*) 0,
                             (Symbol*) new std::string("Mismatching shapes!"));
        Symbol::dim_iterator
                di = e1->result()->dim_begin(),
                dj = e2->result()->dim_begin(),
                dk = e3->result()->dim_begin(),
                de = e1->result()->dim_end();
        for (; di != de; ++ di, ++ dj, ++ dk) {
            Imop* i = new Imop(this, Imop::JNE, (Symbol*) 0, *di, *dj);
            code.push_imop(i);
            i->setJumpDest(err);
            i = new Imop(this, Imop::JNE, (Symbol*) 0, *dj, *dk);
            code.push_imop(i);
            i->setJumpDest(err);
        }

        prevPatchNextList(jmp);
        code.push_imop(jmp);
        code.push_imop(err);

        // loop to set all values of resulting array
        Symbol* counter = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT, 0));
        Symbol* b = st.appendTemporary(static_cast<TypeNonVoid const&>(e1->resultType()));
        Symbol* t = st.appendTemporary(static_cast<TypeNonVoid const&>(resultType()));

        // r = e1
        Imop* i = new Imop(this, Imop::ASSIGN, result(), e2->result(), result()->getSizeSym());
        code.push_imop(i);
        jmp->setJumpDest(i);

        // counter = 0
        i = new Imop(this, Imop::ASSIGN, counter, st.constantUInt(0));
        code.push_imop(i);

        // L0: if (counter >= size) goto next;
        Imop* jge = new Imop(this, Imop::JGE, (Symbol*) 0, counter, result()->getSizeSym());
        code.push_imop(jge);
        addToNextList(jge);

        // b = e1[counter]
        i = new Imop(this, Imop::LOAD, b, e1->result(), counter);
        code.push_imop(i);

        Imop* t0 = new Imop(this, Imop::ADD, counter, counter, st.constantUInt(1));
        Imop* t1 = new Imop(this, Imop::STORE, result(), counter, t);

        // if b goto T0
        i = new Imop(this, Imop::JT, (Symbol*) 0, b);
        code.push_imop(i);
        i->setJumpDest(t0);

        // t = e3[counter]
        // T1: result[counter] = t
        i = new Imop(this, Imop::LOAD, t, e3->result(), counter);
        code.push_imop(i);
        code.push_imop(t1);

        // T0: counter = counter + 1
        i = new Imop(this, Imop::ADD, counter, counter, st.constantUInt(1));
        code.push_imop(t0);

        // goto L0
        i = new Imop(this, Imop::JUMP, (Symbol*) 0);
        code.push_imop(i);
        i->setJumpDest(jge);
    }

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
        setResult(r);
    } else {
        setResult(sym);
    }
    return ICode::OK;
}

ICode::Status TreeNodeExprUInt::generateBoolCode(ICodeList &, SymbolTable &,
                                                 CompileLog &)
{
    assert(false
           && "ICE: TreeNodeExprUInt::generateBoolCode called.");
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

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        generateResultSymbol(st);
        result()->inheritShape(e->result());
    } else {
        assert(r->secrecType().canAssign(resultType()));
        setResult(r);
        copyShapeFrom(e->result(), code);
    }

    // Generate code for unary expression:
    Imop *i = 0;
    if (resultType().isScalar()) {
        i = new Imop(this,
                     type() == NODE_EXPR_UNEG ? Imop::UNEG : Imop::UMINUS,
                     result(), e->result());
    }
    else {
        i = new Imop(this,
                     type() == NODE_EXPR_UNEG ? Imop::UNEG : Imop::UMINUS,
                     result(), e->result(), result()->getSizeSym());
    }

    code.push_imop(i);
    patchFirstImop(i);
    prevPatchNextList(i);

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
            Imop *i = new Imop(this, Imop::RETURNVOID, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
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
    Imop *retClean = new Imop(this, Imop::RETCLEAN, 0, 0, 0);
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

    assert(children().size() > 0 && children().size() <= 4);
    assert(children().at(0)->type() == NODE_IDENTIFIER);

    assert(dynamic_cast<TNI*>(children().at(0)) != 0);
    return static_cast<TNI*>(children().at(0))->value();
}

/*
 * We have 6 cases depending on wether shape is present and if
 * right hand side is scalar or array. In addition we have to take account that
 * RHS may be missing. The cases are as follows:
 * - type x = scalar;
 * - type x = array;
 * - type x shape = scalar;
 * - type x shape = array;
 * - type x;
 * - type x shape;
 */
ICode::Status TreeNodeStmtDecl::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log)
{
    assert(children().size() > 0 && children().size() <= 4);

    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Initialize the new symbol (for initializer target)
    SymbolSymbol *ns = new SymbolSymbol(resultType());
    ns->setScopeType(m_global ? SymbolSymbol::GLOBAL : SymbolSymbol::LOCAL);
    ns->setName(variableName());

    bool isScalar = resultType().isScalar();
    unsigned n = 0;

    // Initialize temporaries for all variables corresponding to shape
    TypeNonVoid const& dimType = TypeNonVoid(DataTypeVar(DataTypeBasic(SECTYPE_PUBLIC, DATATYPE_INT)));
    for (unsigned i = 0; i < resultType().secrecDimType(); ++ i) {
        SymbolSymbol* sym = new SymbolSymbol(dimType);
        sym->setScopeType(m_global ? SymbolSymbol::GLOBAL : SymbolSymbol::LOCAL);
        std::stringstream ss;
        ss << variableName() << "{d" << i << "}";
        sym->setName(ss.str());
        st.appendSymbol(sym);
        ns->setDim(i, sym);
    }

    { // set size symbol
        SymbolSymbol* sizeSym = new SymbolSymbol(dimType);
        sizeSym->setScopeType(m_global ? SymbolSymbol::GLOBAL : SymbolSymbol::LOCAL);
        std::stringstream ss;
        ss << variableName() << "{size}";
        sizeSym->setName(ss.str());
        st.appendSymbol(sizeSym);
        ns->setSizeSym(sizeSym);
    }

    // evaluate shape if given, also compute size
    if (children().size() > 2) {
        if (!isScalar) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), st.constantInt(1));
            code.push_imop(i);
            patchFirstImop(i);
        }

        TreeNode::ChildrenListConstIterator
                ei = children().at(2)->children().begin(),
                ee = children().at(2)->children().end();
        for (; ei != ee; ++ ei, ++ n) {
            assert (dynamic_cast<TreeNodeExpr*>(*ei) != 0);
            TreeNodeExpr* e = static_cast<TreeNodeExpr*>(*ei);
            s = generateSubexprCode(e, code, st, log, ns->getDim(n));
            if (s != ICode::OK) return s;
            Imop* i = new Imop(this, Imop::MUL, ns->getSizeSym(), ns->getSizeSym(), e->result());
            code.push_imop(i);
            patchFirstImop(i);
            prevPatchNextList(i);
        }
    }
    else {
        if (!isScalar) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), st.constantInt(0));
            code.push_imop(i);
            patchFirstImop(i);
        }

        for (unsigned it = 0; it < resultType().secrecDimType(); ++ it) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns->getDim(it), st.constantInt(0));
            code.push_imop(i);
        }
    }

    if (m_procParam) {
        Imop *i = 0;
        Symbol::dim_iterator
                di = ns->dim_begin(),
                de = ns->dim_end();

        if (!isScalar) {
            i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), st.constantInt(1));
            code.push_imop(i);
            patchFirstImop(i);
        }

        for (; di != de; ++ di) {
            i = new Imop(this, Imop::POP, *di);
            code.push_imop(i);
            i = new Imop(this, Imop::MUL, ns->getSizeSym(), ns->getSizeSym(), *di);
            code.push_imop(i);
        }

        if (isScalar)
            i = new Imop(this, Imop::POP, ns);
        else
            i = new Imop(this, Imop::POP, ns, ns->getSizeSym());
        code.push_imop(i);
        patchFirstImop(i);
    } else if (children().size() > 3) {

        // evaluate rhs
        TreeNode *t = children().at(3);
        assert((t->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(t) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        ICode::Status s = generateSubexprCode(e, code, st, log);
        if (s != ICode::OK) return s;

        // type x = foo;
        if (resultType().secrecDimType() > 0 && n == 0) {
            if (resultType().secrecDimType() > e->resultType().secrecDimType()) {
                Imop* i = new Imop(this, Imop::FILL, ns, e->result(), ns->getSizeSym());
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i);
            }
            else {
                assert (resultType().secrecDimType() == e->resultType().secrecDimType());

                Symbol::dim_iterator
                        di = ns->dim_begin(),
                        dj = e->result()->dim_begin(),
                        de = ns->dim_end();
                for (; di != de; ++ di, ++ dj) {
                    Imop* i = new Imop(this, Imop::ASSIGN, *di, *dj);
                    code.push_imop(i);
                    patchFirstImop(i);
                    prevPatchNextList(i);
                }
                if (!isScalar) {
                    Imop* i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), e->result()->getSizeSym());
                    code.push_imop(i);
                }

                Imop* i = new Imop(this, Imop::ASSIGN, ns, e->result(), e->result()->getSizeSym());
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i);
            }
        }

        // arr x[e1,...,en] = foo;
        if (n > 0 && resultType().secrecDimType() == n) {
            if (resultType().secrecDimType() > e->resultType().secrecDimType()) {
                // fill lhs with constant value
                Imop* i = new Imop(this, Imop::FILL, ns, e->result(), ns->getSizeSym());
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i);
            }
            else {
                // check that shapes match and assign
                Imop* err = new Imop(this, Imop::ERROR, (Symbol*) 0, (Symbol*) new std::string("Shape mismatch!"));
                Symbol::dim_iterator
                        di = e->result()->dim_begin(),
                        dj = ns->dim_begin(),
                        de = e->result()->dim_end();
                for (; di != de; ++ di, ++ dj) {
                    Imop* i = new Imop(this, Imop::JNE, (Symbol*) 0, *di, *dj);
                    i->setJumpDest(err);
                    code.push_imop(i);
                    patchFirstImop(i);
                    prevPatchNextList(i);
                }
                Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
                Imop* i = new Imop(this, Imop::ASSIGN, ns, e->result(), ns->getSizeSym());
                jmp->setJumpDest(i);
                code.push_imop(jmp);
                code.push_imop(err);
                code.push_imop(i);
                patchFirstImop(jmp);
                prevPatchNextList(jmp);
            }  
        }

        // scalar_type x = scalar;
        if (resultType().isScalar()) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns, e->result());
            code.push_imop(i);
            patchFirstImop(i);
            prevPatchNextList(i);
        }

        if (prevSubexpr() != 0) addToNextList(prevSubexpr()->nextList()); // uh oh...
    } else {

        if (!isScalar && n == 0) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), st.constantInt(0));
            code.push_imop(i);
            patchFirstImop(i);

            for (unsigned it = 0; it < resultType().secrecDimType(); ++ it) {
                Imop* i = new Imop(this, Imop::ASSIGN, ns->getDim(it), st.constantInt(0));
                code.push_imop(i);
            }
        }

        Imop *i = 0;
        if (isScalar) {
            i = new Imop(this, Imop::ASSIGN, ns, (Symbol*) 0);
        }
        else {
            i = new Imop(this, Imop::FILL, ns, (Symbol*) 0, (Symbol*) 0);
            if (n == 0) {
                i->setArg2(st.constantUInt(0));
            }
            else {
                i->setArg2(ns->getSizeSym());
            }
        }

        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i);

        typedef DataTypeBasic DTB;
        typedef DataTypeVar DTV;
        assert(m_type->dataType().kind() == DataType::VAR);
        assert(dynamic_cast<const DTV*>(&m_type->dataType()) != 0);
        const DTV &dtv(static_cast<const DTV&>(m_type->dataType()));
        assert(dtv.dataType().kind() == DataType::BASIC);
        assert(dynamic_cast<const DTB*>(&dtv.dataType()) != 0);
        const DTB &dtb(static_cast<const DTB&>(dtv.dataType()));

        Symbol *def = 0;
        switch (dtb.dataType()) {
            case DATATYPE_BOOL: def = st.constantBool(false); break;
            case DATATYPE_INT: def = st.constantInt(0); break;
            case DATATYPE_UINT: def = st.constantUInt(0); break;
            case DATATYPE_STRING: def = st.constantString(""); break;
            default: assert(false); // Shouldn't happen!
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
                || !e->resultType().secrecSecType() == SECTYPE_PUBLIC ) {
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

        if (e->resultType().secrecSecType() == SECTYPE_PRIVATE
            && justType.secrecSecType() == SECTYPE_PUBLIC)
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
  TreeNodeStmtAssert
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
    if (e->firstImop()) {
        setFirstImop(e->firstImop());
    }

    std::ostringstream os;
    os << "assert failed at " << location();
    Imop *i = new Imop(this, Imop::ERROR, 0, (Symbol*) new std::string(os.str()));
    code.push_imop(i);
    patchFirstImop(i);

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
    if (e1 != 0 && body->firstImop() != 0)
        e1->patchTrueList(body->firstImop());
    addToNextList(body->breakList());

    // Iteration expression:
    TreeNodeExpr *e2 = 0;
    if (children().at(2)->type() != NODE_EXPR_NONE) {
        assert(dynamic_cast<TreeNodeExpr*>(c2) != 0);
        e2 = static_cast<TreeNodeExpr*>(c2);
        ICode::Status s = e2->generateCode(code, st, log);
        if (s != ICode::OK) return s;
        if (e1 != 0 && body->firstImop() == 0) e1->patchTrueList(e2->firstImop());
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
        j->setJumpDest(e1->firstImop());
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

        Imop *i = new Imop(this, Imop::RETURNVOID, 0, 0, 0);
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
        s = generateSubexprCode(e, code, st, log);
        if (s != ICode::OK) return s;

        // Push data and then shape
        Imop* i = 0;
        if (e->resultType().isScalar())
            i = new Imop(this, Imop::PUSH, 0, e->result());
        else
            i = new Imop(this, Imop::PUSH, 0, e->result(), e->result()->getSizeSym());
        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i);

        Symbol::dim_reverese_iterator
                di = e->result()->dim_rbegin(),
                de = e->result()->dim_rend();
        for (; di != de; ++ di) {
            i = new Imop(this, Imop::PUSH, 0, *di);
            code.push_imop(i);
        }

        i = new Imop(this, Imop::RETURNVOID, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
        i->setReturnDestFirstImop(containingProcedure()->firstImop());
        code.push_imop(i);
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

    Imop *i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    i->setJumpDest(e->firstImop());

    // Patch jump lists:
    setFirstImop(e->firstImop());
    e->patchTrueList(body->firstImop());
    setNextList(e->falseList());
    addToNextList(body->breakList());
    body->patchContinueList(e->firstImop());
    body->patchNextList(e->firstImop());

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeStmtWhile
*******************************************************************************/

ICode::Status TreeNodeStmtPrint::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log)
{
    assert(children().size() == 1);
    TreeNode* c0 = children().at(0);

    assert(c0->type() != NODE_EXPR_NONE);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);

    // Type check:
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (e->resultType().secrecDataType() != DATATYPE_STRING || e->resultType().secrecSecType() != SECTYPE_PUBLIC   ||
        !e->resultType().isScalar()) {
        log.fatal() << "Argument to print statement has to be public string scalar, got "
                    << e->resultType() << " at " << location();
        return ICode::E_TYPE;
    }

    setResultFlags(TreeNodeStmt::FALLTHRU);

    // Generate code:
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;
    Imop* i = new Imop(this, Imop::PRINT, (Symbol*) 0, e->result());
    code.push_imop(i);
    patchFirstImop(i);
    e->patchNextList(i);

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
