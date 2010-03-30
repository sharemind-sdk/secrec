#include "secrec/symboltable.h"

#include <cassert>
#include <iostream>
#include <sstream>


namespace SecreC {

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

SymbolLabel *SymbolTable::label(const Imop *imop) {
    SymbolLabel *label = new SymbolLabel(imop);
    std::ostringstream os("$label$");
    os << m_labelCount++;
    label->setName(os.str());
    appendSymbol(label);
    return label;
}

SymbolComment *SymbolTable::comment(const std::string &comment) {
    /// \todo Implement hashing instead
    std::string name("$comment$" + comment);
    assert(dynamic_cast<SymbolComment*>(find(name)) != 0);
    SymbolComment *s = static_cast<SymbolComment*>(find(name));
    if (s != 0) return s;

    s = new SymbolComment(comment);
    s->setName(name);
    return s;
}

SymbolConstantBool *SymbolTable::constantBool(bool value) {
    const std::string name(value ? "$constBool$true" : "$constBool$false");
    assert(dynamic_cast<SymbolConstantBool*>(find(name)) != 0);
    SymbolConstantBool *s = static_cast<SymbolConstantBool*>(find(name));
    if (s != 0) return s;

    s = new SymbolConstantBool(value);
    s->setName(name);
    return s;
}

SymbolConstantInt *SymbolTable::constantInt(int value) {
    std::ostringstream os("$constInt$");
    os << value;
    std::string name(os.str());
    assert(dynamic_cast<SymbolConstantInt*>(find(name)) != 0);
    SymbolConstantInt *s = static_cast<SymbolConstantInt*>(find(name));
    if (s != 0) return s;

    s = new SymbolConstantInt(value);
    s->setName(name);
    return s;
}

SymbolConstantUInt *SymbolTable::constantUInt(unsigned value) {
    std::ostringstream os("$constUInt$");
    os << value;
    std::string name(os.str());
    assert(dynamic_cast<SymbolConstantUInt*>(find(name)) != 0);
    SymbolConstantUInt *s = static_cast<SymbolConstantUInt*>(find(name));
    if (s != 0) return s;

    s = new SymbolConstantUInt(value);
    s->setName(name);
    return s;
}

SymbolConstantString *SymbolTable::constantString(const std::string &value) {
    /// \todo Implement hashing instead
    std::string name("$constString$" + value);
    assert(dynamic_cast<SymbolConstantString*>(find(name)) != 0);
    SymbolConstantString *s = static_cast<SymbolConstantString*>(find(name));
    if (s != 0) return s;

    s = new SymbolConstantString(value);
    s->setName(name);
    return s;
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
    /// \todo
    out << "{SymbolTable}";
    return out;
}
