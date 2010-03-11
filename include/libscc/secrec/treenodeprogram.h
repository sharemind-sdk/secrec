#ifndef TREENODEPROGRAM_H
#define TREENODEPROGRAM_H

#include "treenode.h"


class TreeNodeProgram: public TreeNodeCodeable {
    public: /* Methods: */
        explicit inline TreeNodeProgram(const YYLTYPE &loc)
            : TreeNodeCodeable(NODE_PROGRAM, loc) {}

        virtual ICode::Status generateCode(ICode::CodeList &code, SymbolTable &st,
                                           std::ostream &es);
};

#endif // TREENODEPROGRAM_H
