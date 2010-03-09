#include "secrec/symboltable.h"

namespace SecreC {

SymbolTable::~SymbolTable() {
    typedef std::vector<Symbol*>::const_iterator SVCI;

    for (SVCI it(m_table.begin()); it != m_table.end(); it++) {
        delete (*it);
    }
}

} // namespace SecreC
