#ifndef TREENODESTRING_H
#define TREENODESTRING_H

#include "treenode.h"


namespace SecreC {

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

} // namespace SecreC

#endif // TREENODESTRING_H
