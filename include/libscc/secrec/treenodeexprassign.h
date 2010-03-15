#ifndef TREENODEEXPRASSIGN_H
#define TREENODEEXPRASSIGN_H

#include "treenodeexpr.h"

namespace SecreC {

class TreeNodeExprAssign: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprAssign(Type type, const YYLTYPE &loc)
            : TreeNodeExpr(type, loc) {}

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0);
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es);
};

} // namespace SecreC

#endif // TREENODEEXPRASSIGN_H
