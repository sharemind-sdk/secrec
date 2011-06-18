#ifndef SECREC_SYMBOL_H
#define SECREC_SYMBOL_H

#include "types.h"


namespace SecreC {

class Imop;
class TreeNodeProcDef;

class Symbol {
    public: /* Types: */
        enum Type { PROCEDURE, CONSTANT, LABEL, SYMBOL, TEMPORARY };
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
        explicit inline Symbol (Type symbolType)
            : m_symbolType(symbolType),
              m_type(TypeVoid ().clone ()),
              m_dims(),
              m_size((Symbol*) 0) {}
        virtual inline ~Symbol() { delete m_type; }

        inline bool isConstant () const { return m_symbolType == CONSTANT; }
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
        inline Imop *target() const { return m_target; }
        inline void setTarget(Imop *target) { m_target = target; }

        virtual std::string toString() const;

    private: /* Fields: */
        const TreeNodeProcDef *m_decl;
        Imop                  *m_target;
};

class SymbolLabel: public Symbol {
    public:
        SymbolLabel (Imop* target);
        inline Imop* target () const { return m_target; }
        virtual std::string toString () const;
    private:
        Imop* m_target;
};

}

inline std::ostream &operator<<(std::ostream &out, const SecreC::Symbol &s) {
    out << s.toString();
    return out;
}

#endif
