#ifndef SECREC_TREENODE_H
#define SECREC_TREENODE_H

#include "parser.h"

#ifdef __cplusplus
#include <cassert>
#include <deque>
#include <string>

#include "intermediate.h"
#include "types.h"
#include "TypeContext.h"
#include "codegenResult.h"

namespace SecreC {

class CodeGen;
class TypeChecker;
class ModuleInfo;
class TreeNodeProcDef;
class TreeNode;
#else
typedef struct TreeNode TreeNode;
#endif

#ifdef __cplusplus
// C interface
extern "C" {
#endif /* #ifdef __cplusplus */

/* C interface for yacc: */

TreeNode *treenode_init(enum SecrecTreeNodeType type,
                        const YYLTYPE *loc);
void treenode_free(TreeNode *node);
enum SecrecTreeNodeType treenode_type(TreeNode *node);
const YYLTYPE *treenode_location(const TreeNode *node);
unsigned treenode_numChildren(const TreeNode *node);
TreeNode *treenode_childAt(const TreeNode *node, unsigned index);
void treenode_appendChild(TreeNode *parent, TreeNode *child);
void treenode_prependChild(TreeNode *parent, TreeNode *child);
void treenode_setLocation(TreeNode *node, YYLTYPE *loc);
void treenode_moveChildren (TreeNode* from, TreeNode* to);

TreeNode *treenode_init_bool(unsigned value, YYLTYPE *loc);
TreeNode *treenode_init_int(int value, YYLTYPE *loc);
TreeNode *treenode_init_uint(unsigned value, YYLTYPE *loc);
TreeNode *treenode_init_string(const char *value, YYLTYPE *loc);
TreeNode *treenode_init_float(const char *value, YYLTYPE *loc);
TreeNode *treenode_init_identifier(const char *value, YYLTYPE *loc);
TreeNode *treenode_init_publicSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_privateSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_dataTypeF(enum SecrecDataType dataType,
                                  YYLTYPE *loc);
TreeNode *treenode_init_dimTypeF(unsigned dimType,
                                 YYLTYPE *loc);
TreeNode *treenode_init_opdef(enum SecrecOperator op, YYLTYPE *loc);

#ifdef __cplusplus
} /* extern "C" */


/******************************************************************
  TreeNode
******************************************************************/

/**
 * \class TreeNode
 * Abstract representation of the SecreC code.
 */
class TreeNode {
public: /* Types: */
    typedef std::deque<TreeNode*> ChildrenList;
    typedef ChildrenList::iterator ChildrenListIterator;
    typedef ChildrenList::const_iterator
        ChildrenListConstIterator;
    typedef std::pair< ChildrenListIterator
                     , ChildrenListIterator>
        ChildrenListRange;
    typedef std::pair< ChildrenListConstIterator
                     , ChildrenListConstIterator>
        ChildrenListConstRange;

public: /* Methods: */
    TreeNode(SecrecTreeNodeType type, const YYLTYPE &loc);
    virtual ~TreeNode();

    TreeNodeProcDef* containingProcedure();
    inline TreeNode* parent() const { return m_parent; }
    inline bool hasParent() const { return m_parent != 0; }
    inline SecrecTreeNodeType type() const { return m_type; }
    inline ChildrenList &children() { return m_children; }
    inline const ChildrenList &children() const {
        return m_children;
    }
    inline const YYLTYPE &location() const { return m_location; }

    void appendChild(TreeNode *child);
    void prependChild(TreeNode *child);
    void setLocation(const YYLTYPE &location);

    ChildrenListIterator begin () {
        return children ().begin ();
    }
    ChildrenListIterator end () {
        return children ().end ();
    }
    ChildrenListConstIterator begin () const {
        return children ().begin ();
    }
    ChildrenListConstIterator end () const {
        return children ().end ();
    }

    std::string toString(unsigned indentation = 2,
                         unsigned startIndent = 0) const;
    virtual std::string stringHelper() const { return ""; }

    std::string toXml(bool full = false) const;
    virtual inline std::string xmlHelper() const { return ""; }

    static const char *typeName(SecrecTreeNodeType type);

