#include "secrec/treenodeexprunary.h"


namespace SecreC {

ICode::Status TreeNodeExprUnary::calculateResultType(SymbolTable &st,
                                                     std::ostream &es)
{
    assert(type() == NODE_EXPR_UMINUS || type() == NODE_EXPR_UNEG);
    assert(children().size() == 1);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);

    if (m_resultType != 0) {
        assert(*m_resultType != 0);
        return ICode::OK;
    }

    m_resultType = new (const SecreC::Type*);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e->calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *eType = e->resultType();

    /// \todo implement for matrixes also
    if (eType->kind() == SecreC::Type::Basic) {
        const BasicType *bType = static_cast<const BasicType*>(eType);
        if (type() == NODE_EXPR_UNEG && bType->varType() == VARTYPE_BOOL) {
            *m_resultType = bType->clone();
            return ICode::OK;
        } else if (type() == NODE_EXPR_UMINUS) {
            if (bType->varType() == VARTYPE_INT) {
                *m_resultType = bType->clone();
                return ICode::OK;
            }
        }
    }

    es << "Invalid expression of type (" << *eType << ") given to unary "
       << (type() == NODE_EXPR_UNEG ? "negation" : "minus")
       << "operator at " << location() << std::endl;

    *m_resultType = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprUnary::generateCode(ICode::CodeList &code,
                                              SymbolTable &st,
                                              std::ostream &es)
{
    assert(m_result == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *rt = resultType();

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression:
    m_result = st.appendTemporary(*rt);

    // Generate code for unary expression:
    /// \todo implement for matrixes also
    Imop *i = new Imop(type() == NODE_EXPR_UNEG ? Imop::UNEG : Imop::UMINUS);
    i->setDest(m_result);
    i->setArg1(e->result());
    code.push_back(i);

    return ICode::OK;
}

} // namespace SecreC
