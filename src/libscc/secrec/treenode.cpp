#include "secrec/treenode.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include "secrec/symboltable.h"
#include "misc.h"

namespace SecreC {

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
        case NODE_FUNDEF_PARAM: return "FUNDEF_PARAM";
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
        case NODE_EXPR_ASSIGN_MUL: /* Fall through: */
        case NODE_EXPR_ASSIGN_DIV: /* Fall through: */
        case NODE_EXPR_ASSIGN_MOD: /* Fall through: */
        case NODE_EXPR_ASSIGN_ADD: /* Fall through: */
        case NODE_EXPR_ASSIGN_SUB: /* Fall through: */
        case NODE_EXPR_ASSIGN:
            return (TreeNode*) (new SecreC::TreeNodeExprAssign(type, *loc));
        case NODE_EXPR_RVARIABLE:
            return (TreeNode*) (new SecreC::TreeNodeExprRVariable(*loc));
        case NODE_EXPR_TERNIF:
            return (TreeNode*) (new SecreC::TreeNodeExprTernary(*loc));
        case NODE_STMT_EXPR:
            return (TreeNode*) (new SecreC::TreeNodeStmtExpr(*loc));
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

extern "C" struct TreeNode *treenode_init_secTypeF(
        enum SecrecSecType secType,
        YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeSecTypeF(secType, *loc);
}

extern "C" struct TreeNode *treenode_init_dataTypeF(
        enum SecrecVarType varType,
        YYLTYPE *loc)
{
    return (TreeNode*) new SecreC::TreeNodeDataTypeF(varType, *loc);
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

/******************************************************************
  TreeNodeBool
******************************************************************/

std::string TreeNodeBool::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"bool:" << stringHelper() << "\"";
    return os.str();
}


/******************************************************************
  TreeNodeCompound
******************************************************************/

ICode::Status TreeNodeCompound::generateCode(ICode::CodeList &code,
                                             SymbolTable &st,
                                             std::ostream &es)
{
    es << "TODO TreeNodeCompound::generateCode()" << std::endl;
    return ICode::E_NOT_IMPLEMENTED;
}

/******************************************************************
  TreeNodeDataTypeArray
******************************************************************/

const DataType &TreeNodeDataTypeArray::dataType() const {
    typedef TreeNodeDataType TNDT;

    assert(children().size() == 1);
    if (m_cachedType != 0) return *m_cachedType;

    assert(dynamic_cast<TNDT*>(children().at(0).data()) != 0);
    TNDT *t = static_cast<TNDT*>(children().at(0).data());

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


/******************************************************************
  TreeNodeDataTypeF
******************************************************************/

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

/******************************************************************
  TreeNodeDecl
******************************************************************/

ICode::Status TreeNodeDecl::generateCode(ICode::CodeList &code,
                                         SymbolTable &st,
                                         std::ostream &es)
{
    typedef TreeNodeIdentifier TNI;
    typedef TreeNodeType       TNT;

    assert(children().size() > 0 && children().size() <= 3);
    assert(children().at(0).data()->type() == NODE_IDENTIFIER);

    assert(dynamic_cast<TNI*>(children().at(0).data()) != 0);
    TNI *id   = static_cast<TNI*>(children().at(0).data());
    assert(dynamic_cast<TNT*>(children().at(1).data()) != 0);
    TNT *type = static_cast<TNT*>(children().at(1).data());

    /// \note Check here for overrides first if new symbol table is needed.
    SymbolSymbol *s = new SymbolSymbol(type->secrecType(), this);
    s->setName(id->value());
    st.appendSymbol(s);

    if (children().size() >= 2) {
        TreeNode *t = children().at(2).data();
        assert((t->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(t) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        ICode::Status s = e->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}


/******************************************************************
  TreeNodeExpr
******************************************************************/

namespace {

void patchList(std::vector<SecreC::Imop*> &list,
               SecreC::Symbol *dest)
{
    typedef std::vector<SecreC::Imop*>::const_iterator IVCI;
    for (IVCI it(list.begin()); it != list.end(); it++) {
        (*it)->setDest(dest);
    }
    list.clear();
}

} // anonymous namespace

void TreeNodeExpr::patchTrueList(Symbol *dest) {
    patchList(m_trueList, dest);
}

void TreeNodeExpr::patchFalseList(Symbol *dest) {
    patchList(m_falseList, dest);
}

void TreeNodeExpr::patchNextList(Symbol *dest) {
    patchList(m_nextList, dest);
}


/******************************************************************
  TreeNodeExpAssign
******************************************************************/

ICode::Status TreeNodeExprAssign::calculateResultType(SymbolTable &st,
                                                      std::ostream &es)
{
    assert(children().size() == 2);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

    if (resultType() != 0) {
        assert(*resultType() != 0);
        return ICode::OK;
    }

    resultType() = new (SecreC::Type*);

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0).data()) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0).data());
    SymbolSymbol *target = id->getSymbol(st, es);
    if (target == 0) return ICode::E_OTHER;

    assert(dynamic_cast<TreeNodeExpr*>(children().at(1).data()) != 0);
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    ICode::Status s = e2->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    const SecreC::Type &targetType = target->secrecType();
    assert(targetType.isVoid() == false);
    const SecreC::Type *eType2 = const_cast<const TreeNodeExpr*>(e2)->resultType();

    /// \todo implement more expressions
    assert(dynamic_cast<const TypeNonVoid*>(&targetType) != 0);
    if (static_cast<const TypeNonVoid*>(&targetType)->kind() == TypeNonVoid::BASIC
        && targetType == *eType2)
    {
        *resultType() = targetType.clone();
        es << "This kind of assignment operation is not yet supported. At "
           << location() << std::endl;
        return ICode::E_NOT_IMPLEMENTED;
    }

    /// \todo Provide better error messages
    es << "Invalid binary operation at " << location() << std::endl;

    *resultType() = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprAssign::generateCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es,
                                               SymbolWithValue *r)
{
    assert(result() == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        SecreC::Type *rt = *resultType();
        result() = st.appendTemporary(*rt);
    } else {
        assert(r->secrecType() == **resultType());
        result() = r;
    }

    // Generate code for child expressions:
    assert(dynamic_cast<TreeNodeExpr*>(children().at(0).data()) != 0);
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    assert(dynamic_cast<TreeNodeExpr*>(children().at(1).data()) != 0);
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(e2);
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for binary expression:
    Imop *i;
    /// \todo implement assignment
    switch (type()) {
        case NODE_EXPR_ASSIGN:     /* Fall through: */
        case NODE_EXPR_ASSIGN_MUL: /* Fall through: */
        case NODE_EXPR_ASSIGN_DIV: /* Fall through: */
        case NODE_EXPR_ASSIGN_MOD: /* Fall through: */
        case NODE_EXPR_ASSIGN_ADD: /* Fall through: */
        case NODE_EXPR_ASSIGN_SUB: /* Fall through: */
        default:
            /// \todo Write better error message
            es << "Assignement is not yet implemented. At " << location()
               << std::endl;
            return ICode::E_NOT_IMPLEMENTED;
    }

    i->setDest(result());
    i->setArg1(const_cast<const TreeNodeExpr*>(e1)->result());
    i->setArg1(const_cast<const TreeNodeExpr*>(e2)->result());
    code.push_back(i);

    // Patch next lists of child expressions:
    e1->patchNextList(st.label(ce2->firstImop()));
    e2->patchNextList(st.label(i));

    return ICode::OK;
}

ICode::Status TreeNodeExprAssign::generateBoolCode(ICode::CodeList &code, SymbolTable &st,
                                                   std::ostream &es)
{
    /// \todo Write assertion about return type

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    /// \todo Implement

    return ICode::E_NOT_IMPLEMENTED;
}


/******************************************************************
  TreeNodeExprBinary
******************************************************************/

ICode::Status TreeNodeExprBinary::calculateResultType(SymbolTable &st,
                                                      std::ostream &es)
{
    assert(children().size() == 2);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

    if (resultType() != 0) {
        assert(*resultType() != 0);
        return ICode::OK;
    }

    resultType() = new (SecreC::Type*);

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e1->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
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
        SecrecVarType d1 = static_cast<const DataTypeBasic*>(&et1->dataType())->varType();
        SecrecSecType s1 = static_cast<const SecTypeBasic*>(&et1->secType())->secType();
        SecrecVarType d2 = static_cast<const DataTypeBasic*>(&et2->dataType())->varType();
        SecrecSecType s2 = static_cast<const SecTypeBasic*>(&et2->secType())->secType();

        switch (type()) {
            case NODE_EXPR_ADD:
                if (d1 != d2) break;
                if ((d1 & (VARTYPE_INT|VARTYPE_UINT|VARTYPE_STRING)) != 0x0) break;
                *resultType() = new SecreC::TypeNonVoid(upperSecType(s1, s2), d1);
                return ICode::OK;
            case NODE_EXPR_SUB:
            case NODE_EXPR_MUL:
            case NODE_EXPR_MOD:
            case NODE_EXPR_DIV:
                if (d1 != d2) break;
                if ((d1 & (VARTYPE_INT|VARTYPE_UINT)) != 0x0) break;
                *resultType() = new SecreC::TypeNonVoid(upperSecType(s1, s2), d1);
                return ICode::OK;

            default:
                *resultType() = 0;
                /// \todo Write better error message
                es << "This kind of binary operation is not yet supported. At "
                   << location() << std::endl;
                return ICode::E_NOT_IMPLEMENTED;
        }
    }

