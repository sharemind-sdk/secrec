#ifndef TREENODE_H
#define TREENODE_H

#include "parser.h"

#ifdef __cplusplus
#include <cassert>
#include <deque>
#include <string>

#include "intermediate.h"
#include "types.h"
#include "codegenResult.h"

namespace SecreC {

class CompileLog;
class CodeGen;
class TypeChecker;
class TreeNodeExpr;
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

TreeNode *treenode_init(enum SecrecTreeNodeType type, const YYLTYPE *loc);
void treenode_free(TreeNode *node);
enum SecrecTreeNodeType treenode_type(TreeNode *node);
const YYLTYPE *treenode_location(const TreeNode *node);
unsigned treenode_numChildren(const TreeNode *node);
TreeNode *treenode_childAt(const TreeNode *node, unsigned index);
void treenode_appendChild(TreeNode *parent, TreeNode *child);
void treenode_prependChild(TreeNode *parent, TreeNode *child);
void treenode_setLocation(TreeNode *node, YYLTYPE *loc);

TreeNode *treenode_init_bool(unsigned value, YYLTYPE *loc);
TreeNode *treenode_init_int(int value, YYLTYPE *loc);
TreeNode *treenode_init_uint(unsigned value, YYLTYPE *loc);
TreeNode *treenode_init_string(const char *value, YYLTYPE *loc);
TreeNode *treenode_init_identifier(const char *value, YYLTYPE *loc);
TreeNode *treenode_init_publicSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_privateSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_dataTypeF(enum SecrecDataType dataType,
                                  YYLTYPE *loc);
TreeNode *treenode_init_dimTypeF(unsigned dimType,
                                 YYLTYPE *loc);

#ifdef __cplusplus
} /* extern "C" */

/**
 * \class TreeNode
 * Abstract syntax tree, or abstract representation of the SecreC code.
 * AST handles all of the type checking logic, mainly because it rewrites
 * the tree in some occasions such as adding of explicit classify nodes.
 */
class TreeNode {
    public: /* Types: */
        typedef std::deque<TreeNode*> ChildrenList;
        typedef ChildrenList::iterator ChildrenListIterator;
        typedef ChildrenList::const_iterator ChildrenListConstIterator;

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

        std::string toString(unsigned indentation = 2, unsigned startIndent = 0)
                const;
        virtual inline std::string stringHelper() const { return ""; }

        std::string toXml(bool full = false) const;
        virtual inline std::string xmlHelper() const { return ""; }

        static const char *typeName(SecrecTreeNodeType type);

    protected: /* Methods: */
        inline void setParentDirectly(TreeNode *parent) { m_parent = parent; }
        inline void setContainingProcedureDirectly(TreeNodeProcDef *p) { m_procedure = p; }
        virtual inline void resetParent(TreeNode *parent) {
            m_parent = parent;
            m_procedure = parent->m_procedure;
        }

    private: /* Fields: */
        TreeNode*                 m_parent;
        TreeNodeProcDef*          m_procedure;
        const SecrecTreeNodeType  m_type;
        ChildrenList              m_children;
        YYLTYPE                   m_location;
};

inline TreeNode::ChildrenListIterator begin_children (TreeNode* node) {
    return node->children ().begin ();
}

inline TreeNode::ChildrenListIterator end_children (TreeNode* node) {
    return node->children ().end ();
}

inline TreeNode::ChildrenListConstIterator begin_children (const TreeNode* node) {
    return node->children ().begin ();
}

inline TreeNode::ChildrenListConstIterator end_children (const TreeNode* node) {
    return node->children ().end ();
}

/******************************************************************
  TreeNodeIdentifier
******************************************************************/

/// Identifier.
class TreeNodeIdentifier: public TreeNode {
    public: /* Methods: */
        inline TreeNodeIdentifier(const std::string &value, const YYLTYPE &loc)
            : TreeNode(NODE_IDENTIFIER, loc), m_value(value) {}

        inline void setValue(const std::string &value) { m_value = value; }
        inline const std::string &value() const { return m_value; }
        SymbolSymbol *getSymbol(SymbolTable &st, CompileLog &log) const;

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;

    private: /* Fields: */
        std::string m_value;
};


