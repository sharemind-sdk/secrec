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

#include "DataType.h"

#include "Context.h"
#include "ContextImpl.h"
#include "Misc.h"
#include "SecurityType.h"
#include "Symbol.h"
#include "Types.h"

#include <sstream>

#include <boost/flyweight.hpp>
#include <boost/flyweight/key_value.hpp>
#include <boost/flyweight/no_locking.hpp>
#include <boost/flyweight/no_tracking.hpp>

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
    assert (dynamic_cast<const DataTypeBuiltinPrimitive*>(dType) != nullptr);
    return static_cast<const DataTypeBuiltinPrimitive*>(dType)->secrecDataType ();
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

/*
 * If you have a private and a public type, this is supposed to check
 * whether the public type classifies to the private type. We need to
 * use upperDataType because one argument could be DATATYPE_NUMERIC.
 */
bool leqDeclassify (const TypeBasic* a, const TypeBasic* b) {
    if (! (a->secrecSecType ()->isPrivate () && b->secrecSecType ()->isPublic ()))
        return false;

    const DataType* x = dtypeDeclassify (a->secrecSecType (), a->secrecDataType ());
    if (x == nullptr)
        return false;

    auto adata =
        static_cast<const DataTypeBuiltinPrimitive*> (x)->secrecDataType ();
    auto bdata =
        static_cast<const DataTypeBuiltinPrimitive*> (b->secrecDataType ())->secrecDataType ();

    if (upperDataType (adata, bdata) == adata)
        return true;

    return false;
}

const DataType* upperDataType (const TypeBasic* a, const TypeBasic* b) {
    if (a == nullptr || b == nullptr)
        return nullptr;

    auto adata = a->secrecDataType ();
    auto bdata = b->secrecDataType ();

    if (adata == bdata)
        return adata;

    if (leqDeclassify (a, b))
        return adata;

    if (leqDeclassify (b, a))
        return bdata;

    if (adata->isBuiltinPrimitive () && bdata->isBuiltinPrimitive ()) {
        auto upper = upperDataType (getSecrecDataType (adata),
                                    getSecrecDataType (bdata));
        return DataTypeBuiltinPrimitive::get (upper);
    }

    if (adata->isUserPrimitive () && bdata->isUserPrimitive () &&
        adata == bdata)
    {
        return adata;
    }

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

unsigned widthInBitsDataType (SecrecDataType dType) {
    switch (dType) {
    case DATATYPE_BOOL:
    case DATATYPE_XOR_UINT8:
    case DATATYPE_UINT8:
    case DATATYPE_INT8:
        return 8;
    case DATATYPE_XOR_UINT16:
    case DATATYPE_UINT16:
    case DATATYPE_INT16:
        return 16;
    case DATATYPE_XOR_UINT32:
    case DATATYPE_UINT32:
    case DATATYPE_INT32:
    case DATATYPE_FLOAT32:
        return 32;
    case DATATYPE_XOR_UINT64:
    case DATATYPE_UINT64:
    case DATATYPE_INT64:
    case DATATYPE_FLOAT64:
        return 64;
    default:
        // Everything else is pointer sized.
        return 64;
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

    if (a->isUserPrimitive () && b->isUserPrimitive ())
        return a == b;

    if (a->isBuiltinPrimitive () != b->isBuiltinPrimitive ())
        return false;

    return latticeDataTypeLEQ (getSecrecDataType (a), getSecrecDataType (b));
}

bool latticeExplicitLEQ (const DataType* a, const DataType* b) {
    assert (a != nullptr && b != nullptr);

    if (a->isComposite () && b->isComposite ())
        return a == b;

    if (a->isComposite () != b->isComposite ())
        return false;

    if (a->isUserPrimitive () && b->isUserPrimitive ())
        return a == b;

    if (a->isBuiltinPrimitive () != b->isBuiltinPrimitive ())
        return false;

    return latticeExplicitLEQ (getSecrecDataType (a), getSecrecDataType (b));
}

bool isFloatingDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (! dType->isBuiltinPrimitive ())
        return false;

    return isFloatingDataType (getSecrecDataType (dType));
}

bool isNumericDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (! dType->isBuiltinPrimitive ())
        return false;

    return isNumericDataType (getSecrecDataType (dType));
}

bool isXorDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (! dType->isBuiltinPrimitive ())
        return false;

    return isXorDataType (getSecrecDataType (dType));
}

bool isSignedNumericDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (! dType->isBuiltinPrimitive ())
        return false;

    return isSignedNumericDataType (getSecrecDataType (dType));
}

bool isUnsignedNumericDataType (const DataType* dType) {
    assert (dType != nullptr);
    if (! dType->isBuiltinPrimitive ())
        return false;

    return isUnsignedNumericDataType (getSecrecDataType (dType));
}

const DataType* dtypeDeclassify (const SecurityType* secType,
                                 const DataType* dType)
{
    if (dType == nullptr)
        return dType;

    if (dType->isBuiltinPrimitive ()) {
        return DataTypeBuiltinPrimitive::get (dtypeDeclassify (getSecrecDataType (dType)));
    }
    else if (dType->isUserPrimitive ()) {
        if (secType == nullptr || secType->isPublic ())
            return nullptr;

        assert (secType->isPrivate ());

        SymbolKind* kind = static_cast<const PrivateSecType*> (secType)->securityKind ();
        const DataTypeUserPrimitive *dtPrim = static_cast<const DataTypeUserPrimitive*> (dType);

        if (! dtPrim->inKind (kind))
            return nullptr;

        auto publicType = dtPrim->publicType (kind);
        if (! publicType)
            return nullptr;

        return *publicType;
    }

    return dType;
}

