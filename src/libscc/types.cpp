#include "types.h"

#include <cassert>
#include <sstream>
#include <iostream>
#include <boost/foreach.hpp>

#include "symbol.h"
#include "context.h"
#include "context_impl.h"


namespace {

using namespace SecreC;

inline const char *SecrecFundDataTypeToString(SecrecDataType dataType) {
    switch (dataType) {
    case DATATYPE_UNDEFINED: return "undefined";
    case DATATYPE_BOOL:      return "bool";
    case DATATYPE_INT8:      return "int8";
    case DATATYPE_INT16:     return "int16";
    case DATATYPE_INT32:     return "int32";
    case DATATYPE_INT64:     return "int64";
    case DATATYPE_UINT8:     return "uint8";
    case DATATYPE_UINT16:    return "uint16";
    case DATATYPE_UINT32:    return "uint32";
    case DATATYPE_UINT64:    return "uint64";
    case DATATYPE_STRING:    return "string";
    case NUM_DATATYPES:
        assert (false && "ICE!");
        break;
    }

    return 0;
}

std::string mangleDataType (DataType* dty) {
    std::ostringstream os;
    os << '(';
    os << *dty->secrecSecType () << ',';
    os << dty->secrecDataType() << ',';
    os << dty->secrecDimType();
    os << ')';
    return os.str ();
}

enum CastStyle {
    CAST_FORBIDDEN = 0,
    CAST_EQUAL     = 1,
    CAST_IMPLICIT  = 2,
    CAST_EXPLICIT  = 3
};

#define F CAST_FORBIDDEN
#define S CAST_EQUAL
#define I CAST_IMPLICIT
#define E CAST_EXPLICIT

const CastStyle dataTypeCasts[NUM_DATATYPES][NUM_DATATYPES] = {
    //  x   B   S   I8  I16 I32 I64 U8  U16 U32 U64
       {S,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I},     //DATATYPE_INVALID,
       {F,  S,  F,  I,  I,  I,  I,  I,  I,  I,  I},     //DATATYPE_BOOL,
       {F,  F,  S,  F,  F,  F,  F,  F,  F,  F,  F},     //DATATYPE_STRING,
       {F,  E,  F,  S,  I,  I,  I,  E,  E,  E,  E},     //DATATYPE_INT8,
       {F,  E,  F,  E,  S,  I,  I,  E,  E,  E,  E},     //DATATYPE_INT16,
       {F,  E,  F,  E,  E,  S,  I,  E,  E,  E,  E},     //DATATYPE_INT32,
       {F,  E,  F,  E,  E,  E,  S,  E,  E,  E,  E},     //DATATYPE_INT64,
       {F,  E,  F,  E,  I,  I,  I,  S,  I,  I,  I},     //DATATYPE_UINT8,
       {F,  E,  F,  E,  E,  I,  I,  E,  S,  I,  I},     //DATATYPE_UINT16,
       {F,  E,  F,  E,  E,  E,  I,  E,  E,  S,  I},     //DATATYPE_UINT32,
       {F,  E,  F,  E,  E,  E,  E,  E,  E,  E,  S}      //DATATYPE_UINT64,
};

#undef F
#undef S
#undef I
#undef E


TypeNonVoid::Kind kindToKind (DataType::Kind k) {
    switch (k) {
        case DataType::BASIC:         return TypeNonVoid::BASIC;
        case DataType::VAR:           return TypeNonVoid::VAR;
        case DataType::PROCEDURE:     return TypeNonVoid::PROCEDURE;
        case DataType::PROCEDUREVOID: return TypeNonVoid::PROCEDUREVOID;
    }
}

}

