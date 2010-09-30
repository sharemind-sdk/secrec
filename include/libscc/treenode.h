#ifndef TREENODE_H
#define TREENODE_H

#include "parser.h"

#ifdef __cplusplus
#include <cassert>
#include <deque>
#include <string>
#include "intermediate.h"
#include "types.h"

namespace SecreC {

class CompileLog;
class TreeNodeExpr;
class TreeNodeProcDef;

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
        inline bool isChildOf(TreeNode *parent) const {
            for (TreeNode *p = m_parent; p != 0; p = p->m_parent)
                if (p == parent) return true;
            return false;
        }
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
        inline void setContainingProcedureDirectly(TreeNodeProcDef *p) { m_procedure = p; }
        virtual inline void resetParent(TreeNode *parent) {
            m_parent = parent;
            m_procedure = parent->m_procedure;
        }
        TreeNodeExpr *classifyChildAtIfNeeded(int index, SecrecSecType otherSecType);

    private: /* Fields: */
        TreeNode        *m_parent;
        TreeNodeProcDef *m_procedure;
        const Type       m_type;
        ChildrenList     m_children;
        YYLTYPE          m_location;
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
struct TreeNode *treenode_init_dimTypeF(unsigned dimType,
                                        YYLTYPE *loc);

#ifdef __cplusplus
} /* extern "C" */
} /* namespace SecreC */

namespace SecreC {

/******************************************************************
  TreeNodeCodeable
******************************************************************/

class TreeNodeCodeable: public TreeNode {
    public: /* Methods: */
        inline TreeNodeCodeable(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc), m_firstImop(0) {}
        virtual inline ~TreeNodeCodeable() {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log) = 0;

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
            : TreeNode(type, loc), m_result(0), m_resultType(0),
              m_firstImop(0) {}
        virtual ~TreeNodeExpr() {
            delete m_resultType;
        }

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log) = 0;
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0) = 0;
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log) = 0;

        inline Symbol *result() const { return m_result; };
        inline bool haveResultType() const { return m_resultType != 0; }
        inline bool havePublicBoolType() const {
            assert(m_resultType != 0);
            return m_resultType->secrecDataType() == DATATYPE_BOOL
                   && m_resultType->secrecSecType() == SECTYPE_PUBLIC;
        }
        inline const SecreC::Type &resultType() const {
            assert(m_resultType != 0);
            return *m_resultType;
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
        inline void setResult(Symbol *r) {
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
        Symbol    *m_result;
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
        inline TreeNodeExprAssign(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  CompileLog &log);
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);

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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);

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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);

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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);

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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);

    private: /* Fields: */
        unsigned m_value;
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
        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log,
                                           Symbol *result = 0);
        virtual ICode::Status generateBoolCode(ICodeList &code,
                                               SymbolTable &st,
                                               CompileLog &log);
};


/******************************************************************
  TreeNodeProcDef
******************************************************************/

class TreeNodeProcDef: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeProcDef(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_PROCDEF, loc), m_cachedType(0)
        {
            setContainingProcedureDirectly(this);
        }
        virtual inline ~TreeNodeProcDef() { delete m_cachedType; }

        virtual inline void resetParent(TreeNode *parent) {
            setParentDirectly(parent);
        }

        const std::string &procedureName() const;
        ICode::Status calculateProcedureType(SymbolTable &st,
                                             CompileLog &log);
        inline bool haveProcedureType() const { return m_cachedType != 0; }
        const SecreC::TypeNonVoid &procedureType() const {
            assert(m_cachedType != 0);
            return *m_cachedType;
        }

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);

    private: /* Methods: */
        ICode::Status addParameters(DataTypeProcedureVoid &dt,
                                    SymbolTable &stable,
                                    CompileLog &log) const;

    private: /* Fields: */
        const SecreC::TypeNonVoid *m_cachedType;
};


/******************************************************************
  TreeNodeProcDefs
******************************************************************/

class TreeNodeProcDefs: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeProcDefs(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_PROCDEFS, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
};


/******************************************************************
  TreeNodeGlobals
******************************************************************/

class TreeNodeGlobals: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeGlobals(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_GLOBALS, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
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

class TreeNodeProgram: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeProgram(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_PROGRAM, loc) {}

        virtual ICode::Status generateCode(ICodeList &code, SymbolTable &st,
                                           CompileLog &log);
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
        inline TreeNodeStmt(Type type, const YYLTYPE &loc)
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
        explicit inline TreeNodeStmtBreak(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_BREAK, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
};

/******************************************************************
  TreeNodeStmtCompound
******************************************************************/

class TreeNodeStmtCompound: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtCompound(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_COMPOUND, loc) {}
        virtual inline ~TreeNodeStmtCompound() {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
};


/******************************************************************
  TreeNodeStmtContinue
******************************************************************/

class TreeNodeStmtContinue: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtContinue(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_CONTINUE, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
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

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);

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

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
};


/******************************************************************
  TreeNodeStmtExpr
******************************************************************/

class TreeNodeStmtExpr: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtExpr(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_EXPR, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
};

/******************************************************************
  TreeNodeStmtAssert
******************************************************************/

class TreeNodeStmtAssert: public TreeNodeStmt {
    public:
        explicit TreeNodeStmtAssert(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_ASSERT, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
};

/******************************************************************
  TreeNodeStmtFor
******************************************************************/

class TreeNodeStmtFor: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtFor(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_FOR, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
};


/******************************************************************
  TreeNodeStmtIf
******************************************************************/

class TreeNodeStmtIf: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtIf(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_IF, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
};


/******************************************************************
  TreeNodeStmtReturn
******************************************************************/

class TreeNodeStmtReturn: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtReturn(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_RETURN, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
};


/******************************************************************
  TreeNodeStmtWhile
******************************************************************/

class TreeNodeStmtWhile: public TreeNodeStmt {
    public: /* Methods: */
        explicit inline TreeNodeStmtWhile(const YYLTYPE &loc)
            : TreeNodeStmt(NODE_STMT_WHILE, loc) {}

        virtual ICode::Status generateCode(ICodeList &code,
                                           SymbolTable &st,
                                           CompileLog &log);
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
