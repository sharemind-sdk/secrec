#ifndef TREENODE_H
#define TREENODE_H

#include "parser.h"

#ifdef __cplusplus
#include <cassert>
#include <deque>
#include <string>
#include "../sccpointer.h"
#include "../intermediate.h"
#include "types.h"

namespace SecreC {

class TreeNode: public SccObject {
    public: /* Types: */
        typedef enum SecrecTreeNodeType Type;
        typedef std::deque<SccPointer<TreeNode> > ChildrenList;
        typedef ChildrenList::iterator ChildrenListIterator;
        typedef ChildrenList::const_iterator ChildrenListConstIterator;

    public: /* Methods: */
        explicit TreeNode(Type type, const YYLTYPE &loc);

        inline TreeNode* parent() const { return m_parent; }
        inline Type type() const { return m_type; }
        inline const std::deque<SccPointer<TreeNode> > &children() const {
            return m_children;
        }
        inline const YYLTYPE &location() const { return m_location; }

        void appendChild(TreeNode *child, bool reparent = true);
        void prependChild(TreeNode *child, bool reparent = true);
        void setLocation(const YYLTYPE &location);

        std::string toString(unsigned indentation = 2, unsigned startIndent = 0)
                const;
        inline virtual std::string stringHelper() const { return ""; }

        std::string toXml(bool full = false) const;
        inline virtual std::string xmlHelper() const { return ""; }

        static const char *typeName(Type type);

    private: /* Fields: */
        TreeNode    *m_parent;
        const Type   m_type;
        ChildrenList m_children;
        YYLTYPE      m_location;
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
struct TreeNode *treenode_init_dataTypeF(enum SecrecVarType varType,
                                         YYLTYPE *loc);
struct TreeNode *treenode_init_dataTypeArray(unsigned value, YYLTYPE *loc);

#ifdef __cplusplus
} /* extern "C" */
} /* namespace SecreC */

std::ostream &operator<<(std::ostream &out, const YYLTYPE &loc);

namespace SecreC {


/******************************************************************
  TreeNodeBool
******************************************************************/

class TreeNodeBool: public TreeNode {
    public: /* Methods: */
        explicit TreeNodeBool(bool value, const YYLTYPE &loc)
            : TreeNode(NODE_LITE_BOOL, loc), m_value(value) {}

        inline void setValue(bool value) { m_value = value; }
        inline bool value() const { return m_value; }

        inline virtual std::string stringHelper() const {
            return (m_value ? "true" : "false");
        }
        std::string xmlHelper() const;

    private: /* Fields: */
        bool m_value;
};

class TreeNodeCodeable: public TreeNode {
    public: /* Methods: */
        TreeNodeCodeable(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc) {}
        virtual inline ~TreeNodeCodeable() {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es) = 0;
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

    private: /* Fields: */
        unsigned m_dim;
        mutable DataType *m_cachedType;
};


/******************************************************************
  TreeNodeDataTypeF
******************************************************************/

class TreeNodeDataTypeF: public TreeNodeDataType {
    public: /* Methods: */
        inline TreeNodeDataTypeF(SecrecVarType varType,
                                 const YYLTYPE &loc)
            : TreeNodeDataType(NODE_DATATYPE_F, loc),
              m_cachedType(varType) {}

        virtual inline const DataType &dataType() const {
            return m_cachedType;
        }

    private: /* Fields: */
        DataTypeBasic m_cachedType;
};


/******************************************************************
  TreeNodeDecl
******************************************************************/

class TreeNodeDecl: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeDecl(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_DECL, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
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
            if (m_resultType != 0) delete *m_resultType;
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
        void patchTrueList(Symbol *dest);
        void patchFalseList(Symbol *dest);
        void patchNextList(Symbol *dest);

    protected:
        inline SymbolWithValue *&result() {
            return m_result;
        };
        inline SecreC::Type **&resultType() {
            return m_resultType;
        }
        inline std::vector<Imop*> &falseList() {
            return m_falseList;
        }
        inline std::vector<Imop*> &trueList() {
            return m_trueList;
        }
        inline std::vector<Imop*> &nextList() {
            return m_nextList;
        }
        inline Imop *&firstImop() {
            return m_firstImop;
        }

    private: /* Fields: */
        SymbolWithValue    *m_result;
        SecreC::Type      **m_resultType;
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
  TreeNodeExprIdentifier
******************************************************************/

class TreeNodeExprIdentifier: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprIdentifier(Type type, const YYLTYPE &loc)
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
            : TreeNodeCodeable(NODE_FUNDEF, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
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

        std::string stringHelper() const;
        std::string xmlHelper() const;

    private: /* Fields: */
        std::string m_value;
};


/******************************************************************
  TreeNodeInt
******************************************************************/

class TreeNodeInt: public TreeNode {
    public: /* Methods: */
        explicit TreeNodeInt(int value, const YYLTYPE &loc)
            : TreeNode(NODE_LITE_INT, loc), m_value(value) {}

        inline void setValue(int value) { m_value = value; }
        inline int value() const { return m_value; }

        std::string stringHelper() const;
        std::string xmlHelper() const;

    private: /* Fields: */
        int m_value;
};


/******************************************************************
  TreeNodeLVariable
******************************************************************/

class TreeNodeLVariable: public TreeNode {
    public: /* Methods: */
        explicit TreeNodeLVariable(const YYLTYPE &loc)
            : TreeNode(NODE_EXPR_LVARIABLE, loc), m_cachedSymbol(0) {}
        inline ~TreeNodeLVariable() { delete m_cachedSymbol; }

        Symbol *symbol(SymbolTable &st, std::ostream &es) const;
        Symbol *symbol() const;
        Symbol::Type symbolType() const;
        SecreC::Type *secrecType() const;

    public:
        mutable Symbol **m_cachedSymbol;
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

    private: /* Fields: */
        SecrecSecType m_secType;
};


/******************************************************************
  TreeNodeString
******************************************************************/

class TreeNodeString: public TreeNode {
    public: /* Methods: */
        explicit TreeNodeString(const std::string &value, const YYLTYPE &loc)
            : TreeNode(NODE_LITE_STRING, loc), m_value(value) {}

        inline void setValue(const std::string &value) { m_value = value; }
        inline const std::string &value() const { return m_value; }

        std::string stringHelper() const;
        std::string xmlHelper() const;

    private: /* Fields: */
        std::string m_value;
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
            : TreeNodeType(NODE_TYPETYPE, loc) {}

        virtual const SecreC::Type &secrecType() const;

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


/******************************************************************
  TreeNodeUInt
******************************************************************/

class TreeNodeUInt: public TreeNode {
    public: /* Methods: */
        explicit TreeNodeUInt(unsigned value, const YYLTYPE &loc)
            : TreeNode(NODE_LITE_UINT, loc), m_value(value) {}

        inline void setValue(unsigned value) { m_value = value; }
        inline unsigned value() const { return m_value; }

        std::string stringHelper() const;
        std::string xmlHelper() const;

    private: /* Fields: */
        unsigned m_value;
};

} // namespace SecreC

#endif /* #ifdef __cplusplus */

#endif /* TREENODE_H */
