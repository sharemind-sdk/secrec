/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "TreeNode.h"

#include "CodeGenResult.h"
#include "Context.h"
#include "DataType.h"
#include "Misc.h"
#include "SecurityType.h"
#include "StringTable.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNodeC.h"
#include "TypeArgument.h"
#include "TypeChecker.h"
#include "Types.h"

#include <algorithm>
#include <iostream>
#include <sharemind/abort.h>
#include <sstream>


namespace /* anonymous */ {

TreeNode * toC(SecreC::TreeNode * node) noexcept
{ return reinterpret_cast<TreeNode *>(node); }

SecreC::TreeNode * toCxx(TreeNode * node) noexcept
{ return reinterpret_cast<SecreC::TreeNode *>(node); }

SecreC::TreeNode const * toCxx(TreeNode const * node) noexcept
{ return reinterpret_cast<SecreC::TreeNode const *>(node); }

} // namespace anonymous

namespace SecreC {

namespace /* anonymous */ {

template <class T >
T * childAt(const TreeNode * node, size_t i) {
    assert(i < node->children().size());
    assert(dynamic_cast<T *>(node->children().at(i)) != nullptr);
    return static_cast<T *>(node->children().at(i));
}

TreeNodeStmt * statementAt(const TreeNode * node, size_t i) {
    return childAt<TreeNodeStmt>(node, i);
}

TreeNodeExpr * expressionAt(const TreeNode * node, size_t i) {
    return childAt<TreeNodeExpr>(node, i);
}

} // namespace anonymous

/*******************************************************************************
  TreeNode
*******************************************************************************/

TreeNode::TreeNode(SecrecTreeNodeType type, const Location & loc)
    : m_parent(nullptr)
    , m_containingProcedure(nullptr)
    , m_type(type)
    , m_location(loc)
{
    // Intentionally empty
}

TreeNode::~TreeNode() {
    for (TreeNode * child : m_children) {
        assert(child != nullptr);
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

    for (const TreeNode * n : m_children) {
        out->m_children.push_back(n->clone(out));
    }

    return out;
}

TreeNodeProcDef * TreeNode::containingProcedure() const {
    if (m_containingProcedure != nullptr) return m_containingProcedure;
    if (m_parent != nullptr) {
        return (m_containingProcedure = m_parent->containingProcedure());
    }
    return nullptr;
}

void TreeNode::appendChild(TreeNode * child) {
    assert(child != nullptr);
    m_children.push_back(child);
    child->resetParent(this);
}

void TreeNode::setLocation(const Location & location) {
    m_location = location;
}

const char *TreeNode::typeName(SecrecTreeNodeType type) {
    switch (type) {
#define O(ENUM, CLASS) case NODE_ ## ENUM : return #ENUM;
    TREENODE_LIST
#undef O
    #ifdef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcovered-switch-default"
    #endif
    default: SHAREMIND_ABORT("TNtN %d", static_cast<int>(type));
    #ifdef __clang__
    #pragma GCC diagnostic pop
    #endif
    }
}

#undef CASE_NODE_NAME

void TreeNode::print (std::ostream& os, unsigned indent, unsigned startIndent) const {
    os << std::string(startIndent, ' ');
    os << typeName(m_type) << ' ';
    if (printHelper (os)) {
        os << ' ';
    }

    os << "at " << location();

    for (TreeNode* child : m_children) {
        os << std::endl;
        assert(child->parent() == this);
        child->print (os, indent, startIndent + indent);
    }
}

void TreeNode::printXml (std::ostream & os, bool full) const {

    if (full) {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    }
    os << '<' << typeName(m_type);
    printXmlHelper (os);

    if (m_children.empty()) {
        os << "/>";
    } else {
        os << '>';
        for (TreeNode * child : m_children) {
            assert(child->parent() == this);
            child->printXml (os, false);
        }
        os << "</" << typeName(m_type) << '>';
    }
}

/*******************************************************************************
  TreeNodeTypeF
*******************************************************************************/

bool TreeNodeTypeF::isVariable () const {
    switch (m_type) {
    case NODE_TYPEVAR:
    case NODE_DATATYPE_VAR_F:
    case NODE_DIMTYPE_VAR_F:
    case NODE_SECTYPE_PRIVATE_F:
        return true;
    default:
        return false;
    }
}

TreeNodeIdentifier* TreeNodeTypeF::identifier () const {
    assert (isVariable () && children ().size () == 1);
    return childAt<TreeNodeIdentifier>(this, 0);
}

/*******************************************************************************
  TreeNodeSecTypeF
*******************************************************************************/

void TreeNodeSecTypeF::setCachedType(const SecurityType * ty) {
    assert(m_cachedType == nullptr);
    assert(ty != nullptr);
    m_cachedType = ty;
}

bool TreeNodeSecTypeF::printHelper (std::ostream & os) const {
    if (isPublic ())
        os << "public";
    else
        os << static_cast<TreeNodeIdentifier *>(children().at(0))->value();
    return true;
}

/*******************************************************************************
  TreeNodeDataTypeConstF
*******************************************************************************/

bool TreeNodeDataTypeConstF::printHelper (std::ostream & os) const {
    os << SecrecFundDataTypeToString (secrecDataType ());
    return true;
}

void TreeNodeDataTypeConstF::printXmlHelper (std::ostream & os) const {
    os << " type=\"" << *cachedType () << "\"";
}

/*******************************************************************************
  TreeNodeDataTypeVarF
*******************************************************************************/

bool TreeNodeDataTypeVarF::printHelper (std::ostream & os) const {
    os << identifier ()->value ();
    return true;
}

void TreeNodeDataTypeVarF::printXmlHelper (std::ostream & os) const {
    os << " type=\"" << identifier ()->value () << "\"";
}

/******************************************************************
  TreeNodeDataTypeTemplateF
******************************************************************/

TreeNodeIdentifier* TreeNodeDataTypeTemplateF::identifier () const {
    assert (children ().size () > 1);
    return childAt<TreeNodeIdentifier>(this, 0);
}

TreeNodeSeqView<TreeNodeTypeArg> TreeNodeDataTypeTemplateF::arguments () const {
    assert (children ().size () > 1);
    return TreeNodeSeqView<TreeNodeTypeArg>(children().begin() + 1, children().end());
}

/*******************************************************************************
  TreeNodeDimTypeConstF
*******************************************************************************/

TreeNodeExprInt * TreeNodeDimTypeConstF::value() const {
    assert (children().size() == 1);
    return childAt<TreeNodeExprInt>(this, 0);
}

/*******************************************************************************
  TreeNodeDimTypeZeroF
*******************************************************************************/

bool TreeNodeDimTypeConstF::printHelper (std::ostream & os) const {
    os << cachedType ();
    return true;
}

void TreeNodeDimTypeConstF::printXmlHelper (std::ostream & os) const {
    os << " dim=\"" << cachedType () << "\"";
}

/*******************************************************************************
  TreeNodeDimTypeConstF
*******************************************************************************/

bool TreeNodeDimTypeZeroF::printHelper (std::ostream & os) const {
    os << "0";
    return true;
}

void TreeNodeDimTypeZeroF::printXmlHelper (std::ostream & os) const {
    os << " dim=\"0\"";
}

/*******************************************************************************
  TreeNodeDimTypeVarF
*******************************************************************************/

bool TreeNodeDimTypeVarF::printHelper (std::ostream & os) const {
    os << identifier ()->value ();
    return true;
}

void TreeNodeDimTypeVarF::printXmlHelper (std::ostream & os) const {
    os << " dim=\"" << identifier ()->value () << "\"";
}

/*******************************************************************************
  TreeNodeType
*******************************************************************************/

const SecreC::Type * TreeNodeType::secrecType() const {
    assert(m_cachedType != nullptr);
    return m_cachedType;
}

TreeNodeSeqView<TreeNodeTypeF> TreeNodeType::types () const {
    assert(children().size() == 3);
    return TreeNodeSeqView<TreeNodeTypeF>(m_children);
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

void printTypeArg(std::ostream& os, const TreeNodeTypeArg& arg) {
    if (arg.type () == NODE_TYPE_ARG_VAR) {
        os << static_cast<const TreeNodeTypeArgVar*> (&arg)->identifier ()->value ();
    }
    else if (arg.type () == NODE_TYPE_ARG_TEMPLATE) {
        const TreeNodeTypeArgTemplate* templ =
            static_cast<const TreeNodeTypeArgTemplate*> (&arg);
        os << "struct " << templ->identifier ()->value () << '<';
        bool first = true;
        for (TreeNodeTypeArg& typeArg : templ->arguments ()) {
            if (! first)
                os << ", ";
            first = false;
            printTypeArg (os, typeArg);
        }
        os << '>';
    }
    else if (arg.type () == NODE_TYPE_ARG_DATA_TYPE_CONST) {
        const TreeNodeTypeArgDataTypeConst* constTy =
            static_cast<const TreeNodeTypeArgDataTypeConst*> (&arg);
        os << SecrecFundDataTypeToString (constTy->secrecDataType ());
    }
    else if (arg.type () == NODE_TYPE_ARG_DIM_TYPE_CONST) {
        const TreeNodeTypeArgDimTypeConst* dim =
            static_cast<const TreeNodeTypeArgDimTypeConst*> (&arg);
        os << dim->value()->stringValue();
    }
    else if (arg.type () == NODE_TYPE_ARG_PUBLIC) {
        os << "public";
    }
}

void TreeNodeType::typeString(std::ostream& os) const {

    if (!isNonVoid()) {
        os << "void";
        return;
    }

    if (m_cachedType) {
        os << PrettyPrint (m_cachedType);
        return;
    }

    // Security type:
    TreeNodeSecTypeF * const st = secType();
    if (!st->isPublic())
        os << st->identifier()->value() << ' ';

    // Data type:
    if (! dataType ()->isVariable ()) {
        if (dataType ()->type () == NODE_DATATYPE_CONST_F) {
            TreeNodeDataTypeConstF* constDataType = static_cast<TreeNodeDataTypeConstF*>(dataType ());
            os << SecrecFundDataTypeToString (constDataType->secrecDataType ());
        }
        else if (dataType ()->type () == NODE_DATATYPE_TEMPLATE_F) {
            TreeNodeDataTypeTemplateF* templDataTy = static_cast<TreeNodeDataTypeTemplateF*>(dataType ());
            os << "struct " << templDataTy->identifier ()->value () << '<';
            bool first = true;
            for (TreeNodeTypeArg& typeArg : templDataTy->arguments ()) {
                if (! first)
                    os << ", ";
                first = false;
                printTypeArg(os, typeArg);
            }
            os << '>';
        }
    }
    else {
        auto const varDataType = static_cast<TreeNodeDataTypeVarF*>(dataType ());
        os << varDataType->identifier ()->value ();
    }

    // Dimensionality type:
    if (dimType()->type() == NODE_DIMTYPE_VAR_F) {
        auto const varDimType = static_cast<TreeNodeDimTypeVarF*>(dimType ());
        os << "[[" << varDimType->identifier ()->value () << "]]";
    }
    else
    if (dimType()->type() == NODE_DIMTYPE_CONST_F) {
        auto const constDimType = static_cast<TreeNodeDimTypeConstF const *>(dimType());
        os << "[[" << constDimType->value()->stringValue() << "]]";
    }
    else
    if (dimType()->type() == NODE_DIMTYPE_ZERO_F) {
        os << "[[0]]";
    }
    else {
        os << "[[UNDEFINED]]";
    }
}

/*******************************************************************************
  OverloadableOperator
*******************************************************************************/

std::string OverloadableOperator::operatorName() const {
    std::ostringstream os;
    os << "__operator" << getOperator();
    return os.str();
}

SecrecOperator OverloadableOperator::getOperator() const {
    if (m_operator == SCOP_NONE) { // unlikeley
        m_operator = getOperatorV();
        assert((m_operator != SCOP_NONE) &&
               "getOperatorV returned undefined operator.");
    }

    return m_operator;
}

/*******************************************************************************
  TreeNodeExpr
*******************************************************************************/

bool TreeNodeExpr::havePublicBoolType() const {
    assert(m_resultType != nullptr);
    return    m_resultType->secrecDataType()->equals (DATATYPE_BOOL)
           && m_resultType->secrecSecType()->isPublic()
           && m_resultType->isScalar();
}

const SecreC::Type * TreeNodeExpr::resultType() const {
    assert(m_resultType != nullptr);
    return m_resultType;
}

void TreeNodeExpr::setResultType(const SecreC::Type * type) {
    assert(m_resultType == nullptr);
    m_resultType = type;
}

void TreeNodeExpr::resetDataType(SecrecDataType dType) {
    assert(dynamic_cast<const TypeNonVoid *>(m_resultType) != nullptr);
    m_resultType = TypeBasic::get(m_resultType->secrecSecType(),
                                  dType,
                                  m_resultType->secrecDimType());
}

void TreeNodeExpr::instantiateDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (dType->isBuiltinPrimitive ()) {
        instantiateDataType (static_cast<const DataTypeBuiltinPrimitive*>(dType)->secrecDataType ());
    }
}

void TreeNodeExpr::instantiateDataType() {
    assert (resultType () != nullptr);

    if (resultType()->isVoid()) {
        return;
    }

    auto const dataType = resultType()->secrecDataType();

    if (dataType->equals(DATATYPE_NUMERIC_FLOAT)) {
        instantiateDataTypeV(DATATYPE_FLOAT64);
        return;
    }

    if (dataType->equals(DATATYPE_NUMERIC)) {
        instantiateDataTypeV(DATATYPE_INT64);
        return;
    }
}

// If possible instantiate abstract data type to given concrete data type
void TreeNodeExpr::instantiateDataType (SecrecDataType dType) {
    assert (resultType () != nullptr);
    if ( ! resultType ()->isVoid ()
        && resultType ()->secrecDataType ()->isAbstractNumeric()
        && dType != DATATYPE_NUMERIC) {
        instantiateDataTypeV (dType);
    }
}

CGBranchResult TreeNodeExpr::codeGenBoolWith (CodeGen&) {
    assert (false && "Not implemented!");
    return CGBranchResult (CGResult::ERROR_FATAL);
}

/*******************************************************************************
  TreeNodeExprArrayConstructor
*******************************************************************************/

TreeNodeSeqView<TreeNodeExpr> TreeNodeExprArrayConstructor::expressions () const {
    return TreeNodeSeqView<TreeNodeExpr>(m_children);
}

/*******************************************************************************
  TreeNodeExprPrefix
*******************************************************************************/

TreeNodeLValue* TreeNodeExprPrefix::lvalue () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeLValue>(this, 0);
}

SecrecOperator TreeNodeExprPrefix::getOperatorV () const {
    switch (type ()) {
        case NODE_EXPR_PREFIX_DEC: return SCOP_BIN_SUB;
        case NODE_EXPR_PREFIX_INC: return SCOP_BIN_ADD;
        default:                   return SCOP_NONE;
    }
}

/*******************************************************************************
  TreeNodeExprPostfix
*******************************************************************************/

TreeNodeLValue* TreeNodeExprPostfix::lvalue () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeLValue>(this, 0);
}

