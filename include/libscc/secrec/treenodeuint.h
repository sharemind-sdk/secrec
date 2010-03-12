#ifndef TREENODEUINT_H
#define TREENODEUINT_H

#include "treenode.h"


namespace SecreC {

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

#endif // TREENODEUINT_H
