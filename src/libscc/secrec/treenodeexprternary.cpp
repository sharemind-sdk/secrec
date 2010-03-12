#include "secrec/treenodeexprternary.h"


namespace SecreC {

ICode::Status TreeNodeExprTernary::calculateResultType(SymbolTable &st,
                                                       std::ostream &es)
{
    assert(children().size() == 3);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(2)->type() & NODE_EXPR_MASK) != 0x0);

    if (m_resultType != 0) {
        assert(*m_resultType != 0);
        return ICode::OK;
    }

    m_resultType = new (const SecreC::Type*);

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e1->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    s = e2->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    s = e3->calculateResultType(st, es);
    if (s != ICode::OK) return s;

    const SecreC::Type *eType1 = e1->resultType();
    const SecreC::Type *eType2 = e2->resultType();
    const SecreC::Type *eType3 = e3->resultType();

    if (eType1->kind() == SecreC::Type::Basic
        && static_cast<const SecreC::BasicType*>(eType1)->varType() == VARTYPE_BOOL
        && static_cast<const SecreC::BasicType*>(eType1)->secType() == SECTYPE_PUBLIC
        && *eType2 == *eType3)
    {
        *m_resultType = eType2->clone();
        return ICode::OK;
    }

    /// \todo Write better error message
    es << "Invalid ternary operation at " << location() << std::endl;

    *m_resultType = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprTernary::generateCode(ICode::CodeList &code,
                                                SymbolTable &st,
                                                std::ostream &es)
{
    assert(m_result == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *rt = resultType();

    // Generate code for child expressions:
    /// \todo Evaluate only one side!!!

    return ICode::E_NOT_IMPLEMENTED;

    /*

    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    TreeNodeExpr *e3 = static_cast<TreeNodeExpr*>(children().at(2).data());
    s = e3->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression:
    m_result = st.appendTemporary(*rt);

    // Generate code for binary expression:
    Imop *i = new Imop();

    i->setDest(m_result);
    i->setArg1(e1->result());
    i->setArg1(e2->result());
    code.push_back(i);

    return ICode::OK;
    */
}

} // namespace SecreC
