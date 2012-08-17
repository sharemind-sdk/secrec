#ifndef SECREC_SYMBOLTABLE_H
#define SECREC_SYMBOLTABLE_H

#include <vector>
#include <string>

namespace SecreC {

class Imop;
class TreeNodeStmtDecl;
class TreeNodeProcDef;
class Symbol;
class SymbolSymbol;
class SymbolLabel;
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
    SymbolTable (const std::string& name = "Global");
    explicit SymbolTable(SymbolTable *parent, const std::string& name = "Local");

    ~SymbolTable();

    void appendSymbol (Symbol* symbol);
    SymbolSymbol* appendTemporary (TypeNonVoid* type);
    SymbolLabel* label (Imop* imop);

    Symbol* find(const std::string& name) const;
    std::vector<Symbol* > findAll (const std::string& name) const;

    SymbolTable* newScope ();
    SymbolTable* parent () const { return m_parent; }
    SymbolTable* globalScope () const { return m_global; }

    /**
     * Add imported module for current scope.
     * \retval false if the scope is already imported
     * \retval true if the import was added
     */
    bool addImport (SymbolTable* st);

    void setName (const std::string& name) { m_name = name; }

    std::string toString(unsigned level = 0, unsigned indent = 4) const;

    /**
     * Find a symbol in current scope, follow imported modules.
     * \retval 0 if name is not found in the current scope.
     */
    Symbol* findFromCurrentScope (const std::string& name) const;

    std::vector<SymbolSymbol*> variablesUpTo (const SymbolTable* end) const;
    std::vector<SymbolSymbol*> variables () const;

private: /* Fields: */
    class OtherSymbols;

    Table                       m_table;    ///< Symbols.
    SymbolTable* const          m_parent;   ///< Parent scope.
    SymbolTable* const          m_global;   ///< Global scope.
    OtherSymbols* const         m_other;    ///< Temporaries and labels.
    std::vector<SymbolTable* >  m_imports;  ///< STs of imported modules.
    std::vector<SymbolTable* >  m_scopes;   ///< Local scopes.
    std::string                 m_name;     ///< Debugging.
};

std::ostream & operator<<(std::ostream & out, const SymbolTable & st);

} // namespace SecreC


#endif // SYMBOLTABLE_H
