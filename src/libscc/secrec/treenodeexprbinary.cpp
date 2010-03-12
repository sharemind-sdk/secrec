#include "secrec/treenodeexprbinary.h"


namespace SecreC {

ICode::Status TreeNodeExprBinary::calculateResultType(SymbolTable &st,
                                                      std::ostream &es)
{
    assert(children().size() == 2);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

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

    const SecreC::Type *eType1 = e1->resultType();
    const SecreC::Type *eType2 = e2->resultType();

    /// \todo implement more expressions
    if (eType1->kind() == SecreC::Type::Basic) {
        SecrecVarType v1 = static_cast<const SecreC::BasicType*>(eType1)->varType();
        SecrecSecType s1 = static_cast<const SecreC::BasicType*>(eType1)->secType();
        SecrecVarType v2 = static_cast<const SecreC::BasicType*>(eType2)->varType();
        SecrecSecType s2 = static_cast<const SecreC::BasicType*>(eType2)->secType();

        switch (type()) {
            case NODE_EXPR_ADD:
                if (v1 != v2) break;
                if ((v1 & (VARTYPE_INT|VARTYPE_UINT|VARTYPE_STRING)) != 0x0) break;
                *m_resultType = new SecreC::BasicType(upperSecType(s1, s2), v1);
                return ICode::OK;
            case NODE_EXPR_SUB:
            case NODE_EXPR_MUL:
            case NODE_EXPR_MOD:
            case NODE_EXPR_DIV:
                if (v1 != v2) break;
                if ((v1 & (VARTYPE_INT|VARTYPE_UINT)) != 0x0) break;
                *m_resultType = new SecreC::BasicType(upperSecType(s1, s2), v1);
                return ICode::OK;

            default:
                *m_resultType = 0;
                /// \todo Write better error message
                es << "This kind of binary operation is not yet supported. At "
                   << location() << std::endl;
                return ICode::E_NOT_IMPLEMENTED;
        }
    }

    /// \todo Write better error message
    es << "Invalid binary operation at " << location() << std::endl;

    *m_resultType = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprBinary::generateCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es)
{
    assert(m_result == 0);

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *rt = resultType();

    // Generate code for child expressions:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate temporary for the result of the unary expression:
    m_result = st.appendTemporary(*rt);

    // Generate code for binary expression:
    Imop *i;
    /// \todo implement more expressions
    switch (type()) {
        case NODE_EXPR_ADD: i = new Imop(Imop::ADD); break;
        case NODE_EXPR_SUB: i = new Imop(Imop::SUB); break;
        case NODE_EXPR_MUL: i = new Imop(Imop::MUL); break;
        case NODE_EXPR_DIV: i = new Imop(Imop::DIV); break;
        case NODE_EXPR_MOD: i = new Imop(Imop::MOD); break;
        default:
            /// \todo Write better error message
            es << "Binary is not yet implemented. At " << location()
               << std::endl;
            return ICode::E_NOT_IMPLEMENTED;
    }

    i->setDest(m_result);
    i->setArg1(e1->result());
    i->setArg1(e2->result());
    code.push_back(i);

    return ICode::OK;
}

} // namespace SecreC