SecrecOperator TreeNodeExprPostfix::getOperatorV () const {
    switch (type ()) {
        case NODE_EXPR_POSTFIX_DEC: return SCOP_BIN_SUB;
        case NODE_EXPR_POSTFIX_INC: return SCOP_BIN_ADD;
        default:                    return SCOP_NONE;
    }
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
    case SCOP_BIN_SHL:  return "<<";
    case SCOP_BIN_SHR:  return ">>";
    default:
        assert(false); // shouldn't happen
    }

    return "?";
}

const char *TreeNodeExprBinary::operatorLongString () const {
    switch (getOperator()) {
    case SCOP_BIN_ADD:  return "addition";
    case SCOP_BIN_BAND: return "bitwise AND";
    case SCOP_BIN_BOR:  return "bitwise OR";
    case SCOP_BIN_DIV:  return "division";
    case SCOP_BIN_EQ:   return "equality";
    case SCOP_BIN_GE:   return "greater-than-or-equal";
    case SCOP_BIN_GT:   return "greater-than";
    case SCOP_BIN_LAND: return "conjunction";
    case SCOP_BIN_LE:   return "less-than-or-equal";
    case SCOP_BIN_LOR:  return "disjunction";
    case SCOP_BIN_LT:   return "less-than";
    case SCOP_BIN_MOD:  return "modulo";
    case SCOP_BIN_MUL:  return "multiplication";
    case SCOP_BIN_NE:   return "not-equals";
    case SCOP_BIN_SUB:  return "subtraction";
    case SCOP_BIN_XOR:  return "XOR";
    case SCOP_BIN_SHL:  return "shift-left";
    case SCOP_BIN_SHR:  return "shift-right";
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
    case NODE_EXPR_BINARY_SHL:  return SCOP_BIN_SHL;
    case NODE_EXPR_BINARY_SHR:  return SCOP_BIN_SHR;
    default:
        assert(false && "This code path should be unreachable.");
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

TreeNodeSeqView<TreeNodeExpr> TreeNodeExprProcCall::params () const {
    assert (!children().empty());
    return TreeNodeSeqView<TreeNodeExpr>(++ children().begin(), children().end());
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

bool TreeNodeExprBool::printHelper(std::ostream & os) const {
    os << (m_value ? "true" : "false");
    return true;
}

void TreeNodeExprBool::printXmlHelper (std::ostream & os) const {
    os << " value=\"bool:";
    printHelper(os);
    os << "\"";
}

/*******************************************************************************
  TreeNodeExprString
*******************************************************************************/

TreeNodeSeqView<TreeNodeStringPart> TreeNodeExprString::parts () const {
    return TreeNodeSeqView<TreeNodeStringPart>(m_children);
}

/*******************************************************************************
  TreeNodeStringPartFragment
*******************************************************************************/

bool TreeNodeStringPartFragment::printHelper (std::ostream & os) const {
    // TODO: escape this string
    os << "\"" << strEscape (m_value.str ()) << "\"";
    return true;
}

void TreeNodeStringPartFragment::printXmlHelper (std::ostream & os) const {
    os << " value=\"string:" << xmlEncode(m_value.str()) << "\"";
}

/*******************************************************************************
  TreeNodeStringPartIdentifier
*******************************************************************************/

bool TreeNodeStringPartIdentifier::printHelper (std::ostream & os) const {
    os << "\"" << m_name << "\"";
    return true;
}

void TreeNodeStringPartIdentifier::printXmlHelper (std::ostream & os) const {
    os << " value=\"string:" << xmlEncode(m_name.str()) << "\"";
}

/*******************************************************************************
  TreeNodeExprFloat
*******************************************************************************/

bool TreeNodeExprFloat::printHelper (std::ostream & os) const {
    os << m_value;
    return true;
}

void TreeNodeExprFloat::printXmlHelper (std::ostream & os) const {
    os << " value=\"double:" << m_value << "\"";
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
  TreeNodeLVariable
*******************************************************************************/

TreeNodeIdentifier* TreeNodeLVariable::identifier () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeIdentifier>(this, 0);
}

/*******************************************************************************
  TreeNodeLIndex
*******************************************************************************/

TreeNodeLValue* TreeNodeLIndex::lvalue () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeLValue>(this, 0);
}

TreeNodeSubscript* TreeNodeLIndex::indices () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeSubscript>(this, 1);
}

/*******************************************************************************
  TreeNodeLSelect
*******************************************************************************/

TreeNodeLValue* TreeNodeLSelect::lvalue () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeLValue>(this, 0);
}

