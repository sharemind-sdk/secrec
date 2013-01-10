#ifndef SECREC_CONSTANT_H
#define SECREC_CONSTANT_H

#include <stdint.h>

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

/******************************************************************
  Constant
******************************************************************/

template <SecrecDataType ty>
class Constant : public SymbolConstant {
private:
    typedef Constant<ty> Self;
    Constant (const Self&); // DO NOT IMPLEMENT
    void operator = (const Self&); // DO NOT IMPLEMENT

private: /* Types: */

    typedef SecrecTypeInfo<ty> trait;
    typedef typename trait::CType CType;

public: /* Methods: */

    static Self* get (Context& cxt, const CType& value);

    inline const CType& value () const {
        return m_value;
    }

protected:
    void print (std::ostream& os) const;

private:
    Constant (const CType& value, TypeNonVoid* type)
        : SymbolConstant(type)
        , m_value(value)
    { }

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

/******************************************************************
  ConstantVector
******************************************************************/

template <SecrecDataType ty>
class ConstantVector : public SymbolConstant {
    typedef ConstantVector<ty> Self;
    ConstantVector (const ConstantVector&); // DO NOT IMPLEMENT
    void operator = (const ConstantVector&); // DO NOT IMPLEMENT
private: /* Types: */
    typedef SecrecTypeInfo<ty> value_trait;

public: /* Methods: */

    static Self* get (Context& cxt, const std::vector<SymbolConstant*>& values);

    size_t size () const { return m_values.size (); }

    Constant<ty>* at (size_t i) const {
        return static_cast<Constant<ty>*>(m_values.at (i));
    }

protected:
    void print (std::ostream& os) const;

private:
    ConstantVector (TypeNonVoid* type, const std::vector<SymbolConstant*>& values)
        : SymbolConstant (type)
        , m_values (values)
    { }

private: /* Fields: */
    const std::vector<SymbolConstant*> m_values;
};

typedef ConstantVector<DATATYPE_BOOL> ConstantBoolVector;
typedef ConstantVector<DATATYPE_INT8> ConstantInt8Vector;
typedef ConstantVector<DATATYPE_UINT8> ConstantUInt8Vector;
typedef ConstantVector<DATATYPE_INT16> ConstantInt16Vector;
typedef ConstantVector<DATATYPE_UINT16> ConstantUInt16Vector;
typedef ConstantVector<DATATYPE_INT32> ConstantInt32Vector;
typedef ConstantVector<DATATYPE_UINT32> ConstantUInt32Vector;
typedef ConstantVector<DATATYPE_INT64> ConstantInt64Vector;
typedef ConstantVector<DATATYPE_INT64> ConstantIntVector;
typedef ConstantVector<DATATYPE_UINT64> ConstantUInt64Vector;
typedef ConstantVector<DATATYPE_UINT64> ConstantUIntVector;
typedef ConstantVector<DATATYPE_FLOAT32> ConstantFloat32Vector;
typedef ConstantVector<DATATYPE_FLOAT64> ConstantFloat64Vector;

} // namespace SecreC

#endif
