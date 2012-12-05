#ifndef SECREC_CONSTANT_H
#define SECREC_CONSTANT_H

#include <stdint.h>
#include <sstream>

#include "symbol.h"

namespace SecreC {

class Context;

template <SecrecDataType ty>
struct SecrecTypeInfo { };

#define DECL_TRAIT(cValTy, secrecValTy)\
    template <> struct SecrecTypeInfo <secrecValTy > {\
        typedef cValTy CType;\
        static const char* CName;\
    };

DECL_TRAIT (std::string, DATATYPE_STRING)
DECL_TRAIT (bool,        DATATYPE_BOOL)
DECL_TRAIT (int8_t,      DATATYPE_INT8)
DECL_TRAIT (int16_t,     DATATYPE_INT16)
DECL_TRAIT (int32_t,     DATATYPE_INT32)
DECL_TRAIT (int64_t,     DATATYPE_INT64)
DECL_TRAIT (uint8_t,     DATATYPE_UINT8)
DECL_TRAIT (uint16_t,    DATATYPE_UINT16)
DECL_TRAIT (uint32_t,    DATATYPE_UINT32)
DECL_TRAIT (uint64_t,    DATATYPE_UINT64)
DECL_TRAIT (uint32_t,    DATATYPE_FLOAT32)
DECL_TRAIT (uint64_t,    DATATYPE_FLOAT64)
#undef DECL_TRAIT

template <SecrecDataType ty >
class Constant : public SymbolConstant {
private:
    typedef Constant<ty> Self;
    Constant (const Self&); // DO NOT IMPLEMENT
    void operator = (const Self&); // DO NOT IMPLEMENT

private: /* Types: */

    typedef SecrecTypeInfo<ty> trait;
    typedef typename trait::CType CType;

public: /* Methods: */

    explicit Constant (const CType& value, TypeNonVoid* type)
        : SymbolConstant(type)
        , m_value(value)
    { }

    static Self* get (Context& cxt, const CType& value);

    inline const CType& value () const {
        return m_value;
    }

protected:
    void print (std::ostream& os) const {
        os << trait::CName << ' ' << m_value;
    }

private: /* Fields: */

    const CType m_value;
};

typedef Constant<DATATYPE_STRING> ConstantString;
typedef Constant<DATATYPE_BOOL> ConstantBool;
typedef Constant<DATATYPE_INT8> ConstantInt8;
typedef Constant<DATATYPE_UINT8> ConstantUInt8;
typedef Constant<DATATYPE_INT16> ConstantInt16;
typedef Constant<DATATYPE_UINT16> ConstantUInt16;
typedef Constant<DATATYPE_INT32> ConstantInt32;
typedef Constant<DATATYPE_UINT32> ConstantUInt32;
typedef Constant<DATATYPE_INT64> ConstantInt64;
typedef Constant<DATATYPE_INT64> ConstantInt;
typedef Constant<DATATYPE_UINT64> ConstantUInt64;
typedef Constant<DATATYPE_UINT64> ConstantUInt;
typedef Constant<DATATYPE_FLOAT32> ConstantFloat32;
typedef Constant<DATATYPE_FLOAT64> ConstantFloat64;

SymbolConstant* defaultConstant (Context& cxt, SecrecDataType ty);
SymbolConstant* numericConstant (Context& cxt, SecrecDataType ty, uint64_t value);
}

#endif
