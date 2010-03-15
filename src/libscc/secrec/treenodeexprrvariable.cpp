#include "secrec/treenodeexprrvariable.h"

#include "secrec/treenodelvariable.h"


namespace SecreC {

ICode::Status TreeNodeExprRVariable::calculateResultType(SymbolTable &st,
                                                         std::ostream &es)
{
    assert(children().size() == 1);
    assert(children().at(0)->type() == NODE_EXPR_LVARIABLE);

    TreeNodeLVariable *l = static_cast<TreeNodeLVariable*>(children().at(0).data());
    if (l->symbol(st, es) == 0) return ICode::E_OTHER;

    if (l->symbolType() != Symbol::SYMBOL) {
        es << "The given variable is not of a proper rvariable type. At "
           << location() << std::endl;

        *resultType() = 0;
        return ICode::E_TYPE;
    }

    *resultType() = l->secrecType()->clone();
    return ICode::OK;
}

ICode::Status TreeNodeExprRVariable::generateCode(ICode::CodeList &code,
                                                  SymbolTable &st,
                                                  std::ostream &es,
                                                  SymbolWithValue *r)
{
    assert(result() == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    TreeNodeLVariable *l = static_cast<TreeNodeLVariable*>(children().at(0).data());
    assert(l->symbolType() == Symbol::SYMBOL);
    if (r == 0) {
        result() = static_cast<const SymbolWithValue*>(l->symbol());
    } else {
        assert(r->secrecType() == **resultType());
        result() = r;

        Imop *i = new Imop(Imop::ASSIGN, r, l->symbol());
        code.push_back(i);
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprRVariable::generateBoolCode(ICode::CodeList &code,
                                                      SymbolTable &st,
                                                      std::ostream &es)
{
    // Type check
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeLVariable *l = static_cast<TreeNodeLVariable*>(children().at(0).data());
    assert(l->symbolType() == Symbol::SYMBOL);

    Imop *i = new Imop(Imop::JT, 0, l->symbol());
    code.push_back(i);
    trueList().push_back(i);

    i = new Imop(Imop::JUMP, 0);
    code.push_back(i);
    falseList().push_back(i);

    return ICode::OK;
}

} // namespace SecreC
