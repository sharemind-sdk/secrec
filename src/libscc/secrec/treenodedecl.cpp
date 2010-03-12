#include "secrec/treenodedecl.h"

#include "secrec/treenodeexpr.h"
#include "secrec/treenodeidentifier.h"
#include "secrec/treenodetype.h"


namespace SecreC {

ICode::Status TreeNodeDecl::generateCode(ICode::CodeList &code,
                                         SymbolTable &st,
                                         std::ostream &es)
{
    typedef TreeNodeIdentifier TNI;
    typedef TreeNodeType       TNT;

    assert(children().size() > 0 && children().size() <= 3);
    assert(children().at(0).data()->type() == NODE_IDENTIFIER);
    assert((children().at(1).data()->type() & NODE_TYPE_MASK) != 0x0);
    TNI *id   = static_cast<TNI*>(children().at(0).data());
    TNT *type = static_cast<TNT*>(children().at(1).data());

    /// \note Check here for overrides first if new symbol table is needed.
    SymbolSymbol *s = new SymbolSymbol(type->secrecType(), this);
    s->setName(id->value());
    st.appendSymbol(s);

    if (children().size() >= 2) {
        TreeNode *t = children().at(2).data();
        assert((t->type() & NODE_EXPR_MASK) != 0x0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        ICode::Status s = e->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}

} // namespace SecreC
