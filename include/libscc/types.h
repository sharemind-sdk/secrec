#ifndef SECREC_TYPES_H
#define SECREC_TYPES_H

#include <cassert>
#include <string>
#include <vector>

#include "parser.h"

namespace SecreC {

class Context;

SecrecDimType upperDimType (SecrecDimType n, SecrecDimType m);
SecrecDataType upperDataType (SecrecDataType a, SecrecDataType b);

bool latticeDimTypeLEQ (SecrecDimType n, SecrecDimType m);
bool latticeDataTypeLEQ (SecrecDataType a, SecrecDataType b);
bool latticeExplicitLEQ (SecrecDataType a, SecrecDataType b);

bool isNumericDataType (SecrecDataType dType);
bool isXorDataType (SecrecDataType dType);
bool isSignedNumericDataType (SecrecDataType dType);
bool isUnsignedNumericDataType (SecrecDataType dType);


/*******************************************************************************
  SecurityType
*******************************************************************************/

class SymbolKind;

class SecurityType {
private:
    SecurityType (const SecurityType&); // DO NOT IMPLEMENT

public: /* Methods: */

    virtual ~SecurityType () { }
    virtual std::string toString () const = 0;
    inline bool isPrivate () const { return !m_isPublic; }
    inline bool isPublic () const { return m_isPublic; }

protected:

    explicit SecurityType (bool isPublic)
        : m_isPublic (isPublic)
    { }

private: /* Fields: */

    const bool m_isPublic;
};

/*******************************************************************************
  PublicSecType
*******************************************************************************/

class PublicSecType : public SecurityType {
public: /* Methods: */

    PublicSecType ()
        : SecurityType (true)
    { }

    std::string toString () const;

    static PublicSecType* get (Context& cxt);
};

/*******************************************************************************
  PrivateSecType
*******************************************************************************/

class PrivateSecType : public SecurityType {
public: /* Methods: */

    PrivateSecType (const std::string& name,
                    SymbolKind* kind)
        : SecurityType (false)
        , m_name (name)
        , m_kind (kind)
    { }

    inline const std::string& name () const { return m_name; }
    inline SymbolKind* securityKind () const { return m_kind; }
    std::string toString () const;

    static PrivateSecType* get (Context& cxt, const std::string& name, SymbolKind* kind);

private: /* Fields: */

    std::string   const m_name;
    SymbolKind*   const m_kind;
};

inline bool latticeSecTypeLEQ (SecurityType* a, SecurityType* b) {
    if (a->isPublic ()) return true;
    if (b->isPublic ()) return false;
    return a == b;
}

inline SecurityType* upperSecType (SecurityType* a, SecurityType* b) {
    if (a->isPublic ()) return b;
    if (b->isPublic ()) return a;
    if (a == b) return a;
    return 0;
}


/*******************************************************************************
  DataType
*******************************************************************************/

class DataType {
private:
    DataType (const DataType&); // DO NOT IMPLEMENT
    void operator = (const DataType&); // DO NOT IMPLEMENT

public: /* Types: */
    enum Kind { BASIC, VAR, PROCEDURE, PROCEDUREVOID };

public: /* Methods: */

    explicit DataType(Kind kind)
        : m_kind(kind) {}

    virtual inline ~DataType() {}

    inline Kind kind() const { return m_kind; }
    inline SecurityType* secrecSecType() const;
    inline SecrecDataType secrecDataType() const;
    inline SecrecDimType secrecDimType() const;

    virtual std::string toString() const = 0;

    virtual inline bool canAssign(const DataType*) const {
        return false;
    }

    /**
     * We define less-than-equal relation on data types to check
     * if a kind of type can be converted into other. This is used to
     * convert public data to private and for interpreting scalar values
     * as arbitrary dimensional arrays.
     */
    virtual inline bool latticeLEQ(DataType const* other) const {
        return m_kind == other->m_kind;
    }

private: /* Fields: */
    Kind const m_kind;
};

/*******************************************************************************
  DataTypeBasic
*******************************************************************************/

class DataTypeBasic: public DataType {
public: /* Methods: */

    explicit DataTypeBasic(SecurityType* secType,
                           SecrecDataType dataType,
                           SecrecDimType dim = 0)
        : DataType(DataType::BASIC)
        , m_secType(secType)
        , m_dataType(dataType)
        , m_dimType(dim)
    { }

    ~DataTypeBasic () { }

    inline SecurityType* secType() const { return m_secType; }
    inline SecrecDataType dataType() const { return m_dataType; }
    inline SecrecDimType dimType() const { return m_dimType; }

    virtual std::string toString() const;

