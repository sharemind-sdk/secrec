#ifndef SECREC_TREENODE_H
#define SECREC_TREENODE_H

#include <cassert>
#include <vector>
#include <string>
#include <boost/range.hpp>

#include "codegenResult.h"
#include "Location.h"
#include "parser.h"
#include "StringRef.h"
#include "typechecker.h"
#include "TypeContext.h"

namespace SecreC {

class CodeGen;
class TypeChecker;
class ModuleInfo;
class TreeNodeProcDef;
class TreeNode;
class SymbolTypeVariable;
class ConstantString;

/******************************************************************
  TreeNode
******************************************************************/

/**
 * \class TreeNode
 * Abstract representation of the SecreC code.
 */
class TreeNode {
public: /* Types: */
    typedef std::vector<TreeNode*> ChildrenList;
    typedef ChildrenList::iterator ChildrenListIterator;
    typedef ChildrenList::const_iterator ChildrenListConstIterator;

public: /* Methods: */
    TreeNode(SecrecTreeNodeType type, const Location & loc);
    virtual ~TreeNode();

    TreeNodeProcDef* containingProcedure() const;
    inline TreeNode* parent() const { return m_parent; }
    inline bool hasParent() const { return m_parent != 0; }
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

protected: /* Methods: */

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

    typedef TreeNode::ChildrenList::iterator        CLI;
    typedef TreeNode::ChildrenList::const_iterator  CLCI;

    template <class value_type, class other_iterator>
    class iterator_base :
        public std::iterator
            < typename std::iterator_traits<other_iterator>::iterator_category
            , value_type
            >
    {
    private: /* Types: */
        typedef iterator_base<value_type, other_iterator> self_type;
        typedef std::iterator<typename std::iterator_traits<other_iterator>::iterator_category, value_type> base_type;
    public: /* Methods: */
        explicit iterator_base (other_iterator iterator)
            : m_iterator (iterator) { }

        typename base_type::reference operator * () const {
            assert (dynamic_cast<typename base_type::pointer>(*m_iterator) != 0);
            return *static_cast<typename base_type::pointer>(*m_iterator);
        }

        typename base_type::pointer operator -> () const {
            assert (dynamic_cast<typename base_type::pointer>(*m_iterator) != 0);
            return static_cast<typename base_type::pointer>(*m_iterator);
        }

        bool operator == (const self_type i) const { return m_iterator == i.m_iterator; }
        bool operator != (const self_type i) const { return m_iterator != i.m_iterator; }
        self_type& operator ++ () {  ++ m_iterator; return *this; }
        self_type operator ++ (int) { const self_type t = *this; ++ m_iterator; return t; }

    private: /* Fields: */
        other_iterator m_iterator;
    };

public: /* Types: */
    typedef SubClass                               value_type;
    typedef SubClass&                              reference;
    typedef const SubClass&                        const_reference;
    typedef SubClass*                              pointer;
    typedef const SubClass*                        const_pointer;
    typedef TreeNode::ChildrenList::size_type      size_type;
    typedef iterator_base<value_type, CLCI>        iterator;
    typedef iterator_base<const value_type, CLCI>  const_iterator;

public: /* Methods: */

    TreeNodeSeqView () { }

    explicit TreeNodeSeqView (const TreeNode::ChildrenList& children)
        : m_begin (children.begin ())
        , m_end (children.end ())
    { }

    TreeNodeSeqView (CLCI begin, CLCI end)
        : m_begin (begin)
        , m_end (end)
    { }

    size_type size () const { return std::distance (m_begin, m_end); }

    bool empty () const { return m_begin == m_end; }

    const_reference operator [] (size_type i) const {
        assert (i < size ());
        assert (dynamic_cast<const_pointer>(*(m_begin + i)) != 0);
        return *static_cast<const_pointer>(*(m_begin + i));
    }

    reference operator [] (size_type i) {
        assert (i < size ());
        assert (dynamic_cast<pointer>(*(m_begin + i)) != 0);
        return *static_cast<pointer>(*(m_begin + i));
    }