/******************************************************************
  TreeNodeSecTypeF
******************************************************************/

/// Security type
class TreeNodeSecTypeF: public TreeNode {
    public: /* Methods: */
        inline TreeNodeSecTypeF(bool isPublic, const YYLTYPE &loc)
            : TreeNode (NODE_SECTYPE_F, loc)
            , m_isPublic (isPublic)
        { }

        inline bool isPublic () const { return m_isPublic; }
        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;

        TreeNodeIdentifier* identifier () const {
            assert (children ().size () == 1);
            assert (dynamic_cast<TreeNodeIdentifier*>(children ().at (0)) != 0);
            return static_cast<TreeNodeIdentifier*>(children ().at (0));
        }

    private: /* Fields: */
        const bool m_isPublic;
};

/******************************************************************
  TreeNodeDataType
******************************************************************/

/// Data type
class TreeNodeDataTypeF: public TreeNode {
    public: /* Methods: */
        inline TreeNodeDataTypeF(SecrecDataType dataType,
                                 const YYLTYPE &loc)
            : TreeNode(NODE_DATATYPE_F, loc), m_dataType(dataType) {}

        const SecrecDataType &dataType() const {
            return m_dataType;
        }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;

    private:
        SecrecDataType m_dataType;
};


/******************************************************************
  TreeNodeDimType
******************************************************************/

/// Dimensionality type
class TreeNodeDimTypeF: public TreeNode {
    public: /* Methods: */
        inline TreeNodeDimTypeF(unsigned dimType,
                                const YYLTYPE &loc)
            : TreeNode(NODE_DIMTYPE_F, loc), m_dimType(dimType) {}

        unsigned dimType() const {
            return m_dimType;
        }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;

    private:
        unsigned m_dimType;
};


/******************************************************************
  TreeNodeType
******************************************************************/

/// Types occuring in code.
class TreeNodeType : public TreeNode {
public: /* Methods: */
    inline TreeNodeType(SecrecTreeNodeType type, const YYLTYPE &loc)
        : TreeNode(type, loc)
        , m_cachedType (0)
    { }

    const SecreC::Type& secrecType () const {
        assert (m_cachedType != 0);
        return *m_cachedType;
    }

    TreeNodeSecTypeF* secType () const {
        assert (children().size() == 3);
        assert (dynamic_cast<TreeNodeSecTypeF*>(children().at (0)) != 0);
        return static_cast<TreeNodeSecTypeF*>(children().at (0));
    }

    TreeNodeDataTypeF* dataType () const {
        assert (children().size() == 3);
        assert (dynamic_cast<TreeNodeDataTypeF*>(children().at (1)) != 0);
        return static_cast<TreeNodeDataTypeF*>(children().at (1));
    }

    TreeNodeDimTypeF* dimType () const {
        assert (children().size() == 3);
        assert (dynamic_cast<TreeNodeDimTypeF*>(children().at (2)) != 0);
        return static_cast<TreeNodeDimTypeF*>(children().at (2));
    }

protected: /* Fields: */

    friend class TypeChecker;

    inline TreeNodeType(SecrecTreeNodeType type, const YYLTYPE &loc, SecreC::Type* ty)
        : TreeNode(type, loc)
        , m_cachedType (ty)
    { }

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
};


/******************************************************************
  TreeNodeTypeVoid
******************************************************************/

/// Void type.
class TreeNodeTypeVoid: public TreeNodeType {
public: /* Methods: */
    explicit inline TreeNodeTypeVoid(const YYLTYPE &loc)
        : TreeNodeType(NODE_TYPEVOID, loc, new TypeVoid ())
    { }
};


/******************************************************************
  TreeNodeExpr
******************************************************************/

/// Representation for expressions, also tracks type of resulting value (if there is one).
class TreeNodeExpr: public TreeNode {
    public: /* Methods: */
        inline TreeNodeExpr(SecrecTreeNodeType type, const YYLTYPE &loc)
            : TreeNode(type, loc), m_resultType(0) { }
        virtual ~TreeNodeExpr() {
            delete m_resultType;
        }

        virtual ICode::Status accept (TypeChecker& tyChecker) = 0;

        inline bool haveResultType() const { return m_resultType != 0; }

