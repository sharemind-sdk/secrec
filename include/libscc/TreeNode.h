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

#ifndef SECREC_TREENODE_H
#define SECREC_TREENODE_H

#include "Location.h"
#include "ParserEnums.h"
#include "StringRef.h"
#include "TypeContext.h"
#include "TreeNodeFwd.h"

#include <cassert>
#include <string>
#include <vector>
#include <stdint.h>

namespace SecreC {

class CGBranchResult;
class CGResult;
class CGStmtResult;
class CodeGen;
class ConstantString;
class ModuleInfo;
class SubscriptInfo;
class Symbol;
class SymbolProcedure;
class SymbolTypeVariable;
class TypeArgument;
class TypeProc;
class TypeUnifier;

/******************************************************************
  TreeNode
******************************************************************/

/**
 * @brief Abstract representation of the SecreC code.
 *
 * TODO: This is not a type-safe representation. We should
 * always store properly typed subtrees. To achieve something
 * comparable the syntactic elements have accessors that return
 * subtrees of proper types. Internally the accessors verify
 * (using assert) that the proper type is provided.
 */
class TreeNode {
public: /* Types: */
    using ChildrenList = std::vector<TreeNode*>;
    using ChildrenListIterator = ChildrenList::iterator;
    using ChildrenListConstIterator = ChildrenList::const_iterator;

public: /* Methods: */
    TreeNode(SecrecTreeNodeType type, const Location & loc);
    virtual ~TreeNode();

    TreeNodeProcDef* containingProcedure() const;
    inline TreeNode* parent() const { return m_parent; }
    inline bool hasParent() const { return m_parent != nullptr; }
    inline SecrecTreeNodeType type() const { return m_type; }
    inline ChildrenList &children() { return m_children; }
    inline const ChildrenList &children() const { return m_children; }
    inline const Location & location() const { return m_location; }

    void appendChild(TreeNode *child);
    void setLocation(const Location & location);

    ChildrenListIterator begin () { return children ().begin (); }
    ChildrenListIterator end () { return children ().end (); }
    ChildrenListConstIterator begin () const { return children ().begin (); }
    ChildrenListConstIterator end () const { return children ().end (); }

    void print (std::ostream& os, unsigned indentation = 2, unsigned startIndent = 0) const;
    void printXml (std::ostream& os, bool full = false) const;

    static const char *typeName(SecrecTreeNodeType type);

    TreeNode* clone (TreeNode* parent) const;

    /**
     * @brief makeLValue Try to construct a lvalue out of the given tree node.
     * This function is needed as the parser it unfortunately not powerful enough
     * to distinguish them wihout major reduce-reduce conflicts.
     * @note Does not perform any type checking.
     * @param loc Location where the error happens.
     * @return NULL if the given tree node is not a lvalue and otherwise a newly constructed lvalue.
     */
    TreeNodeLValue* makeLValue (Location& loc) const { return makeLValueV (loc); }

protected: /* Methods: */

    virtual TreeNodeLValue* makeLValueV (Location& loc) const {
        loc = location ();
        return nullptr;
    }

    virtual inline bool printHelper (std::ostream&) const { return false; }

    virtual inline void printXmlHelper (std::ostream&) const { }

    void setParentDirectly(TreeNode *parent) {
        m_parent = parent;
    }

    void setContainingProcedureDirectly (TreeNodeProcDef *p) {
        m_procedure = p;
    }

    virtual inline void resetParent(TreeNode *parent) {
        m_parent = parent;
        m_procedure = parent->m_procedure;
    }

    // It's assumed that cloneV sets m_type, and m_location.
    // As the children are populated by clone method, it must not
    // explicitly add any by itself. Shallow copy only.
    virtual TreeNode* cloneV () const = 0;

protected: /* Fields: */
    TreeNode*                 m_parent;
    mutable TreeNodeProcDef * m_procedure;
    const SecrecTreeNodeType  m_type;
    ChildrenList              m_children;
    Location                  m_location;
};

/******************************************************************
  TreeNodesView
******************************************************************/

template <class SubClass>
class TreeNodeSeqView {
private: /* Types: */

    using CLI  = TreeNode::ChildrenList::iterator;
    using CLCI = TreeNode::ChildrenList::const_iterator;

    template <class value_type, class other_iterator>
    class iterator_base :
        public std::iterator
            < typename std::iterator_traits<other_iterator>::iterator_category
            , value_type
            >
    {
    private: /* Types: */
        using base_type = std::iterator<
                typename std::iterator_traits<other_iterator>::iterator_category,
                value_type
            >;
    public: /* Methods: */
        explicit iterator_base (other_iterator iterator)
            : m_iterator (std::move(iterator)) { }

        typename base_type::reference operator * () const {
            assert (dynamic_cast<typename base_type::pointer>(*m_iterator) != nullptr);
            return *static_cast<typename base_type::pointer>(*m_iterator);
        }

        typename base_type::pointer operator -> () const {
            assert (dynamic_cast<typename base_type::pointer>(*m_iterator) != nullptr);
            return static_cast<typename base_type::pointer>(*m_iterator);
        }

        bool operator == (const iterator_base i) const { return m_iterator == i.m_iterator; }
        bool operator != (const iterator_base i) const { return m_iterator != i.m_iterator; }
        iterator_base& operator ++ () {  ++ m_iterator; return *this; }
        iterator_base operator ++ (int) { const iterator_base t = *this; ++ m_iterator; return t; }

    private: /* Fields: */
        other_iterator m_iterator;
    };

public: /* Types: */
    using value_type = SubClass;
    using reference = SubClass&;
    using const_reference = const SubClass&;
    using pointer = SubClass*;
    using const_pointer = const SubClass*;
    using size_type = TreeNode::ChildrenList::size_type;
    using iterator = iterator_base<value_type, CLCI>;
    using const_iterator = iterator_base<const value_type, CLCI>;

public: /* Methods: */

    TreeNodeSeqView () { }

    explicit TreeNodeSeqView (const TreeNode::ChildrenList& children)
        : m_begin (children.begin ())
        , m_end (children.end ())
    { }

    TreeNodeSeqView (CLCI begin, CLCI end)
        : m_begin (std::move(begin))
        , m_end (std::move(end))
    { }

    size_type size () const { return std::distance (m_begin, m_end); }

    bool empty () const { return m_begin == m_end; }

    const_reference operator [] (size_type i) const {
        assert (i < size ());
        assert (dynamic_cast<const_pointer>(*(m_begin + i)) != nullptr);
        return *static_cast<const_pointer>(*(m_begin + i));
    }

    reference operator [] (size_type i) {
        assert (i < size ());
        assert (dynamic_cast<pointer>(*(m_begin + i)) != nullptr);
        return *static_cast<pointer>(*(m_begin + i));
    }

    iterator begin () const { return iterator (m_begin); }
    iterator end () const { return iterator (m_end); }
    const_iterator cbegin () const { return iterator (m_begin); }
    const_iterator cend () const { return iterator (m_end); }

private: /* Fields: */
    CLCI m_begin, m_end;
};

/******************************************************************
  TreeNodeInternalUse
******************************************************************/

class TreeNodeInternalUse: public TreeNode {
public: /* Methods: */

