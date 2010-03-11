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

class TreeNodeCodeable: public TreeNode {
    public: /* Methods: */
        TreeNodeCodeable(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc) {}
        virtual inline ~TreeNodeCodeable() {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es) = 0;
};


class TreeNodeExpr: public TreeNodeCodeable {
    public: /* Types: */
        enum Flags { CONSTANT = 0x01, PARENTHESIS = 0x02 };

    public: /* Methods: */
        explicit TreeNodeExpr(Type type, const YYLTYPE &loc)
            : TreeNodeCodeable(type, loc) {}

        virtual const Symbol *result() const = 0;
        virtual const SecreC::Type *resultType() const = 0;
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es) = 0;

    private: /* Fields: */
        /// \todo Add flags.
};

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

class TreeNodeType: public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeType(TreeNode::Type type, const YYLTYPE &loc)
            : TreeNode(type, loc) {}

        virtual const SecreC::Type &secrecType() const = 0;
};

class TreeNodeTypeBasic: public TreeNodeType {
    public: /* Methods: */
        explicit TreeNodeTypeBasic(BasicType::SecType secType,
                                   BasicType::VarType varType,
                                   const YYLTYPE &loc)
            : TreeNodeType(NODE_BASICTYPE, loc), m_type(secType, varType) {}

        virtual inline const SecreC::Type &secrecType() const { return m_type; }
        inline void setSecrecBasicType(const SecreC::BasicType &type) {
            m_type = type;
        }

        inline BasicType::SecType secType() const { return m_type.secType(); }
        inline void setSecType(BasicType::SecType secType) {
            m_type.setSecType(secType);
        }
        inline BasicType::VarType varType() const { return m_type.varType(); }
        inline void setVarType(BasicType::VarType varType) {
            m_type.setVarType(varType);
        }

        std::string stringHelper() const;
        std::string xmlHelper() const;

    private: /* Fields: */
        SecreC::BasicType m_type;
};

class TreeNodeTypeArray: public TreeNodeType {
    public: /* Methods: */
        explicit TreeNodeTypeArray(unsigned value, const YYLTYPE &loc)
            : TreeNodeType(NODE_ARRAYTYPE, loc), m_value(value),
              m_cachedType(0) {}
        virtual inline ~TreeNodeTypeArray() { delete m_cachedType; }

        virtual const SecreC::Type &secrecType() const;

        inline void setValue(unsigned value) { m_value = value; }
        inline unsigned value() const { return m_value; }

        std::string stringHelper() const;
        std::string xmlHelper() const;

    private: /* Fields: */
        unsigned                   m_value;
        mutable SecreC::ArrayType *m_cachedType;
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
struct TreeNode *treenode_init_basictype(enum SecrecSecType secType,
                                         enum SecrecVarType varType,
                                         YYLTYPE *loc);
struct TreeNode *treenode_init_arraytype(unsigned value, YYLTYPE *loc);

#ifdef __cplusplus
} /* extern "C" */
} /* namespace SecreC */

std::ostream &operator<<(std::ostream &out, const YYLTYPE &loc);

#endif /* #ifdef __cplusplus */


#endif /* #ifdef TREENODE_H */
