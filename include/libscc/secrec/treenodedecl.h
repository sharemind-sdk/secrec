#ifndef TREENODEDECL_H
#define TREENODEDECL_H

#include "treenodecodeable.h"

namespace SecreC {

class TreeNodeDecl: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeDecl(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_DECL, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);
};

} // namespace SecreC

#endif // TREENODEDECL_H