    iterator begin () const { return iterator (m_begin); }
    iterator end () const { return iterator (m_end); }
    const_iterator cbegin () const { return iterator (m_begin); }
    const_iterator cend () const { return iterator (m_end); }

private: /* Fields: */
    CLCI m_begin, m_end;
};

template <class BaseClass>
inline typename TreeNodeSeqView<BaseClass>::iterator
        range_begin (TreeNodeSeqView<BaseClass>& x) {
    return x.begin ();
}

template <class BaseClass>
inline typename TreeNodeSeqView<BaseClass>::iterator
        range_begin (const TreeNodeSeqView<BaseClass>& x) {
    return x.begin ();
}

template <class BaseClass>
inline typename TreeNodeSeqView<BaseClass>::iterator
        range_end (TreeNodeSeqView<BaseClass>& x) {
    return x.end ();
}

template <class BaseClass>
inline typename TreeNodeSeqView<BaseClass>::iterator
        range_end (const TreeNodeSeqView<BaseClass>& x) {
    return x.end ();
}

/******************************************************************
  TreeNodeInternalUse
******************************************************************/

class TreeNodeInternalUse: public TreeNode {
public: /* Methods: */

    inline TreeNodeInternalUse(const Location & loc)
        : TreeNode(NODE_INTERNAL_USE, loc)
    { }

protected:
    virtual TreeNode* cloneV () const {
        return new TreeNodeInternalUse (m_location);
    }
};

/******************************************************************
  TreeNodeDimensions
******************************************************************/

// TODO: use this!
class TreeNodeDimensions: public TreeNode {
public: /* Methods: */

    inline TreeNodeDimensions(const Location & loc)
        : TreeNode(NODE_DIMENSIONS, loc)
    { }

protected:
    virtual TreeNode* cloneV () const {
        return new TreeNodeDimensions (m_location);
    }
};

/******************************************************************
  TreeNodeLValue
******************************************************************/

// TODO: use this!
class TreeNodeLValue: public TreeNode {
public: /* Methods: */

    inline TreeNodeLValue(const Location & loc)
        : TreeNode(NODE_LVALUE, loc)
    { }

protected:
    virtual TreeNode* cloneV () const {
        return new TreeNodeLValue (m_location);
    }
};

/******************************************************************
  TreeNodeSubscript
******************************************************************/

// TODO: use this!
class TreeNodeSubscript: public TreeNode {
public: /* Methods: */

    inline TreeNodeSubscript(const Location & loc)
        : TreeNode(NODE_SUBSCRIPT, loc)
    { }

protected:
    virtual TreeNode* cloneV () const {
        return new TreeNodeSubscript (m_location);
    }
};

/******************************************************************
  TreeNodeIndex
******************************************************************/

// TODO: use this!
class TreeNodeIndex: public TreeNode {
public: /* Methods: */

    inline TreeNodeIndex(SecrecTreeNodeType type, const Location & loc)
        : TreeNode(type, loc)
    { }
};

/******************************************************************
  TreeNodeIndexInt
******************************************************************/

// TODO: use this!
class TreeNodeIndexInt: public TreeNodeIndex {
public: /* Methods: */

    inline TreeNodeIndexInt(const Location & loc)
        : TreeNodeIndex(NODE_INDEX_INT, loc)
    { }

protected:
    virtual TreeNode* cloneV () const {
        return new TreeNodeIndexInt (m_location);
    }
};

/******************************************************************
  TreeNodeIndexSlice
******************************************************************/

// TODO: use this!
class TreeNodeIndexSlice: public TreeNodeIndex {
public: /* Methods: */

