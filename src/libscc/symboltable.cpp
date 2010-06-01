#include "symboltable.h"
#include "treenode.h"

#include <cassert>
#include <iostream>
#include <sstream>


namespace {

void printIndent(std::ostream &out, unsigned level, unsigned indent = 4) {
    while (level-- > 0)
        for (unsigned i = 0; i < indent; i++)
            out << " ";
}

} // anonymous namespace

namespace SecreC {

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

std::string SymbolSymbol::toString() const {
    std::ostringstream os;
    os << (m_scopeType == GLOBAL ? "GLOBAL" : "LOCAL") << ' ' << secrecType()
       << ' ' << name() << '{' << this << '}';
    return os.str();
}

/*******************************************************************************
  SymbolTemporary
*******************************************************************************/

std::string SymbolTemporary::toString() const {
    std::ostringstream os;
    os << "TEMPORARY " << (m_scopeType == GLOBAL ? "GLOBAL" : "LOCAL") << ' '
       << secrecType() << ' ' << name() << '{' << this << '}';
    return os.str();
}

/*******************************************************************************
  SymbolProcedure
*******************************************************************************/

SymbolProcedure::SymbolProcedure(const TreeNodeProcDef *procdef)
    : Symbol(Symbol::PROCEDURE, procdef->procedureType()), m_decl(procdef),
      m_target(0)
{
    // Intentionally empty
}

std::string SymbolProcedure::toString() const {
    std::ostringstream os;
    os << "PROCEDURE " << name() << ": " << secrecType();
    return os.str();
}

/*******************************************************************************
  SymbolConstantBool
*******************************************************************************/

std::string SymbolConstantBool::toString() const {
    std::ostringstream os;
    os << "CONSTANT bool " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantInt
*******************************************************************************/

std::string SymbolConstantInt::toString() const {
    std::ostringstream os;
    os << "CONSTANT int " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantString
*******************************************************************************/

std::string SymbolConstantString::toString() const {
    std::ostringstream os;
    os << "CONSTANT string " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantUInt
*******************************************************************************/

std::string SymbolConstantUInt::toString() const {
    std::ostringstream os;
    os << "CONSTANT uint " << m_value;
    return os.str();
}


/*******************************************************************************
  SymbolTable
*******************************************************************************/

SymbolTable::SymbolTable(SymbolTable *parent)
    : m_parent(parent), m_global(m_parent == 0 ? this : m_parent->m_global),
      m_scope(0), m_cont(0), m_last(0), m_tempCount(0), m_constCount(0),
      m_labelCount(0)
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

void SymbolTable::appendGlobalSymbol(Symbol *symbol) {
    m_global->m_table.push_back(symbol);
}

SymbolProcedure *SymbolTable::appendProcedure(const TreeNodeProcDef &procdef) {
    typedef SecTypeProcedureVoid STPV;
    typedef DataTypeProcedureVoid DTPV;

    SymbolProcedure *ns = new SymbolProcedure(&procdef);

    assert(procdef.procedureType().kind() == TypeNonVoid::PROCEDURE
           || procdef.procedureType().kind() == TypeNonVoid::PROCEDUREVOID);
    assert(dynamic_cast<const DTPV*>(&procdef.procedureType().dataType()) != 0);
    const DTPV &dt = static_cast<const DTPV&>(procdef.procedureType().dataType());

    ns->setName("{proc}" + procdef.procedureName() + dt.mangle());
    appendGlobalSymbol(ns);

    return ns;
}

SymbolTemporary *SymbolTable::appendTemporary(const TypeNonVoid &type) {
    // Initialize temporary
    SymbolTemporary *tmp = new SymbolTemporary(TypeNonVoid(type.secType(), DataTypeVar(type.dataType())));
    std::ostringstream os;
    os << "{t}" << m_global->m_tempCount++;
    tmp->setName(os.str());
    appendSymbol(tmp);
    return tmp;
}

SymbolConstantBool *SymbolTable::constantBool(bool value) {
    const char *name = (value ? "{constBool}true" : "{constBool}false");
    Symbol *s = findGlobal(name);
    if (s != 0) {
        assert(dynamic_cast<SymbolConstantBool*>(s) != 0);
        return static_cast<SymbolConstantBool*>(s);
    }

    SymbolConstantBool *sc = new SymbolConstantBool(value);
    sc->setName(name);
    appendGlobalSymbol(sc);
    return sc;
}

SymbolConstantInt *SymbolTable::constantInt(int value) {
    std::ostringstream os;
    os << "{constInt}" << value;
    Symbol *s = findGlobal(os.str());
    if (s != 0) {
        assert(dynamic_cast<SymbolConstantInt*>(s) != 0);
        return static_cast<SymbolConstantInt*>(s);
    }
    SymbolConstantInt *sc = new SymbolConstantInt(value);
    sc->setName(os.str());
    appendGlobalSymbol(sc);
    return sc;
}

SymbolConstantUInt *SymbolTable::constantUInt(unsigned value) {
    std::ostringstream os;
    os << "{constUInt}" << value;
    Symbol *s = findGlobal(os.str());
    if (s != 0) {
        assert(dynamic_cast<SymbolConstantUInt*>(s) != 0);
        return static_cast<SymbolConstantUInt*>(s);
    }

    SymbolConstantUInt *sc = new SymbolConstantUInt(value);
    sc->setName(os.str());
    appendGlobalSymbol(sc);
    return sc;
}

SymbolConstantString *SymbolTable::constantString(const std::string &value) {
    /// \todo Implement hashing instead
    std::string name("{constString}" + value);
    Symbol *s = findGlobal(name);
    if (s != 0) {
        assert(dynamic_cast<SymbolConstantString*>(s) != 0);
        return static_cast<SymbolConstantString*>(s);
    }

    SymbolConstantString *sc = new SymbolConstantString(value);
    sc->setName(name);
    appendGlobalSymbol(sc);
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

Symbol *SymbolTable::findGlobal(const std::string &name) const {
    typedef Table::const_reverse_iterator STI;

    const Table &t(m_global->m_table);
    for (STI it(t.rbegin()); it != t.rend(); it++) {
        if ((*it)->name() == name)
            return (*it);
    }
    return 0;
}

SymbolProcedure *SymbolTable::findGlobalProcedure(const std::string &name,
                                                  const DataTypeProcedureVoid &dt)
{
    typedef Table::const_reverse_iterator STI;

    assert(name.empty() == false);

    std::string fn("{proc}" + name + dt.mangle());

    const Table &t(m_global->m_table);
    for (STI it(t.rbegin()); it != t.rend(); it++) {
        if ((*it)->name() == fn) {
            assert(dynamic_cast<SymbolProcedure*>(*it) != 0);
            return static_cast<SymbolProcedure*>(*it);
        }
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

std::string SymbolTable::toString(unsigned level, unsigned indent,
                                  bool newScope) const
{
    typedef Table::const_iterator STCI;

    std::ostringstream os;

    if (newScope) {
        printIndent(os, level, indent);
        os << "--- NEW SCOPE ---" << std::endl;
    }
    for (STCI it(m_table.begin()); it != m_table.end(); it++) {
        printIndent(os, level, indent);
        os << ' ' << (**it) << std::endl;
    }

    if (m_scope != 0) {
        os << m_scope->toString(level + 1, indent);
    }
    if (m_cont != 0) {
        os << m_cont->toString(level, indent, false);
    }
    return os.str();
}

} // namespace SecreC

