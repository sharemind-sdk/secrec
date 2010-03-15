#include "secrec/treenodeexprbinary.h"


namespace SecreC {

ICode::Status TreeNodeExprBinary::calculateResultType(SymbolTable &st,
                                                      std::ostream &es)
{
    assert(children().size() == 2);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
    assert((children().at(1)->type() & NODE_EXPR_MASK) != 0x0);

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

    const SecreC::Type *eType1 = static_cast<const TreeNodeExpr*>(e1)->resultType();
    const SecreC::Type *eType2 = static_cast<const TreeNodeExpr*>(e2)->resultType();

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
                *resultType() = new SecreC::BasicType(upperSecType(s1, s2), v1);
                return ICode::OK;
            case NODE_EXPR_SUB:
            case NODE_EXPR_MUL:
            case NODE_EXPR_MOD:
            case NODE_EXPR_DIV:
                if (v1 != v2) break;
                if ((v1 & (VARTYPE_INT|VARTYPE_UINT)) != 0x0) break;
                *resultType() = new SecreC::BasicType(upperSecType(s1, s2), v1);
                return ICode::OK;

            default:
                *resultType() = 0;
                /// \todo Write better error message
                es << "This kind of binary operation is not yet supported. At "
                   << location() << std::endl;
                return ICode::E_NOT_IMPLEMENTED;
        }
    }

    /// \todo Write better error message
    es << "Invalid binary operation at " << location() << std::endl;

    *resultType() = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprBinary::generateCode(ICode::CodeList &code,
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

    // Generate code for child expressions:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e1->generateCode(code, st, es);
    if (s != ICode::OK) return s;
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(e2);
    s = e2->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for binary expression:
    Imop *i;
    /// \todo implement more expressions
    switch (type()) {
        case NODE_EXPR_ADD: i = new Imop(Imop::ADD); break;
        case NODE_EXPR_SUB: i = new Imop(Imop::SUB); break;
        case NODE_EXPR_MUL: i = new Imop(Imop::MUL); break;
        case NODE_EXPR_DIV: i = new Imop(Imop::DIV); break;
        case NODE_EXPR_MOD: i = new Imop(Imop::MOD); break;
        case NODE_EXPR_EQ:  i = new Imop(Imop::EQ);  break;
        case NODE_EXPR_GE:  i = new Imop(Imop::GE);  break;
        case NODE_EXPR_GT:  i = new Imop(Imop::GT);  break;
        case NODE_EXPR_LE:  i = new Imop(Imop::LE);  break;
        case NODE_EXPR_LT:  i = new Imop(Imop::LT);  break;
        case NODE_EXPR_NE:  i = new Imop(Imop::NE);  break;
        default:
            /// \todo Write better error message
            es << "Binary is not yet implemented. At " << location()
               << std::endl;
            return ICode::E_NOT_IMPLEMENTED;
    }

    i->setDest(result());
    i->setArg1(static_cast<const TreeNodeExpr*>(e1)->result());
    i->setArg1(static_cast<const TreeNodeExpr*>(e2)->result());
    code.push_back(i);

    // Patch next lists of child expressions:
    e1->patchNextList(st.label(ce2->firstImop()));
    e2->patchNextList(st.label(i));

    return ICode::OK;
}

ICode::Status TreeNodeExprBinary::generateBoolCode(ICode::CodeList &code, SymbolTable &st,
                                                   std::ostream &es)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    assert(type() == NODE_EXPR_EQ
           || type() == NODE_EXPR_GE
           || type() == NODE_EXPR_GT
           || type() == NODE_EXPR_LAND
           || type() == NODE_EXPR_LE
           || type() == NODE_EXPR_LOR
           || type() == NODE_EXPR_LT
           || type() == NODE_EXPR_NE);

    // Generate code for first child expression:
    TreeNodeExpr *e1 = static_cast<TreeNodeExpr*>(children().at(0).data());
    const TreeNodeExpr *ce1 = const_cast<const TreeNodeExpr*>(ce1);
    s = e1->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;
    firstImop() = static_cast<const TreeNodeExpr*>(e1)->firstImop();
    TreeNodeExpr *e2 = static_cast<TreeNodeExpr*>(children().at(1).data());
    const TreeNodeExpr *ce2 = const_cast<const TreeNodeExpr*>(ce2);
    s = e2->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    if (type() == NODE_EXPR_LAND) {
        /// \todo check if true
        e1->patchTrueList(st.label(ce2->firstImop()));
        falseList() = ce1->falseList();

        trueList() = ce2->trueList();
        falseList().insert(falseList().begin(),
                           ce2->falseList().begin(),
                           ce2->falseList().end());

        return ICode::OK;
    }

    if (type() == NODE_EXPR_LOR) {
        /// \todo check if true
        e1->patchFalseList(st.label(ce2->firstImop()));
        trueList() = ce1->trueList();

        falseList() = ce2->falseList();
        trueList().insert(trueList().begin(),
                           ce2->trueList().begin(),
                           ce2->trueList().end());

        return ICode::OK;
    }

    Imop::Type jumpType;
    switch (type()) {
        case NODE_EXPR_EQ: jumpType = Imop::JE; break;
        case NODE_EXPR_GE: jumpType = Imop::JGE; break;
        case NODE_EXPR_GT: jumpType = Imop::JGT; break;
        case NODE_EXPR_LE: jumpType = Imop::JLE; break;
        case NODE_EXPR_LT: jumpType = Imop::JLT; break;
        case NODE_EXPR_NE: jumpType = Imop::JNE; break;
        default:
            assert(false); // Shouldn't happen.
    }
    Imop *tj = new Imop(jumpType, 0,
                       static_cast<const TreeNodeExpr*>(e1)->result(),
                       static_cast<const TreeNodeExpr*>(e2)->result());
    trueList().push_back(tj);
    code.push_back(tj);

    Imop *fj = new Imop(Imop::JUMP);
    falseList().push_back(fj);
    code.push_back(fj);
    return ICode::OK;
}

} // namespace SecreC