    /// \todo Write better error message
    es << "Invalid binary operation at " << location() << std::endl;

    *resultType() = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprBinary::generateCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es,
                                               SymbolWithValue *r)
{
    assert(result() == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        SecreC::Type *rt = *resultType();
        result() = st.appendTemporary(*rt);
    } else {
        assert(r->secrecType() == **resultType());
        result() = r;
    }

    // Generate code for child expressions:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    const TreeNodeExpr *ce1 = const_cast<const TreeNodeExpr*>(e1);
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    firstImop() = ce1->firstImop();

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(e2);
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;

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
    i->setArg1(static_cast<const TreeNodeExpr*>(e2)->result());
    code.push_back(i);

    // Patch next lists of child expressions:
    e1->patchNextList(st.label(ce2->firstImop()));
    e2->patchNextList(st.label(i));

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
        TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
        const TreeNodeExpr *ce1 = const_cast<const TreeNodeExpr*>(e1);
        s = e1->generateBoolCode(code, st, es);
        if (s != ICode::OK) return s;
        firstImop() = static_cast<const TreeNodeExpr*>(e1)->firstImop();

        TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
        const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(e2);
        s = e2->generateBoolCode(code, st, es);
        if (s != ICode::OK) return s;

        // Short circuit the code:
        if (type() == NODE_EXPR_LAND) {
            e1->patchTrueList(st.label(ce2->firstImop()));
            falseList() = ce1->falseList();

            trueList() = ce2->trueList();
            falseList().insert(falseList().begin(),
                               ce2->falseList().begin(),
                               ce2->falseList().end());
        } else {
            assert(type() == NODE_EXPR_LOR);

            e1->patchFalseList(st.label(ce2->firstImop()));
            trueList() = ce1->trueList();

            falseList() = ce2->falseList();
            trueList().insert(trueList().begin(),
                               ce2->trueList().begin(),
                               ce2->trueList().end());
        }

        return ICode::OK;
    }

    // Generate code for child expressions:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    const TreeNodeExpr *ce1 = const_cast<const TreeNodeExpr*>(e1);
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    firstImop() = static_cast<const TreeNodeExpr*>(e1)->firstImop();

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(e2);
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    Imop *tj;
    e1->patchNextList(st.label(ce2->firstImop()));
    e2->patchNextList(st.label(tj));

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

    tj->setArg1(ce1->result());
    tj->setArg2(ce2->result());
    trueList().push_back(tj);
    code.push_back(tj);

    Imop *fj = new Imop(Imop::JUMP);
    falseList().push_back(fj);
    code.push_back(fj);
    return ICode::OK;
}


