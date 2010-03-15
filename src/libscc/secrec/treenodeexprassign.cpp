#include "secrec/treenodeexprassign.h"


namespace SecreC {

ICode::Status TreeNodeExprAssign::calculateResultType(SymbolTable &st,
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
            /// \todo
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

ICode::Status TreeNodeExprAssign::generateCode(ICode::CodeList &code,
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
        case NODE_EXPR_ASSIGN:     /* Fall through: */
        case NODE_EXPR_ASSIGN_MUL: /* Fall through: */
        case NODE_EXPR_ASSIGN_DIV: /* Fall through: */
        case NODE_EXPR_ASSIGN_MOD: /* Fall through: */
        case NODE_EXPR_ASSIGN_ADD: /* Fall through: */
        case NODE_EXPR_ASSIGN_SUB: /* Fall through: */
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

ICode::Status TreeNodeExprAssign::generateBoolCode(ICode::CodeList &code, SymbolTable &st,
                                                   std::ostream &es)
{
    /// \todo Write assertion about return type

    // Type check:
    ICode::Status s = calculateResultType(st, es);
    if (s != ICode::OK) return s;

    return ICode::E_NOT_IMPLEMENTED;
}

} // namespace SecreC
