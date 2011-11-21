#ifndef SECREC_SYMBOLTABLE_H
#define SECREC_SYMBOLTABLE_H

#include <list>

#include "symbol.h"
#include "constant.h"


namespace SecreC {

class Imop;
class TreeNodeStmtDecl;
class TreeNodeProcDef;
class GlobalSymbols;
class Block;

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
        void appendGlobalSymbol(Symbol *symbol);

        SymbolProcedure *appendProcedure(const TreeNodeProcDef &procdef);
        SymbolSymbol *appendTemporary(TypeNonVoid* type);

        SymbolLabel *label (Imop* imop);
        Symbol *find(const std::string &name) const;
        Symbol *findGlobal(const std::string &name) const;
        SymbolProcedure *findGlobalProcedure(const std::string &name,
                                             const DataTypeProcedureVoid &dt);
        SymbolTable *newScope();

        std::string toString(unsigned level = 0, unsigned indent = 4,
                             bool newScope = true) const;

        SymbolTable* parent () const {
            return m_parent;
        }

        /* methods to construct constants */

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
