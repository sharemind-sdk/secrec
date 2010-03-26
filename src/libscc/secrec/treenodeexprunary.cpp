#include "secrec/treenodeexprunary.h"


namespace SecreC {

ICode::Status TreeNodeExprUnary::calculateResultType(SymbolTable &st,
                                                     std::ostream &es)
{
    assert(type() == NODE_EXPR_UMINUS || type() == NODE_EXPR_UNEG);
    assert(children().size() == 1);
    assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);

    if (resultType() != 0) {
        assert(*resultType() != 0);
        return ICode::OK;
    }

    resultType() = new (SecreC::Type*);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    ICode::Status s = e->calculateResultType(st, es);
    if (s != ICode::OK) return s;
    const SecreC::Type *eType = static_cast<const TreeNodeExpr*>(e)->resultType();

    /// \todo implement for matrixes also
    if (!eType->isVoid()
#ifndef NDEBUG
        && (assert(dynamic_cast<const NonVoidType*>(eType) != 0), true)
#endif
        && static_cast<const NonVoidType*>(eType)->kind() == NonVoidType::BASIC)
    {
        const NonVoidType *et = static_cast<const NonVoidType*>(eType);
        assert(dynamic_cast<const BasicDataType*>(&et->dataType()) != 0);
        const BasicDataType &bType = static_cast<const BasicDataType&>(et->dataType());
        if (type() == NODE_EXPR_UNEG && bType.varType() == VARTYPE_BOOL) {
            *resultType() = et->clone();
            return ICode::OK;
        } else if (type() == NODE_EXPR_UMINUS) {
            if (bType.varType() == VARTYPE_INT) {
                *resultType() = et->clone();
                return ICode::OK;
            }
        }
    }

    es << "Invalid expression of type (" << *eType << ") given to unary "
       << (type() == NODE_EXPR_UNEG ? "negation" : "minus")
       << "operator at " << location() << std::endl;

    *resultType() = 0;
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprUnary::generateCode(ICode::CodeList &code,
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

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    s = e->generateCode(code, st, es);
    if (s != ICode::OK) return s;

    // Generate code for unary expression:
    /// \todo implement for matrixes also
    Imop *i = new Imop(type() == NODE_EXPR_UNEG ? Imop::UNEG : Imop::UMINUS);
    i->setDest(result());
    i->setArg1(static_cast<const TreeNodeExpr*>(e)->result());
    code.push_back(i);

    // Patch next list of child expression:
    e->patchNextList(st.label(i));

    return ICode::OK;
}

ICode::Status TreeNodeExprUnary::generateBoolCode(ICode::CodeList &code, SymbolTable &st,
                                                  std::ostream &es)
{
    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    assert(type() == NODE_EXPR_UNEG);

    // Generate code for child expression:
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0).data());
    const TreeNodeExpr *ce = const_cast<const TreeNodeExpr*>(e);
    s = e->generateBoolCode(code, st, es);
    if (s != ICode::OK) return s;

    falseList() = ce->trueList();
    trueList() = ce->falseList();
    firstImop() = ce->firstImop();
    return ICode::OK;
}

} // namespace SecreC
