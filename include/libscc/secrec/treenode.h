#ifndef TREENODE_H
#define TREENODE_H

#include "parser.h"

#ifdef __cplusplus
#include <cassert>
#include <deque>
#include <string>
#include "../intermediate.h"
#include "types.h"

namespace SecreC {

class TreeNodeFundef;

class TreeNode {
    public: /* Types: */
        typedef enum SecrecTreeNodeType Type;
        typedef std::deque<TreeNode*> ChildrenList;
        typedef ChildrenList::iterator ChildrenListIterator;
        typedef ChildrenList::const_iterator ChildrenListConstIterator;

    public: /* Methods: */
        explicit TreeNode(Type type, const YYLTYPE &loc);
        virtual ~TreeNode();

        TreeNodeFundef* containingFunction();
        inline TreeNode* parent() const { return m_parent; }
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

    protected: /* Methods: */
        inline void setParentDirectly(TreeNode *parent) { m_parent = parent; }
        inline void setContainingFunctionDirectly(TreeNodeFundef *f) { m_function = f; }
        virtual inline void resetParent(TreeNode *parent) {
            m_parent = parent;
            m_function = parent->m_function;
        }

    private: /* Fields: */
        TreeNode       *m_parent;
        TreeNodeFundef *m_function;
        const Type      m_type;
        ChildrenList    m_children;
        YYLTYPE         m_location;
};

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
struct TreeNode *treenode_init_dataTypeArray(unsigned value, YYLTYPE *loc);

#ifdef __cplusplus
} /* extern "C" */
} /* namespace SecreC */

std::ostream &operator<<(std::ostream &out, const YYLTYPE &loc);

namespace SecreC {

/******************************************************************
  TreeNodeCodeable
******************************************************************/

class TreeNodeCodeable: public TreeNode {
    public: /* Methods: */
        TreeNodeCodeable(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc), m_firstImop(0) {}
        virtual inline ~TreeNodeCodeable() {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es) = 0;

        inline const std::vector<Imop*> &breakList() const {
            return m_breakList;
        }
        inline const std::vector<Imop*> &continueList() const {
            return m_continueList;
        }
        inline const std::vector<Imop*> &nextList() const {
            return m_nextList;
        }
        inline Imop *firstImop() const {
            return m_firstImop;
        }

        void patchBreakList(Imop *dest);
        void patchContinueList(Imop *dest);
        void patchNextList(Imop *dest);

    protected: /* Methods: */
        inline void setBreakList(const std::vector<Imop*> &bl) {
            assert(m_breakList.empty());
            m_breakList = bl;
        }
        inline void addToBreakList(Imop *i) {
            m_breakList.push_back(i);
        }
        void addToBreakList(const std::vector<Imop*> &bl);
        inline void setContinueList(const std::vector<Imop*> &cl) {
            assert(m_continueList.empty());
            m_continueList = cl;
        }
        inline void addToContinueList(Imop *i) {
            m_continueList.push_back(i);
        }
        void addToContinueList(const std::vector<Imop*> &cl);
        inline void setNextList(const std::vector<Imop*> &nl) {
            assert(m_nextList.empty());
            m_nextList = nl;
        }
        inline void addToNextList(Imop *i) {
            m_nextList.push_back(i);
        }
        void addToNextList(const std::vector<Imop*> &nl);
        inline void setFirstImop(Imop *imop) {
            assert(m_firstImop == 0);
            m_firstImop = imop;
        }
        inline void patchFirstImop(Imop *imop) {
            if (m_firstImop != 0) return;
            m_firstImop = imop;
        }

    private: /* Fields: */
        std::vector<Imop*> m_breakList;
        std::vector<Imop*> m_continueList;
        std::vector<Imop*> m_nextList;
        Imop              *m_firstImop;
};


/******************************************************************
  TreeNodeDataType
******************************************************************/

class TreeNodeDataType: public TreeNode {
    public: /* Methods: */
        inline TreeNodeDataType(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc) {}

        virtual const DataType &dataType() const = 0;
};

/******************************************************************
  TreeNodeDataTypeArray
******************************************************************/

class TreeNodeDataTypeArray: public TreeNodeDataType {
    public: /* Methods: */
        inline TreeNodeDataTypeArray(unsigned dim,
                                     const YYLTYPE &loc)
            : TreeNodeDataType(NODE_DATATYPE_ARRAY, loc),
              m_dim(dim), m_cachedType(0) {}
        inline ~TreeNodeDataTypeArray() {
            delete m_cachedType;
        }

        virtual const DataType &dataType() const;

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;

    private: /* Fields: */
        unsigned m_dim;
        mutable DataType *m_cachedType;
};


/******************************************************************
  TreeNodeDataTypeF
******************************************************************/

class TreeNodeDataTypeF: public TreeNodeDataType {
    public: /* Methods: */
        inline TreeNodeDataTypeF(SecrecDataType dataType,
                                 const YYLTYPE &loc)
            : TreeNodeDataType(NODE_DATATYPE_F, loc),
              m_cachedType(dataType) {}

