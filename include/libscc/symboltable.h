#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "types.h"

#include <list>


namespace SecreC {

class Imop;
class TreeNodeStmtDecl;
class TreeNodeProcDef;

class Symbol {
    public: /* Types: */
        enum Type { PROCEDURE, CONSTANT, SYMBOL, TEMPORARY };
        typedef std::vector<Symbol* >::iterator dim_iterator;
        typedef std::vector<Symbol* >::reverse_iterator dim_reverese_iterator;
        typedef std::vector<Symbol* >::const_iterator const_dim_iterator;

    public: /* Methods: */
        explicit inline Symbol(Type symbolType,
                               const SecreC::TypeNonVoid &valueType)
            : m_symbolType(symbolType),
              m_type(valueType.clone()),
              m_dims(valueType.secrecDimType(), (Symbol*) 0),
              m_size((Symbol*) 0) {}
        virtual inline ~Symbol() { delete m_type; }

        inline Type symbolType() const { return m_symbolType; }
        inline const std::string &name() const { return m_name; }
        inline void setName(const std::string &name) { m_name = name; }
        inline const SecreC::Type &secrecType() const { return *m_type; }

        inline Symbol* getDim (unsigned i) { return m_dims[i]; }
        inline void setDim (unsigned i, Symbol* sym) { m_dims[i] = sym; }
        dim_iterator dim_begin () { return m_dims.begin(); }
        dim_reverese_iterator dim_rbegin() { return m_dims.rbegin(); }
        dim_reverese_iterator dim_rend() { return m_dims.rend(); }
        const_dim_iterator dim_begin () const { return m_dims.begin(); }
        dim_iterator dim_end () { return m_dims.end(); }
        const_dim_iterator dim_end () const { return m_dims.end(); }
        Symbol* getSizeSym () { return m_size; }
        void setSizeSym (Symbol* sym) { m_size = sym; }
        void inheritShape (Symbol* from);
        virtual std::string toString() const = 0;

    private: /* Fields: */
        const Type  m_symbolType;
        std::string m_name;
        SecreC::Type *m_type;
        std::vector<Symbol* > m_dims;
        Symbol* m_size;
};

class SymbolSymbol: public Symbol {
    public: /* Types: */
        enum ScopeType { GLOBAL, LOCAL };

    public: /* Methods: */
        SymbolSymbol(const SecreC::TypeNonVoid &valueType)
            : Symbol(Symbol::SYMBOL, valueType), m_scopeType(LOCAL) {}

        inline ScopeType scopeType() const { return m_scopeType; }
        inline void setScopeType(ScopeType type) { m_scopeType = type; }

        virtual std::string toString() const;

    private: /* Fields: */
        ScopeType   m_scopeType;
};

class SymbolTemporary: public Symbol {
    public: /* Types: */
        enum ScopeType { GLOBAL, LOCAL };

    public: /* Methods: */
        SymbolTemporary(const SecreC::TypeNonVoid &valueType)
            : Symbol(Symbol::TEMPORARY, valueType), m_scopeType(LOCAL) {}

        inline ScopeType scopeType() const { return m_scopeType; }
        inline void setScopeType(ScopeType type) { m_scopeType = type; }

        virtual std::string toString() const;

    private: /* Fields: */
        ScopeType m_scopeType;
};

class SymbolProcedure: public Symbol {
    public: /* Methods: */
        SymbolProcedure(const TreeNodeProcDef *procdef);

        inline const TreeNodeProcDef *decl() const { return m_decl; }
        inline const Imop *target() const { return m_target; }
        inline void setTarget(const Imop *target) { m_target = target; }

        virtual std::string toString() const;

    private: /* Fields: */
        const TreeNodeProcDef *m_decl;
        const Imop           *m_target;
};

class SymbolConstantBool: public Symbol {
    public: /* Methods: */
        explicit SymbolConstantBool(bool value)
            : Symbol(Symbol::CONSTANT, SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_BOOL)), m_value(value) {}

        inline bool value() const { return m_value; }

        virtual std::string toString() const;

    private: /* Fields: */
        const bool m_value;
};

class SymbolConstantInt: public Symbol {
    public: /* Methods: */
        explicit SymbolConstantInt(int value)
            : Symbol(Symbol::CONSTANT, SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT)), m_value(value) {}

        inline int value() const { return m_value; }

        virtual std::string toString() const;

    private: /* Fields: */
        const int m_value;
};

class SymbolConstantUInt: public Symbol {
    public: /* Methods: */
        explicit SymbolConstantUInt(unsigned value)
            : Symbol(Symbol::CONSTANT, SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT)), m_value(value) {}

        inline unsigned value() const { return m_value; }

        virtual std::string toString() const;

    private: /* Fields: */
        const unsigned m_value;
};

class SymbolConstantString: public Symbol {
    public: /* Methods: */
        explicit SymbolConstantString(const std::string &value)
            : Symbol(Symbol::CONSTANT, SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_STRING)), m_value(value) {}

        inline const std::string &value() const { return m_value; }

        virtual std::string toString() const;

    private: /* Fields: */
        const std::string m_value;
};

/**
 * Currently does not track in which order symbols and scopes
 * occur in code. This does not break code gen due to linear
 * nature of the process.
 */
class SymbolTable {
    private: /* Types: */
        typedef std::vector<Symbol*> Table;

    public: /* Methods: */
        SymbolTable(SymbolTable *parent = 0);
        ~SymbolTable();

        void appendSymbol(Symbol *symbol);
        void appendGlobalSymbol(Symbol *symbol);
        SymbolProcedure *appendProcedure(const TreeNodeProcDef &procdef);
        SymbolTemporary *appendTemporary(const TypeNonVoid &type);
        SymbolConstantBool *constantBool(bool value);
        SymbolConstantInt *constantInt(int value);
        SymbolConstantUInt *constantUInt(unsigned value);
        SymbolConstantString *constantString(const std::string &value);
        Symbol *find(const std::string &name) const;
        Symbol *findGlobal(const std::string &name) const;
        SymbolProcedure *findGlobalProcedure(const std::string &name,
                                             const DataTypeProcedureVoid &dt);
        SymbolTable *newScope();

        std::string toString(unsigned level = 0, unsigned indent = 4,
                             bool newScope = true) const;

    private: /* Fields: */
        Table        m_table;
        SymbolTable *m_parent;
        SymbolTable *m_global;
        std::list<SymbolTable* > m_scopes; ///< Used only for deleting and printing
        unsigned long m_tempCount;
        unsigned long m_constCount;
        unsigned long m_labelCount;
};

} // namespace SecreC

inline std::ostream &operator<<(std::ostream &out, const SecreC::Symbol &s) {
    out << s.toString();
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const SecreC::SymbolTable &st) {
    out << st.toString();
    return out;
}

#endif // SYMBOLTABLE_H
