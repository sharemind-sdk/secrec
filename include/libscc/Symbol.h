#ifndef SECREC_SYMBOL_H
#define SECREC_SYMBOL_H

#include "DataType.h"
#include "ParserEnums.h"
#include "StringRef.h"
#include "SymbolFwd.h"
#include "TreeNodeFwd.h"

#include <boost/range/iterator_range.hpp>
#include <cassert>
#include <iterator>
#include <vector>

namespace SecreC {

class Block;
class Imop;
class SecurityType;
class TypeNonVoid;
class TypeContext;
class Location;
class TypeProc;

/*******************************************************************************
  Symbol
*******************************************************************************/

class Symbol {
public: /* Types: */

    typedef SymbolCategory Type;

public: /* Methods: */

    inline Symbol(Type symbolType, TypeNonVoid* valueType)
        : m_symbolType (symbolType)
        , m_type (valueType)
    { }

    explicit inline Symbol (Type symbolType)
        : m_symbolType (symbolType)
        , m_type (nullptr)
    { }

    explicit inline Symbol (Type symbolType, StringRef name)
        : m_symbolType (symbolType)
        , m_type (nullptr)
        , m_name (name.str ())
    { }

    virtual inline ~Symbol() { }

    inline bool isConstant () const { return m_symbolType == SYM_CONSTANT; }
    inline Type symbolType() const { return m_symbolType; }
    inline const std::string &name() const { return m_name; }
    inline void setName(StringRef name) { m_name = name.str(); }
    inline TypeNonVoid* secrecType() const { return m_type; }

    bool isGlobal () const;
    bool isArray () const;

    virtual const Location * location() const { return nullptr; }

protected:
    friend std::ostream& operator << (std::ostream& os, const Symbol& s);
    virtual void print(std::ostream & os) const = 0;

private: /* Fields: */
    Type          const  m_symbolType;  ///< Type of the symbol.
    TypeNonVoid*  const  m_type;        ///< Type of the symbol or NULL.
    std::string          m_name;        ///< Name of the symbol.
};

/*******************************************************************************
  SymbolConstant
*******************************************************************************/

class SymbolConstant : public Symbol {
public: /* Methods: */
    explicit SymbolConstant(TypeNonVoid* valueType)
        : Symbol(SYM_CONSTANT, valueType)
    { }
};

/*******************************************************************************
  SymbolTypeVariable
*******************************************************************************/

class SymbolTypeVariable: public Symbol {
public: /* Methods: */

    SymbolTypeVariable(SymbolCategory symCategory, StringRef name)
        : Symbol (symCategory, name)
    { }

    virtual void setTypeContext (TypeContext& cxt) const = 0;
};

/*******************************************************************************
  SymbolDimensionality
*******************************************************************************/

class SymbolDimensionality : public SymbolTypeVariable {
public: /* Methods: */
    SymbolDimensionality(StringRef name, SecrecDimType dimType)
        : SymbolTypeVariable (SYM_DIM, name)
        , m_dimType (dimType)
    { }

    inline SecrecDimType dimType () const { return m_dimType; }

protected:
    void print(std::ostream & os) const override;
    void setTypeContext (TypeContext& cxt) const override;

private: /* Fields: */
    SecrecDimType const m_dimType;
};

/*******************************************************************************
  SymbolType
*******************************************************************************/

class SymbolDataType : public SymbolTypeVariable {
public: /* Methods: */

    SymbolDataType (StringRef name, DataType* dataType)
        : SymbolTypeVariable (SYM_TYPE, name)
        , m_dataType (dataType)
    { }

    inline DataType* dataType () const { return m_dataType; }

protected:
    void print(std::ostream & os) const override;
    void setTypeContext (TypeContext& cxt) const override;

private: /* Fields: */
    DataType* const m_dataType;
};

/*******************************************************************************
  SymbolDomain
*******************************************************************************/

class SymbolDomain : public SymbolTypeVariable {
public: /* Methods: */

    SymbolDomain(StringRef name, SecurityType * secType)
        : SymbolTypeVariable (SYM_DOMAIN, name)
        , m_secType (secType)
    { }

    inline SecurityType* securityType () const { return m_secType; }

protected:
    void print(std::ostream & os) const override;
    void setTypeContext (TypeContext& cxt) const override;

private: /* Fields: */
    SecurityType* const m_secType;
};

/*******************************************************************************
  SymbolKind
*******************************************************************************/

class SymbolKind : public Symbol {
public: /* Methods: */

