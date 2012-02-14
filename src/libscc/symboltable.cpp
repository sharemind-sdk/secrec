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
  SymbolTable::OtherSymbols
*******************************************************************************/

/**
 * \brief class for handling some other symbols.
 * \todo not sure if i should store them in symbol table
 */
class SymbolTable::OtherSymbols {
private: /* Types: */
    typedef std::map<const Imop*, SymbolLabel* > LabelMap;
public: /* Methods: */
    OtherSymbols ()
        : m_tempCount (0)
    { }

    ~OtherSymbols () {
        BOOST_FOREACH (LabelMap::value_type& v, m_labels) {
            delete v.second;
        }

        BOOST_FOREACH (SymbolSymbol* s, m_temporaries) {
            delete s;
        }
    }

    SymbolLabel* label (Imop* imop) {
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

    SymbolSymbol* temporary (TypeNonVoid* type) {
        SymbolSymbol* tmp = new SymbolSymbol (type, true);
        std::ostringstream os;
        os << "{t}" << m_tempCount ++;
        tmp->setName (os.str ());
        m_temporaries.push_back (tmp);
        return tmp;
    }

    std::string toString () const {
        std::ostringstream os;
        os << "Temporaries:\n";
        BOOST_FOREACH (SymbolSymbol* s, m_temporaries)
            os << '\t' << s->toString () << '\n';

        return os.str ();
    }

private: /* Fields: */

    std::map<const Imop*, SymbolLabel* >  m_labels;
    std::vector<SymbolSymbol* >           m_temporaries;
    unsigned                              m_tempCount;
};

/*******************************************************************************
  SymbolTable
*******************************************************************************/

SymbolTable::SymbolTable (const std::string& name)
    : m_parent (0)
    , m_global (this)
    , m_other (new OtherSymbols ())
    , m_imports (1, this)
    , m_name (name)
{
    // Intentionally empty
}

SymbolTable::SymbolTable (SymbolTable *parent, const std::string& name)
    : m_parent (parent)
    , m_global (parent->m_global)
    , m_other (parent->m_other)
    , m_imports (1, this)
    , m_name (name)
{
    // Intentionally empty
}

SymbolTable::~SymbolTable() {
    BOOST_FOREACH (Symbol* sym, m_table)
        delete sym;

    BOOST_FOREACH (SymbolTable* table, m_scopes)
        delete table;

    if (m_parent == 0)
        delete m_other;
}

bool SymbolTable::addImport (SymbolTable* st) {
    assert (st != 0);
    if (std::find (m_imports.begin (), m_imports.end (), st) != m_imports.end ()) {
        m_imports.push_back (st);
        return true;
    }

    return false;
}

Symbol* SymbolTable::findFromCurrentScope (const std::string& name) const {
    BOOST_REVERSE_FOREACH (SymbolTable* import, m_imports)
        BOOST_REVERSE_FOREACH (Symbol* s, import->m_table)
            if (s->name () == name)
                return s;
    return 0;
}

void SymbolTable::appendSymbol (Symbol* symbol) {
    assert (symbol != 0);
    symbol->setPrevious (findFromCurrentScope (symbol->name ()));
    m_table.push_back (symbol);
}

SymbolLabel* SymbolTable::label (Imop* imop) {
    assert (imop != 0);
    return m_other->label (imop);
}

SymbolSymbol *SymbolTable::appendTemporary (TypeNonVoid* type) {
    return m_other->temporary (type);
}

Symbol *SymbolTable::find (const std::string& name) const {
    const SymbolTable *c = this;
    while (c != 0) {
        Symbol* s = c->findFromCurrentScope (name);
        if (s != 0)
            return s;
        c = c->m_parent;
    }

    return 0;
}

std::vector<Symbol* > SymbolTable::findAll (const std::string& name) const {
    Symbol* s = find (name);
    std::vector<Symbol* > out;
    while (s != 0) {
        out.push_back (s);
        s = s->previos ();
    }

    return out;
}

SymbolTable *SymbolTable::newScope () {
    SymbolTable *scope = new SymbolTable (this);
    m_scopes.push_back (scope);
    return scope;
}

std::string SymbolTable::toString(unsigned level, unsigned indent,
                                  bool newScope) const
{
    std::ostringstream os;

    if (m_parent == 0) {
        os << "--- Other Symbols ---" << std::endl;
        os << m_other->toString ();
    }

    printIndent(os, level, indent);
    os << "==  " << m_name << " ==" << std::endl;


    BOOST_FOREACH (Symbol* sym, m_table) {
        printIndent(os, level, indent);
        os << ' ' << sym->toString () << std::endl;
    }

    BOOST_FOREACH (SymbolTable* table, m_scopes) {
        os << table->toString(level+1, indent);
    }

    return os.str();
}

} // namespace SecreC

