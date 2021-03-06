/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#ifndef SECREC_SYMBOL_H
#define SECREC_SYMBOL_H

#include "ParserEnums.h"
#include "StringRef.h"
#include "SymbolFwd.h"
#include "TreeNodeFwd.h"

#include <boost/optional.hpp>
#include <cassert>
#include <map>
#include <set>
#include <vector>

namespace SecreC {

class Block;
class DataType;
class DataTypeBuiltinPrimitive;
class DataTypeUserPrimitive;
class Imop;
class Location;
class SecurityType;
class TypeContext;
class TypeNonVoid;
class TypeProc;

/*******************************************************************************
  Symbol
*******************************************************************************/

class Symbol {
public: /* Types: */

    using Type = SymbolCategory;

public: /* Methods: */

    inline Symbol(Type symbolType, const TypeNonVoid* valueType)
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
    inline const TypeNonVoid* secrecType() const { return m_type; }

    bool isGlobal() const;
    bool isArray() const;
    bool isString() const;
    bool isSigned() const;

    virtual const Location * location() const { return nullptr; }

protected:
    friend std::ostream& operator << (std::ostream& os, const Symbol& s);
    virtual void print(std::ostream & os) const = 0;

private: /* Fields: */
    Type const m_symbolType; ///< Type of the symbol.
    const TypeNonVoid* const m_type; ///< Type of the symbol or nullptr.
    std::string m_name; ///< Name of the symbol.
};

/*******************************************************************************
  SymbolConstant
*******************************************************************************/

class SymbolConstant : public Symbol {
public: /* Methods: */
    explicit SymbolConstant(const TypeNonVoid* valueType)
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

    SymbolDataType (StringRef name, const DataType* dataType)
        : SymbolTypeVariable (SYM_TYPE, name)
        , m_dataType (dataType)
    { }

    inline const DataType* dataType () const { return m_dataType; }

protected:
    void print(std::ostream & os) const override;
    void setTypeContext (TypeContext& cxt) const override;

private: /* Fields: */
    const DataType* const m_dataType;
};

/*******************************************************************************
  SymbolDomain
*******************************************************************************/

class SymbolDomain : public SymbolTypeVariable {
public: /* Methods: */

    SymbolDomain(StringRef name,
                 const SecurityType * secType,
                 const Location* loc = nullptr)
        : SymbolTypeVariable (SYM_DOMAIN, name)
        , m_secType (secType)
        , m_location (loc)
    { }

    const SecurityType* securityType () const { return m_secType; }
    const Location* location() const override final { return m_location; }

protected:
    void print(std::ostream & os) const override;
    void setTypeContext (TypeContext& cxt) const override;

private: /* Fields: */
    const SecurityType* const m_secType;
    const Location* m_location;
};

/*******************************************************************************
  SymbolKind
*******************************************************************************/

class SymbolKind : public Symbol {

public: /* Types: */

    struct Parameters {
        const DataType* type;
        const boost::optional<const DataTypeBuiltinPrimitive*> publicType;
        const boost::optional<uint64_t> size;

        Parameters(
                DataType const * type_,
                boost::optional<DataTypeBuiltinPrimitive const *> publicType_,
                boost::optional<uint64_t> size_)
            : type(type_)
            , publicType(publicType_)
            , size(size_)
            { }
    };

private: /* Types: */
    using TypeMap = std::map<StringRef, const Parameters*>;

public: /* Methods: */

    SymbolKind (StringRef name)
        : Symbol (SYM_KIND, name)
    { }

    ~SymbolKind() noexcept override {
        for (auto it : m_types)
            delete it.second;
    }

    const Parameters* findType (StringRef name) const;

    void addType (StringRef name,
                  const DataType* type,
                  boost::optional<const DataTypeBuiltinPrimitive*> publicType,
                  boost::optional<uint64_t> size);

protected:
    void print (std::ostream & os) const override;

private: /* Fields: */
    TypeMap m_types;
};

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

class SymbolSymbol: public Symbol {
public: /* Types: */

    enum ScopeType { GLOBAL, LOCAL };

public: /* Methods: */

    explicit SymbolSymbol(StringRef name, const TypeNonVoid * valueType);

    explicit SymbolSymbol(StringRef name, const TypeNonVoid * valueType, bool);

    inline ScopeType scopeType() const { return m_scopeType; }
    inline void setScopeType(ScopeType type) { m_scopeType = type; }
    inline bool isTemporary () const { return m_isTemporary; }

