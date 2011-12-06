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
        os << "Labels:\n";
        BOOST_FOREACH (const LabelMap::value_type& v, m_labels)
            os << '\t' << v.second->toString () << '\n';

        os << "\nTemporaries:\n";
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

SymbolTable::SymbolTable ()
    : m_parent (0)
    , m_global (this)
    , m_other (new OtherSymbols ())
{ }

SymbolTable::SymbolTable (SymbolTable *parent)
    : m_parent (parent)
    , m_global (m_parent->m_global)
    , m_other (m_parent->m_other)
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
        delete m_other;
    }
}

void SymbolTable::appendSymbol(Symbol *symbol) {
    Symbol* prev = find (symbol->name ());
    symbol->setPrevious (prev);
    m_table.push_back(symbol);
}

SymbolLabel* SymbolTable::label (Imop* imop) {
    assert (imop != 0);
    return m_other->label (imop);
}

SymbolSymbol *SymbolTable::appendTemporary(TypeNonVoid* type) {
    return m_other->temporary (type);
}

Symbol *SymbolTable::find(const std::string &name) const {
    const SymbolTable *c = this;
    while (c != 0) {
        BOOST_REVERSE_FOREACH (Symbol* s, c->m_table) {
            if (s->name () == name)
                return s;
        }

        c = c->m_parent;
    }

    return 0;
}

std::list<Symbol* > SymbolTable::findAll (const std::string& name) const {
    Symbol* s = find (name);
    std::list<Symbol* > out;
    while (s) {
        out.push_back (s);
        s = s->previos ();
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
        os << m_other->toString ();
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

