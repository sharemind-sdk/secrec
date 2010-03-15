#include "secrec/treenodeexprternary.h"


namespace SecreC {

ICode::Status TreeNodeExprTernary::calculateResultType(SymbolTable &st,
                                                       std::ostream &es)
{
    assert(children().size() == 3);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(2)->type() & NODE_EXPR_MASK) != 0x0);

    if (resultType() != 0) {
        assert(*resultType() != 0);
        return ICode::OK;
    }

    resultType() = new (SecreC::Type*);

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e1->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    s = e2->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    s = e3->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    const SecreC::Type *eType1 = static_cast<const TreeNodeExpr*>(e1)->resultType();
    const SecreC::Type *eType2 = static_cast<const TreeNodeExpr*>(e2)->resultType();
    const SecreC::Type *eType3 = static_cast<const TreeNodeExpr*>(e3)->resultType();

    if (eType1->kind() == SecreC::Type::Basic
        && static_cast<const SecreC::BasicType*>(eType1)->varType() == VARTYPE_BOOL
        && static_cast<const SecreC::BasicType*>(eType1)->secType() == SECTYPE_PUBLIC
        && *eType2 == *eType3)
    {
        *resultType() = eType2->clone();
        return ICode::OK;
    }

    /// \todo Write better error message
    es << "Invalid ternary operation at " << location() << std::endl;

    *resultType() = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprTernary::generateCode(ICode::CodeList &code,
                                                SymbolTable &st,
                                                std::ostream &es,
                                                SymbolWithValue *r)
{
    assert(result() == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression, if needed:
    if (r == 0) {
        SecreC::Type *rt = *resultType();
        result() = st.appendTemporary(*rt);
    } else {
        assert(r->secrecType() == **resultType());
        result() = r;
    }

    // Generate code for boolean expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for first value child expression:
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(e2);
    s = e2->generateCode(code, st, es, result());
    if (s != ICode::OK) return s;

    // Jump out of the ternary construct:
    Imop *j = new Imop(Imop::JUMP);
    nextList().push_back(j);
    code.push_back(j);

    // Generate code for second value child expression:
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    const TreeNodeExpr *ce3 = const_cast<const TreeNodeExpr*>(e3);
    s = e3->generateCode(code, st, es, result());
    if (s != ICode::OK) return s;

    // Link boolean expression code to the rest of the code:
    e1->patchTrueList(st.label(ce2->firstImop()));
    e1->patchFalseList(st.label(ce3->firstImop()));

    // Handle next lists of value child expressions:
    nextList().insert(nextList().begin(),
                      ce2->nextList().begin(),
                      ce2->nextList().end());
    nextList().insert(nextList().begin(),
                      ce3->nextList().begin(),
                      ce3->nextList().end());

    return ICode::OK;
}

ICode::Status TreeNodeExprTernary::generateBoolCode(ICode::CodeList &code,
                                                    SymbolTable &st,
                                                    std::ostream &es)
{
    /// \todo Write assertion that were have good return type

    // Type check
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    // Generate code for boolean expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for first value child expression:
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(e2);
    s = e2->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for second value child expression:
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    const TreeNodeExpr *ce3 = const_cast<const TreeNodeExpr*>(e3);
    s = e3->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Link conditional expression code to the rest of the code:
    e1->patchTrueList(st.label(ce2->firstImop()));
    e1->patchFalseList(st.label(ce3->firstImop()));

    trueList().insert(trueList().begin(), ce2->trueList().begin(),
                                          ce2->trueList().end());
    trueList().insert(trueList().begin(), ce3->trueList().begin(),
                                          ce3->trueList().end());
    falseList().insert(falseList().begin(), ce2->falseList().begin(),
                                            ce2->falseList().end());
    falseList().insert(falseList().begin(), ce3->falseList().begin(),
                                            ce3->falseList().end());

    return ICode::OK;
}

} // namespace SecreC
