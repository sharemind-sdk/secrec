#ifndef TREENODEBOOL_H
#define TREENODEBOOL_H

#include "treenode.h"

namespace SecreC {

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

} // namespace SecreC

#endif // TREENODEBOOL_H