    inline TreeNodeInternalUse(const Location & loc)
        : TreeNode(NODE_INTERNAL_USE, loc)
    { }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeInternalUse (m_location);
    }
};

/******************************************************************
  TreeNodeDimensions
******************************************************************/

class TreeNodeDimensions: public TreeNode {
public: /* Methods: */

    inline TreeNodeDimensions(const Location & loc)
        : TreeNode(NODE_DIMENSIONS, loc)
    { }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeDimensions (m_location);
    }
};

/******************************************************************
  TreeNodeLValue
******************************************************************/

// TODO: add the type of the left-hand-side!
class TreeNodeLValue: public TreeNode {
public: /* Methods: */

    TreeNodeLValue(SecrecTreeNodeType type, const Location & loc)
        : TreeNode(type, loc)
        , m_secrecType (nullptr)
    { }

    TypeNonVoid* secrecType () const { return m_secrecType; }
    void setSecrecType (TypeNonVoid* type) {
        assert (type != nullptr);
        m_secrecType = type;
    }

    virtual CGResult codeGenWith (CodeGen& cg, SubscriptInfo& subInfo,
                                  bool& isIndexed) = 0;

private: /* Fields: */
    TypeNonVoid* m_secrecType;
};

/******************************************************************
  TreeNodeLVariable
******************************************************************/

class TreeNodeLVariable: public TreeNodeLValue {
public: /* Methods: */
    explicit TreeNodeLVariable (const Location& loc)
        : TreeNodeLValue (NODE_LVALUE_VARIABLE, loc)
    { }

    TreeNodeIdentifier* identifier () const;


    CGResult codeGenWith (CodeGen& cg, SubscriptInfo& subInfo,
                          bool& isIndexed) override final;

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeLVariable (m_location);
    }
};

/******************************************************************
  TreeNodeLIndex
******************************************************************/

class TreeNodeLIndex: public TreeNodeLValue {
public: /* Methods: */
    explicit TreeNodeLIndex (const Location& loc)
        : TreeNodeLValue (NODE_LVALUE_INDEX, loc)
    { }

    TreeNodeLValue* lvalue () const;
    TreeNodeSubscript* indices () const;

    CGResult codeGenWith (CodeGen& cg, SubscriptInfo& subInfo,
                          bool& isIndexed) override final;

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeLIndex (m_location);
    }
};

/******************************************************************
  TreeNodeLSelect
******************************************************************/

class TreeNodeLSelect: public TreeNodeLValue {
public: /* Methods: */
    explicit TreeNodeLSelect (const Location& loc)
        : TreeNodeLValue (NODE_LVALUE_SELECT, loc)
    { }

    TreeNodeLValue* lvalue () const;
    TreeNodeIdentifier* identifier () const;

    CGResult codeGenWith (CodeGen& cg, SubscriptInfo& subInfo,
                          bool& isIndexed) override final;

protected:
    virtual TreeNode* cloneV () const override final {
        return new TreeNodeLSelect (m_location);
    }
};

/******************************************************************
  TreeNodeSubscript
******************************************************************/

class TreeNodeSubscript: public TreeNode {
public: /* Methods: */

    inline TreeNodeSubscript(const Location & loc)
        : TreeNode(NODE_SUBSCRIPT, loc)
    { }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeSubscript (m_location);
    }
};

/******************************************************************
  TreeNodeIndex
******************************************************************/

class TreeNodeIndex: public TreeNode {
public: /* Methods: */

    inline TreeNodeIndex(SecrecTreeNodeType type, const Location & loc)
        : TreeNode(type, loc)
    { }
};

/******************************************************************
  TreeNodeIndexInt
******************************************************************/

class TreeNodeIndexInt: public TreeNodeIndex {
public: /* Methods: */

    inline TreeNodeIndexInt(const Location & loc)
        : TreeNodeIndex(NODE_INDEX_INT, loc)
    { }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeIndexInt (m_location);
    }
};

/******************************************************************
  TreeNodeIndexSlice
******************************************************************/

class TreeNodeIndexSlice: public TreeNodeIndex {
public: /* Methods: */

    inline TreeNodeIndexSlice(const Location & loc)
        : TreeNodeIndex(NODE_INDEX_SLICE, loc)
    { }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeIndexSlice (m_location);
    }
};

/******************************************************************
  TreeNodeIdentifier
******************************************************************/

class TreeNodeIdentifier: public TreeNode {
public: /* Methods: */
    inline TreeNodeIdentifier(StringRef value,
                              const Location & loc)
        : TreeNode(NODE_IDENTIFIER, loc)
        , m_value(std::move(value))
    { }

    inline StringRef value() const { return m_value; }

protected:

    bool printHelper(std::ostream & os) const override final;
    void printXmlHelper (std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeIdentifier (m_value, m_location);
    }

private: /* Fields: */
    const StringRef m_value;
};

/******************************************************************
  TreeNodeTypeF
******************************************************************/

class TreeNodeTypeF: public TreeNode {
public: /* Methods: */
    inline TreeNodeTypeF (SecrecTreeNodeType type, const Location & loc)
        : TreeNode (type, loc)
    { }

    bool isVariable () const;
    TreeNodeIdentifier* identifier () const;
    virtual void setTypeContext (TypeContext& cxt) const = 0;
};

/******************************************************************
  TreeNodeTypeF
******************************************************************/

// Unclassified type variable:
class TreeNodeTypeVarF: public TreeNodeTypeF {
public: /* Methods: */
    inline TreeNodeTypeVarF (const Location & loc)
        : TreeNodeTypeF (NODE_TYPEVAR, loc)
        , m_typeVariable (nullptr)
    { }

    SymbolTypeVariable* typeVariable () const { return m_typeVariable; }
    void setTypeVariable (SymbolTypeVariable* tv) { m_typeVariable = tv; }
    void setTypeContext (TypeContext& cxt) const override final;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeTypeVarF (m_location);
    }

private: /* Fields: */
    SymbolTypeVariable* m_typeVariable;
};


/******************************************************************
  TreeNodeSecTypeF
******************************************************************/

class TreeNodeSecTypeF: public TreeNodeTypeF {
public: /* Methods: */
    inline TreeNodeSecTypeF(SecrecTreeNodeType type, const Location & loc)
        : TreeNodeTypeF (type, loc)
        , m_cachedType (nullptr)
    { }

    inline bool isPublic () const { return m_type == NODE_SECTYPE_PUBLIC_F; }
    SecurityType* cachedType () const { return m_cachedType; }
    void setCachedType (SecurityType* ty);

    void setTypeContext (TypeContext& cxt) const override final;

protected:

    bool printHelper(std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeSecTypeF (m_type, m_location);
    }

private: /* Fields: */
    SecurityType*  m_cachedType;
};

/******************************************************************
  TreeNodeDataTypeF
******************************************************************/

class TreeNodeDataTypeF: public TreeNodeTypeF {
public: /* Methods: */
    inline TreeNodeDataTypeF (SecrecTreeNodeType type,
                              const Location & loc)
        : TreeNodeTypeF(type, loc)
        , m_dataType (nullptr)
    { }

    inline DataType* cachedType () const { return m_dataType; }
    inline void setCachedType (DataType* dataType) { m_dataType = dataType; }
    void setTypeContext (TypeContext& cxt) const override final;

private: /* Fields: */
    DataType* m_dataType;
};

