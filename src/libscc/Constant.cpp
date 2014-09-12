#include "Constant.h"

#include "Context.h"
#include "ContextImpl.h"

#include <cstring>
#include <sstream>
#include <string>

namespace SecreC {

namespace /* anonymous */ {

unsigned widthInBits (SecrecDataType type) {
    switch (type) {
    case DATATYPE_BOOL: return 1;
    case DATATYPE_XOR_UINT8:
    case DATATYPE_UINT8:
    case DATATYPE_INT8:
        return 8;
    case DATATYPE_XOR_UINT16:
    case DATATYPE_UINT16:
    case DATATYPE_INT16:
        return 16;
    case DATATYPE_XOR_UINT32:
    case DATATYPE_FLOAT32:
    case DATATYPE_UINT32:
    case DATATYPE_INT32:
        return 32;
    case DATATYPE_XOR_UINT64:
    case DATATYPE_FLOAT64:
    case DATATYPE_UINT64:
    case DATATYPE_INT64:
        return 64;
    default:
        assert (false && "widthInBits: Unsupported type!"); return 0;
    }
}

APFloat::prec_t floatPrec (SecrecDataType type) {
    switch (type) {
    case DATATYPE_FLOAT32: return 24;
    case DATATYPE_FLOAT64: return 53;
    default:  assert (false && "floatPrec: Unsupported type!"); return 0;
    }
}

APFloat::prec_t floatPrec (DataType* type) {
    assert (type != nullptr && type->isPrimitive ());
    return floatPrec (static_cast<DataTypePrimitive*>(type)->secrecDataType ());
}

} // anonymous namesace

SymbolConstant* defaultConstant (Context& cxt, SecrecDataType ty) {
    switch (ty) {
    case DATATYPE_BOOL: return ConstantInt::getBool (cxt, false);
    case DATATYPE_STRING: return ConstantString::get (cxt, "");
    default:
        if (isNumericDataType (ty)) {
            return numericConstant (cxt, ty, 0);
        }
    }

    return nullptr;
}

SymbolConstant* numericConstant (Context& cxt, SecrecDataType ty, uint64_t value) {
    return numericConstant (cxt, DataTypePrimitive::get (cxt, ty), value);
}

SymbolConstant* defaultConstant (Context& cxt, DataType* ty) {
    assert (ty != nullptr && ! ty->isComposite ());
    return defaultConstant (cxt, static_cast<DataTypePrimitive*>(ty)->secrecDataType ());
}

SymbolConstant* numericConstant (Context& cxt, DataType* ty, uint64_t value) {
    assert (ty != nullptr && isNumericDataType (ty));
    if (isFloatingDataType (ty))
        return ConstantFloat::get (cxt, ty, value);
    else
        return ConstantInt::get (cxt, ty, value);
}


/*******************************************************************************
  APFloat
*******************************************************************************/

// TODO: this function breaks MPFR abstraction
bool APFloat::BitwiseCmp::cmpMpfrStructs (const mpfr_srcptr x, const mpfr_srcptr y) {
    if (x->_mpfr_prec < y->_mpfr_prec) return true;
    if (x->_mpfr_prec > y->_mpfr_prec) return false;
    if (x->_mpfr_sign < y->_mpfr_sign) return true;
    if (x->_mpfr_sign > y->_mpfr_sign) return false;
    if (x->_mpfr_exp  < y->_mpfr_exp)  return true;
    if (x->_mpfr_exp  > y->_mpfr_exp)  return false;
    const size_t num_bytes = mpfr_custom_get_size (x->_mpfr_prec);
    return std::memcmp (x->_mpfr_d, y->_mpfr_d, num_bytes) < 0;
}

// TODO: don't rely on IEEE representation of float!
uint32_t APFloat::ieee32bits () const {
    assert (getPrec () == floatPrec (DATATYPE_FLOAT32));
    #if MPFR_VERSION >= 0x030000
    float float_result = mpfr_get_flt (m_value, SECREC_CONSTANT_MPFR_RNDN);
    #else
    float float_result = mpfr_get_ld (m_value, SECREC_CONSTANT_MPFR_RNDN);
    #endif
    auto result = new (&float_result) uint32_t;
    return *result;
}

// TODO: don't rely on IEEE representation of double!
uint64_t APFloat::ieee64bits () const {
    assert (getPrec () == floatPrec (DATATYPE_FLOAT64));
    double double_result = mpfr_get_d (m_value, SECREC_CONSTANT_MPFR_RNDN);
    auto result = new (&double_result) uint64_t;
    return *result;
}

/*******************************************************************************
  ConstantInt
*******************************************************************************/

ConstantInt* ConstantInt::get (Context& cxt, DataType* type, uint64_t value) {
    assert (type != nullptr && type->isPrimitive ());
    DataTypePrimitive* const primDataType = static_cast<DataTypePrimitive*>(type);
    return ConstantInt::get (cxt, primDataType->secrecDataType (), value);
}

ConstantInt* ConstantInt::get (Context& cxt, SecrecDataType type, uint64_t value) {
    typedef ContextImpl::NumericConstantMap IntMap;
    const APInt apvalue (widthInBits (type), value);
    IntMap& map = cxt.pImpl ()->m_numericConstants[isSignedNumericDataType (type)];
    auto it = map.find (apvalue);
    if (it == map.end ()) {
        auto cvalue = new ConstantInt (TypeBasic::get (cxt, type), apvalue);
        it = map.insert (it, std::make_pair (apvalue, cvalue));
    }

    return it->second;
}

ConstantInt* ConstantInt::getBool (Context& cxt, bool value) {
    return ConstantInt::get (cxt, DATATYPE_BOOL, value);
}

void ConstantInt::print (std::ostream &os) const { os << m_value; }

/*******************************************************************************
  ConstantFloat
*******************************************************************************/

ConstantFloat* ConstantFloat::get (Context& cxt, DataType* type, uint64_t value) {
    return get (cxt, type, APFloat (floatPrec (type), value));
}

ConstantFloat* ConstantFloat::get (Context& cxt, DataType* type, StringRef str) {
    return get (cxt, type, APFloat (floatPrec (type), str));
}

ConstantFloat* ConstantFloat::get (Context& cxt, DataType* type, const APFloat& value) {
    typedef ContextImpl::FloatConstantMap FloatMap;
    FloatMap& map = cxt.pImpl ()->m_floatConstants;
    auto it = map.find (value);
    if (it == map.end ()) {
        auto cfloat = new ConstantFloat (TypeBasic::get (cxt, type), value);
        it = map.insert (it, std::make_pair (value, cfloat));
    }

    return it->second;
}

void ConstantFloat::print (std::ostream &os) const { os << m_value; }

/*******************************************************************************
  ConstantString
*******************************************************************************/

ConstantString* ConstantString::get (Context& cxt, StringRef str) {
    typedef ContextImpl::ConstantStringMap StringMap;
    StringMap& map = cxt.pImpl ()->m_stringLiterals;
    auto it = map.find (str);
    if (it == map.end ()) {
        // Make sure that the string is allocated in the table
        StringRef val = *cxt.pImpl ()->m_stringTable.addString (str);
        auto cstr = new ConstantString (TypeBasic::get (cxt, DATATYPE_STRING), val);
        it = map.insert (it, std::make_pair (val, cstr));
    }

    return it->second;
}

void ConstantString::print (std::ostream &os) const {
    // TODO: not escaped! do we assume that input is always escaped?
    os.put ('"');
    os.write (m_value.data (), m_value.size ());
    os.put ('"');
}

} // namespace SecreC
