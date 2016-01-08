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

#include "Constant.h"

#include "Context.h"
#include "ContextImpl.h"
#include "DataType.h"
#include "Types.h"

#include <cstring>
#include <sstream>
#include <string>
#include <map>

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
    case DATATYPE_BOOL: return ConstantInt::getBool (false);
    case DATATYPE_STRING: return ConstantString::get (cxt, "");
    default:
        if (isNumericDataType (ty)) {
            return numericConstant (ty, 0);
        }
    }

    return nullptr;
}

SymbolConstant* numericConstant (SecrecDataType ty, uint64_t value) {
    return numericConstant (DataTypePrimitive::get (ty), value);
}

SymbolConstant* defaultConstant (Context& cxt, const DataType* ty) {
    assert (ty != nullptr && ! ty->isComposite ());
    return defaultConstant (cxt, static_cast<const DataTypePrimitive*>(ty)->secrecDataType ());
}

SymbolConstant* numericConstant (const DataType* ty, uint64_t value) {
    assert (ty != nullptr && isNumericDataType (ty));
    if (isFloatingDataType (ty))
        return ConstantFloat::get (ty, value);
    else
        return ConstantInt::get (ty, value);
}

/*******************************************************************************
  ConstantInt
*******************************************************************************/

ConstantInt* ConstantInt::get (const DataType* type, uint64_t value) {
    assert (type != nullptr && type->isPrimitive ());
    const auto primDataType = static_cast<const DataTypePrimitive*>(type);
    return ConstantInt::get (primDataType->secrecDataType (), value);
}

// TODO: use flyweight
// TODO: const correctness
ConstantInt* ConstantInt::get (SecrecDataType type, uint64_t value) {
    using NumericConstantMap = std::map<APInt, ConstantInt>;
    static std::array<NumericConstantMap, 2> numericConstants; // 0 - unsigned, 1 - signed

    NumericConstantMap& map = numericConstants[isSignedNumericDataType(type)];
    const APInt apvalue (widthInBits (type), value);
    auto it = map.find (apvalue);
    if (it == map.end ()) {
        const auto cvalue = ConstantInt (TypeBasic::get (type), apvalue);
        it = map.insert (it, std::make_pair (apvalue, cvalue));
    }

    // Note that insert does not invalidate references to elements!
    return &it->second;
}

ConstantInt* ConstantInt::getBool (bool value) {
    return ConstantInt::get (DATATYPE_BOOL, value);
}

void ConstantInt::print (std::ostream &os) const {
    const auto dataType = static_cast<const DataTypePrimitive*>(secrecType ()->secrecDataType ());
    const auto secrecDataType = dataType->secrecDataType ();
    if (isSignedNumericDataType (secrecDataType))
        m_value.sprint (os);
    else
        m_value.uprint (os);
}

/*******************************************************************************
  ConstantFloat
*******************************************************************************/

ConstantFloat* ConstantFloat::get (const DataType* type, uint64_t value) {
    return get (type, APFloat (floatPrec (type), value));
}

ConstantFloat* ConstantFloat::get (const DataType* type, StringRef str) {
    return get (type, APFloat (floatPrec (type), str));
}

// TODO: use flyweight
// TODO: const correctness
ConstantFloat* ConstantFloat::get (const DataType* type, const APFloat& value) {
    using FloatConstantMap = std::map<APFloat, ConstantFloat, APFloat::BitwiseCmp>;
    static FloatConstantMap floatConstants;

    auto it = floatConstants.find (value);
    if (it == floatConstants.end ()) {
        const auto f = ConstantFloat (TypeBasic::get (type), value);
        it = floatConstants.insert (it, std::make_pair (value, f));
    }

    // Note that insert does not invalidate references to elements!
    return &it->second;
}

void ConstantFloat::print (std::ostream &os) const { os << m_value; }

/*******************************************************************************
  ConstantString
*******************************************************************************/

// TODO: use flyweight
// TODO: const correctness
ConstantString* ConstantString::get (Context& cxt, StringRef str) {
    using ConstantStringMap = std::map<StringRef, ConstantString>;
    static ConstantStringMap stringLiterals;

    auto it = stringLiterals.find (str);
    if (it == stringLiterals.end ()) {
        // Make sure that the string is allocated in the table
        const StringRef val = *cxt.pImpl ()->m_stringTable.addString (str);
        const auto cstr = ConstantString (TypeBasic::get (DATATYPE_STRING), val);
        it = stringLiterals.insert (it, std::make_pair (val, cstr));
    }

    // Note that insert does not invalidate references to elements!
    return &it->second;
}

void ConstantString::print (std::ostream &os) const {
    // TODO: not escaped! do we assume that input is always escaped?
    os.put ('"');
    os.write (m_value.data (), m_value.size ());
    os.put ('"');
}

} // namespace SecreC
