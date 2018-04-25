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

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>


namespace SecreC {

namespace /* anonymous */ {

void printIndent(std::ostream &out, unsigned level, unsigned indent = 4) {
    while (level-- > 0)
        for (unsigned i = 0; i < indent; i++)
            out << " ";
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
    using SymbolPtr = std::unique_ptr<Symbol>;
    using SymbolLabelPtr = std::unique_ptr<SymbolLabel>;
    using SymbolSymbolPtr = std::unique_ptr<SymbolSymbol>;

public: /* Methods: */

    SymbolLabel* label (Imop* imop) {
        assert (imop != nullptr);
        auto
                it = m_labels.find (imop);
        if (it != m_labels.end ()) {
            return it->second.get();
        }

        auto label = new SymbolLabel (imop);
        std::ostringstream os;
        os << "{label}" << imop;
        label->setName (os.str ());
        m_labels.emplace_hint(it, imop, label);
        return label;
    }

    SymbolSymbol* temporary (const TypeNonVoid* type) {
        std::ostringstream os;
        os << "{t}" << m_tempCount ++;
        SymbolSymbol * tmp = new SymbolSymbol(os.str(), type, true);
        m_temporaries.emplace_back (tmp);
        return tmp;
    }

    void print (std::ostream& os) const {
        os << "Temporaries:\n";
        for (auto const & s : m_temporaries)
            os << '\t' << *s << '\n';
    }

    void appendSymbol (Symbol* symbol) {
        assert (symbol != nullptr);
        m_table.emplace_back (symbol);
    }

private: /* Fields: */
    std::vector<SymbolPtr> m_table;
    std::map<const Imop*, SymbolLabelPtr> m_labels;
    std::vector<SymbolSymbolPtr> m_temporaries;
    unsigned m_tempCount = 0;
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

std::vector<SymbolSymbol*> SymbolTable::variables () const {
    std::vector<SymbolSymbol*> out;
    for (auto const & sym : boost::adaptors::reverse(m_table)) {
        if (sym->symbolType () == SYM_SYMBOL) {
            assert (dynamic_cast<SymbolSymbol*>(sym.get()) != nullptr);
            SymbolSymbol* ssym = static_cast<SymbolSymbol*>(sym.get());
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
    m_table.emplace_back (symbol);
}

void SymbolTable::appendOtherSymbol (Symbol* symbol) {
    assert (symbol != nullptr);
    m_other->appendSymbol (symbol);
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
    return findAll (
        [=](Symbol* s) {
            return s->symbolType () == type && prefix.isPrefixOf (s->name ());
        });
}

std::vector<Symbol* > SymbolTable::findAll (SymbolCategory type, StringRef name) const {
    std::vector<Symbol* > out;
    for (const SymbolTable* c = this; c != nullptr; c = c->m_parent) {
        const std::vector<Symbol*>& syms = c->findFromCurrentScope (type, name);
        out.insert (out.end (), syms.begin (), syms.end ());
    }

    return out;
}

std::vector<Symbol*> SymbolTable::findFromCurrentScope (SymbolCategory type, StringRef name) const {
    return findFromCurrentScope (
        [=](Symbol* s) {
            return s->symbolType () == type && s->name () == name;
        });
}

SymbolTable *SymbolTable::newScope () {
    auto scope = new SymbolTable (this);
    m_scopes.emplace_back (scope);
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


    for (auto const & sym : m_table) {
        printIndent(os, level, indent);
        os << ' ' << sym->name () << ": " << *sym << std::endl;
    }

    for (auto const & table : m_scopes) {
        table->print (os, level + 1, indent);
    }
}

std::ostream & operator<<(std::ostream & out, const SymbolTable & st) {
    st.print (out);
    return out;
}


} // namespace SecreC

