#ifndef SECREC_SYMBOLTABLE_H
#define SECREC_SYMBOLTABLE_H

#include <vector>
#include "StringRef.h"
#include "symbol_fwd.h"
#include "treenode_fwd.h"

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
    typedef std::vector<Symbol*> Table;

    SymbolTable (const SymbolTable&); // do not implement
    SymbolTable& operator = (const SymbolTable&); // do not implement

public: /* Methods: */
    SymbolTable (StringRef name = "Global");
    explicit SymbolTable(SymbolTable *parent, StringRef name = "Local");

    ~SymbolTable();

    void appendSymbol (Symbol* symbol);
    SymbolSymbol* appendTemporary (TypeNonVoid* type);
    SymbolLabel* label (Imop* imop);

    Symbol* find (SymbolType symbolType, StringRef name) const;

    /**
      Finds symbols given a name prefix, following imported modules.
      \param[in] prefix the name prefix of the symbols to find.
      \returns a vector of pointers to the matching symbols.
    */
    std::vector<Symbol *> findPrefixed(SymbolType type, StringRef prefix) const;


    std::vector<Symbol* > findAll (SymbolType type, StringRef name) const;

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
    Symbol* findFromCurrentScope (SymbolType symbolType, StringRef name) const;

    std::vector<SymbolSymbol*> variablesUpTo (const SymbolTable* end) const;
    std::vector<SymbolSymbol*> variables () const;

private:

    /**
      Finds symbols in current scope given their name prefix, following imported modules.
      \param[in] prefix the name prefix of the symbol to find.
      \returns a vector of pointers to the matching symbols.
    */
    std::vector<Symbol *> findPrefixedFromCurrentScope(SymbolType type, StringRef prefix) const;

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
