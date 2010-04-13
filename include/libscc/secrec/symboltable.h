#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "types.h"


namespace SecreC {

class Imop;
class TreeNodeStmtDecl;
class TreeNodeProcDef;

class Symbol {
    public: /* Types: */
        enum Type { PROCEDURE, CONSTANT, SYMBOL, TEMPORARY };

    public: /* Methods: */
        explicit inline Symbol(Type symbolType)
            : m_symbolType(symbolType) {}
        virtual inline ~Symbol() {}

        inline Type symbolType() const { return m_symbolType; }
        inline const std::string &name() const { return m_name; }
        inline void setName(const std::string &name) { m_name = name; }

        virtual std::string toString() const = 0;

    private: /* Fields: */
        const Type  m_symbolType;
        std::string m_name;
};

class SymbolWithValue: public Symbol {
    public: /* Methods: */
        explicit inline SymbolWithValue(Type symbolType,
                                        const SecreC::Type &valueType)
            : Symbol(symbolType), m_type(valueType.clone()) {}
        virtual ~SymbolWithValue() { delete m_type; }

        inline const SecreC::Type &secrecType() const { return *m_type; }

    private: /* Fields: */
        SecreC::Type *m_type;
};

class SymbolSymbol: public SymbolWithValue {
    public: /* Types: */
        enum ScopeType { GLOBAL, LOCAL };

    public: /* Methods: */
        SymbolSymbol(const SecreC::Type &valueType,
                     const TreeNodeStmtDecl *decl)
            : SymbolWithValue(Symbol::SYMBOL, valueType), m_decl(decl),
              m_scopeType(LOCAL) {}

        inline const TreeNodeStmtDecl *decl() const { return m_decl; }
        inline ScopeType scopeType() const { return m_scopeType; }
        inline void setScopeType(ScopeType type) { m_scopeType = type; }

        virtual std::string toString() const;

    private: /* Fields: */
        const TreeNodeStmtDecl *m_decl;
        ScopeType               m_scopeType;
};

class SymbolTemporary: public SymbolWithValue {
    public: /* Types: */
        enum ScopeType { GLOBAL, LOCAL };

    public: /* Methods: */
        SymbolTemporary(const SecreC::Type &valueType)
            : SymbolWithValue(Symbol::TEMPORARY, valueType), m_scopeType(LOCAL) {}

        inline void setScopeType(ScopeType type) { m_scopeType = type; }

        virtual std::string toString() const;

    private: /* Fields: */
        ScopeType m_scopeType;
};

class SymbolProcedure: public SymbolWithValue {
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

class SymbolConstantBool: public SymbolWithValue {
    public: /* Methods: */
        explicit SymbolConstantBool(bool value)
            : SymbolWithValue(Symbol::CONSTANT, SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_BOOL)), m_value(value) {}

        inline bool value() const { return m_value; }

        virtual std::string toString() const;

    private: /* Fields: */
        const bool m_value;
};

class SymbolConstantInt: public SymbolWithValue {
    public: /* Methods: */
        explicit SymbolConstantInt(int value)
            : SymbolWithValue(Symbol::CONSTANT, SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT)), m_value(value) {}

        inline int value() const { return m_value; }

        virtual std::string toString() const;

    private: /* Fields: */
        const int m_value;
};

class SymbolConstantUInt: public SymbolWithValue {
    public: /* Methods: */
        explicit SymbolConstantUInt(unsigned value)
            : SymbolWithValue(Symbol::CONSTANT, SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_UINT)), m_value(value) {}

        inline unsigned value() const { return m_value; }

        virtual std::string toString() const;

    private: /* Fields: */
        const unsigned m_value;
};

class SymbolConstantString: public SymbolWithValue {
    public: /* Methods: */
        explicit SymbolConstantString(const std::string &value)
            : SymbolWithValue(Symbol::CONSTANT, SecreC::TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_STRING)), m_value(value) {}

        inline const std::string &value() const { return m_value; }

        virtual std::string toString() const;

    private: /* Fields: */
        const std::string m_value;
};

class SymbolTable {
    private: /* Types: */
        typedef std::vector<Symbol*> Table;

    public: /* Methods: */
        SymbolTable(SymbolTable *parent = 0);
        ~SymbolTable();

        void appendSymbol(Symbol *symbol);
        void appendGlobalSymbol(Symbol *symbol);
        SymbolProcedure *appendProcedure(const TreeNodeProcDef &procdef);
        SymbolTemporary *appendTemporary(const Type &type);
        SymbolConstantBool *constantBool(bool value);
        SymbolConstantInt *constantInt(int value);
        SymbolConstantUInt *constantUInt(unsigned value);
        SymbolConstantString *constantString(const std::string &value);
        Symbol *find(const std::string &name) const;
        Symbol *findGlobal(const std::string &name) const;
        SymbolProcedure *findGlobalProcedure(const std::string &name,
                                             const SecTypeProcedureVoid &st,
                                             const DataTypeProcedureVoid &dt);
        SymbolTable *newScope();

        std::string toString(unsigned level = 0, unsigned indent = 4,
                             bool newScope = true) const;

    private: /* Fields: */
        Table        m_table;
        SymbolTable *m_parent;
        SymbolTable *m_global;
        SymbolTable *m_scope;
        SymbolTable *m_cont;
        SymbolTable *m_last;
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
