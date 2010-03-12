#ifndef TREENODEGLOBALS_H
#define TREENODEGLOBALS_H

#include "treenodecodeable.h"


namespace SecreC {

class TreeNodeGlobals: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeGlobals(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_GLOBALS, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};

} // namespace SecreC

#endif // TREENODEGLOBALS_H