TreeNodeIdentifier* TreeNodeLSelect::identifier () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeIdentifier>(this, 1);
}

/*******************************************************************************
  TreeNodeExprAssign
*******************************************************************************/

TreeNodeLValue* TreeNodeExprAssign::leftHandSide () const {
    assert(children().size() == 2);
    return childAt<TreeNodeLValue>(this, 0);
}

TreeNodeExpr* TreeNodeExprAssign::rightHandSide () const {
    assert(children().size() == 2);
    return expressionAt (this, 1);
}

SecrecOperator TreeNodeExprAssign::getOperatorV () const {
    switch (type ()) {
        case NODE_EXPR_BINARY_ASSIGN_MUL: return SCOP_BIN_MUL;
        case NODE_EXPR_BINARY_ASSIGN_DIV: return SCOP_BIN_DIV;
        case NODE_EXPR_BINARY_ASSIGN_MOD: return SCOP_BIN_MOD;
        case NODE_EXPR_BINARY_ASSIGN_ADD: return SCOP_BIN_ADD;
        case NODE_EXPR_BINARY_ASSIGN_SUB: return SCOP_BIN_SUB;
        case NODE_EXPR_BINARY_ASSIGN_AND: return SCOP_BIN_BAND;
        case NODE_EXPR_BINARY_ASSIGN_OR:  return SCOP_BIN_BOR;
        case NODE_EXPR_BINARY_ASSIGN_XOR: return SCOP_BIN_XOR;
        default:                          return SCOP_NONE;
    }
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

TreeNodeSubscript* TreeNodeExprIndex::indices () const {
    assert (children().size() == 2);
    return childAt<TreeNodeSubscript>(this, 1);
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

TreeNodeSeqView<TreeNodeExpr> TreeNodeExprReshape::dimensions () const {
    return TreeNodeSeqView<TreeNodeExpr>(++ m_children.begin (), m_children.end ());
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

    return nullptr;
}


/*******************************************************************************
  TreeNodeExprInt
*******************************************************************************/

bool TreeNodeExprInt::printHelper (std::ostream & os) const {
    os << m_stringValue;
    return true;
}

void TreeNodeExprInt::printXmlHelper (std::ostream & os) const {
    os << " value=\"int:" << m_stringValue << "\"";
}

/*******************************************************************************
  TreeNodeProcDef
*******************************************************************************/

StringRef TreeNodeProcDef::procedureName() const {
    return identifier ()->value ();
}

TreeNodeAnnotation* TreeNodeProcDef::annotation () const {
    assert (children().size() >= 4);
    return childAt<TreeNodeAnnotation>(this, 0);
}

TreeNodeIdentifier* TreeNodeProcDef::identifier () const {
    assert (children().size() >= 4);
    return childAt<TreeNodeIdentifier>(this, 1);
}

TreeNodeType* TreeNodeProcDef::returnType () const {
    assert (children().size() >= 4);
    return childAt<TreeNodeType>(this, 2);
}

TreeNodeStmt* TreeNodeProcDef::body () const {
    assert (children().size() >= 4);
    return childAt<TreeNodeStmt>(this, 3);
}

TreeNodeSeqView<TreeNodeStmtDecl> TreeNodeProcDef::params () const {
    assert (children ().size () >= 4);
    return TreeNodeSeqView<TreeNodeStmtDecl>(children ().begin () + 4, children ().end ());
}

const std::string TreeNodeProcDef::printableSignature() const {
    std::ostringstream oss;
    if (m_procSymbol) {
        oss << *m_procSymbol;
    } else {
        returnType()->typeString(oss);
        oss << ' ' << procedureName() << '(';
        bool first = true;
        for (const TreeNodeStmtDecl& decl : params ()) {
            if (! first)
                oss << ", ";
            first = false;
            decl.varType()->typeString(oss);
            oss << ' ' << decl.variableName();
        }
        oss << ')';
    }
    return oss.str();
}

/*******************************************************************************
  TreeNodeQuantifier
*******************************************************************************/

TreeNodeIdentifier* TreeNodeQuantifier::typeVariable () const {
    assert (! children ().empty ());
    return childAt<TreeNodeIdentifier>(this, 0);
}

/*******************************************************************************
  TreeNodeQuantifierDomain
*******************************************************************************/

// will equal to zero, if kind not specified
TreeNodeIdentifier* TreeNodeQuantifierDomain::kind () const {
    assert (children ().size () == 1 || children ().size () == 2);
    if (children ().size () == 2) {
        return childAt<TreeNodeIdentifier>(this, 1);
    }

    return nullptr;
}

void TreeNodeQuantifierDomain::printQuantifier (std::ostream &os) const {
    os << "domain " << typeVariable ()->value();
    if (kind())
        os << " : " << kind()->value();
}

/*******************************************************************************
  TreeNodeQuantifierDim
*******************************************************************************/

void TreeNodeQuantifierDim::printQuantifier (std::ostream & os) const {
    os << "dim " << typeVariable ()->value ();
}

/*******************************************************************************
  TreeNodeQuantifierData
*******************************************************************************/

void TreeNodeQuantifierData::printQuantifier (std::ostream & os) const {
    os << "type " << typeVariable ()->value ();
}

/*******************************************************************************
  TreeNodeTemplate
*******************************************************************************/

TreeNodeProcDef* TreeNodeTemplate::body () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeProcDef>(this, 1);
}

TreeNodeSeqView<TreeNodeQuantifier> TreeNodeTemplate::quantifiers () const {
    assert (children ().size () == 2);
    return TreeNodeSeqView<TreeNodeQuantifier>(children ().at (0)->children ());
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

TreeNodeSeqView<TreeNodeTypeF> TreeNodeExprQualified::types() const {
    assert (children ().size () >= 2);
    return TreeNodeSeqView<TreeNodeTypeF>(++ children ().begin (), children ().end ());
}

/*******************************************************************************
  TreeNodeExprStrlen
*******************************************************************************/

TreeNodeExpr* TreeNodeExprStrlen::expression () const {
    assert (children ().size () == 1);
    return expressionAt (this, 0);
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

bool TreeNodeIdentifier::printHelper (std::ostream & os) const {
    os << "\"" << m_value << "\"";
    return true;
}

void TreeNodeIdentifier::printXmlHelper (std::ostream & os) const {
    os << " value=\"string:" << xmlEncode(m_value.str()) << "\"";
}

/*******************************************************************************
  TreeNodeStmtDecl
*******************************************************************************/

StringRef TreeNodeStmtDecl::variableName() const {
    return initializer ()->variableName ();
}

TreeNodeSeqView<TreeNodeExpr> TreeNodeStmtDecl::shape () const {
    return initializer ()->shape ();
}

TreeNodeExpr* TreeNodeStmtDecl::rightHandSide () const {
    return initializer ()->rightHandSide ();
}

TreeNodeType* TreeNodeStmtDecl::varType () const {
    assert (children().size () >= 2);
    return childAt<TreeNodeType>(this, 0);
}

TreeNodeVarInit* TreeNodeStmtDecl::initializer () const {
    assert (children().size () >= 2);
    return childAt<TreeNodeVarInit>(this, 1);
}

TreeNodeSeqView<TreeNodeVarInit> TreeNodeStmtDecl::initializers() const {
    assert (children().size () >= 2);
    return TreeNodeSeqView<TreeNodeVarInit> (++ m_children.begin (), m_children.end ());
}

TreeNodeSeqView<TreeNodeExpr> TreeNodeVarInit::shape () const {
    assert(children().size() > 0 && children().size() <= 3);
    if (children().size() > 1) {
        return TreeNodeSeqView<TreeNodeExpr>(children ().at (1)->children ());
    }

    return TreeNodeSeqView<TreeNodeExpr>();
}

bool TreeNodeVarInit::hasRightHandSide() const {
    return children().size() > 2;
}

TreeNodeExpr* TreeNodeVarInit::rightHandSide () const {
    assert(children().size() > 0 && children().size() <= 3);
    if (children ().size () > 2) {
        return expressionAt (this, 2);
    }

    return nullptr;
}

StringRef TreeNodeVarInit::variableName() const {
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

    return nullptr;
}

TreeNodeExpr* TreeNodeStmtFor::conditional () const {
    assert (children ().size () == 4);
    if (children ().at (1)->type () != NODE_EXPR_NONE) {
        return expressionAt (this, 1);
    }

    return nullptr;
}

TreeNodeExpr* TreeNodeStmtFor::iteratorExpr () const {
    assert (children ().size () == 4);
    if (children ().at (2)->type () != NODE_EXPR_NONE) {
        return expressionAt (this, 2);
    }

    return nullptr;
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

    return nullptr;
}

/*******************************************************************************
  TreeNodeStmtReturn
*******************************************************************************/

TreeNodeExpr* TreeNodeStmtReturn::expression () const {
    if (!children ().empty ()) {
        return expressionAt (this, 0);
    }

    return nullptr;
}

bool TreeNodeStmtReturn::hasExpression() const {
    return !children().empty();
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

TreeNodeSeqView<TreeNodeExpr> TreeNodeStmtPrint::expressions() {
    assert (children ().size () == 1);
    return TreeNodeSeqView<TreeNodeExpr>(children ().at (0)->children ());
}

/*******************************************************************************
  TreeNodeStmtSyscall
*******************************************************************************/

TreeNodeExprString* TreeNodeStmtSyscall::name () const {
    assert (children ().size () >= 1);
    return childAt<TreeNodeExprString>(this, 0);
}

TreeNodeSeqView<TreeNodeSyscallParam> TreeNodeStmtSyscall::params () const {
    assert (children ().size () >= 1);
    return TreeNodeSeqView<TreeNodeSyscallParam>(begin () + 1, end ());
}

/*******************************************************************************
  TreeNodeTypeType
*******************************************************************************/

bool TreeNodeTypeType::printHelper (std::ostream & os) const {
    if (m_cachedType != nullptr) {
        os << *secrecType();
        return true;
    }

    return false;
}

/*******************************************************************************
  TreeNodeModule
*******************************************************************************/

TreeNodeModule::~TreeNodeModule() {
    for (TreeNodeProcDef* instance : m_generatedInstances) {
        delete instance;
    }
}

bool TreeNodeModule::hasName() const {
   return children().size() == 2;
}

StringRef TreeNodeModule::name() const {
    assert(children().size() == 1 || children().size() == 2);

    if (hasName()) {
        return childAt<TreeNodeIdentifier>(this, 1)->value();
    }

    return StringRef ("", 0);
}

TreeNodeProgram * TreeNodeModule::program() const {
    assert(children().size() == 1 || children().size() == 2);
    return childAt<TreeNodeProgram>(this, 0);
}

/*******************************************************************************
  TreeNodeImport
*******************************************************************************/

StringRef TreeNodeImport::name() const {
    assert(children().size() == 1);
    return childAt<TreeNodeIdentifier>(this, 0)->value();
}

/*******************************************************************************
  TreeNodeSyscallParam
*******************************************************************************/

TreeNodeExpr* TreeNodeSyscallParam::expression () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeExpr>(this, 0);
}

/*******************************************************************************
  TreeNodeKind
*******************************************************************************/

TreeNodeIdentifier* TreeNodeKind::identifier () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeIdentifier>(this, 0);
}

TreeNodeSeqView<TreeNodeDataTypeDecl> TreeNodeKind::types () const {
    assert (children ().size () == 2);
    return TreeNodeSeqView<TreeNodeDataTypeDecl> (children ().at (1)->children ());
}

/*******************************************************************************
  TreeNodeDomain
*******************************************************************************/

TreeNodeIdentifier* TreeNodeDomain::domainIdentifier () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeIdentifier>(this, 0);
}

TreeNodeIdentifier* TreeNodeDomain::kindIdentifier () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeIdentifier>(this, 1);
}