/******************************************************************
  TreeNodeDataTypeConstF
******************************************************************/

class TreeNodeDataTypeConstF: public TreeNodeDataTypeF {
public: /* Methods: */
    inline TreeNodeDataTypeConstF (SecrecDataType dataType, const Location & loc)
        : TreeNodeDataTypeF (NODE_DATATYPE_CONST_F, loc)
        , m_secrecDataType (dataType)
    { }

    SecrecDataType secrecDataType () const { return m_secrecDataType; }

protected:
    bool printHelper(std::ostream & os) const override final;
    void printXmlHelper (std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        auto result = new TreeNodeDataTypeConstF (m_secrecDataType, m_location);
        result->setCachedType (cachedType ());
        return result;
    }

private: /* Fields: */
    const SecrecDataType m_secrecDataType;
};

/******************************************************************
  TreeNodeDataTypeVarF
******************************************************************/

class TreeNodeDataTypeVarF: public TreeNodeDataTypeF {
public: /* Methods: */
    inline TreeNodeDataTypeVarF (const Location & loc)
        : TreeNodeDataTypeF (NODE_DATATYPE_VAR_F, loc)
    { }

protected:
    bool printHelper(std::ostream & os) const override final;
    void printXmlHelper (std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeDataTypeVarF (m_location);
    }
};

/******************************************************************
  TreeNodeDataTypeTemplateF
******************************************************************/

class TreeNodeDataTypeTemplateF: public TreeNodeDataTypeF {
public: /* Methods: */
    inline TreeNodeDataTypeTemplateF (const Location & loc)
        : TreeNodeDataTypeF (NODE_DATATYPE_TEMPLATE_F, loc)
    { }

    TreeNodeIdentifier* identifier () const;
    TreeNodeSeqView<TreeNodeTypeArg> arguments () const;

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeDataTypeTemplateF (m_location);
    }
};

/******************************************************************
  TreeNodeDimTypeF
******************************************************************/

class TreeNodeDimTypeF: public TreeNodeTypeF {
public: /* Methods: */
    inline TreeNodeDimTypeF(SecrecTreeNodeType type,
                            const Location & loc)
        : TreeNodeTypeF (type, loc)
        , m_dimType (~ SecrecDimType (0))
    { }

    inline SecrecDimType cachedType () const { return m_dimType; }
    inline void setCachedType (SecrecDimType dimType) { m_dimType = dimType; }
    void setTypeContext (TypeContext& cxt) const override final;

private: /* Fields: */
    SecrecDimType m_dimType;
};

/******************************************************************
  TreeNodeDimTypeConstF
******************************************************************/

class TreeNodeDimTypeConstF: public TreeNodeDimTypeF {
public: /* Methods: */
    inline TreeNodeDimTypeConstF(SecrecDimType dimType,
                                 const Location & loc)
        : TreeNodeDimTypeF (NODE_DIMTYPE_CONST_F, loc)
    { setCachedType (dimType); }

protected:
    bool printHelper(std::ostream & os) const override final;
    void printXmlHelper (std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeDimTypeConstF (cachedType (), m_location);
    }
};

/******************************************************************
  TreeNodeDimTypeVarF
******************************************************************/

class TreeNodeDimTypeVarF: public TreeNodeDimTypeF {
public: /* Methods: */
    inline TreeNodeDimTypeVarF (const Location & loc)
        : TreeNodeDimTypeF (NODE_DIMTYPE_VAR_F, loc)
    { }

protected:
    bool printHelper(std::ostream & os) const override final;
    void printXmlHelper (std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeDimTypeVarF (m_location);
    }
};

/******************************************************************
  TreeNodeType
******************************************************************/

class TreeNodeType : public TreeNode {
public: /* Methods: */
    inline TreeNodeType(SecrecTreeNodeType type,
                        const Location & loc)
        : TreeNode(type, loc)
        , m_cachedType (nullptr)
    { }

    Type* secrecType () const;
    TreeNodeSeqView<TreeNodeTypeF> types () const;
    TreeNodeSecTypeF* secType () const;
    TreeNodeDataTypeF* dataType () const;
    TreeNodeDimTypeF* dimType () const;
    bool isNonVoid () const;

    void typeString(std::ostream& os) const;

protected:

    friend class TypeChecker;

protected: /* Fields: */

    Type* m_cachedType;
};

/******************************************************************
  TreeNodeTypeType
******************************************************************/

/// Non-void types.
class TreeNodeTypeType: public TreeNodeType {
public: /* Methods: */
    explicit inline TreeNodeTypeType(const Location & loc)
        : TreeNodeType(NODE_TYPETYPE, loc) {}

protected:

    bool printHelper(std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeTypeType (m_location);
    }
};

/******************************************************************
  TreeNodeTypeVoid
******************************************************************/

class TreeNodeTypeVoid: public TreeNodeType {
public: /* Methods: */
    explicit inline TreeNodeTypeVoid(const Location & loc)
        : TreeNodeType(NODE_TYPEVOID, loc)
    { }

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeTypeVoid (m_location);
    }
};

/******************************************************************
  TreeNodeTypeArg
******************************************************************/

class TreeNodeTypeArg: public TreeNode {
    friend class TypeChecker;
public: /* Methods: */
    explicit inline TreeNodeTypeArg(SecrecTreeNodeType type,
                                    const Location & loc)
        : TreeNode(type, loc)
        , m_typeArgument (nullptr)
    { }

    ~TreeNodeTypeArg ();

    bool hasTypeArgument () const { return m_typeArgument != nullptr; }
    const TypeArgument& typeArgument () const;
    void setTypeArgument (const TypeArgument& typeArgument);

private: /* Types: */
    TypeArgument* m_typeArgument;
};

/******************************************************************
  TreeNodeTypeArgVar
******************************************************************/

class TreeNodeTypeArgVar: public TreeNodeTypeArg {
public: /* Methods: */
    explicit inline TreeNodeTypeArgVar(const Location & loc)
        : TreeNodeTypeArg(NODE_TYPE_ARG_VAR, loc)
    { }

    TreeNodeIdentifier* identifier () const;

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeTypeArgVar (m_location);
    }
};

/******************************************************************
  TreeNodeTypeArgTemplate
******************************************************************/

class TreeNodeTypeArgTemplate: public TreeNodeTypeArg {
public: /* Methods: */
    explicit inline TreeNodeTypeArgTemplate(const Location & loc)
        : TreeNodeTypeArg(NODE_TYPE_ARG_TEMPLATE, loc)
    { }

    TreeNodeIdentifier* identifier () const;
    TreeNodeSeqView<TreeNodeTypeArg> arguments () const;

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeTypeArgTemplate (m_location);
    }
};

/******************************************************************
  TreeNodeTypeArgDataTypeConst
******************************************************************/

class TreeNodeTypeArgDataTypeConst: public TreeNodeTypeArg {
public: /* Methods: */
    explicit inline TreeNodeTypeArgDataTypeConst(SecrecDataType dataType,
                                                 const Location & loc)
        : TreeNodeTypeArg(NODE_TYPE_ARG_DATA_TYPE_CONST, loc)
        , m_secrecDataType (dataType)
    { }