    TreeNode* clone (TreeNode* parent) const;

protected: /* Methods: */
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
    // explicitly add any by itself.
    virtual TreeNode* cloneV () const {
        return new TreeNode (m_type, m_location);
    }

protected: /* Fields: */
    TreeNode*                 m_parent;
    TreeNodeProcDef*          m_procedure;
    const SecrecTreeNodeType  m_type;
    ChildrenList              m_children;
    YYLTYPE                   m_location;
};

/******************************************************************
  TreeNodeIdentifier
******************************************************************/

class TreeNodeIdentifier: public TreeNode {
public: /* Methods: */
    inline TreeNodeIdentifier(const std::string &value,
                              const YYLTYPE &loc)
        : TreeNode(NODE_IDENTIFIER, loc)
        , m_value(value)
    { }

    inline const std::string &value() const { return m_value; }
    virtual std::string stringHelper() const;
    virtual std::string xmlHelper() const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeIdentifier (m_value, m_location);
    }

private: /* Fields: */
    const std::string m_value;
};

/******************************************************************
  TreeNodeSecTypeF
******************************************************************/

class TreeNodeSecTypeF: public TreeNode {
public: /* Methods: */
    inline TreeNodeSecTypeF(bool isPublic, const YYLTYPE &loc)
        : TreeNode (NODE_SECTYPE_F, loc)
        , m_isPublic (isPublic)
        , m_cachedType (0)
    { }

    virtual std::string stringHelper() const;
    virtual std::string xmlHelper() const;

    inline bool isPublic () const { return m_isPublic; }
    SecurityType* cachedType () const { return m_cachedType; }
    void setCachedType (SecurityType* ty);
    TreeNodeIdentifier* identifier () const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeSecTypeF (m_isPublic, m_location);
    }

private: /* Fields: */
    const bool     m_isPublic;
    SecurityType*  m_cachedType;
};

/******************************************************************
  TreeNodeDataType
******************************************************************/

class TreeNodeDataTypeF: public TreeNode {
public: /* Methods: */
    inline TreeNodeDataTypeF(SecrecDataType dataType,
                             const YYLTYPE &loc)
        : TreeNode(NODE_DATATYPE_F, loc)
        , m_dataType(dataType) {}

    const SecrecDataType &dataType() const {
        return m_dataType;
    }

    virtual std::string stringHelper() const;
    virtual std::string xmlHelper() const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeDataTypeF (m_dataType, m_location);
    }

private: /* Fields: */
    const SecrecDataType m_dataType;
};

/******************************************************************
  TreeNodeDimType
******************************************************************/

class TreeNodeDimTypeF: public TreeNode {
public: /* Methods: */
    inline TreeNodeDimTypeF(SecrecDimType dimType,
                            const YYLTYPE &loc)
        : TreeNode(NODE_DIMTYPE_F, loc), m_dimType(dimType) {}

    SecrecDimType dimType() const {
        return m_dimType;
    }

    virtual std::string stringHelper() const;
    virtual std::string xmlHelper() const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeDimTypeF (m_dimType, m_location);
    }

private: /* Fields: */
    const SecrecDimType m_dimType;
};

/******************************************************************
  TreeNodeType
******************************************************************/

class TreeNodeType : public TreeNode {
public: /* Methods: */
    inline TreeNodeType(SecrecTreeNodeType type,
                        const YYLTYPE &loc)
        : TreeNode(type, loc)
        , m_cachedType (0)
    { }

    SecreC::Type* secrecType () const;
    TreeNodeSecTypeF* secType () const;
    TreeNodeDataTypeF* dataType () const;
    TreeNodeDimTypeF* dimType () const;
    bool isNonVoid () const;

protected:

    friend class TypeChecker;

    inline TreeNodeType(SecrecTreeNodeType type,
                        const YYLTYPE &loc,
                        SecreC::Type* ty)
        : TreeNode(type, loc)
        , m_cachedType (ty)
    { }

    virtual TreeNode* cloneV () const = 0;

protected: /* Fields: */

    SecreC::Type* m_cachedType;
};

/******************************************************************
  TreeNodeTypeType
******************************************************************/

/// Non-void types.
class TreeNodeTypeType: public TreeNodeType {
public: /* Methods: */
    explicit inline TreeNodeTypeType(const YYLTYPE &loc)
        : TreeNodeType(NODE_TYPETYPE, loc) {}
    virtual inline ~TreeNodeTypeType() { }

    virtual std::string stringHelper() const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeTypeType (m_location);
    }
};

/******************************************************************
  TreeNodeTypeVoid
******************************************************************/