/*******************************************************************************
  TreeNodeExprSelection
*******************************************************************************/

TreeNodeExpr* TreeNodeExprSelection::expression () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeExpr>(this, 0);
}

TreeNodeIdentifier* TreeNodeExprSelection::identifier () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeIdentifier>(this, 1);
}

/*******************************************************************************
  TreeNodeAttribute
*******************************************************************************/

TreeNodeType* TreeNodeAttribute::type () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeType>(this, 0);
}

TreeNodeIdentifier* TreeNodeAttribute::identifier () const {
    assert (children ().size () == 2);
    return childAt<TreeNodeIdentifier>(this, 1);
}

/*******************************************************************************
  TreeNodeStructDecl
*******************************************************************************/

TreeNodeSeqView<TreeNodeQuantifier> TreeNodeStructDecl::quantifiers() const {
    assert (children ().size () == 3);
    return TreeNodeSeqView<TreeNodeQuantifier>(children ().at (0)->children ());
}

TreeNodeIdentifier* TreeNodeStructDecl::identifier () const {
    assert (children ().size () == 3);
    return childAt<TreeNodeIdentifier>(this, 1);
}

TreeNodeSeqView<TreeNodeAttribute> TreeNodeStructDecl::attributes () const {
    assert (children ().size () == 3);
    return TreeNodeSeqView<TreeNodeAttribute>(children ().at (2)->children ());
}