namespace SecreC {


CastStyle getCastStyle (SecrecDataType from, SecrecDataType to) {
    assert (from < NUM_DATATYPES && to < NUM_DATATYPES);
    return dataTypeCasts[from][to];
}

SecrecDimType upperDimType(SecrecDimType n, SecrecDimType m) {
    assert (n == 0 || m == 0 || n == m);
    if (n == 0) return m;
    return n;
}

SecrecDataType upperDataType (SecrecDataType a, SecrecDataType b) {
    if (latticeDataTypeLEQ (a, b)) return b;
    if (latticeDataTypeLEQ (b, a)) return a;

    SecrecDataType best = DATATYPE_UNDEFINED;
    for (unsigned i = 0; i < (unsigned) NUM_DATATYPES; ++ i) {
        SecrecDataType ty = (SecrecDataType) i;
        if (latticeDataTypeLEQ (a, ty) && latticeDataTypeLEQ (b, ty)) {
            if (best == DATATYPE_UNDEFINED || latticeDataTypeLEQ (ty, best)) {
                best = ty;
            }
        }
    }

    return best;
}

bool latticeDimTypeLEQ (SecrecDimType n, SecrecDimType m) {
    return n == m || n == 0;
}

bool latticeDataTypeLEQ (SecrecDataType a, SecrecDataType b) {
    switch (getCastStyle (a, b)) {
    case CAST_EQUAL:
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


bool isNumericDataType (SecrecDataType dType) {
    return isSignedNumericDataType (dType) ||
           isUnsignedNumericDataType (dType);
}

bool isSignedNumericDataType (SecrecDataType dType) {
    bool isSigned = false;
    switch (dType) {
    case DATATYPE_INT8:
    case DATATYPE_INT16:
    case DATATYPE_INT32:
    case DATATYPE_INT64:
        isSigned = true;
    default:
        break;
    }

    return isSigned;
}

bool isUnsignedNumericDataType (SecrecDataType dType) {
    bool isUnsigned = false;
    switch (dType) {
    case DATATYPE_UINT8:
    case DATATYPE_UINT16:
    case DATATYPE_UINT32:
    case DATATYPE_UINT64:
        isUnsigned = true;
    default:
        break;
    }

    return isUnsigned;
}

/*******************************************************************************
  PublicSecType
*******************************************************************************/

std::string PublicSecType::toString () const {
    return "public";
}

/*******************************************************************************
  PrivateSecType
*******************************************************************************/

std::string PrivateSecType::toString () const {
    return m_name;
}

/*******************************************************************************
  DataTypeBasic
*******************************************************************************/

std::string DataTypeBasic::toString() const {
    std::ostringstream os;
    os << "(" << m_secType->toString () << ","
       << SecrecFundDataTypeToString(m_dataType) << ","
       << m_dimType << ")";
    return os.str();
}


/*******************************************************************************
  DataTypeVar
*******************************************************************************/

std::string DataTypeVar::toString() const {
    return std::string("VAR ") + m_dataType->toString();
}


/*******************************************************************************
  DataTypeProcedureVoid
*******************************************************************************/

// \todo don't use mangle() here
std::string DataTypeProcedureVoid::toString() const {
    return mangle() + " -> void";
}

std::string DataTypeProcedureVoid::mangle() const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    std::ostringstream os;
    os << "(";
    if (m_params.size() > 0) {
        os << mangleDataType (m_params.at (0));
        if (m_params.size() > 1) {
            for (TVCI it(++(m_params.begin())); it != m_params.end(); it++) {
                os << ", " << mangleDataType (*it);
            }
        }
    }
    os << ")";
    return os.str();
}

/*******************************************************************************
  DataTypeProcedure
*******************************************************************************/

std::string DataTypeProcedure::toString() const {
    std::ostringstream os;
    os << mangle() << " -> " << *m_ret;
    return os.str();
}

/*******************************************************************************
  Static methods for constructing/getting types.
*******************************************************************************/

PrivateSecType* PrivateSecType::get (Context& cxt, const std::string& name,
                                     SymbolKind* kind)
{
    ContextImpl& impl = *cxt.pImpl ();
    return impl.privateType (name, kind);
}

PublicSecType* PublicSecType::get (Context& cxt) {
    ContextImpl& impl = *cxt.pImpl ();
    return impl.publicType ();
}

DataTypeBasic* DataTypeBasic::get (Context& cxt,
                                   SecrecDataType dataType,
                                   SecrecDimType dim)
{
    ContextImpl& impl = *cxt.pImpl ();
    assert (dataType != DATATYPE_UNDEFINED);
    assert (dim >= 0);
    return impl.basicDataType (impl.publicType (), dataType, dim);
}

DataTypeBasic* DataTypeBasic::get (Context& cxt,
                                   SecurityType* secType,
                                   SecrecDataType dataType,
                                   SecrecDimType dim)
{
    ContextImpl& impl = *cxt.pImpl ();
    assert (secType != 0);
    assert (dataType != DATATYPE_UNDEFINED);
    assert (dim >= 0);
    return impl.basicDataType (secType, dataType, dim);
}

DataTypeVar* DataTypeVar::get (Context& cxt, DataType* base) {
    ContextImpl& impl = *cxt.pImpl ();
    return impl.varType (base);
}

DataTypeProcedureVoid* DataTypeProcedureVoid::get (Context& cxt,
                                                   const std::vector<DataType*>& params)
{
    ContextImpl& impl = *cxt.pImpl ();
    return impl.voidProcedureType (params);
}

DataTypeProcedureVoid* DataTypeProcedureVoid::get (Context& cxt)
{
    ContextImpl& impl = *cxt.pImpl ();
    return impl.voidProcedureType (std::vector<DataType*> ());
}

DataTypeProcedure* DataTypeProcedure::get (Context& cxt,
                                           const std::vector<DataType*>& params,
                                           DataType* returnType)
{
    ContextImpl& impl = *cxt.pImpl ();
    return impl.procedureType (params, returnType);
}

DataTypeProcedure* DataTypeProcedure::get (Context& cxt,
                                           DataTypeProcedureVoid* params,
                                           DataType* returnType)
{
    ContextImpl& impl = *cxt.pImpl ();
    return impl.procedureType (params->paramTypes (), returnType);
}

TypeVoid* TypeVoid::get (Context& cxt) {
    ContextImpl& impl = *cxt.pImpl ();
    return impl.voidType ();
}

TypeNonVoid* TypeNonVoid::get (Context& cxt, DataType* dtype) {
    ContextImpl& impl = *cxt.pImpl ();
    return impl.nonVoidType (dtype);
}

TypeNonVoid* TypeNonVoid::get (Context& cxt,
                               SecrecDataType dataType,
                               SecrecDimType dimType)
{
    ContextImpl& impl = *cxt.pImpl ();
    assert (dataType != DATATYPE_UNDEFINED);
    assert (dimType >= 0);
    return impl.nonVoidType (impl.basicDataType (impl.publicType (),
                                                 dataType, dimType));
}

TypeNonVoid* TypeNonVoid::get (Context& cxt,
                               SecurityType* secType,
                               SecrecDataType dataType,
                               SecrecDimType dimType)
{
    ContextImpl& impl = *cxt.pImpl ();
    return impl.nonVoidType (impl.basicDataType (secType, dataType, dimType));
}

TypeNonVoid* TypeNonVoid::getIndexType (Context& cxt) {
    return TypeNonVoid::get (cxt, DATATYPE_INT64);
}

/*******************************************************************************
  TypeNonVoid
*******************************************************************************/

TypeNonVoid::TypeNonVoid(DataType* dataType)
     : Type (false)
     , m_kind (kindToKind (dataType->kind ()))
     , m_dataType (dataType)
{ }

std::string TypeNonVoid::toString() const {
    assert(m_dataType != 0);
    std::ostringstream os;
    os << *m_dataType;
    return os.str();
}

/*******************************************************************************
  TypeContext
*******************************************************************************/

std::string TypeContext::toString () const {
    std::ostringstream os;
    os << '(';
    if (haveContextSecType ())
        os << *contextSecType ();
    else
        os << '*';
    os << ',';
    if (haveContextDataType ())
        os << contextDataType ();
    else
        os << '*';
    os << ',';
    if (haveContextDimType ())
        os << contextDimType ();
    else
        os << '*';
    os << ')';
    return os.str ();
}

} // namespace SecreC


/*******************************************************************************
  Global functions
*******************************************************************************/

std::ostream &operator<<(std::ostream &out, const SecrecDataType &type) {
    out << SecrecFundDataTypeToString(type);
    return out;
}
