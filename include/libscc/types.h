#ifndef SECREC_TYPES_H
#define SECREC_TYPES_H

#include <cassert>
#include <string>
#include <vector>

#include "parser.h"
#include "SecurityType.h"
#include "DataType.h"

namespace SecreC {

class Context;

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
    virtual std::string toNormalString() const = 0;

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
    virtual inline std::string toNormalString() const { return "void"; }
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
    virtual inline std::string toNormalString() const {
        return m_dataType->toNormalString();
    }

    virtual inline bool canAssign(const Type* other) const {
        if (other->isVoid ()) return false;
        assert(dynamic_cast<const TypeNonVoid*>(other) != 0);
        const TypeNonVoid* o = static_cast<const TypeNonVoid*>(other);
        return m_dataType->canAssign (o->m_dataType);
    }

    inline bool latticeLEQ(const TypeNonVoid* other) const {
        return m_dataType->latticeLEQ (other->m_dataType);
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
            secrecDataType () == DATATYPE_UINT64 &&
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

inline std::ostream &operator<<(std::ostream &out, const Type &type) {
    out << type.toString();
    return out;
}

} // namespace SecreC


#endif // SECREC_H