class TreeNodeTypeVoid: public TreeNodeType {
public: /* Methods: */
    explicit inline TreeNodeTypeVoid(const YYLTYPE &loc)
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
                        const YYLTYPE &loc)
        : TreeNode (type, loc)
        , m_resultType (0)
    { }

    virtual ~TreeNodeExpr() { }

    virtual ICode::Status accept (TypeChecker& tyChecker) = 0;

    // If possible instantiate abstract data type to given concrete data type
    void instantiateDataType (Context& cxt, SecrecDataType dType = DATATYPE_INT64) {
        assert (resultType () != 0);
        if ( ! resultType ()->isVoid ()
            && resultType ()->secrecDataType () == DATATYPE_NUMERIC
            && dType != DATATYPE_NUMERIC) {
            instantiateDataTypeV (cxt, dType);
        }
    }

    inline bool haveResultType() const {
        return m_resultType != 0;
    }

    /// \todo move to type checker
    bool havePublicBoolType() const;

    SecreC::Type* resultType() const;

    virtual CGResult codeGenWith (CodeGen& cg) = 0;
    virtual CGBranchResult codeGenBoolWith (CodeGen&) {
        assert (false && "Not implemented!");
        return CGBranchResult (CGResult::ERROR_FATAL);
    }

    void setContextSecType (SecurityType* ty) {
        m_contextSecType = ty;
    }

protected: /* Methods: */

    virtual TreeNode* cloneV () const = 0;

    virtual void instantiateDataTypeV (Context& cxt, SecrecDataType dType) {
        (void) cxt;
        (void) dType;
        assert (false && "ICE!");
    }

    void setResultType(SecreC::Type *type);
    void resetDataType (Context& cxt, SecrecDataType dType);

protected: /* Fields: */

    SecreC::Type*   m_resultType; ///< Type of resulting value.
};

/******************************************************************
  TreeNodeStmt
******************************************************************/

/// Statements.
class TreeNodeStmt: public TreeNode {
public: /* Methods: */
    inline TreeNodeStmt(SecrecTreeNodeType type, const YYLTYPE &loc)
        : TreeNode(type, loc) { }

    virtual CGStmtResult codeGenWith (CodeGen&) = 0;

protected:

    virtual TreeNode* cloneV () const = 0;
};

/******************************************************************
  TreeNodeExprInt
******************************************************************/

class TreeNodeExprInt: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprInt(int value, const YYLTYPE &loc)
        : TreeNodeExpr(NODE_LITE_INT, loc), m_value(value) {}

    inline void setValue(int value) { m_value = value; }
    inline int value() const { return m_value; }

    virtual std::string stringHelper() const;
    virtual std::string xmlHelper() const;
    virtual ICode::Status accept (TypeChecker& tyChecker);

    virtual CGResult codeGenWith (CodeGen& cg);
protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprInt (m_value, m_location);
    }

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

private: /* Fields: */
    int m_value;
};

/******************************************************************
  TreeNodeExprAssign
******************************************************************/