    SecrecDataType secrecDataType () const { return m_secrecDataType; }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeTypeArgDataTypeConst (m_secrecDataType, m_location);
    }

private: /* Fields: */
    const SecrecDataType m_secrecDataType;
};

/******************************************************************
  TreeNodeTypeArgDimTypeConst
******************************************************************/

class TreeNodeTypeArgDimTypeConst: public TreeNodeTypeArg {
public: /* Methods: */
    explicit inline TreeNodeTypeArgDimTypeConst(SecrecDimType dimType,
                                                const Location & loc)
        : TreeNodeTypeArg(NODE_TYPE_ARG_DIM_TYPE_CONST, loc)
        , m_secrecDimType (dimType)
    { }

    SecrecDimType secrecDimType () const { return m_secrecDimType; }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeTypeArgDimTypeConst (m_secrecDimType, m_location);
    }

private: /* Fields: */
    const SecrecDimType m_secrecDimType;
};

/******************************************************************
  TreeNodeTypeArgPublic
******************************************************************/

class TreeNodeTypeArgPublic: public TreeNodeTypeArg {
public: /* Methods: */
    explicit inline TreeNodeTypeArgPublic(const Location & loc)
        : TreeNodeTypeArg(NODE_TYPE_ARG_PUBLIC, loc)
    { }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeTypeArgPublic (m_location);
    }
};

/******************************************************************
  TreeNodeExpr
******************************************************************/

/// Representation for expressions, also tracks type of resulting
/// value (if there is one).
class TreeNodeExpr: public TreeNode, public TypeContext {

    friend class TypeChecker;

public: /* Methods: */

    inline TreeNodeExpr(SecrecTreeNodeType type,
                        const Location & loc)
        : TreeNode (type, loc)
        , m_resultType (nullptr)
    { }

    void instantiateDataType (Context& cxt, DataType* dType);

    // If possible instantiate abstract data type to given concrete data type
    void instantiateDataType (Context& cxt, SecrecDataType dType = DATATYPE_INT64);

    bool haveResultType() const { return m_resultType != nullptr; }
    bool havePublicBoolType() const;
    Type* resultType() const;

    virtual CGResult codeGenWith (CodeGen& cg) = 0;
    virtual CGBranchResult codeGenBoolWith (CodeGen&);

protected: /* Methods: */

    virtual void instantiateDataTypeV (Context&, SecrecDataType) {
        assert ("ICE! data types should not be instantiated on given tree node type");
    }

    void setResultType(Type *type);
    void resetDataType (Context& cxt, SecrecDataType dType);

protected: /* Fields: */

    Type*   m_resultType; ///< Type of resulting value.
};

/******************************************************************
  TreeNodeExprNone
******************************************************************/

class TreeNodeExprNone: public TreeNodeExpr {
public: /* Methods: */

    inline TreeNodeExprNone(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_NONE, loc)
    { }

    CGResult codeGenWith (CodeGen& cg) override final;

protected:
    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeExprNone (m_location);
    }
};

/******************************************************************
  TreeNodeStmt
******************************************************************/

/// Statements.
class TreeNodeStmt: public TreeNode {
public: /* Methods: */
    inline TreeNodeStmt(SecrecTreeNodeType type, const Location & loc)
        : TreeNode(type, loc) { }
    virtual ~TreeNodeStmt() { }

    virtual CGStmtResult codeGenWith (CodeGen&) = 0;
};

/******************************************************************
  TreeNodeExprArrayConstructor
******************************************************************/

class TreeNodeExprArrayConstructor : public TreeNodeExpr {
public: /* Methods: */

    inline explicit TreeNodeExprArrayConstructor(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_ARRAY_CONSTRUCTOR, loc) { }

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeSeqView<TreeNodeExpr> expressions () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprArrayConstructor (m_location);
    }

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;
};

/******************************************************************
  TreeNodeExprInt
******************************************************************/

class TreeNodeExprInt: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprInt(uint64_t value, const Location & loc)
        : TreeNodeExpr(NODE_LITE_INT, loc), m_value(value) {}

    inline uint64_t value() const { return m_value; }

    CGResult codeGenWith (CodeGen& cg) override final;

protected:

    bool printHelper(std::ostream & os) const override final;
    void printXmlHelper (std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeExprInt (m_value, m_location);
    }

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;

private: /* Fields: */
    uint64_t m_value;
};

/******************************************************************
  TreeNodeExprSelection
******************************************************************/

class TreeNodeExprSelection: public TreeNodeExpr {
public: /* Methods: */
    explicit TreeNodeExprSelection (const Location & loc)
        : TreeNodeExpr(NODE_EXPR_SELECTION, loc)
    { }

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;
    TreeNodeIdentifier* identifier () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprSelection (m_location);
    }

    TreeNodeLValue* makeLValueV (Location& loc) const override final;
};

/******************************************************************
  TreeNodeExprAssign
******************************************************************/

class TreeNodeExprAssign: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprAssign(SecrecTreeNodeType type, const Location & loc)
        : TreeNodeExpr(type, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeExpr* rightHandSide () const;
    TreeNodeLValue* leftHandSide () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprAssign (m_type, m_location);
    }
};

/******************************************************************
  TreeNodeExprCast
******************************************************************/

/**
 * For now both data and security type casts.
 * \todo Split those when type checking.
 */
class TreeNodeExprCast: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprCast (const Location & loc)
        : TreeNodeExpr(NODE_EXPR_CAST, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;
    TreeNodeDataTypeF* dataType () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprCast (m_location);
    }
};

/******************************************************************
  TreeNodeExprIndex
******************************************************************/

class TreeNodeExprIndex: public TreeNodeExpr {
public:
    inline TreeNodeExprIndex(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_INDEX, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;
    TreeNodeSubscript* indices () const;

protected:

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprIndex (m_location);
    }

    TreeNodeLValue* makeLValueV (Location& loc) const override final;
};

/******************************************************************
  TreeNodeExprSize
******************************************************************/

class TreeNodeExprSize: public TreeNodeExpr {
public:
    inline TreeNodeExprSize(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_SIZE, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprSize (m_location);
    }
};

/******************************************************************
  TreeNodeExprShape
******************************************************************/

class TreeNodeExprShape: public TreeNodeExpr {
public:
    inline TreeNodeExprShape(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_SHAPE, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprShape (m_location);
    }
};

/******************************************************************
  TreeNodeExprCat
******************************************************************/

class TreeNodeExprCat: public TreeNodeExpr {
public:
    inline TreeNodeExprCat(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_CAT, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* leftExpression () const;
    TreeNodeExpr* rightExpression () const;
    TreeNodeExprInt* dimensionality () const;

protected:

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprCat (m_location);
    }
};

/******************************************************************
  TreeNodeExprReshape
******************************************************************/

class TreeNodeExprReshape: public TreeNodeExpr {
public:
    inline TreeNodeExprReshape(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_RESHAPE, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* reshapee () const;

    TreeNodeSeqView<TreeNodeExpr> dimensions () const;

protected:

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprReshape (m_location);
    }
};

/******************************************************************
  TreeNodeExprToString
******************************************************************/

