#include "DataType.h"

#include "Context.h"
#include "ContextImpl.h"
#include "Misc.h"
#include "Symbol.h"

#include <sstream>


namespace SecreC {

namespace /* anonymous */ {

enum CastStyle {
    CAST_FORBIDDEN = 0,
    CAST_EQUAL     = 1,
    CAST_IMPLICIT  = 2,
    CAST_EXPLICIT  = 3
};

#define X CAST_FORBIDDEN
#define S CAST_EQUAL
#define I CAST_IMPLICIT
#define E CAST_EXPLICIT

const CastStyle dataTypeCasts[NUM_DATATYPES][NUM_DATATYPES] = {
    //  x   U   B   S   N   I8  I16 I32 I64 U8  U16 U32 U64 X8  X16 X32 X64 F32 F64
       {X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X},     // DATATYPE_INVALID,
       {X,  S,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X},     // DATATYPE_UNIT,
       {X,  X,  S,  X,  X,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E},     // DATATYPE_BOOL,
       {X,  X,  X,  S,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X},     // DATATYPE_STRING,
       {X,  X,  X,  X,  S,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I},     // DATATYPE_NUMERIC,
       {X,  X,  E,  X,  X,  S,  E,  E,  E,  E,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_INT8,
       {X,  X,  E,  X,  X,  E,  S,  E,  E,  E,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_INT16,
       {X,  X,  E,  X,  X,  E,  E,  S,  E,  E,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_INT32,
       {X,  X,  E,  X,  X,  E,  E,  E,  S,  E,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_INT64,
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  S,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_UINT8,
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  S,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_UINT16,
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  E,  S,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_UINT32,
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  E,  E,  S,  E,  E,  E,  E,  E,  E},     // DATATYPE_UINT64,
       {X,  X,  E,  X,  X,  X,  X,  X,  X,  X,  X,  X,  E,  S,  E,  E,  E,  X,  X},     // DATATYPE_XOR_UINT8
       {X,  X,  E,  X,  X,  X,  X,  X,  X,  X,  X,  X,  E,  E,  S,  E,  E,  X,  X},     // DATATYPE_XOR_UINT16
       {X,  X,  E,  X,  X,  X,  X,  X,  X,  X,  X,  X,  E,  E,  E,  S,  E,  X,  X},     // DATATYPE_XOR_UINT32
       {X,  X,  E,  X,  X,  X,  X,  X,  X,  X,  X,  X,  E,  E,  E,  E,  S,  X,  X},     // DATATYPE_XOR_UINT64
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  E,  E,  E,  X,  X,  X,  X,  S,  E},     // DATATYPE_FLOAT32
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  E,  E,  E,  X,  X,  X,  X,  E,  S}      // DATATYPE_FLOAT64
};

#undef X
#undef S
#undef I
#undef E

CastStyle getCastStyle (SecrecDataType from, SecrecDataType to) {
    assert (from < NUM_DATATYPES && to < NUM_DATATYPES);
    return dataTypeCasts[from][to];
}

SecrecDataType getSecrecDataType (const DataType* dType) {
    assert (dynamic_cast<const DataTypePrimitive*>(dType) != nullptr);
    return static_cast<const DataTypePrimitive*>(dType)->secrecDataType ();
}

} // namespace anonymous

/*******************************************************************************
 SecrecDataType related operations
*******************************************************************************/

SecrecDimType upperDimType(SecrecDimType n, SecrecDimType m) {
    assert (n == 0 || m == 0 || n == m);
    if (n == 0) return m;
    return n;
}

SecrecDataType upperDataType (SecrecDataType a, SecrecDataType b) {
    if (latticeDataTypeLEQ (a, b)) return b;
    if (latticeDataTypeLEQ (b, a)) return a;

    SecrecDataType best = DATATYPE_UNDEFINED;
    for (unsigned i = 0; i < NUM_DATATYPES; ++ i) {
        SecrecDataType ty = static_cast<SecrecDataType>(i);
        if (latticeDataTypeLEQ (a, ty) && latticeDataTypeLEQ (b, ty)) {
            if (best == DATATYPE_UNDEFINED || latticeDataTypeLEQ (ty, best)) {
                best = ty;
            }
        }
    }

    return best;
}

DataType* upperDataType (Context& cxt, DataType* a, DataType* b) {
    if (a == nullptr || b == nullptr)
        return nullptr;

    if (a == b)
        return a;

    if (a->isPrimitive () && b->isPrimitive ())
        return DataTypePrimitive::get (cxt, upperDataType (getSecrecDataType (a), getSecrecDataType (b)));

    return nullptr;
}

bool latticeDimTypeLEQ (SecrecDimType n, SecrecDimType m) {
    return n == m || n == 0;
}

bool latticeDataTypeLEQ (SecrecDataType a, SecrecDataType b) {
    switch (getCastStyle (a, b)) {
    case CAST_EQUAL:
    case CAST_IMPLICIT:
        return true;
    default:
        return false;
    }
}

bool latticeExplicitLEQ (SecrecDataType a, SecrecDataType b) {
    switch (getCastStyle (a, b)) {
    case CAST_EQUAL:
    case CAST_IMPLICIT:
    case CAST_EXPLICIT:
        return true;
    default:
        return false;
    }
}

bool isFloatingDataType (SecrecDataType dType) {
    switch (dType) {
    case DATATYPE_FLOAT32:
    case DATATYPE_FLOAT64:
        return true;
    default:
        return false;
    }
}

bool isNumericDataType (SecrecDataType dType) {
    return isFloatingDataType (dType) ||
           isSignedNumericDataType (dType) ||
           isUnsignedNumericDataType (dType) ||
           isXorDataType (dType);
}

bool isSignedNumericDataType (SecrecDataType dType) {
    switch (dType) {
    case DATATYPE_INT8:
    case DATATYPE_INT16:
    case DATATYPE_INT32:
    case DATATYPE_INT64:
        return true;
    default:
        return false;
    }
}

bool isUnsignedNumericDataType (SecrecDataType dType) {
    switch (dType) {
    case DATATYPE_UINT8:
    case DATATYPE_UINT16:
    case DATATYPE_UINT32:
    case DATATYPE_UINT64:
        return true;
    default:
        return false;
    }
}

bool isXorDataType (SecrecDataType dType) {
    switch (dType) {
    case DATATYPE_XOR_UINT8:
    case DATATYPE_XOR_UINT16:
    case DATATYPE_XOR_UINT32:
    case DATATYPE_XOR_UINT64:
        return true;
    default:
        return false;
    }
}

SecrecDataType dtypeDeclassify (SecrecDataType dtype) {
    switch (dtype) {
    case DATATYPE_XOR_UINT8:  return DATATYPE_UINT8;
    case DATATYPE_XOR_UINT16: return DATATYPE_UINT16;
    case DATATYPE_XOR_UINT32: return DATATYPE_UINT32;
    case DATATYPE_XOR_UINT64: return DATATYPE_UINT64;
    default:                  return dtype;
    }
}

bool latticeDataTypeLEQ (const DataType* a, const DataType* b) {
    assert (a != nullptr && b != nullptr);

    if (a->isComposite () && b->isComposite ())
        return a == b;

    if (a->isComposite () != b->isComposite ())
        return false;

    return latticeDataTypeLEQ (getSecrecDataType (a), getSecrecDataType (b));
}

bool latticeExplicitLEQ (const DataType* a, const DataType* b) {
    assert (a != nullptr && b != nullptr);

    if (a->isComposite () && b->isComposite ())
        return a == b;

    if (a->isComposite () != b->isComposite ())
        return false;

    return latticeExplicitLEQ (getSecrecDataType (a), getSecrecDataType (b));
}

bool isFloatingDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (dType->isComposite ())
        return false;