class TreeNodeExprAssign: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprAssign(SecrecTreeNodeType type, const YYLTYPE &loc)
        : TreeNodeExpr(type, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
    inline TreeNodeExprCast (const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_CAST, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
    inline TreeNodeExprIndex(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_INDEX, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
    inline TreeNodeExprSize(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_SIZE, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
    inline TreeNodeExprShape(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_SHAPE, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
    inline TreeNodeExprCat(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_CAT, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
    inline TreeNodeExprReshape(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_RESHAPE, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    /// \todo yeah, figure a better name out...
    /// "reshapee" as the expression that is being reshaped!
    TreeNodeExpr* reshapee () const;

    /// \todo use range or iterators?
    /// that requires a iterator that casts to expression
    TreeNode::ChildrenListConstRange dimensions ();

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
    inline TreeNodeExprToString(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_TOSTRING, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
                              const YYLTYPE &loc)
        : TreeNodeExpr(type, loc)
    { }

    virtual ICode::Status accept (TypeChecker& tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* leftExpression () const;
    TreeNodeExpr* rightExpression () const;
    const char *operatorString() const;

protected:

    TreeNodeExprBinary (SecrecTreeNodeType type,
                        const YYLTYPE &loc,
                        const OverloadableOperator& ov)
        : TreeNodeExpr(type, loc)
        , OverloadableOperator (ov)
    { }

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual SecrecOperator getOperatorV () const;

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprBinary (m_type, m_location, *this);
    }
};

/******************************************************************
  TreeNodeExprBool
******************************************************************/

class TreeNodeExprBool: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprBool(bool value, const YYLTYPE &loc)
        : TreeNodeExpr(NODE_LITE_BOOL, loc), m_value(value) {}

    inline bool value() const { return m_value; }

    virtual ICode::Status accept (TypeChecker& tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);
    virtual std::string xmlHelper() const;
    virtual inline std::string stringHelper() const {
        return (m_value ? "true" : "false");
    }

protected:

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
                                const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_CLASSIFY, loc)
    {
        m_contextSecType = ty;
    }

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
    inline TreeNodeExprDeclassify(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_DECLASSIFY, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);

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
    explicit inline TreeNodeExprProcCall(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_PROCCALL, loc)
        , m_procedure (0)
    { }

    virtual ICode::Status accept (TypeChecker& tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeIdentifier* procName () const;

    ChildrenListConstRange paramRange () const;

    void setProcedure (SymbolProcedure* proc) {
        m_procedure = proc;
    }

    inline SymbolProcedure *symbolProcedure() const
    { return m_procedure; }

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
    explicit inline TreeNodeExprRVariable(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_RVARIABLE, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeIdentifier* identifier () const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprRVariable (m_location);
    }
};

/******************************************************************
  TreeNodeExprString
******************************************************************/

/// String constants.
class TreeNodeExprString: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprString(const std::string &value,
                              const YYLTYPE &loc)
        : TreeNodeExpr(NODE_LITE_STRING, loc)
        , m_value(value)
    { }

    inline const std::string &value() const { return m_value; }

    virtual std::string stringHelper() const;
    virtual std::string xmlHelper() const;
    virtual ICode::Status accept (TypeChecker& tyChecker);

    virtual CGResult codeGenWith (CodeGen& cg);

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprString (m_value, m_location);
    }

private: /* Fields: */
    const std::string m_value;
};

/******************************************************************
  TreeNodeExprFloat
******************************************************************/

class TreeNodeExprFloat: public TreeNodeExpr {
public: /* Methods: */
    TreeNodeExprFloat (const std::string & value,
                       const YYLTYPE & loc)
        : TreeNodeExpr (NODE_LITE_FLOAT, loc)
        , m_value (value)
    { }

    inline const std::string & value () const { return m_value; }
    virtual std::string stringHelper() const;
    virtual std::string xmlHelper () const;
    virtual ICode::Status accept (TypeChecker & tyChecker);
    virtual CGResult codeGenWith (CodeGen & cg);

protected:

    virtual TreeNode * cloneV () const {
        return new TreeNodeExprString (m_value, m_location);
    }

private: /* Fields: */
    const std::string m_value;
};

/******************************************************************
  TreeNodeExprTernary
******************************************************************/

class TreeNodeExprTernary: public TreeNodeExpr {
public: /* Methods: */
    explicit inline TreeNodeExprTernary(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_TERNIF, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);

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
                              const YYLTYPE &loc)
        : TreeNodeExpr(type, loc)
    { }

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
                               const YYLTYPE &loc)
        : TreeNodeExpr(type, loc)
    { }

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
                             const YYLTYPE &loc)
        : TreeNodeExpr(type, loc)
    { }

    virtual ICode::Status accept (TypeChecker& tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    TreeNodeExprUnary (SecrecTreeNodeType type,
                       const YYLTYPE &loc,
                       const OverloadableOperator& ov)
        : TreeNodeExpr(type, loc)
        , OverloadableOperator (ov)
    { }

    virtual void instantiateDataTypeV (Context &cxt, SecrecDataType dType);

    virtual SecrecOperator getOperatorV () const;

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprUnary (m_type, m_location, *this);
    }
};

/******************************************************************
  TreeNodeExprDomainID
******************************************************************/

/// Unary expressions such as regular (logical) negation.
class TreeNodeExprDomainID: public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprDomainID(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_DOMAINID, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
    inline TreeNodeExprQualified(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_TYPE_QUAL, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);
    virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    TreeNodeExpr* expression () const;
    ChildrenListConstRange types () const;

protected:

    virtual void instantiateDataType (Context &cxt, SecrecDataType dType);

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprQualified (m_location);
    }
};

/******************************************************************
  TreeNodeExprStringFromBytes
******************************************************************/

class TreeNodeExprStringFromBytes : public TreeNodeExpr {
public: /* Methods: */
    inline TreeNodeExprStringFromBytes(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_STRING_FROM_BYTES, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
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
    inline TreeNodeExprBytesFromString(const YYLTYPE &loc)
        : TreeNodeExpr(NODE_EXPR_STRING_FROM_BYTES, loc) {}

    virtual ICode::Status accept (TypeChecker& tyChecker);
    virtual CGResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeExprBytesFromString (m_location);
    }
};

