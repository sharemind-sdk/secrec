#include "secrec/treenodeglobals.h"

#include "secrec/treenodedecl.h"


namespace SecreC {

ICode::Status TreeNodeGlobals::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    typedef ChildrenListConstIterator CLCI;

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_DECL);
        TreeNodeDecl *decl = static_cast<TreeNodeDecl*>((*it).data());
        ICode::Status s = decl->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}

} // namespace SecreC