/*******************************************************************************
  TreeNodeTypeArg
*******************************************************************************/

TreeNodeTypeArg::~TreeNodeTypeArg () {
    delete m_typeArgument;
}

const TypeArgument& TreeNodeTypeArg::typeArgument () const {
    assert (m_typeArgument != nullptr);
    return *m_typeArgument;
}

void TreeNodeTypeArg::setTypeArgument (const TypeArgument& typeArgument) {
    assert (m_typeArgument == nullptr);
    m_typeArgument = new TypeArgument (typeArgument);
}

/*******************************************************************************
  TreeNodeTypeArgVar
*******************************************************************************/

TreeNodeIdentifier* TreeNodeTypeArgVar::identifier () const {
    assert (children ().size () == 1);
    return childAt<TreeNodeIdentifier>(this, 0);
}

/*******************************************************************************
  TreeNodeTypeArgTemplate
*******************************************************************************/

TreeNodeIdentifier* TreeNodeTypeArgTemplate::identifier () const {
    assert (children ().size () > 1);
    return childAt<TreeNodeIdentifier>(this, 0);
}

TreeNodeSeqView<TreeNodeTypeArg> TreeNodeTypeArgTemplate::arguments () const {
    assert (children ().size () > 1);
    return TreeNodeSeqView<TreeNodeTypeArg>(children().begin() + 1, children().end());
}

