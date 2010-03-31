#include "secrec/symboltable.h"
#include "secrec/treenode.h"

#include <cassert>
#include <iostream>
#include <sstream>


namespace SecreC {

/*******************************************************************************
  SymbolFunction
*******************************************************************************/

SymbolFunction::SymbolFunction(const TreeNodeFundef *fundef)
    : SymbolWithValue(Symbol::FUNCTION, fundef->functionType()), m_decl(fundef),
      m_target(0)
{
    // Intentionally empty
}


/*******************************************************************************
  SymbolTable
*******************************************************************************/

SymbolTable::SymbolTable(SymbolTable *parent)
    : m_parent(parent), m_scope(0), m_cont(0), m_last(0), m_tempCount(0),
      m_constCount(0), m_labelCount(0)
{
    // Intentionally empty
}

SymbolTable::~SymbolTable() {
    typedef std::vector<Symbol*>::const_iterator SVCI;

    for (SVCI it(m_table.begin()); it != m_table.end(); it++) {
        delete (*it);
    }

    delete m_scope;
    delete m_cont;
}

void SymbolTable::appendSymbol(Symbol *symbol) {
    (m_last == 0 ? m_table : m_last->m_table).push_back(symbol);
}

SymbolWithValue *SymbolTable::appendTemporary(const Type &type) {
    SymbolWithValue *tmp = new SymbolWithValue(Symbol::TEMPORARY, type);
    std::ostringstream os("$t$");
    os << m_tempCount++;
    tmp->setName(os.str());
    appendSymbol(tmp);
    return tmp;
}

SymbolConstantBool *SymbolTable::constantBool(bool value) {
    const char *name = (value ? "$constBool$true" : "$constBool$false");
    Symbol *s = find(name);
    if (s != 0) {
        assert(dynamic_cast<SymbolConstantBool*>(s) != 0);
        return static_cast<SymbolConstantBool*>(s);
    }

    SymbolConstantBool *sc = new SymbolConstantBool(value);
    sc->setName(name);
    return sc;
}

SymbolConstantInt *SymbolTable::constantInt(int value) {
    std::ostringstream os;
    os << "$constInt$" << value;
    Symbol *s = find(os.str());
    if (s != 0) {
        assert(dynamic_cast<SymbolConstantInt*>(s) != 0);
        return static_cast<SymbolConstantInt*>(s);
    }
    SymbolConstantInt *sc = new SymbolConstantInt(value);
    sc->setName(os.str());
    return sc;
}

SymbolConstantUInt *SymbolTable::constantUInt(unsigned value) {
    std::ostringstream os;
    os << "$constUInt$" << value;
    Symbol *s = find(os.str());
    if (s != 0) {
        assert(dynamic_cast<SymbolConstantUInt*>(s) != 0);
        return static_cast<SymbolConstantUInt*>(s);
    }

    SymbolConstantUInt *sc = new SymbolConstantUInt(value);
    sc->setName(os.str());
    return sc;
}

SymbolConstantString *SymbolTable::constantString(const std::string &value) {
    /// \todo Implement hashing instead
    std::string name("$constString$" + value);
    Symbol *s = find(name);
    if (s != 0) {
        assert(dynamic_cast<SymbolConstantString*>(s) != 0);
        return static_cast<SymbolConstantString*>(s);
    }

    SymbolConstantString *sc = new SymbolConstantString(value);
    sc->setName(name);
    return sc;
}

Symbol *SymbolTable::find(const std::string &name) const {
    typedef Table::const_reverse_iterator STI;

    const SymbolTable *c = this;
    for (;;) {
        const Table &t(c->m_table);
        for (STI it(t.rbegin()); it != t.rend(); it++) {
            if ((*it)->name() == name)
                return (*it);
        }
        if (c->m_parent == 0) break;
        c = c->m_parent;
    }
    return 0;
}

SymbolTable *SymbolTable::newScope() {
    SymbolTable *scope = new SymbolTable(this);
    if (m_last == 0) {
        m_scope = scope;
        m_last = m_cont = new SymbolTable(this);
    } else {
        m_last->m_scope = scope;
        m_last = m_last->m_last = m_last->m_cont = new SymbolTable(this);
    }
    return scope;
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::SymbolTable &t) {
    (void) t;
    out << "{SymbolTable}";
    return out;
}
