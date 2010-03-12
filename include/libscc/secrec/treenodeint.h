#ifndef TREENODEINT_H
#define TREENODEINT_H

#include "treenode.h"

namespace SecreC {

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

} // namespace SecreC

#endif // TREENODEINT_H
