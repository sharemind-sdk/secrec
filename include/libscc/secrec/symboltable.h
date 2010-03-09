#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "secrec/types.h"

namespace SecreC {

class Symbol {
    public: /* Types: */
        enum Type { CONSTANT, GLOBAL_SYMBOL, LOCAL_SYMBOL, TEMPORARY, SSA };

    public: /* Methods: */
        explicit inline Symbol(Type symbolType, const SecreC::Type &secrecType)
            : m_symbolType(symbolType), m_type(secrecType.clone()) {}
        virtual inline ~Symbol() { delete m_type; }

        inline Type symbolType() const { return m_symbolType; }
        inline SecreC::Type &secrecType() const { return *m_type; }

    private: /* Fields: */
        Type m_symbolType;
        SecreC::Type *m_type;
};

class SymbolConstantBool: public Symbol {
    public: /* Methods: */
        explicit SymbolConstantBool(const SecreC::BasicType &type, bool value)
            : Symbol(Symbol::CONSTANT, type), m_value(value) {}

        inline bool value() const { return m_value; }

    private: /* Fields: */
        const bool m_value;
};

class SymbolConstantInt: public Symbol {
    public: /* Methods: */
        explicit SymbolConstantInt(const SecreC::BasicType &type, int value)
            : Symbol(Symbol::CONSTANT, type), m_value(value) {}

        inline int value() const { return m_value; }

    private: /* Fields: */
        const int m_value;
};

class SymbolConstantUInt: public Symbol {
    public: /* Methods: */
        explicit SymbolConstantUInt(const SecreC::BasicType &type,
                                    unsigned value)
            : Symbol(Symbol::CONSTANT, type), m_value(value) {}

        inline unsigned value() const { return m_value; }

    private: /* Fields: */
        const unsigned m_value;
};

class SymbolConstantString: public Symbol {
    public: /* Methods: */
        explicit SymbolConstantString(const SecreC::BasicType &type,
                                      const std::string &value)
            : Symbol(Symbol::CONSTANT, type), m_value(value) {}

        inline const std::string &value() const { return m_value; }

    private: /* Fields: */
        const std::string m_value;
};

class SymbolTable {
    public:
        ~SymbolTable();

        std::vector<Symbol*> &table() { return m_table; }
        const std::vector<Symbol*> &table() const { return m_table; }

    private:
        std::vector<Symbol*> m_table;
};

} // namespace SecreC

#endif // SYMBOLTABLE_H