        /// \todo move to type checker
        inline bool havePublicBoolType() const {
            assert(m_resultType != 0);
            return m_resultType->secrecDataType() == DATATYPE_BOOL
                   && m_resultType->secrecSecType().isPublic ()
                   && m_resultType->isScalar();
        }
        inline const SecreC::Type &resultType() const {
            assert(m_resultType != 0);
            return *m_resultType;
        }

        virtual CGResult codeGenWith (CodeGen& cg) = 0;
        virtual CGBranchResult codeGenBoolWith (CodeGen&) {
            assert (false && "Not implemented!");
            return CGBranchResult (ICode::E_NOT_IMPLEMENTED);
        }

    protected: /* Methods: */

        friend class TypeChecker;

        inline void setResultType(SecreC::Type *type) {
            assert(m_resultType == 0);
            m_resultType = type;
        }

        TreeNodeExpr* expressionAt (unsigned i) const {
            assert (i < children ().size ());
            assert (dynamic_cast<TreeNodeExpr*>(children ().at (i)) != 0);
            return static_cast<TreeNodeExpr*>(children ().at (i));
        }

    private: /* Fields: */
        SecreC::Type       *m_resultType; ///< Type of resulting value.
};


/******************************************************************
  TreeNodeExprInt
******************************************************************/

/// Signed integer constant.
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

    private: /* Fields: */
        int m_value;
};


/******************************************************************
  TreeNodeExprAssign
******************************************************************/

/// Assignment expression.
class TreeNodeExprAssign: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprAssign(SecrecTreeNodeType type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

        TreeNode* slice () const {
            assert (children ().size () == 2);
            TreeNode *e1 = children().at(0);
            if (e1->children().size() == 2) {
                return e1->children ().at (1);
            }

            return 0;
        }

        TreeNodeIdentifier* identifier () const {
            assert (children ().size () == 2);
            TreeNode *e1 = children().at(0);
            assert(e1 != 0);
            assert(e1->type() == NODE_LVALUE);
            assert(e1->children().size() > 0 && e1->children().size() <= 2);
            assert(dynamic_cast<TreeNodeIdentifier*>(e1->children().at(0)) != 0);
            return static_cast<TreeNodeIdentifier*>(e1->children().at(0));
        }

        TreeNodeExpr* rightHandSide () const {
            assert(children().size() == 2);
            return expressionAt (1);
        }
};


/******************************************************************
  TreeNodeExprCast
******************************************************************/

/**
 * For now both data and security type casts.
 * \todo Casts of security types will fail for now.
 * \todo Split those when type checking.
 */
class TreeNodeExprCast: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprCast (const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_CAST, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

        TreeNodeExpr* expression () const {
            assert (children ().size () == 2);
            return expressionAt (1);
        }

        TreeNodeDataTypeF* castType () const {
            assert (dynamic_cast<TreeNodeDataTypeF*>(children ().at (0)));
            return static_cast<TreeNodeDataTypeF*>(children ().at (0));
        }
};


/******************************************************************
  TreeNodeExprIndex
******************************************************************/

/// Indexing expressions.
class TreeNodeExprIndex: public TreeNodeExpr {
    public:
        inline TreeNodeExprIndex(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_INDEX, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);
        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

        TreeNodeExpr* expression () const {
            assert (children().size() == 2);
            return expressionAt (0);
        }

        TreeNode* indices () const {
            assert (children().size() == 2);
            return children ().at (1);
        }
};


/******************************************************************
  TreeNodeExprSize
******************************************************************/

/// Size expression.
class TreeNodeExprSize: public TreeNodeExpr {
    public:
        inline TreeNodeExprSize(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_SIZE, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);
        virtual CGResult codeGenWith (CodeGen& cg);

        TreeNodeExpr* expression () const {
            assert (children().size() == 1);
            return expressionAt (0);
        }
};


/******************************************************************
  TreeNodeExprShape
******************************************************************/

/// Shape expression.
class TreeNodeExprShape: public TreeNodeExpr {
    public:
        inline TreeNodeExprShape(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_SHAPE, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);

        TreeNodeExpr* expression () const {
            assert (children().size() == 1);
            return expressionAt (0);
        }
};


/******************************************************************
  TreeNodeExprCat
******************************************************************/