        virtual inline const DataType &dataType() const {
            return m_cachedType;
        }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;

    private: /* Fields: */
        DataTypeBasic m_cachedType;
};


/******************************************************************
  TreeNodeExpr
******************************************************************/

class TreeNodeExpr: public TreeNode {
    public: /* Types: */
        enum Flags { CONSTANT = 0x01, PARENTHESIS = 0x02 };

    public: /* Methods: */
        explicit TreeNodeExpr(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc), m_result(0), m_resultType(0),
              m_firstImop(0) {}
        virtual ~TreeNodeExpr() {
            delete m_resultType;
        }

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es) = 0;
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0) = 0;
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es) = 0;

        inline SymbolWithValue *result() const {
            assert(m_result != 0);
            return m_result;
        };
        inline SecreC::Type *resultType() const {
            return m_resultType;
        }
        inline const std::vector<Imop*> &falseList() const {
            return m_falseList;
        }
        inline const std::vector<Imop*> &trueList() const {
            return m_trueList;
        }
        inline const std::vector<Imop*> &nextList() const {
            return m_nextList;
        }
        inline Imop *firstImop() const {
            return m_firstImop;
        }
        void patchTrueList(Imop *dest);
        void patchFalseList(Imop *dest);
        void patchNextList(Imop *dest);

    protected: /* Methods: */
        inline void setResult(SymbolWithValue *r) {
            m_result = r;
        }
        inline void setFalseList(const std::vector<Imop*> &fl) {
            assert(m_falseList.empty());
            m_falseList = fl;
        }
        inline void addToFalseList(Imop *i) {
            m_falseList.push_back(i);
        }
        void addToFalseList(const std::vector<Imop*> &fl);
        inline void setTrueList(const std::vector<Imop*> &tl) {
            assert(m_trueList.empty());
            m_trueList = tl;
        }
        inline void addToTrueList(Imop *i) {
            m_trueList.push_back(i);
        }
        void addToTrueList(const std::vector<Imop*> &bl);
        inline void setNextList(const std::vector<Imop*> &nl) {
            assert(m_nextList.empty());
            m_nextList = nl;
        }
        inline void addToNextList(Imop *i) {
            m_nextList.push_back(i);
        }
        void addToNextList(const std::vector<Imop*> &bl);
        inline void setResultType(SecreC::Type *type) {
            assert(m_resultType == 0);
            m_resultType = type;
        }
        inline void setFirstImop(Imop *imop) {
            assert(m_firstImop == 0);
            m_firstImop = imop;
        }
        inline void patchFirstImop(Imop *imop) {
            if (m_firstImop != 0) return;
            m_firstImop = imop;
        }

    private: /* Fields: */
        SymbolWithValue    *m_result;
        SecreC::Type       *m_resultType;
        std::vector<Imop*>  m_falseList;
        std::vector<Imop*>  m_trueList;
        std::vector<Imop*>  m_nextList;
        Imop               *m_firstImop;

        /// \todo Add flags.
};


/******************************************************************
  TreeNodeExprAssign
******************************************************************/

class TreeNodeExprAssign: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprAssign(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);
};


/******************************************************************
  TreeNodeExprBinary
******************************************************************/

class TreeNodeExprBinary: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprBinary(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);
};


/******************************************************************
  TreeNodeExprBool
******************************************************************/

class TreeNodeExprBool: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprBool(bool value, const YYLTYPE &loc)
            : TreeNodeExpr(NODE_LITE_BOOL, loc), m_value(value) {}

        inline void setValue(bool value) { m_value = value; }
        inline bool value() const { return m_value; }

        virtual inline std::string stringHelper() const {
            return (m_value ? "true" : "false");
        }
        virtual std::string xmlHelper() const;
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);

    private: /* Fields: */
        bool m_value;
};


/******************************************************************
  TreeNodeExprInt
******************************************************************/

class TreeNodeExprInt: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprInt(int value, const YYLTYPE &loc)
            : TreeNodeExpr(NODE_LITE_INT, loc), m_value(value) {}

        inline void setValue(int value) { m_value = value; }
        inline int value() const { return m_value; }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);

    private: /* Fields: */
        int m_value;
};


/******************************************************************
  TreeNodeExprRVariable
******************************************************************/

class TreeNodeExprRVariable: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprRVariable(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_RVARIABLE, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);
};


/******************************************************************
  TreeNodeExprString
******************************************************************/

class TreeNodeExprString: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprString(const std::string &value,
                                    const YYLTYPE &loc)
            : TreeNodeExpr(NODE_LITE_STRING, loc), m_value(value) {}

        inline void setValue(const std::string &value) { m_value = value; }
        inline const std::string &value() const { return m_value; }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);

    private: /* Fields: */
        std::string m_value;
};


/******************************************************************
  TreeNodeExprTernary
******************************************************************/

class TreeNodeExprTernary: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprTernary(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_TERNIF, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);
};


/******************************************************************
  TreeNodeExprUInt
******************************************************************/

