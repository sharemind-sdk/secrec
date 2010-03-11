#include "secrec/treenodeprogram.h"


ICode::Status TreeNodeProgram::generateCode(ICode::CodeList &code,
                                            SymbolTable &st,
                                            std::ostream &es)
{
    if (children().size() >= 1) {
        assert(children().size() < 3);

        TreeNode *child = children().at(0);
        if (children().size() >= 2) {
            assert(child->type() == NODE_GLOBALS);

            // Handle global declarations:
            TreeNodeGlobals *t = static_cast<TreeNodeGlobals*>(child);
            ICode::Status s = t->generateCode(code, st, es);
            if (s != ICode::OK) return s;

            child = children().at(1);
        }

        // Handle functions:
        assert(child->type() == NODE_FUNDEFS);
        TreeNodeFundefs *t = static_cast<TreeNodeFundefs*>(child);
        ICode::Status s = t->generateCode(code, st, es);
        if (s != ICode::OK) return s;

        // Handle calling main():
        /// \todo
        return s;
    } else {
        return ICode::E_EMPTY_PROGRAM;
    }
}
