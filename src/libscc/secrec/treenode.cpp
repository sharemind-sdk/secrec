#include "secrec/treenode.h"

#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include "secrec/symboltable.h"

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
        case NODE_EXPR_LVARIABLE: return "LVARIABLE";
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
  Class TreeNodeProgram
*******************************************************************************/

ICode::Status TreeNodeProgram::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    if (children().size() >= 1) {
        assert(children().size() < 3);

        TreeNode *child = children().at(0);
        if (children().size() >= 2) {
            assert(child->type() == NODE_GLOBALS);

            // Handle global declarations:
            TreeNodeGlobals *t = static_cast<TreeNodeGlobals*>(child);
            ICode::Status s = t->generateCode(code, st, es);
            if (s != ICode::OK) return s;

            child = children().at(1);
        }

        // Handle functions:
        assert(child->type() == NODE_FUNDEFS);
        TreeNodeFundefs *t = static_cast<TreeNodeFundefs*>(child);
        ICode::Status s = t->generateCode(code, st, es);
        if (s != ICode::OK) return s;

        // Handle calling main():
        /// \todo
        return s;
    } else {
        return ICode::E_EMPTY_PROGRAM;
    }
}


/*******************************************************************************
  Class TreeNodeGlobals
*******************************************************************************/

ICode::Status TreeNodeGlobals::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    typedef ChildrenListConstIterator CLCI;

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_DECL);
        TreeNodeDecl *decl = static_cast<TreeNodeDecl*>((*it).data());
        ICode::Status s = decl->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}


/*******************************************************************************
  Class TreeNodeDecl
*******************************************************************************/

ICode::Status TreeNodeDecl::generateCode(ICode::CodeList &code,
                                         SymbolTable &st,
                                         std::ostream &es)
{
    typedef TreeNodeIdentifier TNI;
    typedef TreeNodeType       TNT;

    assert(children().size() > 0 && children().size() <= 3);
    assert(children().at(0).data()->type() == NODE_IDENTIFIER);
    assert((children().at(1).data()->type() & NODE_TYPE_MASK) != 0x0);
    TNI *id   = static_cast<TNI*>(children().at(0).data());
    TNT *type = static_cast<TNT*>(children().at(1).data());

    /// \note Check here for overrides first if new symbol table is needed.
    SymbolSymbol *s = new SymbolSymbol(type->secrecType(), this);
    s->setName(id->value());
    st.appendSymbol(s);

    if (children().size() >= 2) {
        TreeNode *t = children().at(2).data();
        assert((t->type() & NODE_EXPR_MASK) != 0x0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        ICode::Status s = e->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}


/*******************************************************************************
  Class TreeNodeFundefs
*******************************************************************************/

ICode::Status TreeNodeFundefs::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    typedef ChildrenListConstIterator CLCI;

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_FUNDEF);
        TreeNodeFundef *fundef = static_cast<TreeNodeFundef*>((*it).data());
        ICode::Status s = fundef->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}


/*******************************************************************************
  Class TreeNodeExprUnary
*******************************************************************************/