    SymbolKind(StringRef name)
        : Symbol (SYM_KIND, name)
    { }

protected:
    void print(std::ostream & os) const override;
};

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

class SymbolSymbol: public Symbol {
public: /* Types: */

    enum ScopeType { GLOBAL, LOCAL };

public: /* Methods: */

    explicit SymbolSymbol(StringRef name, TypeNonVoid * valueType);

    explicit SymbolSymbol(StringRef name, TypeNonVoid * valueType, bool);

    inline ScopeType scopeType() const { return m_scopeType; }
    inline void setScopeType(ScopeType type) { m_scopeType = type; }
    inline bool isTemporary () const { return m_isTemporary; }

    inline SymbolSymbol* getDim (SecrecDimType i) { return m_dims[i]; }
    inline void setDim (SecrecDimType i, SymbolSymbol* sym) {
        if (sym != nullptr) {
            sym->setParent (this);
            m_dims[i] = sym;
        }
    }
    SymbolSymbol* getSizeSym () { return m_size; }
    void setSizeSym (SymbolSymbol* sym) {
        if (sym) {
            sym->setParent (this);
            m_size = sym;
        }
    }

    void inheritShape (Symbol* from);

    const std::vector<SymbolSymbol*>& fields () const { return m_fields; }
    void appendField (SymbolSymbol* sym) {
        sym->m_parent = this;
        m_fields.push_back (sym);
    }

    void setParent (SymbolSymbol* parent) { m_parent = parent; }
    SymbolSymbol* parent () const { return m_parent; }

    virtual const Location * location() const override;

protected:
    void print(std::ostream & os) const override;

protected:

    template <typename B, typename E>
    friend class DimIterator;

private: /* Fields: */

    ScopeType                   m_scopeType;
    std::vector<SymbolSymbol*>  m_dims;
    SymbolSymbol*               m_size;
    std::vector<SymbolSymbol*>  m_fields;
    const bool                  m_isTemporary;
    SymbolSymbol*               m_parent; // TODO: this is a HUGE hack!
};

SymbolSymbol* lookupField (SymbolSymbol* val, StringRef fieldName);

/**
 * @brief flattenSymbol Flatten the given symbol for returning it.
 * Flattening is performed from leftmost field to right, depth first.
 * Dimensionalities are passed before the symbol itself.
 * @param sym The symbol to be flattened.
 * @return All relevant compontents of the input symbol.
 */
std::vector<Symbol*> flattenSymbol (Symbol* sym);

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
typedef DimIterator<SymbolSymbol*, SymbolSymbol> dim_iterator;
typedef DimIterator<const SymbolSymbol*, SymbolSymbol> dim_const_iterator;
inline dim_iterator dim_begin (SymbolSymbol* symbol) { return dim_iterator (symbol); }
inline dim_iterator dim_end (SymbolSymbol* symbol) { return dim_iterator (symbol, true); }
inline dim_const_iterator dim_begin (const SymbolSymbol* symbol) { return dim_const_iterator (symbol); }
inline dim_const_iterator dim_end (const SymbolSymbol* symbol) { return dim_const_iterator (symbol, true); }
inline dim_iterator dim_begin (Symbol* symbol) { return dim_begin (dynamic_cast<SymbolSymbol*>(symbol)); }
inline dim_iterator dim_end (Symbol* symbol) { return dim_end (dynamic_cast<SymbolSymbol*>(symbol)); }
inline dim_const_iterator dim_begin (const Symbol* symbol) { return dim_begin (dynamic_cast<const SymbolSymbol*>(symbol)); }
inline dim_const_iterator dim_end (const Symbol* symbol) { return dim_end (dynamic_cast<const SymbolSymbol*>(symbol)); }
inline boost::iterator_range<dim_iterator> dim_range (Symbol* symbol) { return {dim_begin (symbol), dim_end (symbol)}; }
inline boost::iterator_range<dim_const_iterator> dim_range (const Symbol* symbol) { return {dim_begin (symbol), dim_end (symbol)}; }
/// \}

/*******************************************************************************
  SymbolProcedure
*******************************************************************************/

// TODO: initialize global variables in procedures
class SymbolProcedure: public Symbol {
public: /* Methods: */
    SymbolProcedure(StringRef name, TypeProc* type);

    inline Imop *target() const { return m_target; }
    inline void setTarget(Imop *target) { m_target = target; }

