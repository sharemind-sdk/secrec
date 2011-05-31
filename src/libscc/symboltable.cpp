#include "symboltable.h"
#include "treenode.h"

#include <cassert>
#include <iostream>
#include <map>
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
  GlobalSymbols
*******************************************************************************/

/**
 * \brief class for tracking global symbols
 */
class GlobalSymbols {
public:
    GlobalSymbols ();
    ~GlobalSymbols ();

    ConstantBool*   constantBool (bool value);
    ConstantInt*    constantInt (int value);
    ConstantUInt*   constantUInt (unsigned value);
    ConstantString* constantString (const std::string &value);
    SymbolLabel*    label (Imop* imop);

private:
    ConstantBool*                           m_constantTrue;
    ConstantBool*                           m_constantFalse;
    std::map<int, ConstantInt* >            m_constantInts;
    std::map<unsigned, ConstantUInt* >      m_constantUInts;
    std::map<std::string, ConstantString* > m_constantStrings;
    std::map<Imop*, SymbolLabel* >          m_labels;
};

GlobalSymbols::GlobalSymbols ()
    : m_constantTrue (0),
      m_constantFalse (0)
{ }

template <typename T, typename V >
static void deleteValues (typename std::map<T, V* >& kvs) {
    typename std::map<T, V* >::iterator i;
    for (i = kvs.begin (); i != kvs.end (); ++ i) {
        delete i->second;
    }

    kvs.clear ();
}

GlobalSymbols::~GlobalSymbols () {
    delete m_constantTrue;
    delete m_constantFalse;
    deleteValues (m_constantInts);
    deleteValues (m_constantUInts);
    deleteValues (m_constantStrings);
    deleteValues (m_labels);
}

ConstantBool* GlobalSymbols::constantBool (bool value) {
    ConstantBool* result = value ? m_constantTrue : m_constantFalse;

    if (result == 0) {
        result = new ConstantBool (value);
        if (value) {
            const char* name = "{constBool}true";
            result->setName (name);
            m_constantTrue = result;
        }
        else {
            const char* name = "{constBool}false";
            result->setName (name);
            m_constantFalse = result;
        }
    }

    return result;
}

ConstantInt* GlobalSymbols::constantInt (int value) {
    std::map<int, ConstantInt*>::iterator
            it = m_constantInts.find (value);
    if (it != m_constantInts.end ()) {
        return it->second;
    }

    ConstantInt* cVal = new ConstantInt (value);
    std::ostringstream os;
    os << "{constInt}" << value;
    cVal->setName (os.str ());
    m_constantInts.insert (it, std::make_pair (value, cVal));
    return cVal;
}

ConstantUInt* GlobalSymbols::constantUInt (unsigned value) {
    std::map<unsigned, ConstantUInt*>::iterator
            it = m_constantUInts.find (value);
    if (it != m_constantUInts.end ()) {
        return it->second;
    }

    ConstantUInt* cVal = new ConstantUInt (value);
    std::ostringstream os;
    os << "{constUInt}" << value;
    cVal->setName (os.str ());
    m_constantUInts.insert (it, std::make_pair (value, cVal));
    return cVal;
}

ConstantString* GlobalSymbols::constantString (const std::string &value) {
    std::map<std::string, ConstantString*>::iterator
            it = m_constantStrings.find (value);
    if (it != m_constantStrings.end ()) {
        return it->second;
    }

    ConstantString* cVal = new ConstantString (value);
    std::string name("{constString}" + value);
    cVal->setName (name);
    m_constantStrings.insert (it, std::make_pair (value, cVal));
    return cVal;
}

SymbolLabel* GlobalSymbols::label (Imop *imop) {
    assert (imop != 0);
    std::map<Imop*, SymbolLabel*>::iterator
            it = m_labels.find (imop);
    if (it != m_labels.end ()) {
        return it->second;
    }

    SymbolLabel* label = new SymbolLabel (imop);
    std::ostringstream os;
    os << "{label}" << imop;
    label->setName (os.str ());
    m_labels.insert (it, std::make_pair (imop, label));
    return label;
}

/*******************************************************************************
  SymbolTable
*******************************************************************************/

SymbolTable::SymbolTable(SymbolTable *parent)
    : m_parent(parent),
      m_global(m_parent == 0 ? this : m_parent->m_global),
      m_globalST (m_parent == 0 ? new GlobalSymbols () : m_parent->m_globalST),
      m_tempCount(0),
      m_constCount(0),
      m_labelCount(0)
{
    // Intentionally empty
}

SymbolTable::~SymbolTable() {
    typedef std::vector<Symbol*>::const_iterator SVCI;
    typedef std::list<SymbolTable*>::const_iterator STLCI;

    for (SVCI it(m_table.begin()); it != m_table.end(); it++) {
        delete (*it);
    }

    for (STLCI it(m_scopes.begin()); it != m_scopes.end(); ++ it ) {
        delete *(it);
    }

    if (m_parent == 0) {
        delete m_globalST;
    }
}

void SymbolTable::appendSymbol(Symbol *symbol) {
    m_table.push_back(symbol);
}

void SymbolTable::appendGlobalSymbol(Symbol *symbol) {
    m_global->m_table.push_back(symbol);
}

SymbolProcedure *SymbolTable::appendProcedure(const TreeNodeProcDef &procdef) {
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
    SymbolTemporary *tmp = new SymbolTemporary(DataTypeVar(type.dataType()));
    std::ostringstream os;
    os << "{t}" << m_global->m_tempCount++;
    tmp->setName(os.str());
    appendSymbol(tmp);
    return tmp;
}

ConstantBool *SymbolTable::constantBool(bool value) {
    return m_globalST->constantBool (value);
}

ConstantInt *SymbolTable::constantInt(int value) {
    return m_globalST->constantInt (value);
}

ConstantUInt *SymbolTable::constantUInt(unsigned value) {
    return m_globalST->constantUInt (value);
}

ConstantString *SymbolTable::constantString(const std::string &value) {
    return m_globalST->constantString (value);
}

Symbol* SymbolTable::defaultConstant (SecrecDataType dataType) {
    Symbol* result = 0;
    switch (dataType) {
        case DATATYPE_BOOL:    result = constantBool(false); break;
        case DATATYPE_INT:     result = constantInt(0); break;
        case DATATYPE_UINT:    result = constantUInt(0); break;
        case DATATYPE_STRING:  result = constantString(""); break;
        default:
            assert (false && "Don't know how to produce default constant of given type.");
    }

    return result;
}

SymbolLabel* SymbolTable::label (Imop* imop) {
    return m_globalST->label (imop);
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
    m_scopes.push_back(scope);
    return scope;
}

std::string SymbolTable::toString(unsigned level, unsigned indent,
                                  bool newScope) const
{
    typedef Table::const_iterator STCI;
    typedef std::list<SymbolTable*>::const_iterator STLCI;

    std::ostringstream os;

    if (newScope) {
        printIndent(os, level, indent);
        os << "--- NEW SCOPE ---" << std::endl;
    }

    for (STCI it(m_table.begin()); it != m_table.end(); it++) {
        printIndent(os, level, indent);
        os << ' ' << (**it) << std::endl;
    }

    for (STLCI it(m_scopes.begin()); it != m_scopes.end(); it++) {
        os << (*it)->toString(level+1, indent);
    }

    return os.str();
}

} // namespace SecreC