    inline TreeNodeIndexSlice(const Location & loc)
        : TreeNodeIndex(NODE_INDEX_SLICE, loc)
    { }

protected:
    virtual TreeNode* cloneV () const {
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
        , m_value(value)
    { }

    inline StringRef value() const { return m_value; }

protected:

    virtual bool printHelper(std::ostream & os) const;
    virtual void printXmlHelper (std::ostream & os) const;
    virtual TreeNode* cloneV () const {
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
    virtual TypeChecker::Status accept(TypeChecker & tyChecker) = 0;
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
        , m_typeVariable (0)
    { }

    SymbolTypeVariable* typeVariable () const { return m_typeVariable; }
    void setTypeVariable (SymbolTypeVariable* tv) { m_typeVariable = tv; }
    virtual TypeChecker::Status accept(TypeChecker& tyChecker);
    virtual void setTypeContext (TypeContext& cxt) const;

protected:

    virtual TreeNode* cloneV () const {
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
        , m_cachedType (0)
    { }

    inline bool isPublic () const { return m_type == NODE_SECTYPE_PUBLIC_F; }
    SecurityType* cachedType () const { return m_cachedType; }
    void setCachedType (SecurityType* ty);

    virtual TypeChecker::Status accept(TypeChecker& tyChecker);
    virtual void setTypeContext (TypeContext& cxt) const;

protected:

    virtual bool printHelper(std::ostream & os) const;
    virtual TreeNode* cloneV () const {
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
        , m_dataType (DATATYPE_UNDEFINED) {}

    inline SecrecDataType cachedType () const { return m_dataType; }
    inline void setCachedType (SecrecDataType dataType) { m_dataType = dataType; }
    virtual void setTypeContext (TypeContext& cxt) const;

private: /* Fields: */
    SecrecDataType m_dataType;
};

/******************************************************************
  TreeNodeDataTypeConstF
******************************************************************/

class TreeNodeDataTypeConstF: public TreeNodeDataTypeF {
public: /* Methods: */
    inline TreeNodeDataTypeConstF (SecrecDataType dataType, const Location & loc)
        : TreeNodeDataTypeF (NODE_DATATYPE_CONST_F, loc)
    { setCachedType (dataType); }

    virtual TypeChecker::Status accept(TypeChecker& tyChecker);

protected:
    virtual bool printHelper(std::ostream & os) const;
    virtual void printXmlHelper (std::ostream & os) const;
    virtual TreeNode* cloneV () const {
        return new TreeNodeDataTypeConstF (cachedType (), m_location);
    }
};

/******************************************************************
  TreeNodeDataTypeVarF
******************************************************************/

class TreeNodeDataTypeVarF: public TreeNodeDataTypeF {
public: /* Methods: */
    inline TreeNodeDataTypeVarF (const Location & loc)
        : TreeNodeDataTypeF (NODE_DATATYPE_VAR_F, loc)
    { }

    virtual TypeChecker::Status accept(TypeChecker& tyChecker);

protected:
    virtual bool printHelper(std::ostream & os) const;
    virtual void printXmlHelper (std::ostream & os) const;
    virtual TreeNode* cloneV () const {
        return new TreeNodeDataTypeVarF (m_location);
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
    virtual void setTypeContext (TypeContext& cxt) const;

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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);

protected:
    virtual bool printHelper(std::ostream & os) const;
    virtual void printXmlHelper (std::ostream & os) const;
    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);

protected:
    virtual bool printHelper(std::ostream & os) const;
    virtual void printXmlHelper (std::ostream & os) const;
    virtual TreeNode* cloneV () const {
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
        , m_cachedType (0)
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

    virtual bool printHelper(std::ostream & os) const;
    virtual TreeNode* cloneV () const {
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

    virtual TreeNode* cloneV () const {
        return new TreeNodeTypeVoid (m_location);
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
        , m_resultType (0)
    { }

    // If possible instantiate abstract data type to given concrete data type
    void instantiateDataType (Context& cxt, SecrecDataType dType = DATATYPE_INT64) {
        assert (resultType () != 0);
        if ( ! resultType ()->isVoid ()
            && resultType ()->secrecDataType () == DATATYPE_NUMERIC
            && dType != DATATYPE_NUMERIC) {
            instantiateDataTypeV (cxt, dType);
        }
    }

    bool haveResultType() const { return m_resultType != 0; }
    bool havePublicBoolType() const;
    Type* resultType() const;

    virtual TypeChecker::Status accept(TypeChecker & tyChecker) = 0;
    virtual CGResult codeGenWith (CodeGen& cg) = 0;
    virtual CGBranchResult codeGenBoolWith (CodeGen&) {
        assert (false && "Not implemented!");
        return CGBranchResult (CGResult::ERROR_FATAL);
    }

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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

protected:
    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);
    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeSeqView<TreeNodeExpr> expressions () const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprArrayConstructor (m_location);
    }

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);
};

/******************************************************************
  TreeNodeExprInt
******************************************************************/

class TreeNodeExprInt: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprInt(uint64_t value, const Location & loc)
        : TreeNodeExpr(NODE_LITE_INT, loc), m_value(value) {}

    inline uint64_t value() const { return m_value; }

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

protected:

    virtual bool printHelper(std::ostream & os) const;
    virtual void printXmlHelper (std::ostream & os) const;
    virtual TreeNode* cloneV () const {
        return new TreeNodeExprInt (m_value, m_location);
    }

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

private: /* Fields: */
    uint64_t m_value;
};

/******************************************************************
  TreeNodeExprAssign
******************************************************************/

class TreeNodeExprAssign: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprAssign(SecrecTreeNodeType type, const Location & loc)
        : TreeNodeExpr(type, loc) {}

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNode* slice () const;
    TreeNodeIdentifier* identifier () const;
    TreeNodeExpr* rightHandSide () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* expression () const;
    TreeNodeDataTypeF* dataType () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* expression () const;
    TreeNode* indices () const;

protected:

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprIndex (m_location);
    }
};

/******************************************************************
  TreeNodeExprSize
******************************************************************/

class TreeNodeExprSize: public TreeNodeExpr {
public:
    inline TreeNodeExprSize(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_SIZE, loc) {}

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* leftExpression () const;
    TreeNodeExpr* rightExpression () const;
    TreeNodeExprInt* dimensionality () const;

protected:

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* reshapee () const;