/// Concatenation expression.
class TreeNodeExprCat: public TreeNodeExpr {
    public:
        inline TreeNodeExprCat(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_CAT, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);

        TreeNodeExpr* leftExpression () const {
            assert (children().size() == 3 ||
                    children().size() == 2);
            return expressionAt (0);
        }
        TreeNodeExpr* rightExpression () const {
            assert (children().size() == 3 ||
                    children().size() == 2);
            return expressionAt (1);
        }

        TreeNodeExprInt* dimensionality () const {
            if (children ().size () == 3) {
                TreeNodeExpr* e = expressionAt (2);
                assert (dynamic_cast<TreeNodeExprInt*>(e) != 0);
                return static_cast<TreeNodeExprInt*>(e);
            }

            return 0;
        }
};


/******************************************************************
  TreeNodeExprReshape
******************************************************************/

/// Reshape expression.
class TreeNodeExprReshape: public TreeNodeExpr {
    public:
        inline TreeNodeExprReshape(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_RESHAPE, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);

        /// \todo yeah, figure a better name out...
        /// "reshapee" as the expression that is being reshaped!
        TreeNodeExpr* reshapee () const {
            assert (children ().size() >= 1);
            return expressionAt (0);
        }

        /// \todo iterators
        TreeNodeExpr* dimensionality (unsigned i) {
            return expressionAt (i + 1); /// will perform the range check too
        }
};


/******************************************************************
  TreeNodeExprBinary
******************************************************************/

/// Binary expressions.
class TreeNodeExprBinary: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprBinary(SecrecTreeNodeType type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        const char *operatorString() const;

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

        TreeNodeExpr* leftExpression () const {
            assert (children ().size () == 2);
            return expressionAt (0);
        }

        TreeNodeExpr* rightExpression () const {
            assert (children ().size () == 2);
            return expressionAt (1);
        }
};


/******************************************************************
  TreeNodeExprBool
******************************************************************/

/// Boolean constant.
class TreeNodeExprBool: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprBool(bool value, const YYLTYPE &loc)
            : TreeNodeExpr(NODE_LITE_BOOL, loc), m_value(value) {}

        inline void setValue(bool value) { m_value = value; }
        inline bool value() const { return m_value; }

        virtual inline std::string stringHelper() const {
            return (m_value ? "true" : "false");
        }
        virtual std::string xmlHelper() const;
        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    private: /* Fields: */
        bool m_value;
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
        inline TreeNodeExprClassify(SymbolDomain* dom, const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_CLASSIFY, loc)
            , m_expectedDomain (dom)
        { }

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);

        TreeNodeExpr* expression () const {
            assert (children ().size () == 1);
            return expressionAt (0);
        }

        SymbolDomain* expectedDomain () const {
            return m_expectedDomain;
        }

    private:
        SymbolDomain* const m_expectedDomain;
};


/******************************************************************
  TreeNodeExprDeclassify
******************************************************************/

/// Declassify expression.
class TreeNodeExprDeclassify: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprDeclassify(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_DECLASSIFY, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprProcCall
******************************************************************/

/// Procedure call expression.
class TreeNodeExprProcCall: public TreeNodeExpr {
    public: /* Methods: */
        explicit inline TreeNodeExprProcCall(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_PROCCALL, loc) {}


        virtual ICode::Status accept (TypeChecker& tyChecker);


        void setProcedure (SymbolProcedure* proc) {
            m_procedure = proc;
        }

        inline SymbolProcedure *symbolProcedure() const
            { return m_procedure; }

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

        TreeNodeIdentifier* procName () const {
            assert (!children ().empty ());
            assert(children().at(0)->type() == NODE_IDENTIFIER);
            assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0)) != 0);
            return static_cast<TreeNodeIdentifier*>(children().at(0));
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


        TreeNodeIdentifier* identifier () const {
            assert (children ().size () == 1);
            assert (dynamic_cast<TreeNodeIdentifier*>(children ().at (0)));
            return static_cast<TreeNodeIdentifier*>(children ().at (0));
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


        inline void setValue(const std::string &value) {
            m_value = value;
        }
        inline const std::string &value() const { return m_value; }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;
        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);

    private: /* Fields: */
        std::string m_value;
};


