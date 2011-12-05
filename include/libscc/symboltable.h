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
class GlobalSymbols;

/*******************************************************************************
  SymbolTable
*******************************************************************************/

class SymbolTable {
    private: /* Types: */
        typedef std::vector<Symbol*> Table;

        SymbolTable (const SymbolTable&); // do not implement
        SymbolTable& operator = (const SymbolTable&); // do not implement

    public: /* Methods: */
        explicit SymbolTable(SymbolTable *parent = 0);
        ~SymbolTable();

        void appendSymbol(Symbol *symbol);

        SymbolSymbol *appendTemporary(TypeNonVoid* type);

        std::list<Symbol* > findAll (const std::string& name) const;

        SymbolLabel *label (Imop* imop);
        Symbol *find(const std::string &name) const;
        SymbolTable *newScope();

        std::string toString(unsigned level = 0, unsigned indent = 4,
                             bool newScope = true) const;

        SymbolTable* parent () const {
            return m_parent;
        }

        SymbolTable* globalScope () {
            SymbolTable* st = this;
            while (st->parent ()) {
                st = st->parent ();
            }

            return st;
        }

    private: /* Fields: */
        Table                     m_table;
        SymbolTable*              m_parent;
        GlobalSymbols*            m_global;
        std::list<SymbolTable* >  m_scopes;
};

} // namespace SecreC

inline std::ostream &operator<<(std::ostream &out, const SecreC::SymbolTable &st) {
    out << st.toString();
    return out;
}

#endif // SYMBOLTABLE_H
