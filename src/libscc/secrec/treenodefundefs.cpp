#include "secrec/treenodefundefs.h"

#include "secrec/treenodefundef.h"


namespace SecreC {

ICode::Status TreeNodeFundefs::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    typedef ChildrenListConstIterator CLCI;

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_FUNDEF);
        TreeNodeFundef *fundef = static_cast<TreeNodeFundef*>((*it).data());
        ICode::Status s = fundef->generateCode(code, st, es);
        if (s != ICode::OK) return s;
    }
    return ICode::OK;
}

} // namespace SecreC