/******************************************************************
  TreeNodeExprRVariable
******************************************************************/

ICode::Status TreeNodeExprRVariable::calculateResultType(SymbolTable &st,
                                                         std::ostream &es)
{
    assert(children().size() == 1);
    assert(children().at(0)->type() == NODE_IDENTIFIER);

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0).data()) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0).data());
    SymbolSymbol *s = id->getSymbol(st, es);
    if (s == 0) return ICode::E_OTHER;

    *resultType() = s->secrecType().clone();
    return ICode::OK;
}

ICode::Status TreeNodeExprRVariable::generateCode(ICode::CodeList &code,
                                                  SymbolTable &st,
                                                  std::ostream &es,
                                                  SymbolWithValue *r)
{
    assert(result() == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0).data()) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0).data());

    if (r == 0) {
        result() = id->getSymbol(st, es);
    } else {
        assert(r->secrecType() == **resultType());
        result() = r;

        Imop *i = new Imop(Imop::ASSIGN, r, id->getSymbol(st, es));
        code.push_back(i);
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

    assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0).data()) != 0);
    TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0).data());

    Imop *i = new Imop(Imop::JT, 0, id->getSymbol(st, es));
    code.push_back(i);
    trueList().push_back(i);

    i = new Imop(Imop::JUMP, 0);
    code.push_back(i);
    falseList().push_back(i);

    return ICode::OK;
}


