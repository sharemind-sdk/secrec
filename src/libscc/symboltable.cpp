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
 * \brief class for handling global symbols.
 * \a GlobalSymbols class tracks symbols for constants, procedure names,
 *    global variables, and temporary variables created in intermediate code
 *    generation process.
 */
class GlobalSymbols {
public:
    GlobalSymbols ();
    ~GlobalSymbols ();

    // constants
    ConstantBool*    constantBool (bool value);
    ConstantInt*     constantInt (int value);
    ConstantUInt*    constantUInt (unsigned value);
    ConstantString*  constantString (const std::string &value);

    // labels
    SymbolLabel*     label (Imop* imop);

    // symbols for variables and procedures
    Symbol*          find (const std::string& name) const;
    void             append (Symbol* sym);
    SymbolTemporary* temporary (const TypeNonVoid &type);

    //
    std::string toString () const;

private:
    ConstantBool*                           m_constantTrue;
    ConstantBool*                           m_constantFalse;
    std::map<int, ConstantInt* >            m_constantInts;
    std::map<unsigned, ConstantUInt* >      m_constantUInts;
    std::map<std::string, ConstantString* > m_constantStrings;
    std::map<Imop*, SymbolLabel* >          m_labels;
    std::map<std::string, Symbol* >         m_globals;
    unsigned                                m_tempCount;
};

GlobalSymbols::GlobalSymbols ()
    : m_constantTrue (0),
      m_constantFalse (0),
      m_tempCount (0)
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
    deleteValues (m_globals);
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

void GlobalSymbols::append (Symbol *sym) {
    assert (sym != 0);
    std::map<std::string, Symbol*>::iterator
            it = m_globals.find (sym->name ());
    if (it != m_globals.end ()) {
        assert (false && "Appening symbol already in global symbol table.");
        return;
    }

    m_globals.insert (it, std::make_pair (sym->name (), sym));
}

Symbol* GlobalSymbols::find (const std::string &name) const {
    std::map<std::string, Symbol*>::const_iterator
            it = m_globals.find (name);
    Symbol* out = 0;
    if (it != m_globals.end ()) {
        out = it->second;
    }

    return out;
}

SymbolTemporary* GlobalSymbols::temporary (const TypeNonVoid &type) {
    SymbolTemporary* tmp = new SymbolTemporary (DataTypeVar(type.dataType()));
    std::ostringstream os;
    os << "{t}" << m_tempCount ++;
    tmp->setName (os.str ());
    append (tmp);
    return tmp;
}

template <typename T, typename V >
static void printValues (typename std::map<T, V* > const& kvs, std::ostringstream& os) {
    typename std::map<T, V* >::const_iterator i;
    for (i = kvs.begin (); i != kvs.end (); ++ i) {
        os << *i->second << '\n';
    }
}

std::string GlobalSymbols::toString () const {
    std::ostringstream os;

    os << "* Boolean constants:\n";
    if (m_constantFalse != 0) os << *m_constantFalse << '\n';
    if (m_constantTrue != 0)  os << *m_constantTrue << '\n';

    os << "* Int constants:\n";
    printValues (m_constantInts, os);

    os << "* UInt constants:\n";
    printValues (m_constantUInts, os);

    os << "* String constants:\n";
    printValues (m_constantStrings, os);

    os << "* Labels:\n";
    printValues (m_labels, os);

    os << "* Globals:\n";
    printValues (m_globals, os);

    return os.str ();
}

/*******************************************************************************
  SymbolTable
*******************************************************************************/

SymbolTable::SymbolTable (SymbolTable *parent)
    : m_parent (parent),
      m_global (m_parent == 0 ? new GlobalSymbols () : m_parent->m_global)
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
        delete m_global;
    }
}

void SymbolTable::appendSymbol(Symbol *symbol) {
    m_table.push_back(symbol);
}

void SymbolTable::appendGlobalSymbol(Symbol *symbol) {
    m_global->append (symbol);
}

ConstantBool *SymbolTable::constantBool(bool value) {
    return m_global->constantBool (value);
}

ConstantInt *SymbolTable::constantInt(int value) {
    return m_global->constantInt (value);
}

ConstantUInt *SymbolTable::constantUInt(unsigned value) {
    return m_global->constantUInt (value);
}

ConstantString *SymbolTable::constantString(const std::string &value) {
    return m_global->constantString (value);
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
    return m_global->label (imop);
}

Symbol *SymbolTable::findGlobal(const std::string &name) const {
    return m_global->find (name);
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
    return m_global->temporary (type);
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
        if (c->m_parent == 0) {
            return m_global->find (name);
        }

        c = c->m_parent;
    }
    return 0;
}

SymbolProcedure *SymbolTable::findGlobalProcedure(const std::string &name,
                                                  const DataTypeProcedureVoid &dt)
{
    assert(name.empty() == false);

    std::string fn("{proc}" + name + dt.mangle());
    Symbol* sym = m_global->find (fn);
    if (sym != 0) {
        assert(dynamic_cast<SymbolProcedure*>(sym) != 0);
        return static_cast<SymbolProcedure*>(sym);
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

    if (m_parent == 0) {
        os << "--- GLOBAL SCOPE ---" << std::endl;
        os << m_global->toString ();
    }

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

