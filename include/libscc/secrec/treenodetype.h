#ifndef TREENODETYPE_H
#define TREENODETYPE_H

#include "treenode.h"


namespace SecreC {

class TreeNodeType: public TreeNode {
    public: /* Methods: */
        explicit inline TreeNodeType(TreeNode::Type type, const YYLTYPE &loc)
            : TreeNode(type, loc) {}

        virtual const SecreC::Type &secrecType() const = 0;
};

} // namespace SecreC

#endif // TREENODETYPE_H