    return isFloatingDataType (getSecrecDataType (dType));
}

bool isNumericDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (dType->isComposite ())
        return false;

    return isNumericDataType (getSecrecDataType (dType));
}

bool isXorDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (dType->isComposite ())
        return false;

    return isXorDataType (getSecrecDataType (dType));
}

bool isSignedNumericDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (dType->isComposite ())
        return false;

    return isSignedNumericDataType (getSecrecDataType (dType));
}

bool isUnsignedNumericDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (dType->isComposite ())
        return false;

    return isUnsignedNumericDataType (getSecrecDataType (dType));
}

DataType* dtypeDeclassify (Context& cxt, DataType* dType) {
    assert (dType != nullptr);
    if (dType->isPrimitive ()) {
        return DataTypePrimitive::get (cxt, dtypeDeclassify (getSecrecDataType (dType)));
    }

    return dType;
}

/*******************************************************************************
  DataType
*******************************************************************************/

bool DataType::equals (SecrecDataType other) const {
    if (! isPrimitive ())
        return false;

    return getSecrecDataType (this) == other;
}

/*******************************************************************************
  DataTypePrimitive
*******************************************************************************/

void DataTypePrimitive::print (std::ostream& os) const {
    os << SecrecFundDataTypeToString (m_dataType);
}

DataTypePrimitive* DataTypePrimitive::get (Context& cxt, SecrecDataType dataType) {
    auto& map = cxt.pImpl ()->m_primitiveTypes;
    const auto index = dataType;
    auto i = map.find (index);
    if (i == map.end ()) {
        i = map.insert (i, std::make_pair (index, new DataTypePrimitive (dataType)));
    }

    return i->second;
}

/*******************************************************************************
  DataTypeStruct
*******************************************************************************/

void DataTypeStruct::print (std::ostream& os) const {
    os << "struct " << m_name << " { ";
    for (const auto & elem : m_fields) {
        os << *elem.type << " " << elem.name << "; ";
    }

    os << "}";
}

DataTypeStruct* DataTypeStruct::find (Context& cxt, StringRef name,
                                      const DataTypeStruct::TypeArgumentList& args)
{
    auto& map = cxt.pImpl ()->m_structTypes;
    const auto index = std::make_pair (name, args);
    const auto i = map.find (index);
    return i == map.end () ? nullptr : i->second;
}

DataTypeStruct* DataTypeStruct::get (Context& cxt, StringRef name,
                                     const DataTypeStruct::FieldList& fields,
                                     const DataTypeStruct::TypeArgumentList& args)
{
    auto& map = cxt.pImpl ()->m_structTypes;
    const auto index = std::make_pair (name, args);
    auto i = map.find (index);
    if (i == map.end ()) {
        i = map.insert (i, std::make_pair (index, new DataTypeStruct (name, args, fields)));
    }

    return i->second;
}

} // namespace SecreC