/******************************************************************
  TreeNodeExprTernary
******************************************************************/

/// Ternary expression.
class TreeNodeExprTernary: public TreeNodeExpr {
    public: /* Methods: */
        explicit inline TreeNodeExprTernary(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_TERNIF, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

        TreeNodeExpr* conditional () const {
            assert(children().size() == 3);
            assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
            assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
            return static_cast<TreeNodeExpr*>(children().at(0));
        }

        TreeNodeExpr* trueBranch () const {
            assert(children().size() == 3);
            assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);
            assert(dynamic_cast<TreeNodeExpr*>(children().at(1)) != 0);
            return static_cast<TreeNodeExpr*>(children().at(1));
        }

        TreeNodeExpr* falseBranch () const {
            assert(children().size() == 3);
            assert((children().at(2)->type() & NODE_EXPR_MASK) != 0x0);
            assert(dynamic_cast<TreeNodeExpr*>(children().at(2)) != 0);
            return static_cast<TreeNodeExpr*>(children().at(2));
        }
};


/******************************************************************
  TreeNodeExprUInt
******************************************************************/

/// Unsigned integer constant.
class TreeNodeExprUInt: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprUInt(unsigned value, const YYLTYPE &loc)
            : TreeNodeExpr(NODE_LITE_UINT, loc), m_value(value) {}

        inline void setValue(unsigned value) { m_value = value; }
        inline unsigned value() const { return m_value; }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;
        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);

    private: /* Fields: */
        unsigned m_value;
};


/******************************************************************
  TreeNodeExprPrefix
******************************************************************/

/// Prefix increment and decrement.
class TreeNodeExprPrefix: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprPrefix(SecrecTreeNodeType type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprPostfix
******************************************************************/

/// Postfix increment and decrement.
class TreeNodeExprPostfix: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprPostfix(SecrecTreeNodeType type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprUnary
******************************************************************/

/// Unary expressions such as regular (logical) negation.
class TreeNodeExprUnary: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprUnary(SecrecTreeNodeType type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status accept (TypeChecker& tyChecker);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

        TreeNodeExpr* expression () const {
            return expressionAt (0);
        }
};

/******************************************************************
  TreeNodeStmtKind
******************************************************************/

/// Declaration statement. Also tracks if the scope is global or not.
class TreeNodeKind : public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeKind(const YYLTYPE &loc)
            : TreeNode (NODE_KIND, loc) { }
        virtual inline ~TreeNodeKind() { }
        CGStmtResult codeGenWith (CodeGen& cg);
};

/******************************************************************
  TreeNodeStmtDomain
******************************************************************/

/// Declaration statement. Also tracks if the scope is global or not.
class TreeNodeDomain : public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeDomain(const YYLTYPE &loc)
            : TreeNode (NODE_DOMAIN, loc) { }
        virtual inline ~TreeNodeDomain() { }
        CGStmtResult codeGenWith (CodeGen& cg);
};



/******************************************************************
  TreeNodeProcDef
******************************************************************/

/// Procedure definition.
class TreeNodeProcDef: public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeProcDef(const YYLTYPE &loc)
        : TreeNode(NODE_PROCDEF, loc), m_cachedType(0), m_procSymbol (0)
        {
            setContainingProcedureDirectly(this);
        }
        virtual inline ~TreeNodeProcDef() { delete m_cachedType; }

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

        inline bool haveProcedureType() const { return m_cachedType != 0; }
        const SecreC::TypeNonVoid &procedureType() const {
            assert(m_cachedType != 0);
            return *m_cachedType;
        }

        TreeNodeType* returnType () const {
            assert (children ().size () > 1);
            assert(dynamic_cast<TreeNodeType*>(children().at(1)) != 0);
            return static_cast<TreeNodeType*>(children().at(1));
        }

        ChildrenListConstIterator paramBegin () const {
            assert (children ().size () > 2);
            return children ().begin () + 3;
        }

        ChildrenListConstIterator paramEnd () const {
            return children ().end ();
        }

        CGStmtResult codeGenWith (CodeGen& cg);

    protected: /* Methods: */

        friend class TypeChecker;

    protected: /* Fields: */
        const SecreC::TypeNonVoid *m_cachedType;
        SymbolProcedure           *m_procSymbol;
};