    virtual bool latticeLEQ(const DataType* _other) const {
        const DataTypeBasic* other = static_cast<const DataTypeBasic*>(_other);
        return  DataType::latticeLEQ (other)
                && latticeSecTypeLEQ(m_secType, other->m_secType)
                && latticeDataTypeLEQ(m_dataType, other->m_dataType)
                && latticeDimTypeLEQ(m_dimType, other->m_dimType);
    }

    static DataTypeBasic* get (Context& cxt,
                               SecrecDataType dataType,
                               SecrecDimType dim = 0);

    static DataTypeBasic* get (Context& cxt,
                               SecurityType* secType,
                               SecrecDataType dataType,
                               SecrecDimType dim = 0);

private: /* Fields: */
    SecurityType*   const m_secType;
    SecrecDataType  const m_dataType;
    SecrecDimType   const m_dimType;
};

/*******************************************************************************
  DataTypeVar
*******************************************************************************/

class DataTypeVar: public DataType {
public: /* Methods: */

    explicit DataTypeVar(DataType* dataType)
        : DataType(DataType::VAR), m_dataType(dataType) {}

    virtual inline ~DataTypeVar() { }

    DataType* dataType() const { return m_dataType; }

    virtual std::string toString() const;
    virtual inline bool canAssign(const DataType* other) const {
        return other->latticeLEQ(m_dataType);
    }

    virtual inline bool latticeLEQ(const DataType* other) const {
        return
            m_dataType->DataType::latticeLEQ(other) &&
            m_dataType->latticeLEQ(static_cast<const DataTypeVar*>(other)->m_dataType);
    }

    static DataTypeVar* get (Context& cxt, DataType* base);


private: /* Fields: */
    DataType* const m_dataType;
};

/*******************************************************************************
  DataTypeProcedureVoid
*******************************************************************************/

class DataTypeProcedureVoid: public DataType {
public: /* Methods: */
    explicit DataTypeProcedureVoid(const std::vector<DataType*>& params,
                                   DataType::Kind kind = PROCEDUREVOID)
        : DataType(kind)
        , m_params (params) {}
    explicit DataTypeProcedureVoid(DataType::Kind kind = PROCEDUREVOID)
        : DataType(kind) {}
    virtual ~DataTypeProcedureVoid() { }

    virtual std::string toString() const;
    std::string mangle() const;

    inline const std::vector<DataType*> &paramTypes() const { return m_params; }

    virtual bool latticeLEQ(const DataType*) const {
        assert (false && "We don't define lattice structure on function types yet!");
        return false;
    }

    static DataTypeProcedureVoid* get (Context& cxt, const std::vector<DataType*>& params);
    static DataTypeProcedureVoid* get (Context& cxt);

private: /* Fields: */
    std::vector<DataType*> const m_params;
};

/*******************************************************************************
  DataTypeProcedure
*******************************************************************************/

class DataTypeProcedure: public DataTypeProcedureVoid {
public: /* Methods: */

    explicit DataTypeProcedure(const std::vector<DataType*>& params, DataType* returnType)
        : DataTypeProcedureVoid(params, PROCEDURE)
        , m_ret(returnType)
    { }

    explicit DataTypeProcedure(DataType* returnType)
        : DataTypeProcedureVoid(PROCEDURE)
        , m_ret(returnType)
    { }

    virtual inline ~DataTypeProcedure() { }

    virtual std::string toString() const;

    inline DataType* returnType() const { return m_ret; }

    virtual inline bool canAssign(const DataType* other) const {
        return other->latticeLEQ(m_ret);
    }

    virtual bool latticeLEQ(const DataType*) const {
        assert (false && "We don't define lattice structure on function types yet!");
        return false;
    }

    static DataTypeProcedure* get (Context& cxt, const std::vector<DataType*>& params, DataType* returnType);
    static DataTypeProcedure* get (Context& cxt, DataTypeProcedureVoid* params, DataType* returnType);


private: /* Fields: */
    DataType* m_ret;
};

inline SecurityType* DataType::secrecSecType() const {
    switch (m_kind) {
    case BASIC:
        assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
        return static_cast<const DataTypeBasic*>(this)->secType();
    case VAR:
        assert(dynamic_cast<const DataTypeVar*>(this) != 0);
        return static_cast<const DataTypeVar*>(this)->dataType()->secrecSecType();
    case PROCEDURE:
        assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
        return static_cast<const DataTypeProcedure*>(this)->returnType()->secrecSecType();
    case PROCEDUREVOID:
    default:
        assert (false && "Invalid security type!");
        return 0;
    }
}

inline SecrecDataType DataType::secrecDataType() const {
    switch (m_kind) {
        case BASIC:
            assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
            return static_cast<const DataTypeBasic*>(this)->dataType();
        case VAR:
            assert(dynamic_cast<const DataTypeVar*>(this) != 0);
            return static_cast<const DataTypeVar*>(this)->dataType()->secrecDataType();
        case PROCEDURE:
            assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
            return static_cast<const DataTypeProcedure*>(this)->returnType()->secrecDataType();
        case PROCEDUREVOID:
        default:
            return DATATYPE_UNDEFINED;
    }
}

inline SecrecDimType DataType::secrecDimType() const {
    switch (m_kind) {
        case BASIC:
            assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
            return static_cast<const DataTypeBasic*>(this)->dimType();
        case VAR:
            assert(dynamic_cast<const DataTypeVar*>(this) != 0);
            return static_cast<const DataTypeVar*>(this)->dataType()->secrecDimType();
        case PROCEDURE:
            assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
            return static_cast<const DataTypeProcedure*>(this)->returnType()->secrecDimType();
        case PROCEDUREVOID:
        default:
            return -1;
    }
}

/*******************************************************************************
  Type
*******************************************************************************/

class Type {
private:
    Type (const Type&); // DO NOT IMPLEMENT
    void operator = (const Type&); // DO NOT IMPLEMENT

public: /* Methods: */