    virtual StringRef procedureName () const { return name (); }
    virtual const TreeNodeProcDef * decl () const { assert (false); return nullptr; }
    virtual const Location * location () const override { return nullptr; }
    virtual SymbolProcedure* shortOf () const { return nullptr; }

protected:
    void print(std::ostream & os) const override;

private: /* Fields: */
    Imop* m_target;
};

/*******************************************************************************
  SymbolUserProcedure
*******************************************************************************/

// User defined procedure:
class SymbolUserProcedure : public SymbolProcedure {
public: /* Methods: */

    SymbolUserProcedure (StringRef name,
                         const TreeNodeProcDef * decl,
                         SymbolProcedure * shortOf = nullptr);

    StringRef procedureName () const override;
    virtual const TreeNodeProcDef * decl () const override { return m_decl; }
    virtual const Location * location() const override;
    virtual SymbolProcedure* shortOf () const override { return m_shortOf; }

protected:
    void print(std::ostream & os) const override;

private: /* Fields: */
    const TreeNodeProcDef * const  m_decl;
    SymbolProcedure *              m_shortOf;
};

/*******************************************************************************
  SymbolStruct
*******************************************************************************/

class SymbolStruct: public Symbol {
public: /* Methods: */
    explicit SymbolStruct (StringRef name, TreeNodeStructDecl* structDecl);
    TreeNodeStructDecl* decl () const { return m_structDecl; }
    const Location* location () const override;
protected:
    void print(std::ostream & os) const override;
private: /* Fields: */
    TreeNodeStructDecl* const m_structDecl;
};

/*******************************************************************************
  SymbolTemplate
*******************************************************************************/

class SymbolTemplate: public Symbol {
public: /* Types: */

    struct Weight {
        unsigned m_typeVariableCount;
        unsigned m_qualifiedTypeVariableCount;
        unsigned m_quantifiedParamCount;

        Weight ()
            : m_typeVariableCount (~ unsigned (0))
            , m_qualifiedTypeVariableCount (~ unsigned (0))
            , m_quantifiedParamCount (~ unsigned (0))
        { }

        Weight (unsigned a, unsigned b, unsigned c)
            : m_typeVariableCount (a)
            , m_qualifiedTypeVariableCount (b)
            , m_quantifiedParamCount (c)
        { }

        inline bool operator == (const Weight& other) const {
            return m_typeVariableCount == other.m_typeVariableCount &&
                   m_qualifiedTypeVariableCount == other.m_qualifiedTypeVariableCount &&
                   m_quantifiedParamCount == other.m_quantifiedParamCount;
        }

        inline bool operator != (const Weight& other) const { return !(*this == other); }

        inline bool operator < (const Weight& other) const {
            const unsigned left[3] = { m_typeVariableCount, m_qualifiedTypeVariableCount, m_quantifiedParamCount };
            const unsigned right[3] = { other.m_typeVariableCount, other.m_qualifiedTypeVariableCount, other.m_quantifiedParamCount };
            for (unsigned i = 0; i < 3; ++ i) {
                if (left[i] < right[i]) return true;
                if (left[i] > right[i]) return false;
            }

            return false;
        }

        inline bool operator > (const Weight& other) const {
            return (other < *this);
        }
    };

public: /* Methods: */
    SymbolTemplate (TreeNodeTemplate *templ,
                    bool expectsSecType,
                    bool expectsDataType,
                    bool expectsDimType);

    inline TreeNodeTemplate *decl() const { return m_templ; }

    virtual const Location * location() const override;
    inline bool expectsSecType () const { return m_expectsSecType; }
    inline bool expectsDimType () const { return m_expectsDimType; }
    inline bool expectsDataType () const { return m_expectsDataType; }
    const Weight& weight () const  { return m_weight; }

protected:
    void print(std::ostream & os) const override;

private: /* Fields: */
    TreeNodeTemplate*  const  m_templ;
    bool               const  m_expectsSecType; ///< Expects context to supply security type
    bool               const  m_expectsDataType; ///< Expects context to supply data type
    bool               const  m_expectsDimType; ///< Expects context to supply dimensionality type
    Weight             const  m_weight;
};


/*******************************************************************************
  SymbolLabel
*******************************************************************************/

class SymbolLabel: public Symbol {
public: /* Methods: */
    explicit SymbolLabel (Imop* target);
    explicit SymbolLabel (Block* block);

    const Imop* target () const;
    inline Block* block () const { return m_block; }

    void setBlock (Block* block) {
        m_target = nullptr;
        m_block = block;
    }

protected:
    void print(std::ostream & os) const override;

private: /* Fields: */
    Imop*   m_target;
    Block*  m_block;
};

inline std::ostream &operator<<(std::ostream &out, const Symbol &s) {
    s.print(out);
    return out;
}

} // namespace SecreC

#endif