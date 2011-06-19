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
class TreeNodeExpr;
class TreeNodeProcDef;
class TreeNode;

// C interface
extern "C" {

#endif /* #ifdef __cplusplus */

/* C interface for yacc: */

struct TreeNode *treenode_init(enum SecrecTreeNodeType type, const YYLTYPE *loc);
void treenode_free(struct TreeNode *node);
enum SecrecTreeNodeType treenode_type(struct TreeNode *node);
const YYLTYPE *treenode_location(const struct TreeNode *node);
unsigned treenode_numChildren(struct TreeNode *node);
struct TreeNode *treenode_childAt(struct TreeNode *node, unsigned index);
void treenode_appendChild(struct TreeNode *parent, struct TreeNode *child);
void treenode_prependChild(struct TreeNode *parent, struct TreeNode *child);
void treenode_setLocation(struct TreeNode *node, YYLTYPE *loc);

struct TreeNode *treenode_init_bool(unsigned value, YYLTYPE *loc);
struct TreeNode *treenode_init_int(int value, YYLTYPE *loc);
struct TreeNode *treenode_init_uint(unsigned value, YYLTYPE *loc);
struct TreeNode *treenode_init_string(const char *value, YYLTYPE *loc);
struct TreeNode *treenode_init_identifier(const char *value, YYLTYPE *loc);
struct TreeNode *treenode_init_secTypeF(enum SecrecSecType secType,
                                        YYLTYPE *loc);
struct TreeNode *treenode_init_dataTypeF(enum SecrecDataType dataType,
                                         YYLTYPE *loc);
struct TreeNode *treenode_init_dimTypeF(unsigned dimType,
                                        YYLTYPE *loc);

#ifdef __cplusplus
} /* extern "C" */

class TreeNode {
    public: /* Types: */
        typedef enum SecrecTreeNodeType Type;
        typedef std::deque<TreeNode*> ChildrenList;
        typedef ChildrenList::iterator ChildrenListIterator;
        typedef ChildrenList::const_iterator ChildrenListConstIterator;

    public: /* Methods: */
        TreeNode(Type type, const YYLTYPE &loc);
        virtual ~TreeNode();

        TreeNodeProcDef* containingProcedure();
        inline TreeNode* parent() const { return m_parent; }
        inline bool hasParent() const { return m_parent != 0; }
        inline Type type() const { return m_type; }
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

        static const char *typeName(Type type);

        TreeNodeExpr *classifyChildAtIfNeeded(int index, SecrecSecType otherSecType);

    protected: /* Methods: */
        inline void setParentDirectly(TreeNode *parent) { m_parent = parent; }
        inline void setContainingProcedureDirectly(TreeNodeProcDef *p) { m_procedure = p; }
        virtual inline void resetParent(TreeNode *parent) {
            m_parent = parent;
            m_procedure = parent->m_procedure;
        }

    private: /* Fields: */
        TreeNode        *m_parent;
        TreeNodeProcDef *m_procedure;
        const Type       m_type;
        ChildrenList     m_children;
        YYLTYPE          m_location;
};


/******************************************************************
  TreeNodeSecTypeF
******************************************************************/

class TreeNodeSecTypeF: public TreeNode {
    public: /* Methods: */
        inline TreeNodeSecTypeF(SecrecSecType secType, const YYLTYPE &loc)
            : TreeNode(NODE_SECTYPE_F, loc), m_secType(secType) {}

        SecrecSecType secType() const { return m_secType; }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;

    private: /* Fields: */
        SecrecSecType m_secType;
};


/******************************************************************
  TreeNodeDataType
******************************************************************/

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
  TreeNodeExpr
******************************************************************/

class TreeNodeExpr: public TreeNode {
    public: /* Types: */
        enum Flags { CONSTANT = 0x01, PARENTHESIS = 0x02 };

    public: /* Methods: */
        inline TreeNodeExpr(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc), m_result(0), m_resultType(0) { }
        virtual ~TreeNodeExpr() {
            delete m_resultType;
        }

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log) = 0;

        /// @brief common usage is: if (checkAndLogIfVoid(log)) return ICode::E_TYPE;
        /// @return true if type is void, otherwise false
        bool checkAndLogIfVoid(CompileLog& log);

        inline bool haveResultType() const { return m_resultType != 0; }
        inline bool havePublicBoolType() const {
            assert(m_resultType != 0);
            return m_resultType->secrecDataType() == DATATYPE_BOOL
                   && m_resultType->secrecSecType() == SECTYPE_PUBLIC
                   && m_resultType->isScalar();
        }
        inline const SecreC::Type &resultType() const {
            assert(m_resultType != 0);
            return *m_resultType;
        }

        virtual CGResult codeGenWith (CodeGen& cg) = 0;
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg) {
            assert (false && "Not implemented!");
            (void) cg;
            return CGBranchResult (ICode::E_NOT_IMPLEMENTED);
        }

    protected: /* Methods: */

        inline void setResultType(SecreC::Type *type) {
            assert(m_resultType == 0);
            m_resultType = type;
        }

    private: /* Fields: */
        Symbol             *m_result;
        SecreC::Type       *m_resultType;
};


