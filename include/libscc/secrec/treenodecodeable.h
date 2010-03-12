#ifndef TREENODECODEABLE_H
#define TREENODECODEABLE_H

#include "treenode.h"
#include "../intermediate.h"

namespace SecreC {

class TreeNodeCodeable: public TreeNode {
    public: /* Methods: */
        TreeNodeCodeable(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc) {}
        virtual inline ~TreeNodeCodeable() {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es) = 0;
};

} // namespace SecreC

#endif // TREENODECODEABLE_H