class TreeNodeExprToString: public TreeNodeExpr {
public:
    inline TreeNodeExprToString(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_TOSTRING, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprToString (m_location);
    }
};

/******************************************************************
  OverloadableOperator
******************************************************************/

class OverloadableOperator {
public: /* Methods: */

    OverloadableOperator ()
        : m_operator (SCOP_NONE)
        , m_symbolProcedure (nullptr)
    { }

    virtual ~OverloadableOperator() { }

    SecrecOperator getOperator () const;

    std::string operatorName () const;

    SymbolProcedure* procSymbol () const {
        return m_symbolProcedure;
    }

    void setProcSymbol (SymbolProcedure* proc) {
        m_symbolProcedure = proc;
    }

    bool isOverloaded () const { return procSymbol () != nullptr; }

protected:

    virtual SecrecOperator getOperatorV () const = 0;

private: /* Fields: */

    mutable SecrecOperator m_operator; // cached
    SymbolProcedure* m_symbolProcedure;
};

/******************************************************************
  TreeNodeExprBinary
******************************************************************/

/// Binary expressions.
class TreeNodeExprBinary: public TreeNodeExpr,
                          public OverloadableOperator {
public: /* Methods: */
    inline TreeNodeExprBinary(SecrecTreeNodeType type,
                              const Location & loc)
        : TreeNodeExpr(type, loc)
    { }

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeExpr* leftExpression () const;
    TreeNodeExpr* rightExpression () const;
    const char *operatorString() const;
    const char *operatorLongString() const;

protected:

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;

    SecrecOperator getOperatorV () const override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprBinary (m_type, m_location);
    }
};

/******************************************************************
  TreeNodeExprBool
******************************************************************/

class TreeNodeExprBool: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprBool(bool value, const Location & loc)
        : TreeNodeExpr(NODE_LITE_BOOL, loc), m_value(value) {}

    inline bool value() const { return m_value; }

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

protected:

    void printXmlHelper (std::ostream & os) const override final;
    bool printHelper(std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeExprBool (m_value, m_location);
    }


private: /* Fields: */
    const bool m_value;
};

/******************************************************************
  TreeNodeExprClassify
******************************************************************/

/**
 * Classify expression. Those do not occur naturally in code,
 * always added by type checker.
 */
class TreeNodeExprClassify: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprClassify(SecurityType* ty,
                                const Location & loc)
        : TreeNodeExpr(NODE_EXPR_CLASSIFY, loc)
    {
        m_contextSecType = ty;
    }

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final;
};

/******************************************************************
  TreeNodeExprDeclassify
******************************************************************/

class TreeNodeExprDeclassify: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprDeclassify(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_DECLASSIFY, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprDeclassify (m_location);
    }
};

/******************************************************************
  TreeNodeExprProcCall
******************************************************************/

class TreeNodeExprProcCall: public TreeNodeExpr {
public: /* Methods: */
    explicit inline TreeNodeExprProcCall(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_PROCCALL, loc)
        , m_procedure (nullptr)
    { }

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeIdentifier* procName () const;
    TreeNodeSeqView<TreeNodeExpr> params () const;

    void setProcedure (SymbolProcedure* proc) {
        m_procedure = proc;
    }

    inline SymbolProcedure *symbolProcedure() const {
        return m_procedure;
    }

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprProcCall (m_location);
    }

protected: /* Fields: */
    SymbolProcedure *m_procedure;
};

/******************************************************************
  TreeNodeExprRVariable
******************************************************************/

/// Variable in right hand side.
class TreeNodeExprRVariable: public TreeNodeExpr {
public: /* Methods: */
    explicit inline TreeNodeExprRVariable(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_RVARIABLE, loc)
        , m_valueSymbol (nullptr)
    { }

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeIdentifier* identifier () const;
    void setValueSymbol (Symbol* sym) { m_valueSymbol = sym; }
    Symbol* valueSymbol () const { return m_valueSymbol; }

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprRVariable (m_location);
    }

    TreeNodeLValue* makeLValueV (Location&) const override final;

private: /* Fields: */
    Symbol* m_valueSymbol;
};

/******************************************************************
  TreeNodeExprString
******************************************************************/

/// String constants.
class TreeNodeExprString: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprString(const Location & loc)
        : TreeNodeExpr(NODE_LITE_STRING, loc)
    { }

    TreeNodeSeqView<TreeNodeStringPart> parts () const;

    bool isConstant () const;
    CGResult codeGenWith (CodeGen& cg) override final;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprString (m_location);
    }
};

/******************************************************************
  TreeNodeStringPart
******************************************************************/

class TreeNodeStringPart: public TreeNode {
public: /* Methods: */

    TreeNodeStringPart (SecrecTreeNodeType type,
                        const Location& loc)
        : TreeNode (type, loc)
    { }

    virtual bool isConstant () const = 0;
    virtual StringRef staticValue () const = 0;
    virtual CGResult codeGenWith (CodeGen& cg) = 0;
};

/******************************************************************
  TreeNodeStringPartFragment
******************************************************************/

class TreeNodeStringPartFragment: public TreeNodeStringPart {
public: /* Methods: */

    TreeNodeStringPartFragment (StringRef value, const Location& loc)
        : TreeNodeStringPart (NODE_STRING_PART_FRAGMENT, loc)
        , m_value (std::move(value))
    { }

    bool isConstant () const override final { return true; }
    StringRef staticValue () const override final { return m_value; }
    CGResult codeGenWith (CodeGen& cg) override final;

protected:

    bool printHelper(std::ostream & os) const override final;
    void printXmlHelper (std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeStringPartFragment (m_value, m_location);
    }

public: /* Private: */
    const StringRef m_value;
};

/******************************************************************
  TreeNodeStringPartIdentifier
******************************************************************/

class TreeNodeStringPartIdentifier: public TreeNodeStringPart {
public: /* Methods: */

    TreeNodeStringPartIdentifier (StringRef name, const Location& loc)
        : TreeNodeStringPart (NODE_STRING_PART_IDENTIFIER, loc)
        , m_name (std::move(name))
        , m_value (nullptr)
        , m_secrecType (nullptr)
    { }

    StringRef name () const { return m_name; }
    ConstantString* value () const { return m_value; }
    void setValue (ConstantString* value) { m_value = value; }
    TypeNonVoid* secrecType () const { return m_secrecType; }
    void setSecrecType (TypeNonVoid* secrecType) { m_secrecType = secrecType; }

    bool isConstant () const override final { return m_value != nullptr; }
    StringRef staticValue () const override final;
    CGResult codeGenWith (CodeGen& cg) override final;

protected:

    bool printHelper(std::ostream & os) const override final;
    void printXmlHelper (std::ostream & os) const override final;
    TreeNode* cloneV () const override final {
        return new TreeNodeStringPartIdentifier (m_name, m_location);
    }

public: /* Private: */
    const StringRef m_name;
    ConstantString* m_value;
    TypeNonVoid*    m_secrecType;
};

/******************************************************************
  TreeNodeExprFloat
******************************************************************/

class TreeNodeExprFloat: public TreeNodeExpr {
public: /* Methods: */
    TreeNodeExprFloat (StringRef value,
                       const Location & loc)
        : TreeNodeExpr (NODE_LITE_FLOAT, loc)
        , m_value (std::move(value))
    { }