ICode::Status TreeNodeExprUnary::calculateResultType(SymbolTable &st,
                                                     std::ostream &es)
{
    assert(type() == NODE_EXPR_UMINUS || type() == NODE_EXPR_UNEG);
    assert(children().size() == 1);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);

    if (m_resultType != 0) {
        assert(*m_resultType != 0);
        return ICode::OK;
    }

    m_resultType = new (const SecreC::Type*);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e->calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *eType = e->resultType();

    /// \todo implement for matrixes also
    if (eType->kind() == SecreC::Type::Basic) {
        const BasicType *bType = static_cast<const BasicType*>(eType);
        if (type() == NODE_EXPR_UNEG && bType->varType() == VARTYPE_BOOL) {
            *m_resultType = bType->clone();
            return ICode::OK;
        } else if (type() == NODE_EXPR_UMINUS) {
            if (bType->varType() == VARTYPE_INT) {
                *m_resultType = bType->clone();
                return ICode::OK;
            }
        }
    }

    es << "Invalid expression of type (" << *eType << ") given to unary "
       << (type() == NODE_EXPR_UNEG ? "negation" : "minus")
       << "operator at " << location() << std::endl;

    *m_resultType = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprUnary::generateCode(ICode::CodeList &code,
                                              SymbolTable &st,
                                              std::ostream &es)
{
    assert(m_result == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *rt = resultType();

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression:
    m_result = st.appendTemporary(*rt);

    // Generate code for unary expression:
    /// \todo implement for matrixes also
    Imop *i = new Imop(type() == NODE_EXPR_UNEG ? Imop::UNEG : Imop::UMINUS);
    i->setDest(m_result);
    i->setArg1(e->result());
    code.push_back(i);

    return ICode::OK;
}


/*******************************************************************************
  Class TreeNodeExprBinary
*******************************************************************************/

ICode::Status TreeNodeExprBinary::calculateResultType(SymbolTable &st,
                                                      std::ostream &es)
{
    assert(children().size() == 2);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

    if (m_resultType != 0) {
        assert(*m_resultType != 0);
        return ICode::OK;
    }

    m_resultType = new (const SecreC::Type*);

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e1->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    s = e2->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    const SecreC::Type *eType1 = e1->resultType();
    const SecreC::Type *eType2 = e2->resultType();

    /// \todo implement more expressions
    if (eType1->kind() == SecreC::Type::Basic) {
        SecrecVarType v1 = static_cast<const SecreC::BasicType*>(eType1)->varType();
        SecrecSecType s1 = static_cast<const SecreC::BasicType*>(eType1)->secType();
        SecrecVarType v2 = static_cast<const SecreC::BasicType*>(eType2)->varType();
        SecrecSecType s2 = static_cast<const SecreC::BasicType*>(eType2)->secType();

        switch (type()) {
            case NODE_EXPR_ADD:
                if (v1 != v2) break;
                if ((v1 & (VARTYPE_INT|VARTYPE_UINT|VARTYPE_STRING)) != 0x0) break;
                *m_resultType = new SecreC::BasicType(upperSecType(s1, s2), v1);
                return ICode::OK;
            case NODE_EXPR_SUB:
            case NODE_EXPR_MUL:
            case NODE_EXPR_MOD:
            case NODE_EXPR_DIV:
                if (v1 != v2) break;
                if ((v1 & (VARTYPE_INT|VARTYPE_UINT)) != 0x0) break;
                *m_resultType = new SecreC::BasicType(upperSecType(s1, s2), v1);
                return ICode::OK;

            default:
                *m_resultType = 0;
                /// \todo Write better error message
                es << "This kind of binary operation is not yet supported. At "
                   << location() << std::endl;
                return ICode::E_NOT_IMPLEMENTED;
        }
    }

    /// \todo Write better error message
    es << "Invalid binary operation at " << location() << std::endl;

    *m_resultType = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprBinary::generateCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es)
{
    assert(m_result == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *rt = resultType();

    // Generate code for child expressions:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression:
    m_result = st.appendTemporary(*rt);

    // Generate code for binary expression:
    Imop *i;
    /// \todo implement more expressions
    switch (type()) {
        case NODE_EXPR_ADD: i = new Imop(Imop::ADD); break;
        case NODE_EXPR_SUB: i = new Imop(Imop::SUB); break;
        case NODE_EXPR_MUL: i = new Imop(Imop::MUL); break;
        case NODE_EXPR_DIV: i = new Imop(Imop::DIV); break;
        case NODE_EXPR_MOD: i = new Imop(Imop::MOD); break;
        default:
            /// \todo Write better error message
            es << "Binary is not yet implemented. At " << location()
               << std::endl;
            return ICode::E_NOT_IMPLEMENTED;
    }

    i->setDest(m_result);
    i->setArg1(e1->result());
    i->setArg1(e2->result());
    code.push_back(i);

    return ICode::OK;
}

/*******************************************************************************
  Class TreeNodeExprTernary
*******************************************************************************/

ICode::Status TreeNodeExprTernary::calculateResultType(SymbolTable &st,
                                                       std::ostream &es)
{
    assert(children().size() == 3);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(2)->type() & NODE_EXPR_MASK) != 0x0);

    if (m_resultType != 0) {
        assert(*m_resultType != 0);
        return ICode::OK;
    }

    m_resultType = new (const SecreC::Type*);

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e1->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    s = e2->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    s = e3->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    const SecreC::Type *eType1 = e1->resultType();
    const SecreC::Type *eType2 = e2->resultType();
    const SecreC::Type *eType3 = e3->resultType();

    if (eType1->kind() == SecreC::Type::Basic
        && static_cast<const SecreC::BasicType*>(eType1)->varType() == VARTYPE_BOOL
        && static_cast<const SecreC::BasicType*>(eType1)->secType() == SECTYPE_PUBLIC
        && *eType2 == *eType3)
    {
        *m_resultType = eType2->clone();
        return ICode::OK;
    }

    /// \todo Write better error message
    es << "Invalid ternary operation at " << location() << std::endl;

    *m_resultType = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprTernary::generateCode(ICode::CodeList &code,
                                                SymbolTable &st,
                                                std::ostream &es)
{
    assert(m_result == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *rt = resultType();

    // Generate code for child expressions:
    /// \todo Evaluate only one side!!!

    return ICode::E_NOT_IMPLEMENTED;

    /*

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    s = e3->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression:
    m_result = st.appendTemporary(*rt);

    // Generate code for binary expression:
    Imop *i = new Imop();

    i->setDest(m_result);
    i->setArg1(e1->result());
    i->setArg1(e2->result());
    code.push_back(i);

    return ICode::OK;
    */
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

std::string TreeNodeTypeBasic::stringHelper() const {
    std::ostringstream os;
    os << "\"" << m_type.toString() << "\"";
    return os.str();
}

std::string TreeNodeTypeBasic::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"basic:" << m_type.toString() << "\"";
    return os.str();
}


/*******************************************************************************
  Class TreeNodeArrayType
*******************************************************************************/

const SecreC::Type &TreeNodeTypeArray::secrecType() const {
    typedef TreeNodeType TNT;

    assert(children().size() == 1);
    if (m_cachedType != 0) return *m_cachedType;

    assert(dynamic_cast<TNT*>(children().at(0).data()) != 0);
    TNT *t = static_cast<TNT*>(children().at(0).data());

    m_cachedType = new SecreC::ArrayType(t->secrecType(), m_value);
    return *m_cachedType;
}

std::string TreeNodeTypeArray::stringHelper() const {
    std::ostringstream os;
    os << m_value;
    return os.str();
}

std::string TreeNodeTypeArray::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"array:" << m_value << "\"";
    return os.str();
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
        case NODE_FUNDEFS:
            return (TreeNode*) (new SecreC::TreeNodeFundefs(*loc));
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
        case NODE_EXPR_ASSIGN_MUL: /* Fall through: */
        case NODE_EXPR_ASSIGN_DIV: /* Fall through: */
        case NODE_EXPR_ASSIGN_MOD: /* Fall through: */
        case NODE_EXPR_ASSIGN_ADD: /* Fall through: */
        case NODE_EXPR_ASSIGN_SUB: /* Fall through: */
        case NODE_EXPR_ASSIGN:
            return (TreeNode*) (new SecreC::TreeNodeExprBinary(type, *loc));
        case NODE_EXPR_TERNIF:
            return (TreeNode*) (new SecreC::TreeNodeExprTernary(*loc));
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
    return (TreeNode*) ((const SecreC::TreeNode*) node)->children().at(index).data();
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

    return (TreeNode*) new SecreC::TreeNodeBool(value, *loc);
}

extern "C" struct TreeNode *treenode_init_int(int value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeInt(value, *loc);
}

extern "C" struct TreeNode *treenode_init_uint(unsigned value, YYLTYPE *loc) {
    return (TreeNode*) new SecreC::TreeNodeUInt(value, *loc);
}

extern "C" struct TreeNode *treenode_init_string(const char *value,
                                                 YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeString(value, *loc);
}

extern "C" struct TreeNode *treenode_init_identifier(const char *value,
                                                     YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeIdentifier(value, *loc);
}

extern "C" struct TreeNode *treenode_init_basictype(enum SecrecSecType secType,
                                                    enum SecrecVarType varType,
                                                    YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeTypeBasic(secType, varType, *loc);
}

extern "C" struct TreeNode *treenode_init_arraytype(unsigned value,
                                                    YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeTypeArray(value, *loc);
}

std::ostream &operator<<(std::ostream &out, const YYLTYPE &loc) {
    /// \todo filename
    out << "(" << loc.first_line << "," << loc.first_column
        << ")(" << loc.last_line << "," << loc.last_column << ")";
    return out;
}
