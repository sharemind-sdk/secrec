#ifndef TREENODEEXPRUNARY_H
#define TREENODEEXPRUNARY_H

#include "treenodeexpr.h"


namespace SecreC {

class TreeNodeExprUnary: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprUnary(Type type, const YYLTYPE &loc)
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

#endif // TREENODEEXPRUNARY_H