/******************************************************************
  TreeNodeQuantifier
******************************************************************/

class TreeNodeQuantifier : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeQuantifier(const YYLTYPE &loc)
        : TreeNode(NODE_TEMPLATE_QUANT, loc) {}

    TreeNodeIdentifier* domain () {
        assert (children ().size () == 1 || children ().size () == 2);
        assert (dynamic_cast<TreeNodeIdentifier*>(children ().at (0)) != 0);
        return static_cast<TreeNodeIdentifier*>(children ().at (0));
    }

    // will equal to zero, if kind not specified
    TreeNodeIdentifier* kind () {
        assert (children ().size () == 1 || children ().size () == 2);
        if (children ().size () == 2) {
            assert (dynamic_cast<TreeNodeIdentifier*>(children ().at (1)) != 0);
            return static_cast<TreeNodeIdentifier*>(children ().at (1));
        }

        return 0;
    }
};


/******************************************************************
  TreeNodeTemplate
******************************************************************/

class TreeNodeTemplate : public TreeNode {
public: /* Methods: */
    explicit inline TreeNodeTemplate(const YYLTYPE &loc)
        : TreeNode(NODE_TEMPLATE_DECL, loc) {}

    TreeNodeProcDef* body () const {
        assert (children ().size () == 2);
        assert (dynamic_cast<TreeNodeProcDef*>(children ().at (1)) != 0);
        return static_cast<TreeNodeProcDef*>(children ().at (1));
    }

    TreeNode* quantifiers () const {
        assert (children ().size () == 2);
        return children ().at (0);
    }
};

/******************************************************************
  TreeNodeProgram
******************************************************************/

/// Representation of program.
class TreeNodeProgram: public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeProgram(const YYLTYPE &loc)
            : TreeNode(NODE_PROGRAM, loc) {}

        ICode::Status codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmt
******************************************************************/

/// Statements.
class TreeNodeStmt: public TreeNode {
public: /* Methods: */
    inline TreeNodeStmt(SecrecTreeNodeType type, const YYLTYPE &loc)
        : TreeNode(type, loc) {}


    virtual CGStmtResult codeGenWith (CodeGen&) {
        assert (false && "Statement code gen unimplemented.");
        return CGStmtResult (ICode::E_NOT_IMPLEMENTED);
    }

protected:

    TreeNodeStmt* statementAt (unsigned i) const {
        assert (i < children ().size ());
        assert (dynamic_cast<TreeNodeStmt*>(children ().at (i)) != 0);
        return static_cast<TreeNodeStmt*>(children ().at (i));
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
};


/******************************************************************
  TreeNodeStmtDecl
******************************************************************/

/// Declaration statement. Also tracks if the scope is global or not.
class TreeNodeStmtDecl: public TreeNodeStmt {
public: /* Methods: */
    explicit inline TreeNodeStmtDecl(const YYLTYPE &loc)
        : TreeNodeStmt(NODE_DECL, loc), m_type(0), m_global(false),
          m_procParam(false) {}
    virtual inline ~TreeNodeStmtDecl() { delete m_type; }

    const std::string &variableName() const;


    virtual CGStmtResult codeGenWith (CodeGen& cg);

    inline const SecreC::TypeNonVoid &resultType() const {
        assert(m_type != 0);
        return *m_type;
    }
    inline bool haveResultType() const { return m_type != 0; }

    inline bool global() const { return m_global; }
    inline void setGlobal(bool isGlobal = true) { m_global = isGlobal; }
    inline bool procParam() const { return m_procParam; }
    inline void setProcParam(bool procParam = true) { m_procParam = procParam; }

    TreeNodeType* varType () const {
        assert(children().size() >= 2);
        assert(dynamic_cast<TreeNodeType*>(children().at(1)) != 0);
        return static_cast<TreeNodeType*>(children().at(1));
    }

    TreeNode* shape () const {
        if (children().size() > 2) {
            return children ().at (2);
        }

        return 0;
    }

    TreeNodeExpr* rightHandSide () const {
        if (children ().size () > 3) {
            assert(dynamic_cast<TreeNodeExpr*>(children().at(3)) != 0);
            return static_cast<TreeNodeExpr*>(children().at(3));
        }

        return 0;
    }

protected:

