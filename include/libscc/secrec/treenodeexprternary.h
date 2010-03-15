#ifndef TREENODEEXPRTERNARY_H
#define TREENODEEXPRTERNARY_H

#include "treenodeexpr.h"


namespace SecreC {

class TreeNodeExprTernary: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprTernary(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_TERNIF, loc) {}

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

#endif // TREENODEEXPRTERNARY_H
