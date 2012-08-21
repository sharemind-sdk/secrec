#ifndef SECREC_SYMBOL_H
#define SECREC_SYMBOL_H

#include "parser.h"
#include <iterator>
#include <vector>

namespace SecreC {

class Block;
class Imop;
class SecurityType;
class TreeNodeProcDef;
class TreeNodeTemplate;
class TypeNonVoid;

/*******************************************************************************
  Symbol
*******************************************************************************/

class Symbol {
public: /* Types: */
    enum Type {
        PROCEDURE,
        TEMPLATE,
        CONSTANT,
        LABEL,
        SYMBOL,
        PKIND,
        PDOMAIN
    };

public: /* Methods: */

    inline Symbol(Type symbolType, TypeNonVoid* valueType)
        : m_symbolType (symbolType)
        , m_type (valueType)
        , m_previous (0)
    { }

    explicit inline Symbol (Type symbolType)
        : m_symbolType (symbolType)
        , m_type (0)
        , m_previous (0)
    { }

    virtual inline ~Symbol() { }

    inline bool isConstant () const { return m_symbolType == CONSTANT; }
    inline Type symbolType() const { return m_symbolType; }
    inline const std::string &name() const { return m_name; }
    inline void setName(const std::string &name) { m_name = name; }
    inline TypeNonVoid* secrecType() const { return m_type; }
    Symbol* previos () const { return m_previous; }
    void setPrevious (Symbol* prev) { m_previous = prev; }

    bool isGlobal () const;
    bool isArray () const;

    virtual std::ostream & print(std::ostream & os) const = 0;

private: /* Fields: */
    Type          const  m_symbolType;  ///< Type of the symbol.
    TypeNonVoid*  const  m_type;        ///< Type of the symbol or NULL.
    std::string          m_name;        ///< Name of the symbol.
    Symbol*              m_previous;    ///< Previous symbol with same name.
};

/*******************************************************************************
  SymbolConstant
*******************************************************************************/

class SymbolConstant : public Symbol {
public: /* Methods: */
    inline SymbolConstant(TypeNonVoid* valueType)
        : Symbol(CONSTANT,valueType)
    { }

    virtual inline ~SymbolConstant() { }

    virtual std::ostream & print(std::ostream & os) const = 0;
};

/*******************************************************************************
  SymbolKind
*******************************************************************************/

class SymbolKind : public Symbol {
public: /* Methods: */

    SymbolKind ()
        : Symbol (Symbol::PKIND)
    { }

    virtual std::ostream & print(std::ostream & os) const;
};

/*******************************************************************************
  SymbolDomain
*******************************************************************************/

class SymbolDomain : public Symbol {
public: /* Methods: */

    SymbolDomain (SecurityType* secType)
        : Symbol (Symbol::PDOMAIN)
        , m_secType (secType)
    { }

    inline SecurityType* securityType () const { return m_secType; }
    virtual std::ostream & print(std::ostream & os) const;

private:

    SecurityType* m_secType;
};

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

class SymbolSymbol: public Symbol {
public: /* Types: */

    enum ScopeType { GLOBAL, LOCAL };

public: /* Methods: */

    explicit SymbolSymbol (TypeNonVoid* valueType);

    explicit SymbolSymbol (TypeNonVoid* valueType, bool);

    inline ScopeType scopeType() const { return m_scopeType; }
    inline void setScopeType(ScopeType type) { m_scopeType = type; }
    inline bool isTemporary () const { return m_isTemporary; }

    inline Symbol* getDim (SecrecDimType i) { return m_dims[i]; }
    inline void setDim (SecrecDimType i, Symbol* sym) { m_dims[i] = sym; }
    Symbol* getSizeSym () { return m_size; }
    void setSizeSym (Symbol* sym) { m_size = sym; }
    void inheritShape (Symbol* from);

    virtual std::ostream & print(std::ostream & os) const;

protected:

    template <typename B, typename E>
    friend class DimIterator;

private: /* Fields: */

    ScopeType              m_scopeType;
    std::vector<Symbol* >  m_dims;
    Symbol*                m_size;
    const bool             m_isTemporary;
};

typedef SymbolSymbol SymbolTemporary;

template <typename BaseTy, typename ElemTy >
class DimIterator : public std::iterator<std::bidirectional_iterator_tag, ElemTy* > {
public: /* Types: */

    typedef DimIterator<BaseTy, ElemTy > Self;

public: /* Methods: */

    explicit inline DimIterator (BaseTy symbol)
        : m_symbol (symbol)
        , m_index (0)
    { }

    explicit inline DimIterator (BaseTy symbol, bool)
        : m_symbol (symbol)
        , m_index (symbol ? symbol->m_dims.size () : 0)
    { }

    inline const Self& operator = (const Self& i) {
        assert (m_symbol == i.m_symbol);
        m_index = i.m_index;
        return *this;
    }

    inline bool operator == (const Self& i) const { return m_index == i.m_index; }
    inline bool operator != (const Self& i) const { return m_index != i.m_index; }
    inline ElemTy* operator*() const { return m_symbol->m_dims[m_index]; }
    inline ElemTy* operator->() const { return operator * (); }
    inline ElemTy*& operator*() { return m_symbol->m_dims[m_index]; }
    inline ElemTy*& operator->() { return operator * (); }
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
inline std::pair<dim_iterator, dim_iterator> dim_range (Symbol* symbol) { return std::make_pair (dim_begin (symbol), dim_end (symbol)); }
inline std::pair<dim_const_iterator, dim_const_iterator> dim_range (const Symbol* symbol) { return std::make_pair (dim_begin (symbol), dim_end (symbol)); }
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

    virtual std::ostream & print(std::ostream & os) const;

private: /* Fields: */
    const TreeNodeProcDef*  const  m_decl;
    Imop*                          m_target;
};


/*******************************************************************************
  SymbolTemplate
*******************************************************************************/

class SymbolTemplate: public Symbol {
public: /* Methods: */
    SymbolTemplate(const TreeNodeTemplate *templ);

    inline const TreeNodeTemplate *decl() const { return m_templ; }

    virtual std::ostream & print(std::ostream & os) const;

private: /* Fields: */
    const TreeNodeTemplate* m_templ;
};


/*******************************************************************************
  SymbolLabel
*******************************************************************************/

class SymbolLabel: public Symbol {
public:
    explicit SymbolLabel (Imop* target);
    explicit SymbolLabel (Block* block);

    const Imop* target () const;
    inline Block* block () const { return m_block; }

    void setBlock (Block* block) {
        m_target = 0;
        m_block = block;
    }

    virtual std::ostream & print(std::ostream & os) const;

private:
    Imop*   m_target;
    Block*  m_block;
};

inline std::ostream &operator<<(std::ostream &out, const Symbol &s) {
    return s.print(out);
}

} // namespace SecreC

#endif