/*******************************************************************************
  TreeNodeTypeArgDimTypeConst
*******************************************************************************/

TreeNodeExprInt * TreeNodeTypeArgDimTypeConst::value() const {
    assert (children ().size () == 1);
    return childAt<TreeNodeExprInt>(this, 0);
}

SecrecDimType TreeNodeTypeArgDimTypeConst::secrecDimType() const {
    assert (value()->haveActualValue());
    return static_cast<SecrecDimType>(value()->actualValue());
}

/*******************************************************************************
  TreeNodeDataTypeDecl
*******************************************************************************/

TreeNodeSeqView<TreeNodeDataTypeDeclParam> TreeNodeDataTypeDecl::parameters () const {
    assert (children ().size () == 1);
    auto child = childAt<TreeNodeInternalUse>(this, 0);
    return TreeNodeSeqView<TreeNodeDataTypeDeclParam>(child->children ());
}

/*******************************************************************************
  TreeNodeAnnotation
*******************************************************************************/

TreeNodeIdentifier* TreeNodeAnnotation::identifier () const {
    if (children().size() >= 1)
        return childAt<TreeNodeIdentifier>(this, 0);
    return nullptr;
}

} // namespace SecreC

/*******************************************************************************
  C interface for Yacc
*******************************************************************************/

