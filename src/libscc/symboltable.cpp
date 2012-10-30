#include "symboltable.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <boost/foreach.hpp>

#include "symbol.h"
#include "treenode.h"

namespace SecreC {

namespace /* anonymous */ {

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
        std::ostringstream os;
        os << "{t}" << m_tempCount ++;
        SymbolSymbol * tmp = new SymbolSymbol(os.str(), type, true);
        m_temporaries.push_back (tmp);
        return tmp;
    }

    std::string toString () const {
        std::ostringstream os;
        os << "Temporaries:\n";
        BOOST_FOREACH (SymbolSymbol* s, m_temporaries)
            os << '\t' << *s << '\n';

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
    if (std::find (m_imports.begin (), m_imports.end (), st) == m_imports.end ()) {
        m_imports.push_back (st);
        return true;
    }

    return false;
}

Symbol* SymbolTable::findFromCurrentScope (StringRef name) const {
    BOOST_REVERSE_FOREACH (SymbolTable* import, m_imports)
        BOOST_REVERSE_FOREACH (Symbol* s, import->m_table)
            if (s->name () == name)
                return s;
    return 0;
}

std::vector<Symbol *>
SymbolTable::findPrefixedFromCurrentScope(StringRef prefix) const {
    std::vector<Symbol *> r;
    BOOST_REVERSE_FOREACH(SymbolTable * import, m_imports)
        BOOST_REVERSE_FOREACH(Symbol * s, import->m_table)
            if (prefix.isPrefixOf(s->name()))
                r.push_back(s);
    return r;
}

std::vector<SymbolSymbol*> SymbolTable::variables () const {
    std::vector<SymbolSymbol*> out;
    BOOST_REVERSE_FOREACH (Symbol* sym, m_table) {
        if (sym->symbolType () == Symbol::SYMBOL) {
            assert (dynamic_cast<SymbolSymbol*>(sym) != 0);
            SymbolSymbol* ssym = static_cast<SymbolSymbol*>(sym);
            assert (! ssym->isTemporary ());
            out.push_back (ssym);
        }
    }

    return out;
}

std::vector<SymbolSymbol*> SymbolTable::variablesUpTo (const SymbolTable* end) const {
    std::vector<SymbolSymbol*> out;
    for (const SymbolTable* st = this; st != end; st = st->parent ()) {
        const std::vector<SymbolSymbol*>& local = st->variables ();
        out.insert (out.end (), local.begin (), local.end ());
    }

    return out;
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

Symbol *SymbolTable::find (StringRef name) const {
    const SymbolTable *c = this;
    while (c != 0) {
        Symbol* s = c->findFromCurrentScope (name);
        if (s != 0)
            return s;
        c = c->m_parent;
    }

    return 0;
}

std::vector<Symbol *>
SymbolTable::findPrefixed(StringRef prefix) const {
    std::vector<Symbol *> r;
    const SymbolTable * c = this;
    while (c != 0) {
        std::vector<Symbol *> r2 = c->findPrefixedFromCurrentScope(prefix);
        BOOST_FOREACH(Symbol * s2, r2) {
            bool overridden = false;
            BOOST_FOREACH(Symbol * s, r)
                if (s2->name() != s->name()) {
                    overridden = true;
                    break;
                }
            if (!overridden)
                r.push_back(s2);
        }

        c = c->m_parent;
    }

    return r;
}

std::vector<Symbol* > SymbolTable::findAll (StringRef name) const {
    std::vector<Symbol* > out;
    for (Symbol* s = find (name); s != 0; s = s->previos ()) {
        out.push_back (s);
    }

    return out;
}

SymbolTable *SymbolTable::newScope () {
    SymbolTable *scope = new SymbolTable (this);
    m_scopes.push_back (scope);
    return scope;
}

std::string SymbolTable::toString(unsigned level, unsigned indent) const {
    std::ostringstream os;

    if (m_parent == 0) {
        os << "--- Other Symbols ---" << std::endl;
        os << m_other->toString ();
    }

    printIndent(os, level, indent);
    os << "==  " << m_name << " (";
    BOOST_FOREACH (SymbolTable* import, m_imports) {
        if (import != this) {
            os << ", ";
        }

        os << import;
    }

    os << ") ==" << std::endl;


    BOOST_FOREACH (Symbol* sym, m_table) {
        printIndent(os, level, indent);
        os << ' ' << *sym << std::endl;
    }

    BOOST_FOREACH (SymbolTable* table, m_scopes) {
        os << table->toString(level+1, indent);
    }

    return os.str();
}

std::ostream & operator<<(std::ostream & out, const SymbolTable & st) {
    out << st.toString ();
    return out;
}


} // namespace SecreC