    inline StringRef value () const { return m_value; }
    CGResult codeGenWith (CodeGen & cg) override final;

protected:
    void instantiateDataTypeV(Context & cxt, SecrecDataType dType) override final;
    bool printHelper(std::ostream & os) const override final;
    void printXmlHelper (std::ostream & os) const override final;
    TreeNode * cloneV () const override final {
        return new TreeNodeExprFloat (m_value, m_location);
    }

private: /* Fields: */
    const StringRef m_value;
};

/******************************************************************
  TreeNodeExprTernary
******************************************************************/

class TreeNodeExprTernary: public TreeNodeExpr {
public: /* Methods: */
    explicit inline TreeNodeExprTernary(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_TERNIF, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeExpr* conditional () const;
    TreeNodeExpr* trueBranch () const;
    TreeNodeExpr* falseBranch () const;

protected:

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprTernary (m_location);
    }
};

/******************************************************************
  TreeNodeExprPrefix
******************************************************************/

/// Prefix increment and decrement.
class TreeNodeExprPrefix: public TreeNodeExpr,
                          public OverloadableOperator {
public: /* Methods: */
    inline TreeNodeExprPrefix(SecrecTreeNodeType type,
                              const Location & loc)
        : TreeNodeExpr(type, loc)
    { }

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeLValue* lvalue () const;

protected:

    SecrecOperator getOperatorV () const override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprPrefix (m_type, m_location);
    }
};

/******************************************************************
  TreeNodeExprPostfix
******************************************************************/

/// Postfix increment and decrement.
class TreeNodeExprPostfix: public TreeNodeExpr,
                           public OverloadableOperator {
public: /* Methods: */
    inline TreeNodeExprPostfix(SecrecTreeNodeType type,
                               const Location & loc)
        : TreeNodeExpr(type, loc)
    { }

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeLValue* lvalue () const;

protected:

    SecrecOperator getOperatorV () const override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprPostfix (m_type, m_location);
    }
};

/******************************************************************
  TreeNodeExprUnary
******************************************************************/

/// Unary expressions such as regular (logical) negation.
class TreeNodeExprUnary: public TreeNodeExpr,
                         public OverloadableOperator {
public: /* Methods: */
    inline TreeNodeExprUnary(SecrecTreeNodeType type,
                             const Location & loc)
        : TreeNodeExpr(type, loc)
    { }

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;

    SecrecOperator getOperatorV () const override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprUnary (m_type, m_location);
    }
};

/******************************************************************
  TreeNodeExprDomainID
******************************************************************/

/// Unary expressions such as regular (logical) negation.
class TreeNodeExprDomainID: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprDomainID(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_DOMAINID, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeSecTypeF* securityType () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprDomainID (m_location);
    }
};

/******************************************************************
  TreeNodeExprQualified
******************************************************************/

class TreeNodeExprQualified : public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprQualified(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_TYPE_QUAL, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;
    CGBranchResult codeGenBoolWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;
    TreeNodeSeqView<TreeNodeTypeF> types () const;

protected:

    void instantiateDataTypeV (Context &cxt, SecrecDataType dType) override final;

    TreeNode* cloneV () const override final {
        return new TreeNodeExprQualified (m_location);
    }
};

/******************************************************************
  TreeNodeExprStrlen
******************************************************************/

// TODO: make this a builtin function
class TreeNodeExprStrlen : public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprStrlen(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_STRLEN, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;
    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprStrlen (m_location);
    }
};

/******************************************************************
  TreeNodeExprStringFromBytes
******************************************************************/

// TODO: make this a builtin function
class TreeNodeExprStringFromBytes : public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprStringFromBytes(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_STRING_FROM_BYTES, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprStringFromBytes (m_location);
    }
};

/******************************************************************
  TreeNodeExprBytesFromString
******************************************************************/

// TODO: make this a builtin function
class TreeNodeExprBytesFromString : public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprBytesFromString(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_BYTES_FROM_STRING, loc) {}

    CGResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeExprBytesFromString (m_location);
    }
};

/******************************************************************
  TreeNodeKind
******************************************************************/

class TreeNodeKind : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeKind(const Location & loc)
        : TreeNode (NODE_KIND, loc) { }

    TreeNodeIdentifier* identifier () const;

    TreeNodeSeqView<TreeNodeDataTypeDecl> types () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeKind (m_location);
    }
};

/******************************************************************
  TreeNodeDomain
******************************************************************/

class TreeNodeDomain : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeDomain(const Location & loc)
        : TreeNode (NODE_DOMAIN, loc) { }

    TreeNodeIdentifier* domainIdentifier () const;
    TreeNodeIdentifier* kindIdentifier () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeDomain (m_location);
    }
};

/******************************************************************
  TreeNodeAttribute
******************************************************************/

class TreeNodeAttribute: public TreeNode {
public: /* Methods: */

    explicit TreeNodeAttribute (const Location & loc)
        :  TreeNode (NODE_ATTRIBUTE, loc)
    { }

    TreeNodeType* type () const;
    TreeNodeIdentifier* identifier () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeAttribute (m_location);
    }
};

/******************************************************************
  TreeNodeStructDecl
******************************************************************/

class TreeNodeStructDecl: public TreeNode {
public: /* Methods: */

    explicit TreeNodeStructDecl (const Location & loc)
        : TreeNode (NODE_STRUCT_DECL, loc)
        , m_containingModule (nullptr)
    { }

    TreeNodeSeqView<TreeNodeQuantifier> quantifiers() const;
    TreeNodeIdentifier* identifier () const;
    TreeNodeSeqView<TreeNodeAttribute> attributes () const;

    bool isQuantified () const {
        return !quantifiers ().empty ();
    }

    ModuleInfo* containingModule () const {
        return m_containingModule;
    }

    void setContainingModule (ModuleInfo& mod) {
        m_containingModule = &mod;
    }

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStructDecl (m_location);
    }

private: /* Fields: */
    ModuleInfo* m_containingModule;
};

/******************************************************************
  TreeNodeProcDef
******************************************************************/

/// Procedure definition.
class TreeNodeProcDef: public TreeNode {
protected: /* Methods: */

    explicit inline TreeNodeProcDef(SecrecTreeNodeType type,
                                    const Location & loc)
        : TreeNode (type, loc)
        , m_cachedType(nullptr)
        , m_procSymbol (nullptr)
    {
        setContainingProcedureDirectly(this);
    }

public:

    explicit inline TreeNodeProcDef(const Location & loc)
        : TreeNode(NODE_PROCDEF, loc)
        , m_cachedType(nullptr)
        , m_procSymbol (nullptr)
    {
        setContainingProcedureDirectly(this);
    }

    inline void resetParent(TreeNode *parent) override final {
        setParentDirectly(parent);
    }

    void setSymbol (SymbolProcedure* sym) {
        assert (sym != nullptr);
        m_procSymbol = sym;
    }

    SymbolProcedure* symbol () const {
        return m_procSymbol;
    }

    StringRef procedureName() const;
    const std::string printableSignature() const;

    inline bool haveProcedureType() const {
        return m_cachedType != nullptr;
    }

