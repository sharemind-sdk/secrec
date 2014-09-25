#include "Constant.h"

#include "Context.h"
#include "ContextImpl.h"
#include "DataType.h"

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
  ConstantInt
*******************************************************************************/

ConstantInt* ConstantInt::get (Context& cxt, DataType* type, uint64_t value) {
    assert (type != nullptr && type->isPrimitive ());
    DataTypePrimitive* const primDataType = static_cast<DataTypePrimitive*>(type);
    return ConstantInt::get (cxt, primDataType->secrecDataType (), value);
}

ConstantInt* ConstantInt::get (Context& cxt, SecrecDataType type, uint64_t value) {
    const APInt apvalue (widthInBits (type), value);
    auto& map = cxt.pImpl ()->m_numericConstants[isSignedNumericDataType (type)];
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

void ConstantInt::print (std::ostream &os) const {
    const auto dataType = static_cast<DataTypePrimitive*>(secrecType ()->secrecDataType ());
    const auto secrecDataType = dataType->secrecDataType ();
    if (isSignedNumericDataType (secrecDataType))
        m_value.sprint (os);
    else
        m_value.uprint (os);
}

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
    auto& map = cxt.pImpl ()->m_floatConstants;
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
    auto& map = cxt.pImpl ()->m_stringLiterals;
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