/******************************************************************
  TreeNodeStmtKind
******************************************************************/

/// Declaration statement. Also tracks if the scope is global.
class TreeNodeKind : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeKind(const YYLTYPE &loc)
        : TreeNode (NODE_KIND, loc) { }
    virtual inline ~TreeNodeKind() { }

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeKind (m_location);
    }
};

/******************************************************************
  TreeNodeStmtDomain
******************************************************************/

/// Declaration statement. Also tracks if the scope is global.
class TreeNodeDomain : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeDomain(const YYLTYPE &loc)
        : TreeNode (NODE_DOMAIN, loc) { }
    virtual inline ~TreeNodeDomain() { }

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
                                    const YYLTYPE &loc)
        : TreeNode (type, loc)
        , m_cachedType(0)
        , m_procSymbol (0)
    {
        setContainingProcedureDirectly(this);
    }

public:

    explicit inline TreeNodeProcDef(const YYLTYPE &loc)
        : TreeNode(NODE_PROCDEF, loc)
        , m_cachedType(0)
        , m_procSymbol (0)
    {
        setContainingProcedureDirectly(this);
    }

    virtual inline ~TreeNodeProcDef() { }

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

    const std::string &procedureName() const;

    inline bool haveProcedureType() const {
        return m_cachedType != 0;
    }

    SecreC::TypeNonVoid* procedureType() const {
        assert(m_cachedType != 0);
        return m_cachedType;
    }

    TreeNodeIdentifier* identifier () const;
    TreeNodeType* returnType () const;
    TreeNodeStmt* body () const;
    ChildrenListConstRange paramRange () const;
    ChildrenListConstIterator paramBegin () const;
    ChildrenListConstIterator paramEnd () const;

protected: /* Methods: */

    friend class TypeChecker;

    virtual TreeNode* cloneV () const {
        return new TreeNodeProcDef (m_location);
    }

protected: /* Fields: */
    SecreC::TypeNonVoid*  m_cachedType;
    SymbolProcedure*      m_procSymbol;
};

/******************************************************************
  TreeNodeOpDef
******************************************************************/

class TreeNodeOpDef: public TreeNodeProcDef {
public: /* Methods: */

    explicit inline TreeNodeOpDef(SecrecOperator op,
                                  const YYLTYPE &loc)
        : TreeNodeProcDef (NODE_OPDEF, loc)
        , m_operator (op)
    { }

    inline ~TreeNodeOpDef() { }

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
    explicit inline TreeNodeQuantifier(const YYLTYPE &loc)
        : TreeNode(NODE_TEMPLATE_QUANT, loc) {}

    TreeNodeIdentifier* domain ();

    // will equal to zero, if kind not specified
    TreeNodeIdentifier* kind ();

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeQuantifier (m_location);
    }
};

/******************************************************************
  TreeNodeTemplate
******************************************************************/

class TreeNodeTemplate : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeTemplate(const YYLTYPE &loc)
        : TreeNode(NODE_TEMPLATE_DECL, loc)
        , m_contextDependance (false)
        , m_containingModule (0)
    { }

    TreeNodeProcDef* body () const;
    TreeNode::ChildrenList& quantifiers () const;

    void setContextDependance (bool contextDependance) {
        m_contextDependance = contextDependance;
    }

    bool isContextDependent () const {
        return m_contextDependance;
    }

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
    bool m_contextDependance; /**< true if the template resolution
                                   requires context */
    ModuleInfo* m_containingModule;
};

/******************************************************************
  TreeNodeProgram
******************************************************************/

/// Representation of program.
class TreeNodeProgram: public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeProgram(const YYLTYPE &loc)
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
    inline TreeNodeImport(const YYLTYPE &loc)
        : TreeNode (NODE_IMPORT, loc)
    { }

    virtual ~TreeNodeImport () { }
    const std::string& name () const;

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
    inline TreeNodeModule(const YYLTYPE &loc)
        : TreeNode (NODE_MODULE, loc)
    { }

    virtual ~TreeNodeModule () { }

    bool hasName () const;
    std::string name () const;
    TreeNodeProgram* program () const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeModule (m_location);
    }
};

/******************************************************************
  TreeNodeStmtBreak
******************************************************************/

