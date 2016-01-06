/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "SymbolTable.h"

#include "Symbol.h"
#include "TreeNode.h"

#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>

using boost::adaptors::reverse;

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
    using LabelMap = std::map<const Imop*, SymbolLabel* >;
public: /* Methods: */
    OtherSymbols ()
        : m_tempCount (0)
    { }

    ~OtherSymbols () {
        for (LabelMap::value_type& v : m_labels) {
            delete v.second;
        }

        for (SymbolSymbol* s : m_temporaries) {
            delete s;
        }
    }

    SymbolLabel* label (Imop* imop) {
        assert (imop != nullptr);
        auto
                it = m_labels.find (imop);
        if (it != m_labels.end ()) {
            return it->second;
        }

        auto label = new SymbolLabel (imop);
        std::ostringstream os;
        os << "{label}" << imop;
        label->setName (os.str ());
        m_labels.insert (it, std::make_pair (imop, label));
        return label;
    }

    SymbolSymbol* temporary (const TypeNonVoid* type) {
        std::ostringstream os;
        os << "{t}" << m_tempCount ++;
        SymbolSymbol * tmp = new SymbolSymbol(os.str(), type, true);
        m_temporaries.push_back (tmp);
        return tmp;
    }

    void print (std::ostream& os) const {
        os << "Temporaries:\n";
        for (SymbolSymbol* s : m_temporaries)
            os << '\t' << *s << '\n';
    }

private: /* Fields: */

    std::map<const Imop*, SymbolLabel* >  m_labels;
    std::vector<SymbolSymbol* >           m_temporaries;
    unsigned                              m_tempCount;
};

/*******************************************************************************
  SymbolTable
*******************************************************************************/

SymbolTable::SymbolTable (StringRef name)
    : m_parent (nullptr)
    , m_global (this)
    , m_other (new OtherSymbols ())
    , m_imports (1, this)
    , m_name (std::move(name))
{
    // Intentionally empty
}

SymbolTable::SymbolTable (SymbolTable *parent, StringRef name)
    : m_parent (parent)
    , m_global (parent->m_global)
    , m_other (parent->m_other)
    , m_imports (1, this)
    , m_name (std::move(name))
{
    // Intentionally empty
}

SymbolTable::~SymbolTable() {
    for (Symbol* sym : m_table)
        delete sym;

    for (SymbolTable* table : m_scopes)
        delete table;

    if (m_parent == nullptr)
        delete m_other;
}

bool SymbolTable::addImport (SymbolTable* st) {
    assert (st != nullptr);
    if (std::find (m_imports.begin (), m_imports.end (), st) == m_imports.end ()) {
        m_imports.push_back (st);
        return true;
    }

    return false;
}

std::vector<Symbol *>
SymbolTable::findFromCurrentScope (SymbolCategory type, StringRef name) const {
    std::vector<Symbol *> r;
    for (SymbolTable* import : reverse (m_imports))
        for (Symbol* s : reverse (import->m_table))
            if (s->symbolType () == type && s->name () == name)
                r.push_back (s);
    return r;
}

std::vector<Symbol *>
SymbolTable::findPrefixedFromCurrentScope(SymbolCategory type, StringRef prefix) const {
    std::vector<Symbol *> r;
    for (SymbolTable * import : reverse (m_imports))
        for(Symbol * s : reverse (import->m_table))
            if (s->symbolType () == type && prefix.isPrefixOf(s->name()))
                r.push_back(s);
    return r;
}

std::vector<SymbolSymbol*> SymbolTable::variables () const {
    std::vector<SymbolSymbol*> out;
    for (Symbol* sym : reverse (m_table)) {
        if (sym->symbolType () == SYM_SYMBOL) {
            assert (dynamic_cast<SymbolSymbol*>(sym) != nullptr);
            SymbolSymbol* ssym = static_cast<SymbolSymbol*>(sym);
            assert (! ssym->isTemporary ());
            out.push_back (ssym);
        }
    }

    return out;
}

std::vector<SymbolSymbol*> SymbolTable::variablesUpTo (const SymbolTable* end) const {
    std::vector<SymbolSymbol*> out;
    for (const SymbolTable* st = this; st != nullptr && st != end; st = st->parent ()) {
        const std::vector<SymbolSymbol*>& local = st->variables ();
        out.insert (out.end (), local.begin (), local.end ());
    }

    return out;
}

void SymbolTable::appendSymbol (Symbol* symbol) {
    assert (symbol != nullptr);
    m_table.push_back (symbol);
}

SymbolLabel* SymbolTable::label (Imop* imop) {
    assert (imop != nullptr);
    return m_other->label (imop);
}

SymbolSymbol *SymbolTable::appendTemporary (const TypeNonVoid* type) {
    return m_other->temporary (type);
}

Symbol *SymbolTable::find (SymbolCategory type, StringRef name) const {
    for (const SymbolTable* c = this; c != nullptr; c = c->m_parent) {
        const std::vector<Symbol*>& syms = c->findFromCurrentScope (type, name);
        if (syms.empty ()) continue;
        if (syms.size () > 1) return nullptr;
        return syms.front ();
    }

    return nullptr;
}

std::vector<Symbol *>
SymbolTable::findPrefixed(SymbolCategory type, StringRef prefix) const {
    std::vector<Symbol *> r;
    const SymbolTable * c = this;
    while (c != nullptr) {
        for (Symbol * s2 : c->findPrefixedFromCurrentScope(type, prefix)) {
            bool overridden = false;
            for (Symbol * s : r)
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

std::vector<Symbol* > SymbolTable::findAll (SymbolCategory type, StringRef name) const {
    std::vector<Symbol* > out;
    for (const SymbolTable* c = this; c != nullptr; c = c->m_parent) {
        const std::vector<Symbol*>& syms = c->findFromCurrentScope (type, name);
        out.insert (out.end (), syms.begin (), syms.end ());
    }

    return out;
}

SymbolTable *SymbolTable::newScope () {
    auto scope = new SymbolTable (this);
    m_scopes.push_back (scope);
    return scope;
}

void SymbolTable::print (std::ostream& os, unsigned level, unsigned indent) const {
    if (m_parent == nullptr) {
        os << "--- Other Symbols ---" << std::endl;
        m_other->print (os);
    }

    printIndent(os, level, indent);
    os << "==  " << m_name << " (";
    for (SymbolTable* import : m_imports) {
        if (import != this) {
            os << ", ";
        }

        os << import;
    }

    os << ") ==" << std::endl;


    for (Symbol* sym : m_table) {
        printIndent(os, level, indent);
        os << ' ' << sym->name () << ": " << *sym << std::endl;
    }

    for (SymbolTable* table : m_scopes) {
        table->print (os, level + 1, indent);
    }
}

std::ostream & operator<<(std::ostream & out, const SymbolTable & st) {
    st.print (out);
    return out;
}


} // namespace SecreC

