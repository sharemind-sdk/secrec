#ifndef SECREC_SYMBOLTABLE_H
#define SECREC_SYMBOLTABLE_H

#include <list>
#include <vector>

#include "symbol.h"

/**
 * \todo Rename the m_global field.
 */

namespace SecreC {

class Imop;
class TreeNodeStmtDecl;
class TreeNodeProcDef;

/*******************************************************************************
  SymbolTable
*******************************************************************************/

class SymbolTable {
    private: /* Types: */
        typedef std::vector<Symbol*> Table;

        SymbolTable (const SymbolTable&); // do not implement
        SymbolTable& operator = (const SymbolTable&); // do not implement

    public: /* Methods: */
        SymbolTable ();
        explicit SymbolTable(SymbolTable *parent);

        ~SymbolTable();

        void appendSymbol(Symbol *symbol);
        SymbolSymbol *appendTemporary(TypeNonVoid* type);
        SymbolLabel *label (Imop* imop);

        Symbol *find(const std::string &name) const;
        std::list<Symbol* > findAll (const std::string& name) const;

        SymbolTable *newScope();
        SymbolTable* parent () const { return m_parent; }
        SymbolTable* globalScope () const { return m_global; }

        std::string toString(unsigned level = 0, unsigned indent = 4,
                             bool newScope = true) const;


    private: /* Fields: */
        class OtherSymbols;

        Table                     m_table;
        SymbolTable* const        m_parent;  ///< Parent scope.
        SymbolTable* const        m_global;  ///< Global scope.
        OtherSymbols* const       m_other;   ///< Temporaries and labels.
        std::list<SymbolTable* >  m_scopes;
};

} // namespace SecreC

inline std::ostream &operator<<(std::ostream &out, const SecreC::SymbolTable &st) {
    out << st.toString();
    return out;
}

#endif // SYMBOLTABLE_H
