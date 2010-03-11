#ifndef TREENODEFUNDEF_H
#define TREENODEFUNDEF_H

#include "treenode.h"


class TreeNodeFundef: public TreeNodeCodeable {
    public: /* Methods: */
        explicit TreeNodeFundef(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_DECL, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};

#endif // TREENODEFUNDEF_H