    friend class TypeChecker;

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

    TreeNodeExpr* conditional () const {
        assert (children ().size () == 2);
        assert (dynamic_cast<TreeNodeExpr*> (children ().at (1)) != 0);
        return static_cast<TreeNodeExpr*> (children ().at (1));
    }

    TreeNodeStmt* body () const {
        assert (children ().size () == 2);
        return statementAt (0);
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

    TreeNodeExpr* expression () const {
        assert (children ().size () == 1);
        assert (dynamic_cast<TreeNodeExpr*> (children ().at (0)) != 0);
        return static_cast<TreeNodeExpr*> (children ().at (0));
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

    TreeNodeExpr* expression () const {
        assert (children ().size () == 1);
        assert (dynamic_cast<TreeNodeExpr*> (children ().at (0)) != 0);
        return static_cast<TreeNodeExpr*> (children ().at (0));
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

    TreeNode* initializer () const {
        assert (children ().size () == 4);
        if (children ().at (0)->type () != NODE_EXPR_NONE) {
            return children ().at (0);
        }

        return 0;
    }

    TreeNodeExpr* conditional () const {
        assert (children ().size () == 4);
        if (children ().at (1)->type () != NODE_EXPR_NONE) {
            assert (dynamic_cast<TreeNodeExpr*> (children ().at (1)) != 0);
            return static_cast<TreeNodeExpr*> (children ().at (1));
        }

        return 0;
    }

    TreeNodeExpr* iteratorExpr () const {
        assert (children ().size () == 4);
        if (children ().at (2)->type () != NODE_EXPR_NONE) {
            assert (dynamic_cast<TreeNodeExpr*> (children ().at (2)) != 0);
            return static_cast<TreeNodeExpr*> (children ().at (2));
        }

        return 0;
    }

    TreeNodeStmt* body () const {
        assert (children ().size () == 4);
        assert (dynamic_cast<TreeNodeStmt*>(children ().at (3)) != 0);
        return static_cast<TreeNodeStmt*>(children ().at (3));
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

    TreeNodeExpr* conditional () const {
        assert (children ().size () == 2 || children ().size () == 3);
        assert (dynamic_cast<TreeNodeExpr*> (children ().at (0)) != 0);
        return (static_cast<TreeNodeExpr*> (children ().at (0)));
    }

    TreeNodeStmt* trueBranch () const {
        assert (children ().size () == 2 || children ().size () == 3);
        return statementAt (1);
    }

    TreeNodeStmt* falseBranch () const {
        assert (children ().size () == 2 || children ().size () == 3);
        if (children ().size () == 3) {
            return statementAt (2);
        }

        return 0;
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

    TreeNodeExpr* expression () const {
        if (!children ().empty ()) {
            assert (dynamic_cast<TreeNodeExpr*> (children ().at (0)) != 0);
            return static_cast<TreeNodeExpr*> (children ().at (0));
        }

        return 0;
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

    TreeNodeExpr* conditional () const {
        assert (children ().size () == 2);
        assert (dynamic_cast<TreeNodeExpr*> (children ().at (0)) != 0);
        return static_cast<TreeNodeExpr*> (children ().at (0));
    }

    TreeNodeStmt* body () const {
        assert (children ().size () == 2);
        return statementAt (1);
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

    TreeNodeExpr* expression () const {
        assert (children ().size () == 1);
        assert (dynamic_cast<TreeNodeExpr*> (children ().at (0)) != 0);
        return static_cast<TreeNodeExpr*> (children ().at (0));
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

    TreeNodeExprString* expression () const {
        assert (children ().size () == 1);
        assert (dynamic_cast<TreeNodeExprString*> (children ().at (0)) != 0);
        return static_cast<TreeNodeExprString*> (children ().at (0));
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

    TreeNodeExpr* expression () const {
        assert (children ().size () == 1);
        assert (dynamic_cast<TreeNodeExpr*> (children ().at (0)) != 0);
        return static_cast<TreeNodeExpr*> (children ().at (0));
    }
};

} // namespace SecreC

#endif /* #ifdef __cplusplus */

#endif /* TREENODE_H */
