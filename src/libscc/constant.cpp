#include "constant.h"

#include <string>
#include <sstream>
#include <cstring>

#include <boost/foreach.hpp>

#include "context.h"
#include "context_impl.h"

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

    return 0;
}

SymbolConstant* numericConstant (Context& cxt, SecrecDataType ty, uint64_t value) {
    assert (isNumericDataType (ty));
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

// TODO: don't rely on IEEE representation!
uint32_t APFloat::ieee32bits () const {
    assert (getPrec () == floatPrec (DATATYPE_FLOAT32));
    float float_result = mpfr_get_flt (m_value, MPFR_RNDN);
    uint32_t* result = new (&float_result) uint32_t;
    return *result;
}

// TODO: don't rely on IEEE representation!
uint64_t APFloat::ieee64bits () const {
    assert (getPrec () == floatPrec (DATATYPE_FLOAT64));
    double double_result = mpfr_get_d (m_value, MPFR_RNDN);
    uint64_t* result = new (&double_result) uint64_t;
    return *result;
}

/*******************************************************************************
  ConstantInt
*******************************************************************************/

ConstantInt* ConstantInt::get (Context& cxt, SecrecDataType type, uint64_t value) {
    typedef ContextImpl::NumericConstantMap IntMap;
    const APInt apvalue (widthInBits (type), value);
    IntMap& map = cxt.pImpl ()->m_numericConstants[isSignedNumericDataType (type)];
    IntMap::iterator it = map.find (apvalue);
    if (it == map.end ()) {
        ConstantInt* cvalue = new ConstantInt (TypeNonVoid::get (cxt, type), apvalue);
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

ConstantFloat* ConstantFloat::get (Context& cxt, SecrecDataType type, uint64_t value) {
    return get (cxt, type, APFloat (floatPrec (type), value));
}

ConstantFloat* ConstantFloat::get (Context& cxt, SecrecDataType type, StringRef str) {
    return get (cxt, type, APFloat (floatPrec (type), str));
}

ConstantFloat* ConstantFloat::get (Context& cxt, SecrecDataType type, const APFloat& value) {
    typedef ContextImpl::FloatConstantMap FloatMap;
    FloatMap& map = cxt.pImpl ()->m_floatConstants;
    FloatMap::iterator it = map.find (value);
    if (it == map.end ()) {
        ConstantFloat* cfloat = new ConstantFloat (TypeNonVoid::get (cxt, type), value);
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
    StringMap::iterator it = map.find (str);
    if (it == map.end ()) {
        // Make sure that the string is allocated in the table
        StringRef val = *cxt.pImpl ()->m_stringTable.addString (str);
        ConstantString* cstr = new ConstantString (TypeNonVoid::get (cxt, DATATYPE_STRING), val);
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