#define SELECTNODE(ENUM,NAME)\
    case NODE_ ## ENUM: \
        return toC(new SecreC::TreeNode ## NAME(*loc))

#define SELECTNODETYPE(ENUM,NAME)\
    case NODE_ ## ENUM: \
        return toC(new SecreC::TreeNode ## NAME(type, *loc))

#define SELECTEXPR(ENUM, NAME) SELECTNODE (EXPR_ ## ENUM, Expr ## NAME)
#define SELECTEXPRTYPE(ENUM, NAME) SELECTNODETYPE (EXPR_ ## ENUM, Expr ## NAME)
#define SELECTSTMT(ENUM, NAME) SELECTNODE (STMT_ ## ENUM, Stmt ## NAME)

TreeNode * treenode_init(enum SecrecTreeNodeType type, const YYLTYPE * loc) {
    switch (type) {

    SELECTNODE(CASTDEF, CastDef);
    SELECTNODE(PROGRAM, Program);
    SELECTNODE(PROCDEF, ProcDef);
    SELECTNODE(DECL, StmtDecl);
    SELECTNODE(TYPETYPE, TypeType);
    SELECTNODE(TYPEVOID, TypeVoid);
    SELECTNODE(EXPR_CAST, ExprCast);
    SELECTNODE(KIND, Kind);
    SELECTNODE(DOMAIN, Domain);
    SELECTNODE(TEMPLATE_QUANTIFIER_DATA, QuantifierData);
    SELECTNODE(TEMPLATE_QUANTIFIER_DOMAIN, QuantifierDomain);
    SELECTNODE(TEMPLATE_QUANTIFIER_DIM, QuantifierDim);
    SELECTNODE(TEMPLATE_DECL, Template);
    SELECTNODE(VAR_INIT, VarInit);
    SELECTNODE(MODULE, Module);
    SELECTNODE(IMPORT, Import);
    SELECTNODE(DIMTYPE_VAR_F, DimTypeVarF);
    SELECTNODE(DIMTYPE_ZERO_F, DimTypeZeroF);
    SELECTNODE(DIMTYPE_CONST_F, DimTypeConstF);
    SELECTNODE(TYPEVAR, TypeVarF);
    SELECTNODE(LITE_STRING, ExprString);
    SELECTNODE(STRUCT_DECL, StructDecl);
    SELECTNODE(ATTRIBUTE, Attribute);

    SELECTNODE(INTERNAL_USE, InternalUse);
    SELECTNODE(DIMENSIONS, Dimensions);
    SELECTNODE(LVALUE_VARIABLE, LVariable);
    SELECTNODE(LVALUE_INDEX, LIndex);
    SELECTNODE(LVALUE_SELECT, LSelect);
    SELECTNODE(SUBSCRIPT, Subscript);
    SELECTNODE(INDEX_INT, IndexInt);
    SELECTNODE(INDEX_SLICE, IndexSlice);
    SELECTNODE(ANNOTATION, Annotation);

    SELECTNODE(TYPE_ARG_VAR, TypeArgVar);
    SELECTNODE(TYPE_ARG_TEMPLATE, TypeArgTemplate);
    SELECTNODE(TYPE_ARG_PUBLIC, TypeArgPublic);
    SELECTNODE(DATATYPE_TEMPLATE_F, DataTypeTemplateF);

    SELECTNODETYPE(PUSH, SyscallParam);
    SELECTNODETYPE(PUSHCREF, SyscallParam);
    SELECTNODETYPE(PUSHREF, SyscallParam);
    SELECTNODETYPE(SYSCALL_RETURN, SyscallParam);

    SELECTEXPR(NONE, None);
    SELECTEXPR(DECLASSIFY, Declassify);
    SELECTEXPR(PROCCALL, ProcCall);
    SELECTEXPR(RVARIABLE, RVariable);
    SELECTEXPR(INDEX, Index);
    SELECTEXPR(SIZE, Size);
    SELECTEXPR(SHAPE, Shape);
    SELECTEXPR(CAT, Cat);
    SELECTEXPR(RESHAPE, Reshape);
    SELECTEXPR(TOSTRING, ToString);
    SELECTEXPR(STRING_FROM_BYTES, StringFromBytes);
    SELECTEXPR(BYTES_FROM_STRING, BytesFromString);
    SELECTEXPR(DOMAINID, DomainID);
    SELECTEXPR(TYPE_QUAL, Qualified);
    SELECTEXPR(ARRAY_CONSTRUCTOR, ArrayConstructor);
    SELECTEXPR(SELECTION, Selection);
    SELECTEXPR(STRLEN, Strlen);

    SELECTEXPR(TERNIF, Ternary);
    SELECTEXPRTYPE(UINV, Unary);
    SELECTEXPRTYPE(UNEG, Unary);
    SELECTEXPRTYPE(UMINUS, Unary);

    SELECTEXPRTYPE(POSTFIX_INC, Postfix);
    SELECTEXPRTYPE(POSTFIX_DEC, Postfix);
    SELECTEXPRTYPE(PREFIX_INC, Prefix);
    SELECTEXPRTYPE(PREFIX_DEC, Prefix);

    SELECTEXPRTYPE(BINARY_ADD, Binary);
    SELECTEXPRTYPE(BINARY_DIV, Binary);
    SELECTEXPRTYPE(BINARY_EQ, Binary);
    SELECTEXPRTYPE(BINARY_GE, Binary);
    SELECTEXPRTYPE(BINARY_GT, Binary);
    SELECTEXPRTYPE(BINARY_LAND, Binary);
    SELECTEXPRTYPE(BINARY_LE, Binary);
    SELECTEXPRTYPE(BINARY_LOR, Binary);
    SELECTEXPRTYPE(BINARY_LT, Binary);
    SELECTEXPRTYPE(BINARY_MATRIXMUL, Binary);
    SELECTEXPRTYPE(BINARY_MOD, Binary);
    SELECTEXPRTYPE(BINARY_MUL, Binary);
    SELECTEXPRTYPE(BINARY_NE, Binary);
    SELECTEXPRTYPE(BINARY_SUB, Binary);
    SELECTEXPRTYPE(BITWISE_AND, Binary);
    SELECTEXPRTYPE(BITWISE_OR, Binary);
    SELECTEXPRTYPE(BITWISE_XOR, Binary);
    SELECTEXPRTYPE(BINARY_SHL, Binary);
    SELECTEXPRTYPE(BINARY_SHR, Binary);

    SELECTEXPRTYPE(BINARY_ASSIGN_ADD, Assign);
    SELECTEXPRTYPE(BINARY_ASSIGN_AND, Assign);
    SELECTEXPRTYPE(BINARY_ASSIGN_DIV, Assign);
    SELECTEXPRTYPE(BINARY_ASSIGN_MOD, Assign);
    SELECTEXPRTYPE(BINARY_ASSIGN_MUL, Assign);
    SELECTEXPRTYPE(BINARY_ASSIGN_OR, Assign);
    SELECTEXPRTYPE(BINARY_ASSIGN_SUB, Assign);
    SELECTEXPRTYPE(BINARY_ASSIGN_XOR, Assign);
    SELECTEXPRTYPE(BINARY_ASSIGN, Assign);

    SELECTSTMT(BREAK, Break);
    SELECTSTMT(CONTINUE, Continue);
    SELECTSTMT(COMPOUND, Compound);
    SELECTSTMT(DOWHILE, DoWhile);
    SELECTSTMT(EXPR, Expr);
    SELECTSTMT(ASSERT, Assert);
    SELECTSTMT(FOR, For);
    SELECTSTMT(IF, If);
    SELECTSTMT(RETURN, Return);
    SELECTSTMT(WHILE, While);
    SELECTSTMT(PRINT, Print);
    SELECTSTMT(SYSCALL, Syscall);

    default:

        std::cerr << SecreC::TreeNode::typeName (type) << std::endl;
        assert (false && "Node of this type should not be initialized with treenode_init");
        return nullptr;
    }
}

void treenode_free(TreeNode * node) {
    delete toCxx(node);
}

enum SecrecTreeNodeType treenode_type(TreeNode * node) {
    return toCxx(node)->type();
}

YYLTYPE treenode_location(const TreeNode * node) {
    return toCxx(node)->location().toYYLTYPE();
}

unsigned treenode_numChildren(const TreeNode * node) {
    return toCxx(node)->children().size();
}

TreeNode * treenode_childAt(const TreeNode * node, unsigned index) {
    return toC(toCxx(node)->children().at(index));
}

void treenode_appendChild(TreeNode * parent, TreeNode * child) {
    return toCxx(parent)->appendChild(toCxx(child));
}

void treenode_setLocation(TreeNode * node, YYLTYPE * loc) {
    return toCxx(node)->setLocation(*loc);
}

void treenode_moveChildren(TreeNode * cfrom, TreeNode * cto) {
    auto const from = toCxx(cfrom);
    auto const to = toCxx(cto);
    for (SecreC::TreeNode * child : from->children()) {
        to->appendChild(child);
    }

    from->children().clear();
}

TreeNode * treenode_init_bool(unsigned value, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeExprBool(value, *loc));
}

TreeNode * treenode_init_int(TYPE_STRINGREF value, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeExprInt(*value, *loc));
}

