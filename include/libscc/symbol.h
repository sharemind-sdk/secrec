#ifndef SECREC_SYMBOL_H
#define SECREC_SYMBOL_H

#include "types.h"


namespace SecreC {

class Imop;
class TreeNodeProcDef;

/*******************************************************************************
  Symbol
*******************************************************************************/

class Symbol {
    public: /* Types: */
        enum Type {
            PROCEDURE,
            CONSTANT,
            LABEL,
            SYMBOL,
            PKIND,
            PDOMAIN
        };
    public: /* Methods: */

        inline Symbol(Type symbolType, const SecreC::TypeNonVoid &valueType)
            : m_symbolType(symbolType)
            , m_type(valueType.clone())
        { }

        explicit inline Symbol (Type symbolType)
            : m_symbolType(symbolType)
            , m_type(TypeVoid ().clone ())
        { }

        virtual inline ~Symbol() {
            delete m_type;
        }

        inline bool isConstant () const { return m_symbolType == CONSTANT; }
        inline Type symbolType() const { return m_symbolType; }
        inline const std::string &name() const { return m_name; }
        inline void setName(const std::string &name) { m_name = name; }
        inline const SecreC::Type &secrecType() const { return *m_type; }

        virtual std::string toString() const = 0;

    private: /* Fields: */
        const Type  m_symbolType;
        std::string m_name;
        SecreC::Type *m_type;
};

/*******************************************************************************
  SymbolKind
*******************************************************************************/

class SymbolKind : public Symbol {
public: /* Methods: */

    SymbolKind ()
        : Symbol (Symbol::PKIND)
    { }

    virtual std::string toString () const;
};

/*******************************************************************************
  SymbolDomain
*******************************************************************************/

class SymbolDomain : public Symbol {
public: /* Methods: */

    SymbolDomain (const SymbolKind* kind)
        : Symbol (Symbol::PDOMAIN)
        , m_kind (kind)
    { }

    const SymbolKind* kind () const { return m_kind; }
    virtual std::string toString () const;

private:

    const SymbolKind* const m_kind;
};

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

class SymbolSymbol: public Symbol {
    public: /* Types: */

        enum ScopeType { GLOBAL, LOCAL };

    public: /* Methods: */

        explicit SymbolSymbol (const SecreC::TypeNonVoid &valueType)
            : Symbol (Symbol::SYMBOL, valueType)
            , m_scopeType (LOCAL)
            , m_dims (valueType.secrecDimType())
            , m_size (0)
            , m_isTemporary (false)
        { }

        explicit SymbolSymbol (const SecreC::TypeNonVoid &valueType, bool)
            : Symbol (Symbol::SYMBOL, valueType)
            , m_scopeType (LOCAL)
            , m_dims (valueType.secrecDimType ())
            , m_size (0)
            , m_isTemporary (true)
        { }

        inline ScopeType scopeType() const { return m_scopeType; }
        inline void setScopeType(ScopeType type) { m_scopeType = type; }
        inline bool isTemporary () const { return m_isTemporary; }

        inline Symbol* getDim (unsigned i) { return m_dims[i]; }
        inline void setDim (unsigned i, Symbol* sym) { m_dims[i] = sym; }
        Symbol* getSizeSym () { return m_size; }
        void setSizeSym (Symbol* sym) { m_size = sym; }
        void inheritShape (Symbol* from);

        virtual std::string toString() const;

    protected:

        template <typename B, typename E>
        friend class DimIterator;

    private: /* Fields: */

        ScopeType              m_scopeType;
        std::vector<Symbol* >  m_dims;
        Symbol*                m_size;
        const bool             m_isTemporary;
};

template <typename BaseTy, typename ElemTy >
class DimIterator : std::iterator<std::bidirectional_iterator_tag, ElemTy > {
public: /* Types: */

    typedef std::iterator<std::bidirectional_iterator_tag, ElemTy > Super;
    typedef DimIterator<BaseTy, ElemTy > Self;
    typedef typename Super::pointer pointer;

public: /* Methods: */

    explicit inline DimIterator (BaseTy symbol)
        : m_symbol (symbol)
        , m_index (0)
    { }

    explicit inline DimIterator (BaseTy symbol, bool)
        : m_symbol (symbol)
        , m_index (0)
    {
        if (symbol) {
            m_index = symbol->m_dims.size ();
        }
    }

    inline const Self& operator = (const Self& i) {
        assert (m_symbol == i.m_symbol);
        m_index = i.m_index;
        return *this;
    }

    inline bool operator == (const Self& i) const { return m_index == i.m_index; }
    inline bool operator != (const Self& i) const { return m_index != i.m_index; }
    inline pointer operator*() const { return m_symbol->m_dims[m_index]; }
    inline pointer operator->() const { return operator * (); }
    inline Self& operator ++ () { ++ m_index; return *this; }
    inline Self  operator ++ (int) { Self tmp = *this; ++ m_index; return tmp; }
    inline Self& operator -- () { -- m_index; return *this; }
    inline Self  operator -- (int) { Self tmp = *this; -- m_index; return tmp; }

private: /* Methods: */

    const BaseTy   m_symbol;
    unsigned       m_index;
};

/// \{
typedef DimIterator<SymbolSymbol*, Symbol> dim_iterator;
typedef DimIterator<const SymbolSymbol*, Symbol> dim_const_iterator;
inline dim_iterator dim_begin (SymbolSymbol* symbol) { return dim_iterator (symbol); }
inline dim_iterator dim_end (SymbolSymbol* symbol) { return dim_iterator (symbol, true); }
inline dim_const_iterator dim_begin (const SymbolSymbol* symbol) { return dim_const_iterator (symbol); }
inline dim_const_iterator dim_end (const SymbolSymbol* symbol) { return dim_const_iterator (symbol, true); }
inline dim_iterator dim_begin (Symbol* symbol) { return dim_begin (dynamic_cast<SymbolSymbol*>(symbol)); }
inline dim_iterator dim_end (Symbol* symbol) { return dim_end (dynamic_cast<SymbolSymbol*>(symbol)); }
inline dim_const_iterator dim_begin (const Symbol* symbol) { return dim_begin (dynamic_cast<const SymbolSymbol*>(symbol)); }
inline dim_const_iterator dim_end (const Symbol* symbol) { return dim_end (dynamic_cast<const SymbolSymbol*>(symbol)); }
/// \}

/*******************************************************************************
  SymbolProcedure
*******************************************************************************/

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

/*******************************************************************************
  SymbolLabel
*******************************************************************************/

class SymbolLabel: public Symbol {
    public:
        SymbolLabel (Imop* target);
        inline const Imop* target () const { return m_target; }
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