    TreeNodeSeqView<TreeNodeExpr> dimensions () const;

protected:

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
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
        , m_symbolProcedure (0)
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

    bool isOverloaded () const { return procSymbol () != 0; }

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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* leftExpression () const;
    TreeNodeExpr* rightExpression () const;
    const char *operatorString() const;

protected:

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual SecrecOperator getOperatorV () const;

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

protected:

    virtual void printXmlHelper (std::ostream & os) const;
    virtual bool printHelper(std::ostream & os) const;
    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const;
};

/******************************************************************
  TreeNodeExprDeclassify
******************************************************************/

class TreeNodeExprDeclassify: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprDeclassify(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_DECLASSIFY, loc) {}

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual TreeNode* cloneV () const {
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
        , m_procedure (0)
    { }

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeIdentifier* procName () const;
    TreeNodeSeqView<TreeNodeExpr> params () const;

    void setProcedure (SymbolProcedure* proc) {
        m_procedure = proc;
    }

    inline SymbolProcedure *symbolProcedure() const {
        return m_procedure;
    }

protected:

    virtual TreeNode* cloneV () const {
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
        , m_valueSymbol (0) { }

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeIdentifier* identifier () const;
    void setValueSymbol (Symbol* sym) { m_valueSymbol = sym; }
    Symbol* valueSymbol () const { return m_valueSymbol; }

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprRVariable (m_location);
    }

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
    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

protected:

    virtual TreeNode* cloneV () const {
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
    virtual TypeChecker::Status accept (TypeChecker & tyChecker) = 0;
    virtual CGResult codeGenWith (CodeGen& cg) = 0;
};

/******************************************************************
  TreeNodeStringPartFragment
******************************************************************/

class TreeNodeStringPartFragment: public TreeNodeStringPart {
public: /* Methods: */

    TreeNodeStringPartFragment (StringRef value, const Location& loc)
        : TreeNodeStringPart (NODE_STRING_PART_FRAGMENT, loc)
        , m_value (value)
    { }

    bool isConstant () const { return true; }
    StringRef staticValue () const { return m_value; }
    TypeChecker::Status accept (TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

protected:

    virtual bool printHelper(std::ostream & os) const;
    virtual void printXmlHelper (std::ostream & os) const;
    virtual TreeNode* cloneV () const {
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
        , m_name (name)
        , m_value (0)
        , m_secrecType (0)
    { }

    StringRef name () const { return m_name; }
    ConstantString* value () const { return m_value; }
    void setValue (ConstantString* value) { m_value = value; }
    TypeNonVoid* secrecType () const { return m_secrecType; }
    void setSecrecType (TypeNonVoid* secrecType) { m_secrecType = secrecType; }

    bool isConstant () const { return m_value != 0; }
    StringRef staticValue () const;
    TypeChecker::Status accept (TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

protected:

    virtual bool printHelper(std::ostream & os) const;
    virtual void printXmlHelper (std::ostream & os) const;
    virtual TreeNode* cloneV () const {
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
        , m_value (value)
    { }

    inline StringRef value () const { return m_value; }
    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen & cg);

protected:
    virtual void instantiateDataTypeV(Context & cxt, SecrecDataType dType);
    virtual bool printHelper(std::ostream & os) const;
    virtual void printXmlHelper (std::ostream & os) const;
    virtual TreeNode * cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* conditional () const;
    TreeNodeExpr* trueBranch () const;
    TreeNodeExpr* falseBranch () const;

protected:

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprTernary (m_location);
    }
};

/******************************************************************
  TreeNodeExprPrefix
******************************************************************/

/// Prefix increment and decrement.
class TreeNodeExprPrefix: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprPrefix(SecrecTreeNodeType type,
                              const Location & loc)
        : TreeNodeExpr(type, loc)
    { }

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprPrefix (m_type, m_location);
    }
};

/******************************************************************
  TreeNodeExprPostfix
******************************************************************/

/// Postfix increment and decrement.
class TreeNodeExprPostfix: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprPostfix(SecrecTreeNodeType type,
                               const Location & loc)
        : TreeNodeExpr(type, loc)
    { }

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

protected:

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual SecrecOperator getOperatorV () const;

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeSecTypeF* securityType () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* expression () const;
    TreeNodeSeqView<TreeNodeTypeF> types () const;

protected:

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprQualified (m_location);
    }
};

