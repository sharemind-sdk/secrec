#include "secrec/treenodelvariable.h"

#include "secrec/treenodeidentifier.h"


namespace SecreC {

Symbol::Type TreeNodeLVariable::symbolType() const {
    assert(children().size() == 1);
    assert(children().at(0)->type() == NODE_IDENTIFIER);
    assert(symbol() != 0);

    return symbol()->symbolType();
}

SecreC::Type *TreeNodeLVariable::secrecType() const {
    assert(symbol() != 0);
    if (symbolType() == Symbol::FUNCTION) return 0;
    assert(symbolType() == Symbol::SYMBOL);

    assert(dynamic_cast<const SymbolWithValue*>(symbol()) != 0);
    return &static_cast<const SymbolWithValue*>(symbol())->secrecType();
}


Symbol *TreeNodeLVariable::symbol(SymbolTable &st, std::ostream &es) const {
    if (m_cachedSymbol == 0) {
        m_cachedSymbol = new (SecreC::Symbol*);
        assert(dynamic_cast<TreeNodeIdentifier*>(children().at(0).data()) != 0);
        TreeNodeIdentifier *id = static_cast<TreeNodeIdentifier*>(children().at(0).data());
        *m_cachedSymbol = st.find(id->value());
        if (*m_cachedSymbol == 0) {
            es << "Symbol \"" << id->value() << "\" not in scope.";
        }
    }
    return *m_cachedSymbol;
}

Symbol *TreeNodeLVariable::symbol() const {
    assert(m_cachedSymbol != 0);
    return *m_cachedSymbol;
}

} // namespace SecreC