/// Break statement.
class TreeNodeStmtBreak: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtBreak(const YYLTYPE &loc)
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
    explicit inline TreeNodeStmtCompound(const YYLTYPE &loc)
        : TreeNodeStmt(NODE_STMT_COMPOUND, loc) {}
    virtual inline ~TreeNodeStmtCompound() {}

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
    explicit inline TreeNodeStmtContinue(const YYLTYPE &loc)
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
    TreeNodeVarInit (const YYLTYPE &loc)
        : TreeNode (NODE_VAR_INIT, loc)
    { }

    virtual ~TreeNodeVarInit () { }

    const std::string &variableName() const;

    /// \retval 0 if shape is not specified
    TreeNode* shape () const;

    /// \retval 0 if right hand side is not defined
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

    explicit TreeNodeStmtDecl (const YYLTYPE &loc)
        : TreeNodeStmt (NODE_DECL, loc)
        , m_type (0)
        , m_global (false)
        , m_procParam (false) { }

    virtual ~TreeNodeStmtDecl() { }

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    inline SecreC::TypeNonVoid* resultType() const {
        assert(m_type != 0);
        return m_type;
    }

    inline bool haveResultType() const { return m_type != 0; }

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
    ChildrenListConstRange initializers () const;
    TreeNodeType* varType () const;

    const std::string &variableName() const {
        return initializer ()->variableName ();
    }

    TreeNode* shape () const {
        return initializer ()->shape ();
    }

    TreeNodeExpr* rightHandSide () const {
        return initializer ()->rightHandSide ();
    }

protected:

    friend class TypeChecker;

    virtual TreeNode* cloneV () const {
        return new TreeNodeStmtDecl (m_location);
    }

protected: /* Fields: */
    SecreC::TypeNonVoid *m_type;
    bool m_global;
    bool m_procParam;
};

/******************************************************************
  TreeNodeStmtDoWhile
******************************************************************/

/// Do-while statement.
class TreeNodeStmtDoWhile: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtDoWhile(const YYLTYPE &loc)
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
    explicit inline TreeNodeStmtExpr(const YYLTYPE &loc)
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
    explicit TreeNodeStmtAssert(const YYLTYPE &loc)
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
    explicit inline TreeNodeStmtFor(const YYLTYPE &loc)
        : TreeNodeStmt(NODE_STMT_FOR, loc) {}

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    ICode::Status accept (TypeChecker& tyChecker);

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
    explicit inline TreeNodeStmtIf(const YYLTYPE &loc)
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
    explicit inline TreeNodeStmtReturn(const YYLTYPE &loc)
        : TreeNodeStmt(NODE_STMT_RETURN, loc) {}

    virtual CGStmtResult codeGenWith (CodeGen& cg);

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
    explicit inline TreeNodeStmtWhile(const YYLTYPE &loc)
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

/// String printing statement.
class TreeNodeStmtPrint: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtPrint(const YYLTYPE &loc)
        : TreeNodeStmt(NODE_STMT_PRINT, loc) {}

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    ChildrenList& expressions ();

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeStmtPrint (m_location);
    }
};

/******************************************************************
  TreeNodeStmtSyscall
******************************************************************/

class TreeNodeStmtSyscall : public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtSyscall(const YYLTYPE &loc)
        : TreeNodeStmt(NODE_STMT_SYSCALL, loc) {}

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeExprString* expression () const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeStmtSyscall  (m_location);
    }
};

/******************************************************************
  TreeNodeStmtPush
******************************************************************/

class TreeNodeStmtPush: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtPush(const YYLTYPE &loc)
        : TreeNodeStmt(NODE_STMT_PUSH, loc) {}

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeExpr* expression () const;

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeStmtPush (m_location);
    }
};

/******************************************************************
  TreeNodeStmtPushRef
******************************************************************/

class TreeNodeStmtPushRef: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtPushRef(const YYLTYPE &loc,
                                        bool isConstant = false)
        : TreeNodeStmt(NODE_STMT_PUSH, loc)
        , m_isConstant (isConstant)
    {}

    virtual CGStmtResult codeGenWith (CodeGen& cg);

    TreeNodeIdentifier* identifier () const;

    bool isConstant () const { return m_isConstant; }

protected:

    virtual TreeNode* cloneV () const {
        return new TreeNodeStmtPushRef (m_location, m_isConstant);
    }

private: /* Fields: */
    const bool m_isConstant;
};

} // namespace SecreC

#endif /* #ifdef __cplusplus */

#endif /* TREENODE_H */