/******************************************************************
  TreeNodeExprAssign
******************************************************************/

class TreeNodeExprAssign: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprAssign(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprIndex
******************************************************************/

class TreeNodeExprIndex: public TreeNodeExpr {
    public:
        inline TreeNodeExprIndex(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_INDEX, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);
        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprSize
******************************************************************/

class TreeNodeExprSize: public TreeNodeExpr {
    public:
        inline TreeNodeExprSize(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_SIZE, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);
        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprShape
******************************************************************/

class TreeNodeExprShape: public TreeNodeExpr {
    public:
        inline TreeNodeExprShape(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_SHAPE, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprCat
******************************************************************/

class TreeNodeExprCat: public TreeNodeExpr {
    public:
        inline TreeNodeExprCat(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_CAT, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprReshape
******************************************************************/

class TreeNodeExprReshape: public TreeNodeExpr {
    public:
        inline TreeNodeExprReshape(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_RESHAPE, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprFRead
******************************************************************/

class TreeNodeExprFRead: public TreeNodeExpr {
    public:
        inline TreeNodeExprFRead(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_RESHAPE, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprBinary
******************************************************************/

class TreeNodeExprBinary: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprBinary(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        const char *operatorString() const;
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprBool
******************************************************************/

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
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    private: /* Fields: */
        bool m_value;
};


/******************************************************************
  TreeNodeExprClassify
******************************************************************/

class TreeNodeExprClassify: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprClassify(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_CLASSIFY, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprDeclassify
******************************************************************/

class TreeNodeExprDeclassify: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprDeclassify(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_DECLASSIFY, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprProcCall
******************************************************************/

class TreeNodeExprProcCall: public TreeNodeExpr {
    public: /* Methods: */
        explicit inline TreeNodeExprProcCall(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_PROCCALL, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);


        inline SymbolProcedure *symbolProcedure() const
            { return m_procedure; }

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);

    private: /* Fields: */
        SymbolProcedure *m_procedure;
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
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);


        virtual CGResult codeGenWith (CodeGen& cg);

    private: /* Fields: */
        int m_value;
};


/******************************************************************
  TreeNodeExprRVariable
******************************************************************/

class TreeNodeExprRVariable: public TreeNodeExpr {
    public: /* Methods: */
        explicit inline TreeNodeExprRVariable(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_RVARIABLE, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprString
******************************************************************/

class TreeNodeExprString: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprString(const std::string &value, const YYLTYPE &loc)
            : TreeNodeExpr(NODE_LITE_STRING, loc), m_value(value) {}

        inline void setValue(const std::string &value) { m_value = value; }
        inline const std::string &value() const { return m_value; }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);

    private: /* Fields: */
        std::string m_value;
};


/******************************************************************
  TreeNodeExprTernary
******************************************************************/

class TreeNodeExprTernary: public TreeNodeExpr {
    public: /* Methods: */
        explicit inline TreeNodeExprTernary(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_TERNIF, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprUInt
******************************************************************/

class TreeNodeExprUInt: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprUInt(unsigned value, const YYLTYPE &loc)
            : TreeNodeExpr(NODE_LITE_UINT, loc), m_value(value) {}

        inline void setValue(unsigned value) { m_value = value; }
        inline unsigned value() const { return m_value; }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);

    private: /* Fields: */
        unsigned m_value;
};


/******************************************************************
  TreeNodeExprPrefix
******************************************************************/

class TreeNodeExprPrefix: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprPrefix(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprPostfix
******************************************************************/

class TreeNodeExprPostfix: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprPostfix(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeExprUnary
******************************************************************/

class TreeNodeExprUnary: public TreeNodeExpr {
    public: /* Methods: */
        inline TreeNodeExprUnary(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);

        virtual CGResult codeGenWith (CodeGen& cg);
        virtual CGBranchResult codeGenBoolWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeProcDef
******************************************************************/

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
        ICode::Status calculateProcedureType(SymbolTable &st,
                                             CompileLog &log);
        inline bool haveProcedureType() const { return m_cachedType != 0; }
        const SecreC::TypeNonVoid &procedureType() const {
            assert(m_cachedType != 0);
            return *m_cachedType;
        }


        CGStmtResult codeGenWith (CodeGen& cg);

    private: /* Methods: */
        ICode::Status addParameters(DataTypeProcedureVoid &dt,
                                    SymbolTable &stable,
                                    CompileLog &log) const;

    private: /* Fields: */
        const SecreC::TypeNonVoid *m_cachedType;
        SymbolProcedure           *m_procSymbol;
};


/******************************************************************
  TreeNodeProcDefs
******************************************************************/

class TreeNodeProcDefs: public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeProcDefs(const YYLTYPE &loc)
            : TreeNode(NODE_PROCDEFS, loc) {}

        CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeGlobals
******************************************************************/

class TreeNodeGlobals: public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeGlobals(const YYLTYPE &loc)
            : TreeNode(NODE_GLOBALS, loc) {}

        CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeIdentifier
******************************************************************/

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
  TreeNodeProgram
******************************************************************/

class TreeNodeProgram: public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeProgram(const YYLTYPE &loc)
            : TreeNode(NODE_PROGRAM, loc) {}

        ICode::Status codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmt
******************************************************************/

class TreeNodeStmt: public TreeNode {
    public: /* Methods: */
        inline TreeNodeStmt(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg) {
            assert (false && "Statememnt code gen unimplemented.");
            (void) cg;
            return CGStmtResult (ICode::E_NOT_IMPLEMENTED);
        }
};


/******************************************************************
  TreeNodeStmtBreak
******************************************************************/

class TreeNodeStmtBreak: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtBreak(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_BREAK, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmtCompound
******************************************************************/

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

class TreeNodeStmtContinue: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtContinue(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_CONTINUE, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmtDecl
******************************************************************/

class TreeNodeStmtDecl: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtDecl(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_DECL, loc), m_type(0), m_global(false),
              m_procParam(false) {}
        virtual inline ~TreeNodeStmtDecl() { delete m_type; }

        const std::string &variableName() const;

        virtual CGStmtResult codeGenWith (CodeGen& cg);

        ICode::Status calculateResultType(SymbolTable &st,
                                          CompileLog &log);
        inline const SecreC::TypeNonVoid &resultType() const {
            assert(m_type != 0);
            return *m_type;
        }
        inline bool haveResultType() const { return m_type != 0; }

        inline bool global() const { return m_global; }
        inline void setGlobal(bool isGlobal = true) { m_global = isGlobal; }
        inline bool procParam() const { return m_procParam; }
        inline void setProcParam(bool procParam = true) { m_procParam = procParam; }

    private: /* Fields: */
        SecreC::TypeNonVoid *m_type;
        bool m_global;
        bool m_procParam;
};


/******************************************************************
  TreeNodeStmtDoWhile
******************************************************************/

class TreeNodeStmtDoWhile: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtDoWhile(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_DOWHILE, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmtExpr
******************************************************************/

class TreeNodeStmtExpr: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtExpr(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_EXPR, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};

/******************************************************************
  TreeNodeStmtAssert
******************************************************************/

class TreeNodeStmtAssert: public TreeNodeStmt {
    public:
        explicit TreeNodeStmtAssert(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_ASSERT, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmtFor
******************************************************************/

class TreeNodeStmtFor: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtFor(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_FOR, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmtIf
******************************************************************/

class TreeNodeStmtIf: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtIf(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_IF, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmtReturn
******************************************************************/

class TreeNodeStmtReturn: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtReturn(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_RETURN, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmtWhile
******************************************************************/

class TreeNodeStmtWhile: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtWhile(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_WHILE, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeStmtPrint
******************************************************************/

class TreeNodeStmtPrint: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtPrint(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_PRINT, loc) {}

        virtual CGStmtResult codeGenWith (CodeGen& cg);
};


/******************************************************************
  TreeNodeType
******************************************************************/

class TreeNodeType: public TreeNode {
    public: /* Methods: */
        inline TreeNodeType(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc) {}

        virtual const SecreC::Type &secrecType() const = 0;
};


/******************************************************************
  TreeNodeTypeType
******************************************************************/

class TreeNodeTypeType: public TreeNodeType {
    public: /* Methods: */
        explicit inline TreeNodeTypeType(const YYLTYPE &loc)
            : TreeNodeType(NODE_TYPETYPE, loc), m_cachedType(0) {}
        virtual inline ~TreeNodeTypeType() { delete m_cachedType; }

        virtual const SecreC::Type &secrecType() const;
        virtual std::string stringHelper() const;

    private: /* Fields: */
        mutable SecreC::TypeNonVoid *m_cachedType;
};


/******************************************************************
  TreeNodeTypeVoid
******************************************************************/

class TreeNodeTypeVoid: public TreeNodeType {
    public: /* Methods: */
        explicit inline TreeNodeTypeVoid(const YYLTYPE &loc)
            : TreeNodeType(NODE_TYPEVOID, loc) {}

        virtual inline const SecreC::Type &secrecType() const {
            return m_typeVoid;
        }

    private: /* Fields: */
        const SecreC::TypeVoid m_typeVoid;
};


} // namespace SecreC

#endif /* #ifdef __cplusplus */

#endif /* TREENODE_H */
