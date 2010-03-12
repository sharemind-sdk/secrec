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