    TypeProc* procedureType() const {
        assert(m_cachedType != nullptr);
        return m_cachedType;
    }

    bool isOperator() const {
        return m_type == NODE_OPDEF;
    }

    TreeNodeIdentifier* identifier () const;
    TreeNodeType* returnType () const;
    TreeNodeStmt* body () const;
    TreeNodeSeqView<TreeNodeStmtDecl> params () const;

protected: /* Methods: */

    friend class TypeChecker;

    virtual TreeNode* cloneV () const override {
        return new TreeNodeProcDef (m_location);
    }

protected: /* Fields: */
    TypeProc*         m_cachedType;
    SymbolProcedure*  m_procSymbol;
};

/******************************************************************
  TreeNodeOpDef
******************************************************************/

class TreeNodeOpDef: public TreeNodeProcDef {
public: /* Methods: */

    explicit inline TreeNodeOpDef(SecrecOperator op,
                                  const Location & loc)
        : TreeNodeProcDef (NODE_OPDEF, loc)
        , m_operator (op)
    { }

    SecrecOperator getOperator () const { return m_operator; }

    CGStmtResult codeGenWith (CodeGen& cg);

protected: /* Methods: */

    friend class TypeChecker;

    TreeNode* cloneV () const override final {
        return new TreeNodeOpDef (m_operator, m_location);
    }

protected: /* Fields: */
    const SecrecOperator m_operator;
};

/******************************************************************
  TreeNodeQuantifier
******************************************************************/

class TreeNodeQuantifier : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeQuantifier(SecrecTreeNodeType type, const Location & loc)
        : TreeNode(type, loc) { }

    virtual void printQuantifier (std::ostream& os) const = 0;

    TreeNodeIdentifier* typeVariable () const;

    bool isDataTypeQuantifier () const { return m_type == NODE_TEMPLATE_QUANTIFIER_DATA; }

    bool isDomainQuantifier () const { return m_type == NODE_TEMPLATE_QUANTIFIER_DOMAIN; }

protected:
    virtual TreeNode* cloneV () const = 0;
};

/******************************************************************
  TreeNodeQuantifierDomain
******************************************************************/

class TreeNodeQuantifierDomain : public TreeNodeQuantifier {
public: /* Methods: */
    explicit inline TreeNodeQuantifierDomain(const Location & loc)
        : TreeNodeQuantifier(NODE_TEMPLATE_QUANTIFIER_DOMAIN, loc) {}

    // will equal to zero, if kind not specified
    TreeNodeIdentifier* kind () const;

    void printQuantifier (std::ostream & os) const override final;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeQuantifierDomain (m_location);
    }
};

/******************************************************************
  TreeNodeQuantifierDim
******************************************************************/

class TreeNodeQuantifierDim : public TreeNodeQuantifier {
public: /* Methods: */
    explicit inline TreeNodeQuantifierDim (const Location & loc)
        : TreeNodeQuantifier(NODE_TEMPLATE_QUANTIFIER_DIM, loc) {}

    void printQuantifier (std::ostream & os) const override final;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeQuantifierDim (m_location);
    }
};

/******************************************************************
  TreeNodeQuantifierData
******************************************************************/

class TreeNodeQuantifierData : public TreeNodeQuantifier {
public: /* Methods: */
    explicit inline TreeNodeQuantifierData (const Location & loc)
        : TreeNodeQuantifier(NODE_TEMPLATE_QUANTIFIER_DATA, loc) {}

    void printQuantifier (std::ostream & os) const override final;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeQuantifierData (m_location);
    }
};

/******************************************************************
  TreeNodeTemplate
******************************************************************/

class TreeNodeTemplate : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeTemplate(const Location & loc)
        : TreeNode(NODE_TEMPLATE_DECL, loc)
        , m_containingModule (nullptr)
    { }

    TreeNodeProcDef* body () const;

    TreeNodeSeqView<TreeNodeQuantifier> quantifiers() const;

    ModuleInfo* containingModule () const {
        return m_containingModule;
    }

    void setContainingModule (ModuleInfo& mod) {
        m_containingModule = &mod;
    }

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeTemplate (m_location);
    }

private: /* Fields: */
    ModuleInfo* m_containingModule;
};

/******************************************************************
  TreeNodeProgram
******************************************************************/

/// Representation of program.
class TreeNodeProgram: public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeProgram(const Location & loc)
        : TreeNode(NODE_PROGRAM, loc) {}

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeProgram (m_location);
    }
};

/******************************************************************
  TreeNodeImport
******************************************************************/

class TreeNodeImport : public TreeNode {
public: /* Methods: */
    inline TreeNodeImport(const Location & loc)
        : TreeNode (NODE_IMPORT, loc)
    { }

    StringRef name () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeImport (m_location);
    }
};

/******************************************************************
  TreeNodeModule
******************************************************************/

class TreeNodeModule : public TreeNode {
public: /* Methods: */
    inline TreeNodeModule(const Location & loc)
        : TreeNode (NODE_MODULE, loc)
    { }

    ~TreeNodeModule();

    bool hasName () const;
    StringRef name () const;
    TreeNodeProgram* program () const;

    void addGeneratedInstance (TreeNodeProcDef * instance) {
        m_generatedInstances.push_back(instance);
    }

protected:

    TreeNode* cloneV () const override final {
        assert (false && "ICE: cloning TreeNodeModule!");
        return new TreeNodeModule (m_location);
    }

    std::vector<TreeNodeProcDef*> m_generatedInstances;
};

/******************************************************************
  TreeNodeStmtBreak
******************************************************************/

/// Break statement.
class TreeNodeStmtBreak: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtBreak(const Location & loc)
        : TreeNodeStmt(NODE_STMT_BREAK, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtBreak (m_location);
    }
};

/******************************************************************
  TreeNodeStmtCompound
******************************************************************/

/// Compund statement. Anything between curly bracers.
class TreeNodeStmtCompound: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtCompound(const Location & loc)
        : TreeNodeStmt(NODE_STMT_COMPOUND, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtCompound (m_location);
    }
};

/******************************************************************
  TreeNodeStmtContinue
******************************************************************/

/// Continue statement.
class TreeNodeStmtContinue: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtContinue(const Location & loc)
        : TreeNodeStmt(NODE_STMT_CONTINUE, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtContinue (m_location);
    }
};

/******************************************************************
  TreeNodeVarInit
******************************************************************/

class TreeNodeVarInit : public TreeNode {
public: /* Methods: */
    TreeNodeVarInit (const Location & loc)
        : TreeNode (NODE_VAR_INIT, loc)
    { }

    StringRef variableName() const;
    TreeNodeSeqView<TreeNodeExpr> shape () const;
    bool hasRightHandSide() const;
    TreeNodeExpr* rightHandSide () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeVarInit (m_location);
    }
};

/******************************************************************
  TreeNodeStmtDecl
******************************************************************/

/// Declaration statement. Also tracks if the scope is global.
class TreeNodeStmtDecl: public TreeNodeStmt {
public: /* Methods: */

    explicit TreeNodeStmtDecl (const Location & loc, bool global = false, bool procParam = false)
        : TreeNodeStmt (NODE_DECL, loc)
        , m_type (nullptr)
        , m_global (global)
        , m_procParam (procParam)
    { }

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    inline TypeNonVoid* resultType() const {
        assert(m_type != nullptr);
        return m_type;
    }