/******************************************************************
  TreeNodeExprTernary
******************************************************************/

ICode::Status TreeNodeExprTernary::calculateResultType(SymbolTable &st,
                                                       std::ostream &es)
{
    assert(children().size() == 3);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(2)->type() & NODE_EXPR_MASK) != 0x0);

    if (resultType() != 0) {
        assert(*resultType() != 0);
        return ICode::OK;
    }

    resultType() = new (SecreC::Type*);

    assert(dynamic_cast<TreeNodeExpr*>(children().at(0).data()) != 0);
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e1->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    assert(dynamic_cast<TreeNodeExpr*>(children().at(1).data()) != 0);
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    s = e2->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    assert(dynamic_cast<TreeNodeExpr*>(children().at(2).data()) != 0);
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    s = e3->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    const SecreC::Type *eType1 = const_cast<const TreeNodeExpr*>(e1)->resultType();
    const SecreC::Type *eType2 = const_cast<const TreeNodeExpr*>(e2)->resultType();
    const SecreC::Type *eType3 = const_cast<const TreeNodeExpr*>(e3)->resultType();

    if (!eType1->isVoid()) {
        assert(dynamic_cast<const TypeNonVoid*>(eType1) != 0);
        const TypeNonVoid *cType = static_cast<const TypeNonVoid*>(eType1);

        if (cType->kind() == TypeNonVoid::BASIC
            && cType->dataType().kind() == DataType::BASIC
            && cType->secType().kind() == SecType::BASIC
            && static_cast<const SecreC::DataTypeBasic&>(cType->dataType()).varType() == VARTYPE_BOOL
            && static_cast<const SecreC::SecTypeBasic&>(cType->secType()).secType()== SECTYPE_PUBLIC
            && *eType2 == *eType3)
        {
            *resultType() = eType2->clone();
            return ICode::OK;
        }
    }

    /// \todo Provide better error messages
    es << "Invalid ternary operation at " << location() << std::endl;

    *resultType() = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprTernary::generateCode(ICode::CodeList &code,
                                                SymbolTable &st,
                                                std::ostream &es,
                                                SymbolWithValue *r)
{
    assert(result() == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        SecreC::Type *rt = *resultType();
        result() = st.appendTemporary(*rt);
    } else {
        assert(r->secrecType() == **resultType());
        result() = r;
    }

    // Generate code for boolean expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for first value child expression:
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(e2);
    s = e2->generateCode(code, st, es, result());
    if (s != ICode::OK) return s;

    // Jump out of the ternary construct:
    Imop *j = new Imop(Imop::JUMP);
    nextList().push_back(j);
    code.push_back(j);

    // Generate code for second value child expression:
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    const TreeNodeExpr *ce3 = const_cast<const TreeNodeExpr*>(e3);
    s = e3->generateCode(code, st, es, result());
    if (s != ICode::OK) return s;

    // Link boolean expression code to the rest of the code:
    e1->patchTrueList(st.label(ce2->firstImop()));
    e1->patchFalseList(st.label(ce3->firstImop()));

    // Handle next lists of value child expressions:
    nextList().insert(nextList().begin(),
                      ce2->nextList().begin(),
                      ce2->nextList().end());
    nextList().insert(nextList().begin(),
                      ce3->nextList().begin(),
                      ce3->nextList().end());

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
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for first value child expression:
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(e2);
    s = e2->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for second value child expression:
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    const TreeNodeExpr *ce3 = const_cast<const TreeNodeExpr*>(e3);
    s = e3->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Link conditional expression code to the rest of the code:
    e1->patchTrueList(st.label(ce2->firstImop()));
    e1->patchFalseList(st.label(ce3->firstImop()));

    trueList().insert(trueList().begin(), ce2->trueList().begin(),
                                          ce2->trueList().end());
    trueList().insert(trueList().begin(), ce3->trueList().begin(),
                                          ce3->trueList().end());
    falseList().insert(falseList().begin(), ce2->falseList().begin(),
                                            ce2->falseList().end());
    falseList().insert(falseList().begin(), ce3->falseList().begin(),
                                            ce3->falseList().end());

    return ICode::OK;
}


/******************************************************************
  TreeNodeExprUnary
******************************************************************/