    explicit inline Type(bool isVoid)
        : m_isVoid(isVoid)
    { }

    virtual inline ~Type() { }

    inline bool isVoid() const { return m_isVoid; }

    virtual std::string toString() const = 0;

    virtual inline bool canAssign(const Type*) const {
        return false;
    }

    inline bool isPublicIntScalar () const;
    inline SecurityType* secrecSecType() const;
    inline SecrecDataType secrecDataType() const;
    inline SecrecDimType secrecDimType() const;
    inline bool isScalar () const { return secrecDimType() == 0; }

private: /* Fields: */

    const bool m_isVoid;
};

/*******************************************************************************
  TypeVoid
*******************************************************************************/

class TypeVoid: public Type {
public: /* Methods: */

    inline TypeVoid ()
        : Type(true)
    { }

    static TypeVoid* get (Context& cxt);

    virtual inline std::string toString() const { return "void"; }
};

/*******************************************************************************
  TypeNonVoid
*******************************************************************************/

class TypeNonVoid: public Type {
public: /* Types: */

    enum Kind {
        BASIC,        /**< DataTypeBasic.         */
        VAR,          /**< DataTypeVar.           */
        PROCEDURE,    /**< DataTypeProcedure.     */
        PROCEDUREVOID /**< DataTypeProcedureVoid. */
    };

public: /* Methods: */

    TypeNonVoid(DataTypeProcedureVoid* dataType)
        : Type(false)
        , m_kind(PROCEDUREVOID)
        , m_dataType(dataType)
    { }

    TypeNonVoid(DataType* dataType);

    virtual ~TypeNonVoid() { }

    inline Kind kind() const { return m_kind; }
    inline DataType* dataType() const { return m_dataType; }

    virtual std::string toString() const;

    virtual inline bool canAssign(const Type* other) const {
        if (other->isVoid ()) return false;
        assert(dynamic_cast<const TypeNonVoid*>(other) != 0);
        const TypeNonVoid* o = static_cast<const TypeNonVoid*>(other);
        return m_dataType->canAssign (o->m_dataType);
    }

    static TypeNonVoid* get (Context& cxt, DataType* dtype);
    static TypeNonVoid* get (Context& cxt, SecrecDataType dataType,
                             SecrecDimType dimType = 0);
    static TypeNonVoid* get (Context& cxt, SecurityType* secType,
                             SecrecDataType dataType,
                             SecrecDimType dimType = 0);
    static TypeNonVoid* getIndexType (Context& cxt);
    static TypeNonVoid* getPublicBoolType (Context& cxt);

private: /* Fields: */
    Kind       const m_kind;
    DataType*  const m_dataType;
};

inline bool Type::isPublicIntScalar () const {
    return !isVoid () &&
            secrecDataType () == DATATYPE_INT64 &&
            secrecSecType ()->isPublic () &&
            secrecDimType () == 0;
}

inline SecurityType* Type::secrecSecType() const {
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    return static_cast<const TypeNonVoid&>(*this).dataType()->secrecSecType();
}

inline SecrecDataType Type::secrecDataType() const {
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    return static_cast<const TypeNonVoid&>(*this).dataType()->secrecDataType();
}

inline SecrecDimType Type::secrecDimType() const {
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    return static_cast<const TypeNonVoid&>(*this).dataType()->secrecDimType();
}

} // namespace SecreC

inline std::ostream &operator<<(std::ostream &out, const SecreC::SecurityType& type) {
    out << type.toString ();
    return out;
}

std::ostream &operator<<(std::ostream &out, const SecrecDataType& type);

inline std::ostream &operator<<(std::ostream &out, const SecreC::Type &type) {
    out << type.toString();
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const SecreC::DataType &type) {
    out << type.toString();
    return out;
}


#endif // SECREC_H
