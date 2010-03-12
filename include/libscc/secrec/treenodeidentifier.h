#ifndef TREENODEIDENTIFIER_H
#define TREENODEIDENTIFIER_H

#include "treenode.h"

namespace SecreC {

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

} // namespace SecreC

#endif // TREENODEIDENTIFIER_H
