#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "types.h"


namespace SecreC {

class Imop;
class TreeNodeDecl;

class Symbol {
    public: /* Types: */
        enum Type { COMMENT, FUNCTION, CONSTANT, SYMBOL, TEMPORARY, LABEL };

    public: /* Methods: */
        explicit inline Symbol(Type symbolType)
            : m_symbolType(symbolType) {}
        virtual inline ~Symbol() {}

        inline Type symbolType() const { return m_symbolType; }
        inline const std::string &name() const { return m_name; }
        inline void setName(const std::string &name) { m_name = name; }

    private: /* Fields: */
        const Type  m_symbolType;
        std::string m_name;
};

class SymbolComment: public Symbol {
    public: /* Methods: */
        explicit inline SymbolComment(const std::string &comment)
            : Symbol(Symbol::COMMENT), m_comment(comment) {}

        const std::string &comment() const { return m_comment; }

    private: /* Fields: */
        std::string m_comment;
};

class SymbolWithValue: public Symbol {
    public: /* Methods: */
        explicit inline SymbolWithValue(Type symbolType,
                                        const SecreC::Type &valueType)
            : Symbol(symbolType), m_type(valueType.clone()) {}
        virtual ~SymbolWithValue() { delete m_type; }

        inline SecreC::Type &secrecType() const { return *m_type; }

    private: /* Fields: */
        SecreC::Type *m_type;
};

class SymbolSymbol: public SymbolWithValue {
    public: /* Types: */
        enum ScopeType { GLOBAL, LOCAL };

    public: /* Methods: */
        SymbolSymbol(const SecreC::Type &valueType, const TreeNodeDecl *decl)
            : SymbolWithValue(Symbol::SYMBOL, valueType), m_decl(decl),
              m_scopeType(LOCAL) {}

        inline const TreeNodeDecl *decl() const { return m_decl; }
        inline ScopeType scopeType() const { return m_scopeType; }
        inline void setScopeType(ScopeType type) { m_scopeType = type; }

    private: /* Fields: */
        const TreeNodeDecl *m_decl;
        ScopeType           m_scopeType;
};

class SymbolConstantBool: public SymbolWithValue {
    public: /* Methods: */
        explicit SymbolConstantBool(bool value)
            : SymbolWithValue(Symbol::CONSTANT, SecreC::BasicType(SECTYPE_PUBLIC, VARTYPE_BOOL)), m_value(value) {}

        inline bool value() const { return m_value; }

    private: /* Fields: */
        const bool m_value;
};

class SymbolConstantInt: public SymbolWithValue {
    public: /* Methods: */
        explicit SymbolConstantInt(int value)
            : SymbolWithValue(Symbol::CONSTANT, SecreC::BasicType(SECTYPE_PUBLIC, VARTYPE_INT)), m_value(value) {}

        inline int value() const { return m_value; }

    private: /* Fields: */
        const int m_value;
};

class SymbolConstantUInt: public SymbolWithValue {
    public: /* Methods: */
        explicit SymbolConstantUInt(unsigned value)
            : SymbolWithValue(Symbol::CONSTANT, SecreC::BasicType(SECTYPE_PUBLIC, VARTYPE_UINT)), m_value(value) {}

        inline unsigned value() const { return m_value; }

    private: /* Fields: */
        const unsigned m_value;
};

class SymbolConstantString: public SymbolWithValue {
    public: /* Methods: */
        explicit SymbolConstantString(const std::string &value)
            : SymbolWithValue(Symbol::CONSTANT, SecreC::BasicType(SECTYPE_PUBLIC, VARTYPE_STRING)), m_value(value) {}

        inline const std::string &value() const { return m_value; }

    private: /* Fields: */
        const std::string m_value;
};

class SymbolLabel: public Symbol {
    public: /* Methods: */
        SymbolLabel(const Imop *target)
            : Symbol(Symbol::LABEL), m_target(target) {}
        inline const Imop *target() const { return m_target; }

    private: /* Fields: */
        const Imop *m_target;
};

class SymbolTable {
    private: /* Types: */
        typedef std::vector<Symbol*> Table;

    public: /* Methods: */
        SymbolTable(SymbolTable *parent = 0);
        ~SymbolTable();

        void appendSymbol(Symbol *symbol);
        SymbolWithValue *appendTemporary(const Type &type);
        Symbol *label(const Imop *imop);
        Symbol *comment(const std::string &comment);
        Symbol *constantBool(bool value);
        Symbol *constantInt(int value);
        Symbol *constantUInt(unsigned value);
        Symbol *constantString(const std::string &value);
        Symbol *find(const std::string &name) const;
        SymbolTable *newScope();

    private: /* Fields: */
        Table        m_table;
        SymbolTable *m_parent;
        SymbolTable *m_scope;
        SymbolTable *m_cont;
        SymbolTable *m_last;
        unsigned long m_tempCount;
        unsigned long m_constCount;
        unsigned long m_labelCount;
};

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::SymbolTable &t);

#endif // SYMBOLTABLE_H
