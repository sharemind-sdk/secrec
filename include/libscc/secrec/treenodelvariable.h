#ifndef TREENODEEXPRLVARIABLE_H
#define TREENODEEXPRLVARIABLE_H


#include "treenode.h"


namespace SecreC {

class TreeNodeLVariable: public TreeNode {
    public: /* Methods: */
        explicit TreeNodeLVariable(const YYLTYPE &loc)
            : TreeNode(NODE_EXPR_RVARIABLE, loc), m_cachedSymbol(0) {}
        inline ~TreeNodeLVariable() { delete m_cachedSymbol; }

        Symbol *symbol(SymbolTable &st, std::ostream &es) const;
        Symbol *symbol() const;
        Symbol::Type symbolType() const;
        SecreC::Type *secrecType() const;

    public:
        mutable Symbol **m_cachedSymbol;
};

} // namespace SecreC

#endif // TREENODEEXPRLVARIABLE_H
