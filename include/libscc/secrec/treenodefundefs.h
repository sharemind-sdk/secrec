#ifndef TREENODEFUNDEFS_H
#define TREENODEFUNDEFS_H

#include "treenodecodeable.h"


namespace SecreC {

class TreeNodeFundefs: public TreeNodeCodeable {
    public: /* Methods: */
        explicit TreeNodeFundefs(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_FUNDEFS, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};

} // namespace SecreC

#endif // TREENODEFUNDEFS_H
