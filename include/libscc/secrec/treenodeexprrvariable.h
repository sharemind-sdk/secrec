#ifndef TREENODEEXPRRVARIABLE_H
#define TREENODEEXPRRVARIABLE_H

#include "treenodeexpr.h"


namespace SecreC {

class TreeNodeExprRVariable: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprRVariable(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_RVARIABLE, loc) {}

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

#endif // TREENODEEXPRRVARIABLE_H
