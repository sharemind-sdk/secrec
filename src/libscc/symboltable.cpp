#include "symboltable.h"

#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <boost/foreach.hpp>

#include "treenode.h"


namespace {

using namespace SecreC;

void printIndent(std::ostream &out, unsigned level, unsigned indent = 4) {
    while (level-- > 0)
        for (unsigned i = 0; i < indent; i++)
            out << " ";
}


template <typename T, typename V >
static void deleteValues (typename std::map<T, V* >& kvs) {
    typename std::map<T, V* >::iterator i;
    for (i = kvs.begin (); i != kvs.end (); ++ i) {
        delete i->second;
    }

    kvs.clear ();
}

template <typename T, typename V >
void printValues (typename std::map<T, V* > const& kvs, std::ostringstream& os) {
    typename std::map<T, V* >::const_iterator i;
    for (i = kvs.begin (); i != kvs.end (); ++ i) {
        os << *i->second << '\n';
    }
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

    // labels
    SymbolLabel*     label (Imop* imop);

    // symbols for variables and procedures
    Symbol*          find (const std::string& name) const;
    void             append (Symbol* sym);
    SymbolSymbol*    temporary (TypeNonVoid* type);

    //
    std::string toString () const;

private:

    std::map<const Imop*, SymbolLabel* >    m_labels;
    std::map<std::string, Symbol* >         m_globals;
    unsigned                                m_tempCount;
};

GlobalSymbols::GlobalSymbols ()
    : m_tempCount (0)
{ }

GlobalSymbols::~GlobalSymbols () {
    deleteValues (m_labels);
    deleteValues (m_globals);
}

SymbolLabel* GlobalSymbols::label (Imop *imop) {
    assert (imop != 0);
    std::map<const Imop*, SymbolLabel*>::iterator
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

SymbolSymbol* GlobalSymbols::temporary (TypeNonVoid* type) {
    SymbolSymbol* tmp = new SymbolSymbol (type, true);
    std::ostringstream os;
    os << "{t}" << m_tempCount ++;
    tmp->setName (os.str ());
    append (tmp);
    return tmp;
}

std::string GlobalSymbols::toString () const {
    std::ostringstream os;

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
    : m_parent (parent)
    , m_global (m_parent == 0 ? new GlobalSymbols () : m_parent->m_global)
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

SymbolLabel* SymbolTable::label (Imop* imop) {
    assert (imop != 0);
    return m_global->label (imop);
}

SymbolSymbol *SymbolTable::appendTemporary(TypeNonVoid* type) {
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

std::list<Symbol* > SymbolTable::findAll (const std::string& name) const {
    typedef Table::const_reverse_iterator STI;
    std::list<Symbol* > out;
    const SymbolTable *c = this;
    for (;;) {
        const Table &t(c->m_table);
        for (STI it (t.rbegin()); it != t.rend(); it++) {
            if ((*it)->name() == name)
                out.push_back (*it);
        }
        if (c->m_parent == 0) {
            Symbol* s = m_global->find (name);
            if (s != 0)
                out.push_back (s);
            break;
        }

        c = c->m_parent;
    }

    return out;
}

SymbolTable *SymbolTable::newScope () {
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

