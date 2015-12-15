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

#ifndef SECREC_SYMBOLTABLE_H
#define SECREC_SYMBOLTABLE_H

#include "StringRef.h"
#include "SymbolFwd.h"
#include "TreeNodeFwd.h"

#include <vector>

namespace SecreC {

/**
 * \todo Try to use StringRef instead of std::string here.
 */

class Imop;
class TypeNonVoid;

/*******************************************************************************
  SymbolTable
*******************************************************************************/

class SymbolTable {
private: /* Types: */
    using Table = std::vector<Symbol*>;

    SymbolTable (const SymbolTable&); // do not implement
    SymbolTable& operator = (const SymbolTable&); // do not implement

public: /* Methods: */
    SymbolTable (StringRef name = "Global");
    explicit SymbolTable(SymbolTable *parent, StringRef name = "Local");

    ~SymbolTable();

    void appendSymbol (Symbol* symbol);
    void appendOtherSymbol (Symbol* symbol);
    SymbolSymbol* appendTemporary (TypeNonVoid* type);
    SymbolLabel* label (Imop* imop);

    Symbol* find (SymbolCategory symbolType, StringRef name) const;

    template <SymbolCategory symbolType>
    typename SymbolTraits<symbolType>::Type* find (StringRef name) const {
        return static_cast<typename SymbolTraits<symbolType>::Type*>(find (symbolType, name));
    }

    /**
      Finds symbols given a name prefix, following imported modules.
      \param[in] prefix the name prefix of the symbols to find.
      \returns a vector of pointers to the matching symbols.
    */
    std::vector<Symbol *> findPrefixed(SymbolCategory type, StringRef prefix) const;


    std::vector<Symbol* > findAll (SymbolCategory type, StringRef name) const;

    SymbolTable* newScope ();
    SymbolTable* parent () const { return m_parent; }
    SymbolTable* globalScope () const { return m_global; }

    /**
     * Add imported module for current scope.
     * \retval false if the scope is already imported
     * \retval true if the import was added
     */
    bool addImport (SymbolTable* st);

    void setName (StringRef name) { m_name = name; }

    void print (std::ostream& os, unsigned level = 0, unsigned indent = 4) const;

    /**
      Find a symbol in current scope given its name, following imported modules.
      \param[in] name the name of the symbol to find.
      \returns a pointer to the symbol or NULL no such symbol was found.
    */
    std::vector<Symbol *> findFromCurrentScope (SymbolCategory symbolType, StringRef name) const;

    /**
       Find all procedures with the given name (regardless of type).
       \param[in] name of the procedure to find.
       \returns a vector of pointers to the matching symbols.
    */
    std::vector<SymbolProcedure*> findAllProcedures (StringRef name) const;

    /**
       Finds all procedures with the given name (regardless of type)
       from the current scope, following imported modules.
       \param[in] name of the procedure to find.
       \returns a vector of pointers to the matching symbols.
    */
    std::vector<SymbolProcedure*> findAllProceduresFromCurrentScope (StringRef name) const;

    std::vector<SymbolSymbol*> variablesUpTo (const SymbolTable* end) const;
    std::vector<SymbolSymbol*> variables () const;

private:

    /**
      Finds symbols in current scope given their name prefix, following imported modules.
      \param[in] prefix the name prefix of the symbol to find.
      \returns a vector of pointers to the matching symbols.
    */
    std::vector<Symbol *> findPrefixedFromCurrentScope(SymbolCategory type, StringRef prefix) const;

private: /* Fields: */

    class OtherSymbols;

    Table                       m_table;    ///< Symbols.
    SymbolTable* const          m_parent;   ///< Parent scope.
    SymbolTable* const          m_global;   ///< Global scope.
    OtherSymbols* const         m_other;    ///< Temporaries and labels.
    std::vector<SymbolTable* >  m_imports;  ///< STs of imported modules.
    std::vector<SymbolTable* >  m_scopes;   ///< Local scopes.
    StringRef                   m_name;     ///< Debugging.
};

std::ostream & operator<<(std::ostream & out, const SymbolTable & st);

} // namespace SecreC


#endif // SYMBOLTABLE_H