ICode::Status TreeNodeExprUnary::calculateResultType(SymbolTable &st,
                                                     std::ostream &es)
{
    assert(type() == NODE_EXPR_UMINUS || type() == NODE_EXPR_UNEG);
    assert(children().size() == 1);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);

    if (resultType() != 0) {
        assert(*resultType() != 0);
        return ICode::OK;
    }

    resultType() = new (SecreC::Type*);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
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
        if (type() == NODE_EXPR_UNEG && bType.varType() == VARTYPE_BOOL) {
            *resultType() = et->clone();
            return ICode::OK;
        } else if (type() == NODE_EXPR_UMINUS) {
            if (bType.varType() == VARTYPE_INT) {
                *resultType() = et->clone();
                return ICode::OK;
            }
        }
    }

    es << "Invalid expression of type (" << *eType << ") given to unary "
       << (type() == NODE_EXPR_UNEG ? "negation" : "minus")
       << "operator at " << location() << std::endl;

    *resultType() = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprUnary::generateCode(ICode::CodeList &code,
                                              SymbolTable &st,
                                              std::ostream &es,
                                              SymbolWithValue *r)
{
    assert(result() == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        SecreC::Type *rt = *resultType();
        result() = st.appendTemporary(*rt);
    } else {
        assert(r->secrecType() == **resultType());
        result() = r;
    }

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for unary expression:
    /// \todo implement for matrixes also
    Imop *i = new Imop(type() == NODE_EXPR_UNEG ? Imop::UNEG : Imop::UMINUS);
    i->setDest(result());
    i->setArg1(static_cast<const TreeNodeExpr*>(e)->result());
    code.push_back(i);

    // Patch next list of child expression:
    e->patchNextList(st.label(i));

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
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    const TreeNodeExpr *ce = const_cast<const TreeNodeExpr*>(e);
    s = e->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    falseList() = ce->trueList();
    trueList() = ce->falseList();
    firstImop() = ce->firstImop();
    return ICode::OK;
}


/******************************************************************
  TreeNodeFundef
******************************************************************/

ICode::Status TreeNodeFundef::generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es)
{
    es << "TODO TreeNodeFundef::generateCode: Add function to symbol table" << std::endl;

    std::cout << typeName(children().at(2)->type()) << std::endl;
    /// \bug void main() { public int a; a = 42; } : dynamic cast fails
    assert(dynamic_cast<TreeNodeCodeable*>(children().at(2).data()) != 0);
    TreeNodeCodeable *body = static_cast<TreeNodeCodeable*>(children().at(2).data());
    ICode::Status s = body->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    return ICode::OK;
}

/******************************************************************
  TreeNodeFundefs
******************************************************************/

ICode::Status TreeNodeFundefs::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    typedef ChildrenListConstIterator CLCI;

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_FUNDEF);
        assert(dynamic_cast<TreeNodeFundef*>((*it).data()) != 0);
        TreeNodeFundef *fundef = static_cast<TreeNodeFundef*>((*it).data());
        ICode::Status s = fundef->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}

/******************************************************************
  TreeNodeGlobals
******************************************************************/

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


/******************************************************************
  TreeNodeIdentifier
******************************************************************/

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


/******************************************************************
  TreeNodeInt
******************************************************************/

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


/******************************************************************
  TreeNodeProgram
******************************************************************/

ICode::Status TreeNodeProgram::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    Imop *mainCall = new Imop(Imop::FUNCALL, 0, 0);
    code.push_back(mainCall);
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


/******************************************************************
  TreeNodeSecTypeF
******************************************************************/

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


/******************************************************************
  TreeNodeStmtExpr
******************************************************************/

ICode::Status TreeNodeStmtExpr::generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es)
{
    assert(children().size() == 1);
    assert(dynamic_cast<TreeNodeExpr*>(children().at(0).data()) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    return ICode::OK;
}

/******************************************************************
  TreeNodeString
******************************************************************/

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


/******************************************************************
  TreeNodeTypeType
******************************************************************/

const SecreC::Type &TreeNodeTypeType::secrecType() const {
    typedef TreeNodeDataType TNDT;
    typedef TreeNodeSecTypeF TNST;

    assert(children().size() == 2);
    if (m_cachedType != 0) return *m_cachedType;

    assert(dynamic_cast<TNST*>(children().at(0).data()) != 0);
    TNST *st = static_cast<TNST*>(children().at(0).data());
    assert(dynamic_cast<TNDT*>(children().at(1).data()) != 0);
    TNDT *dt = static_cast<TNDT*>(children().at(1).data());

    m_cachedType = new SecreC::TypeNonVoid(st->secType(), dt->dataType());
    return *m_cachedType;
}

std::string TreeNodeTypeType::stringHelper() const {
    return secrecType().toString();
}


/******************************************************************
  TreeNodeUInt
******************************************************************/

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

} // namespace SecreC