class TreeNodeExprUInt: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprUInt(unsigned value, const YYLTYPE &loc)
            : TreeNodeExpr(NODE_LITE_UINT, loc), m_value(value) {}

        inline void setValue(unsigned value) { m_value = value; }
        inline unsigned value() const { return m_value; }

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);

    private: /* Fields: */
        unsigned m_value;
};


/******************************************************************
  TreeNodeExprUnary
******************************************************************/

class TreeNodeExprUnary: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprUnary(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);
};


/******************************************************************
  TreeNodeFundef
******************************************************************/

class TreeNodeFundef: public TreeNodeCodeable {
    public: /* Methods: */
        explicit TreeNodeFundef(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_FUNDEF, loc), m_cachedType(0)
        {
            setContainingFunctionDirectly(this);
        }
        virtual inline ~TreeNodeFundef() { delete m_cachedType; }

        virtual inline void resetParent(TreeNode *parent) {
            setParentDirectly(parent);
        }

        const SecreC::TypeNonVoid &functionType() const;

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);

    private: /* Fields: */
        mutable const SecreC::TypeNonVoid *m_cachedType;
};


/******************************************************************
  TreeNodeFundefs
******************************************************************/

class TreeNodeFundefs: public TreeNodeCodeable {
    public: /* Methods: */
        explicit TreeNodeFundefs(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_FUNDEFS, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};


/******************************************************************
  TreeNodeGlobals
******************************************************************/

class TreeNodeGlobals: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeGlobals(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_GLOBALS, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};


/******************************************************************
  TreeNodeIdentifier
******************************************************************/

class TreeNodeIdentifier: public TreeNode {
    public: /* Methods: */
        explicit TreeNodeIdentifier(const std::string &value,
                                    const YYLTYPE &loc)
            : TreeNode(NODE_IDENTIFIER, loc), m_value(value) {}

        inline void setValue(const std::string &value) { m_value = value; }
        inline const std::string &value() const { return m_value; }
        SymbolSymbol *getSymbol(SymbolTable &st, std::ostream &es) const;

        virtual std::string stringHelper() const;
        virtual std::string xmlHelper() const;

    private: /* Fields: */
        std::string m_value;
};


/******************************************************************
  TreeNodeProgram
******************************************************************/

class TreeNodeProgram: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeProgram(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_PROGRAM, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code, SymbolTable &st,
                                           std::ostream &es);
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
  TreeNodeStmt
******************************************************************/

class TreeNodeStmt: public TreeNodeCodeable {
    public: /* Types: */
        enum ResultClass {
            FALLTHRU = 0x01,
            RETURN   = 0x02,
            BREAK    = 0x04,
            CONTINUE = 0x08,
            MASK     = 0x0f
        };

    public: /* Methods: */
        TreeNodeStmt(Type type, const YYLTYPE &loc)
            : TreeNodeCodeable(type, loc), m_resultFlags(0) {}

        inline int resultFlags() const { return m_resultFlags; }
        inline void setResultFlags(int flags) { m_resultFlags = flags; }

    private: /* Fields: */
        int m_resultFlags;
};


/******************************************************************
  TreeNodeStmtBreak
******************************************************************/

class TreeNodeStmtBreak: public TreeNodeStmt {
    public: /* Methods: */
        TreeNodeStmtBreak(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_BREAK, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};

/******************************************************************
  TreeNodeStmtCompound
******************************************************************/

class TreeNodeStmtCompound: public TreeNodeStmt {
    public: /* Methods: */
        TreeNodeStmtCompound(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_COMPOUND, loc) {}
        virtual inline ~TreeNodeStmtCompound() {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};


/******************************************************************
  TreeNodeStmtContinue
******************************************************************/

class TreeNodeStmtContinue: public TreeNodeStmt {
    public: /* Methods: */
        TreeNodeStmtContinue(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_CONTINUE, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};


/******************************************************************
  TreeNodeStmtDecl
******************************************************************/

class TreeNodeStmtDecl: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtDecl(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_DECL, loc), m_global(false) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);

        inline bool global() const { return m_global; }
        inline void setGlobal(bool isGlobal = true) { m_global = isGlobal; }

    private: /* Fields: */
        bool m_global;
};


/******************************************************************
  TreeNodeStmtExpr
******************************************************************/

class TreeNodeStmtExpr: public TreeNodeStmt {
    public: /* Methods: */
        TreeNodeStmtExpr(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_EXPR, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};


/******************************************************************
  TreeNodeStmtFor
******************************************************************/

class TreeNodeStmtFor: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtFor(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_FOR, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};


/******************************************************************
  TreeNodeStmtIf
******************************************************************/

class TreeNodeStmtIf: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtIf(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_IF, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};


/******************************************************************
  TreeNodeStmtReturn
******************************************************************/

class TreeNodeStmtReturn: public TreeNodeStmt {
    public: /* Methods: */
        TreeNodeStmtReturn(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_RETURN, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};


/******************************************************************
  TreeNodeType
******************************************************************/

class TreeNodeType: public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeType(Type type, const YYLTYPE &loc)
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
