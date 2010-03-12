#ifndef TREENODEPROGRAM_H
#define TREENODEPROGRAM_H

#include "treenodecodeable.h"


namespace SecreC {

class TreeNodeProgram: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeProgram(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_PROGRAM, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code, SymbolTable &st,
                                           std::ostream &es);
};

} // namespace SecreC

#endif // TREENODEPROGRAM_H