/*******************************************************************************
  DataType
*******************************************************************************/

bool DataType::equals (SecrecDataType other) const {
    if (! isBuiltinPrimitive ())
        return false;

    return getSecrecDataType (this) == other;
}

/*******************************************************************************
  DataTypeBuiltinPrimitive
*******************************************************************************/

void DataTypeBuiltinPrimitive::print (std::ostream& os) const {
    os << SecrecFundDataTypeToString (m_dataType);
}

const DataTypeBuiltinPrimitive* DataTypeBuiltinPrimitive::get (SecrecDataType dataType) {
    using namespace ::boost::flyweights;
    using fw_t = flyweight<
        key_value<SecrecDataType, DataTypeBuiltinPrimitive>,
        no_locking, no_tracking>;
    return &fw_t{dataType}.get();
}

bool DataTypeBuiltinPrimitive::equals (const DataType* other) const {
    assert (other != nullptr);

    if (! other->isPrimitive ()) {
        return false;
    }
    else if (other->isBuiltinPrimitive ()) {
        return this == other;
    }
    else if (other->isUserPrimitive ()) {
        return static_cast<const DataTypeUserPrimitive*> (other)->equals (m_dataType);
    }

    return false;
}

/*******************************************************************************
  DataTypeUserPrimitive
*******************************************************************************/

void DataTypeUserPrimitive::print (std::ostream& os) const {
    os << m_name;
}

DataTypeUserPrimitive* DataTypeUserPrimitive::get (Context& cxt, StringRef name)
{
    auto& map = cxt.pImpl ()->m_userPrimitiveTypes;
    auto i = map.find (name);
    if (i == map.end ()) {
        auto ty = new DataTypeUserPrimitive (name);
        i = map.insert (i, std::make_pair (name, ty));
    }

    return i->second;
}

bool DataTypeUserPrimitive::equals (SecrecDataType type) const {
    StringRef tyStr = SecrecFundDataTypeToString (type);
    return tyStr == m_name;
}

bool DataTypeUserPrimitive::equals (const DataType* other) const {
    assert (other != nullptr);

    if (! other->isPrimitive ()) {
        return false;
    }
    else if (other->isBuiltinPrimitive ()) {
        return equals (static_cast<const DataTypeBuiltinPrimitive*> (other)->secrecDataType ());
    }
    else if (other->isUserPrimitive ()) {
        return this == other;
    }

    return false;
}

void DataTypeUserPrimitive::addParameters (SymbolKind* kind,
                                           boost::optional<const DataTypeBuiltinPrimitive*> publicType,
                                           boost::optional<uint64_t> size)
{
    assert (kind != nullptr);
    m_parameters.insert (std::make_pair (kind, Parameters {publicType, size}));
}

bool DataTypeUserPrimitive::inKind (SymbolKind* kind) const {
    assert (kind != nullptr);
    return m_parameters.find (kind) != m_parameters.end ();
}

boost::optional<const DataTypeBuiltinPrimitive*>
DataTypeUserPrimitive::publicType (SymbolKind* kind) const {
    assert (kind != nullptr);

    boost::optional<const DataTypeBuiltinPrimitive*> res = boost::none;
    auto it = m_parameters.find (kind);

    if (it != m_parameters.end ()) {
        res = it->second.publicType;
    }

    return res;
}

boost::optional<uint64_t> DataTypeUserPrimitive::size (SymbolKind* kind) const {
    assert (kind != nullptr);

    boost::optional<uint64_t> res = boost::none;
    auto it = m_parameters.find (kind);

    if (it != m_parameters.end ()) {
        res = it->second.size;
    }

    return res;
}

bool DataTypeUserPrimitive::Compare::operator() (const SymbolKind* const k1,
                                                 const SymbolKind* const k2) const
{
    return k1->name () < k2->name ();
}

/*******************************************************************************
  DataTypeStruct
*******************************************************************************/

void DataTypeStruct::print (std::ostream& os) const {
    os << "struct " << m_name;

    if (!m_typeArgs.empty ()) {
        os << "<";
        bool first = true;
        for (const auto & typeArg : m_typeArgs) {
            if (!first)
                os << ", ";
            first = false;
            os << typeArg;
        }
        os << ">";
    }
}

const DataTypeStruct* DataTypeStruct::get (StringRef name,
                                           const DataTypeStruct::FieldList& fields,
                                           const DataTypeStruct::TypeArgumentList& args)
{
    using StructTypeMap = std::map<
        std::pair<StringRef, std::vector<TypeArgument> >,
        std::unique_ptr<const DataTypeStruct> >;
    static StructTypeMap structTypes;

    const auto index = std::make_pair(name, args);
    auto it = structTypes.find(index);
    if (it == structTypes.end()) {
        it = structTypes.insert (it, std::make_pair (
            index, std::unique_ptr<DataTypeStruct>{new DataTypeStruct {name, args, fields}}));
    }

    return it->second.get();
}

} // namespace SecreC