TreeNode * treenode_init_str_fragment(TYPE_STRINGREF value, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeStringPartFragment (*value, *loc));
}

TreeNode * treenode_init_str_ident(TYPE_STRINGREF value, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeStringPartIdentifier (*value, *loc));
}

TreeNode * treenode_init_float(TYPE_STRINGREF value, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeExprFloat(*value, *loc));
}

TreeNode * treenode_init_identifier(TYPE_STRINGREF value, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeIdentifier(*value, *loc));
}

TreeNode * treenode_init_publicSecTypeF(YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeSecTypeF(NODE_SECTYPE_PUBLIC_F, *loc));
}

TreeNode * treenode_init_privateSecTypeF(YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeSecTypeF(NODE_SECTYPE_PRIVATE_F, *loc));
}

TreeNode * treenode_init_dataTypeVarF (YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeDataTypeVarF (*loc));
}

TreeNode * treenode_init_dataTypeConstF(enum SecrecDataType dataType, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeDataTypeConstF(dataType, *loc));
}

TreeNode * treenode_init_opdef(TYPE_STRINGTABLE table, enum SecrecOperator op, YYLTYPE * loc) {
    auto const node = toC(new SecreC::TreeNodeOpDef(op, *loc));
    std::ostringstream os;
    os << "__operator" << op;
    treenode_appendChild(node, treenode_init(NODE_ANNOTATION, loc));
    treenode_appendChild(node, treenode_init_identifier(table->addString (os.str()), loc));
    return node;
}

TreeNode * treenode_init_castdef(TYPE_STRINGTABLE table, YYLTYPE * loc) {
    auto const node = toC(new SecreC::TreeNodeCastDef(*loc));
    static std::string castStr = "__cast";
    treenode_appendChild(node, treenode_init(NODE_ANNOTATION, loc));
    treenode_appendChild(node, treenode_init_identifier(table->addString(castStr), loc));
    return node;
}

TreeNode* treenode_init_lvalue(TreeNode *node, YYLTYPE* loc) {
    assert (node != nullptr && loc != nullptr);
    SecreC::Location secrecLoc (*loc);
    auto const result = toC(toCxx(node)->makeLValue(secrecLoc));
    *loc = secrecLoc.toYYLTYPE ();
    return result;
}

TreeNode *treenode_init_typeArgDataTypeConst (enum SecrecDataType dataType, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeTypeArgDataTypeConst(dataType, *loc));
}

TreeNode *treenode_init_dataTypeDecl (TYPE_STRINGREF name, YYLTYPE *loc) {
    return toC(new SecreC::TreeNodeDataTypeDecl(*name, *loc));
}

TreeNode *treenode_init_dataTypeDeclParamPublic (enum SecrecDataType dataType, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeDataTypeDeclParamPublic(dataType, *loc));
}

TreeNode *treenode_init_dataTypeDeclParamSize (uint64_t size, YYLTYPE * loc) {
    return toC(new SecreC::TreeNodeDataTypeDeclParamSize(size, *loc));
}

TYPE_STRINGREF secrec_fund_datatype_to_string (TYPE_STRINGTABLE table, SecrecDataType ty)
{
    const char *str = SecrecFundDataTypeToString(ty);
    return table->addString(str, strlen(str));
}
