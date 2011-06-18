#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "symbol.h"
#include "constant.h"

#include <list>

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
        void appendGlobalSymbol(Symbol *symbol);
        SymbolProcedure *appendProcedure(const TreeNodeProcDef &procdef);
        SymbolTemporary *appendTemporary(const TypeNonVoid &type);

        SymbolLabel *label (Imop* imop);
        Symbol *find(const std::string &name) const;
        Symbol *findGlobal(const std::string &name) const;
        SymbolProcedure *findGlobalProcedure(const std::string &name,
                                             const DataTypeProcedureVoid &dt);
        SymbolTable *newScope();

        std::string toString(unsigned level = 0, unsigned indent = 4,
                             bool newScope = true) const;


        /* methods to construct constants */
        Symbol* defaultConstant (SecrecDataType dataType);
        ConstantBool *constantBool(bool value);
        ConstantInt *constantInt(int value);
        ConstantUInt *constantUInt(unsigned value);
        ConstantString *constantString(const std::string &value);

    private: /* Fields: */
        Table        m_table;
        SymbolTable *m_parent;
        GlobalSymbols* m_global;
        std::list<SymbolTable* > m_scopes; ///< Used only for deleting and printing
};

} // namespace SecreC

inline std::ostream &operator<<(std::ostream &out, const SecreC::SymbolTable &st) {
    out << st.toString();
    return out;
}

#endif // SYMBOLTABLE_H