    std::vector<SymbolSymbol*> const & dims() const noexcept
    { return m_dims; }

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

using SymbolTemporary = SymbolSymbol;


/*******************************************************************************
  SymbolProcedure
*******************************************************************************/

// TODO: initialize global variables in procedures
class SymbolProcedure: public Symbol {
public: /* Methods: */
    SymbolProcedure(StringRef name, const TypeProc* type);

    inline Imop *target() const { return m_target; }
    inline void setTarget(Imop *target) { m_target = target; }

    virtual StringRef procedureName () const { return name (); }
    virtual const TreeNodeProcDef * decl () const { assert (false); return nullptr; }
    virtual const Location * location () const override { return nullptr; }

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
                         const TreeNodeProcDef * decl);

    StringRef procedureName () const override;
    virtual const TreeNodeProcDef * decl () const override { return m_decl; }
    virtual const Location * location() const override;

protected:
    void print(std::ostream & os) const override;

private: /* Fields: */
    const TreeNodeProcDef * const  m_decl;
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
public: /* Methods: */
    inline TreeNodeTemplate* decl() const { return m_templ; }
    virtual const Location* location() const override;
    const std::set<StringRef>& dataTypeQuantifiers () const { return m_dataTypeQuantifiers; }
    const std::set<StringRef>& domainQuantifiers () const { return m_domainQuantifiers; }

protected: /* Methods: */
    SymbolTemplate(SymbolCategory cat, TreeNodeTemplate* templ);

    void print(std::ostream & os) const override;

protected: /* Fields: */
    TreeNodeTemplate* const m_templ;
    std::set<StringRef> m_dataTypeQuantifiers;
    std::set<StringRef> m_domainQuantifiers;
};

/*******************************************************************************
  SymbolProcedureTemplate
*******************************************************************************/

class SymbolProcedureTemplate: public SymbolTemplate {
public: /* Types: */

    struct Weight {
        std::size_t m_typeVariableCount;
        std::size_t m_qualifiedTypeVariableCount;
        std::size_t m_quantifiedParamCount;

        Weight ()
            : m_typeVariableCount (~ std::size_t (0))
            , m_qualifiedTypeVariableCount (~ std::size_t (0))
            , m_quantifiedParamCount (~ std::size_t (0))
        { }

        Weight (std::size_t a, std::size_t b, std::size_t c)
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
            const std::size_t left[3] = { m_typeVariableCount, m_qualifiedTypeVariableCount, m_quantifiedParamCount };
            const std::size_t right[3] = { other.m_typeVariableCount, other.m_qualifiedTypeVariableCount, other.m_quantifiedParamCount };
            for (std::size_t i = 0; i < 3; ++ i) {
                if (left[i] < right[i]) return true;
                if (left[i] > right[i]) return false;
            }

            return false;
        }

        inline bool operator > (const Weight& other) const {
            return (other < *this);
        }

        inline std::size_t typeVariableCount () const {
            return m_typeVariableCount;
        }
    };

public: /* Methods: */
    SymbolProcedureTemplate (TreeNodeTemplate *templ,
                             bool expectsSecType,
                             bool expectsDataType,
                             bool expectsDimType);

    inline bool expectsSecType () const { return m_expectsSecType; }
    inline bool expectsDimType () const { return m_expectsDimType; }
    inline bool expectsDataType () const { return m_expectsDataType; }
    const Weight& weight () const  { return m_weight; }

private: /* Fields: */
    bool               const  m_expectsSecType; ///< Expects context to supply security type
    bool               const  m_expectsDataType; ///< Expects context to supply data type
    bool               const  m_expectsDimType; ///< Expects context to supply dimensionality type
    Weight             const  m_weight;
};

/*******************************************************************************
  SymbolOperatorTemplate
*******************************************************************************/

// Note: also used for casts (what would be a common name for casts and operators?)
class SymbolOperatorTemplate: public SymbolTemplate {

public: /* Methods: */
    SymbolOperatorTemplate (TreeNodeTemplate *templ, bool expectsDataType);

    inline std::size_t quantifiedParamCount () const {
        return m_quantifiedParamCount;
    }

    inline std::size_t domainWeight () const {
        return m_domainWeight;
    }

    inline bool expectsDataType () const {
        return m_expectsDataType;
    }

private: /* Fields: */
    bool m_expectsDataType;
    std::size_t m_quantifiedParamCount;
    std::size_t m_domainWeight;
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