/******************************************************************
  TreeNodeExprStringFromBytes
******************************************************************/

class TreeNodeExprStringFromBytes : public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprStringFromBytes(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_STRING_FROM_BYTES, loc) {}

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprStringFromBytes (m_location);
    }
};

/******************************************************************
  TreeNodeExprBytesFromString
******************************************************************/

class TreeNodeExprBytesFromString : public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprBytesFromString(const Location & loc)
        : TreeNodeExpr(NODE_EXPR_STRING_FROM_BYTES, loc) {}

    virtual TypeChecker::Status accept(TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
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

protected:

    virtual TreeNode* cloneV () const {
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

    virtual TreeNode* cloneV () const {
        return new TreeNodeDomain (m_location);
    }
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
        , m_cachedType(0)
        , m_procSymbol (0)
    {
        setContainingProcedureDirectly(this);
    }

public:

    explicit inline TreeNodeProcDef(const Location & loc)
        : TreeNode(NODE_PROCDEF, loc)
        , m_cachedType(0)
        , m_procSymbol (0)
    {
        setContainingProcedureDirectly(this);
    }

    virtual inline void resetParent(TreeNode *parent) {
        setParentDirectly(parent);
    }

    void setSymbol (SymbolProcedure* sym) {
        assert (sym != 0);
        m_procSymbol = sym;
    }

    SymbolProcedure* symbol () const {
        return m_procSymbol;
    }

    StringRef procedureName() const;
    const std::string printableSignature() const;

    inline bool haveProcedureType() const {
        return m_cachedType != 0;
    }

    TypeProc* procedureType() const {
        assert(m_cachedType != 0);
        return m_cachedType;
    }

    TreeNodeIdentifier* identifier () const;
    TreeNodeType* returnType () const;
    TreeNodeStmt* body () const;
    TreeNodeSeqView<TreeNodeStmtDecl> params () const;

protected: /* Methods: */

    friend class TypeChecker;

    virtual TreeNode* cloneV () const {
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

    virtual TreeNode* cloneV () const {
        return new TreeNodeOpDef (m_operator, m_location);
    }

protected: /* Fields: */
    const SecrecOperator  m_operator;
};

/******************************************************************
  TreeNodeQuantifier
******************************************************************/

class TreeNodeQuantifier : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeQuantifier(SecrecTreeNodeType type, const Location & loc)
        : TreeNode(type, loc) { }

    virtual void printQuantifier (std::ostream& os) const = 0;

    virtual TypeChecker::Status accept (TypeChecker & typeChecker) = 0;

    TreeNodeIdentifier* typeVariable () const;

protected:
    virtual TreeNode* cloneV () const = 0;
};

/******************************************************************
  TreeNodeDomainQuantifier
******************************************************************/

class TreeNodeDomainQuantifier : public TreeNodeQuantifier {
public: /* Methods: */
    explicit inline TreeNodeDomainQuantifier(const Location & loc)
        : TreeNodeQuantifier(NODE_TEMPLATE_DOMAIN_QUANT, loc) {}

    // will equal to zero, if kind not specified
    TreeNodeIdentifier* kind () const;

    virtual void printQuantifier (std::ostream & os) const;
    virtual TypeChecker::Status accept (TypeChecker & typeChecker);

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeDomainQuantifier (m_location);
    }
};