    inline bool haveResultType() const { return m_type != nullptr; }
    void setResultType (TypeNonVoid* type) { m_type = type; }

    inline bool global() const { return m_global; }
    inline void setGlobal(bool isGlobal = true) {
        m_global = isGlobal;
    }
    inline bool procParam() const { return m_procParam; }
    inline void setProcParam(bool procParam = true) {
        m_procParam = procParam;
    }

    /// Returns the first variable initializer.
    TreeNodeVarInit* initializer () const;
    TreeNodeSeqView<TreeNodeVarInit> initializers () const;
    TreeNodeType* varType () const;
    StringRef variableName() const;
    TreeNodeSeqView<TreeNodeExpr> shape () const;
    TreeNodeExpr* rightHandSide () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtDecl (m_location, m_global, m_procParam);
    }

protected: /* Fields: */
    TypeNonVoid *m_type;
    bool m_global;
    bool m_procParam;
};

/******************************************************************
  TreeNodeStmtDoWhile
******************************************************************/

/// Do-while statement.
class TreeNodeStmtDoWhile: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtDoWhile(const Location & loc)
        : TreeNodeStmt(NODE_STMT_DOWHILE, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* conditional () const;
    TreeNodeStmt* body () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtDoWhile (m_location);
    }
};

/******************************************************************
  TreeNodeStmtExpr
******************************************************************/

/// Expression statements. Any expression can occur as a statement.
class TreeNodeStmtExpr: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtExpr(const Location & loc)
        : TreeNodeStmt(NODE_STMT_EXPR, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtExpr (m_location);
    }
};

/******************************************************************
  TreeNodeStmtAssert
******************************************************************/

/// Assert statement.
class TreeNodeStmtAssert: public TreeNodeStmt {
public: /* Methods: */
    explicit TreeNodeStmtAssert(const Location & loc)
        : TreeNodeStmt(NODE_STMT_ASSERT, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtAssert (m_location);
    }
};

/******************************************************************
  TreeNodeStmtFor
******************************************************************/

/// For statement.
class TreeNodeStmtFor: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtFor(const Location & loc)
        : TreeNodeStmt(NODE_STMT_FOR, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    TreeNode* initializer () const;
    TreeNodeExpr* conditional () const;
    TreeNodeExpr* iteratorExpr () const;
    TreeNodeStmt* body () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtFor (m_location);
    }
};

/******************************************************************
  TreeNodeStmtIf
******************************************************************/

/// If and if-then-else statement.
class TreeNodeStmtIf: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtIf(const Location & loc)
        : TreeNodeStmt(NODE_STMT_IF, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* conditional () const;
    TreeNodeStmt* trueBranch () const;
    TreeNodeStmt* falseBranch () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtIf (m_location);
    }
};

/******************************************************************
  TreeNodeStmtReturn
******************************************************************/

/// Regular return and value returning statement.
class TreeNodeStmtReturn: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtReturn(const Location & loc)
        : TreeNodeStmt(NODE_STMT_RETURN, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    bool hasExpression() const;
    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtReturn (m_location);
    }
};

/******************************************************************
  TreeNodeStmtWhile
******************************************************************/

/// While statement.
class TreeNodeStmtWhile: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtWhile(const Location & loc)
        : TreeNodeStmt(NODE_STMT_WHILE, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExpr* conditional () const;
    TreeNodeStmt* body () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtWhile (m_location);
    }
};

/******************************************************************
  TreeNodeStmtPrint
******************************************************************/

class TreeNodeStmtPrint: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtPrint(const Location & loc)
        : TreeNodeStmt(NODE_STMT_PRINT, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    TreeNodeSeqView<TreeNodeExpr> expressions ();

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtPrint (m_location);
    }
};

/******************************************************************
  TreeNodeSyscallParam
******************************************************************/

class TreeNodeSyscallParam : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeSyscallParam(SecrecTreeNodeType type, const Location & loc)
        : TreeNode (type, loc) {}

    TreeNodeExpr* expression () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeSyscallParam  (m_type, m_location);
    }
};

/******************************************************************
  TreeNodeStmtSyscall
******************************************************************/

class TreeNodeStmtSyscall : public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtSyscall(const Location & loc)
        : TreeNodeStmt(NODE_STMT_SYSCALL, loc) {}

    CGStmtResult codeGenWith (CodeGen& cg) override final;

    TreeNodeExprString* name () const;
    TreeNodeSeqView<TreeNodeSyscallParam> params () const;

protected:

    TreeNode* cloneV () const override final {
        return new TreeNodeStmtSyscall  (m_location);
    }
};

/******************************************************************
  TreeNodeDataTypeDecl
******************************************************************/

class TreeNodeDataTypeDecl : public TreeNode {
public: /* Methods: */

    explicit inline TreeNodeDataTypeDecl (StringRef typeName, const Location & loc)
        : TreeNode (NODE_DATATYPE_DECL, loc)
        , m_typeName (typeName)
        {}

    StringRef typeName () const { return m_typeName; }

    TreeNodeSeqView<TreeNodeDataTypeDeclParam> parameters () const;

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeDataTypeDecl (m_typeName, m_location);
    }

private: /* Fields: */
    StringRef m_typeName;
};

/******************************************************************
  TreeNodeDataTypeDeclParam
******************************************************************/

class TreeNodeDataTypeDeclParam : public TreeNode {
public: /* Methods: */

    explicit inline TreeNodeDataTypeDeclParam (SecrecTreeNodeType ty, const Location & loc)
        : TreeNode (ty, loc) {}

    bool isPublicParam () const { return type () == NODE_DATATYPE_DECL_PARAM_PUBLIC; }
};

/******************************************************************
  TreeNodeDataTypeDeclParamPublic
******************************************************************/

class TreeNodeDataTypeDeclParamPublic : public TreeNodeDataTypeDeclParam {
public: /* Methods: */

    explicit inline TreeNodeDataTypeDeclParamPublic (SecrecDataType ty, const Location & loc)
        : TreeNodeDataTypeDeclParam (NODE_DATATYPE_DECL_PARAM_PUBLIC, loc)
        , m_dataType (ty)
        {}

    SecrecDataType secrecDataType () const { return m_dataType; }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeDataTypeDeclParamPublic (m_dataType, m_location);
    }

private: /* Fields: */
    const SecrecDataType m_dataType;
};

/******************************************************************
  TreeNodeDataTypeDeclParamSize
******************************************************************/

class TreeNodeDataTypeDeclParamSize : public TreeNodeDataTypeDeclParam {
public: /* Methods: */

    explicit inline TreeNodeDataTypeDeclParamSize (uint64_t size, const Location & loc)
        : TreeNodeDataTypeDeclParam (NODE_DATATYPE_DECL_PARAM_SIZE, loc)
        , m_size (size)
        {}

    uint64_t size () const { return m_size; }

protected:
    TreeNode* cloneV () const override final {
        return new TreeNodeDataTypeDeclParamSize (m_size, m_location);
    }

private: /* Fields: */
    const uint64_t m_size;
};

} /* namespace SecreC */

#endif /* TREENODE_H */