/******************************************************************
  TreeNodeDimensionalityQuantifier
******************************************************************/

class TreeNodeDimQuantifier : public TreeNodeQuantifier {
public: /* Methods: */
    explicit inline TreeNodeDimQuantifier (const Location & loc)
        : TreeNodeQuantifier(NODE_TEMPLATE_DIM_QUANT, loc) {}

    void printQuantifier (std::ostream & os) const;
    virtual TypeChecker::Status accept (TypeChecker & typeChecker);

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeDimQuantifier (m_location);
    }
};

/******************************************************************
  TreeNodeDataQuantifier
******************************************************************/

class TreeNodeDataQuantifier : public TreeNodeQuantifier {
public: /* Methods: */
    explicit inline TreeNodeDataQuantifier (const Location & loc)
        : TreeNodeQuantifier(NODE_TEMPLATE_DATA_QUANT, loc) {}

    void printQuantifier (std::ostream & os) const;
    virtual TypeChecker::Status accept (TypeChecker & typeChecker);

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeDataQuantifier (m_location);
    }
};

/******************************************************************
  TreeNodeTemplate
******************************************************************/

class TreeNodeTemplate : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeTemplate(const Location & loc)
        : TreeNode(NODE_TEMPLATE_DECL, loc)
        , m_containingModule (0)
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

    virtual TreeNode* cloneV () const {
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

    virtual TreeNode* cloneV () const {
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

    virtual TreeNode* cloneV () const {
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

    virtual TreeNode* cloneV () const {
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


    virtual CGStmtResult codeGenWith (CodeGen& cg);

protected:

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

protected:

    virtual TreeNode* cloneV () const {
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


    virtual CGStmtResult codeGenWith (CodeGen& cg);

protected:

    virtual TreeNode* cloneV () const {
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

    virtual TreeNode* cloneV () const {
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
        , m_type (0)
        , m_global (global)
        , m_procParam (procParam)
    { }

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    inline TypeNonVoid* resultType() const {
        assert(m_type != 0);
        return m_type;
    }

    inline bool haveResultType() const { return m_type != 0; }
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

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* conditional () const;
    TreeNodeStmt* body () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNode* initializer () const;
    TreeNodeExpr* conditional () const;
    TreeNodeExpr* iteratorExpr () const;
    TreeNodeStmt* body () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* conditional () const;
    TreeNodeStmt* trueBranch () const;
    TreeNodeStmt* falseBranch () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    bool hasExpression() const;
    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* conditional () const;
    TreeNodeStmt* body () const;

protected:

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeSeqView<TreeNodeExpr> expressions ();

protected:

    virtual TreeNode* cloneV () const {
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

    virtual TreeNode* cloneV () const {
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

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeExprString* name () const;
    TreeNodeSeqView<TreeNodeSyscallParam> params () const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeStmtSyscall  (m_location);
    }
};

} /* namespace SecreC */

namespace boost {

template<class BaseClass>
struct range_mutable_iterator<SecreC::TreeNodeSeqView<BaseClass> > {
    typedef typename SecreC::TreeNodeSeqView<BaseClass>::iterator type;
};

template<class BaseClass>
struct range_const_iterator<SecreC::TreeNodeSeqView<BaseClass> > {
    typedef typename SecreC::TreeNodeSeqView<BaseClass>::iterator type;
};

} // namespace boost

#endif /* TREENODE_H */
