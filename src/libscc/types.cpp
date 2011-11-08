#include "types.h"
#include "symbol.h"

#include <cassert>
#include <sstream>

#include <iostream>


namespace {

using namespace SecreC;

inline const char *SecrecFundDataTypeToString(SecrecDataType dataType) {
    switch (dataType) {
        case DATATYPE_INVALID: return "invalid";
        case DATATYPE_BOOL:    return "bool";
        case DATATYPE_INT8:    return "int8";
        case DATATYPE_INT16:   return "int16";
        case DATATYPE_INT32:   return "int32";
        case DATATYPE_INT64:   return "int64";
        case DATATYPE_INT:     return "int";
        case DATATYPE_UINT8:   return "uint8";
        case DATATYPE_UINT16:  return "uint16";
        case DATATYPE_UINT32:  return "uint32";
        case DATATYPE_UINT64:  return "uint64";
        case DATATYPE_UINT:    return "uint";
        case DATATYPE_STRING:  return "string";
        case NUM_DATATYPES:    assert (false && "ICE!"); break;
    }
    return 0;
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
    //  x   B   S   I8  I16 I32 I64 I   U8  U16 U32 U64, U
       {S,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,   I},     //DATATYPE_INVALID,
       {F,  S,  F,  I,  I,  I,  I,  I,  I,  I,  I,  I,   I},     //DATATYPE_BOOL,
       {F,  F,  S,  F,  F,  F,  F,  F,  F,  F,  F,  F,   F},     //DATATYPE_STRING,
       {F,  E,  F,  S,  I,  I,  I,  I,  E,  E,  E,  E,   E},     //DATATYPE_INT8,
       {F,  E,  F,  E,  S,  I,  I,  I,  E,  E,  E,  E,   E},     //DATATYPE_INT16,
       {F,  E,  F,  E,  E,  S,  I,  I,  E,  E,  E,  E,   E},     //DATATYPE_INT32,
       {F,  E,  F,  E,  E,  E,  S,  S,  E,  E,  E,  E,   E},     //DATATYPE_INT64,
       {F,  E,  F,  E,  E,  E,  S,  S,  E,  E,  E,  E,   E},     //DATATYPE_INT,
       {F,  E,  F,  E,  I,  I,  I,  I,  S,  I,  I,  I,   I},     //DATATYPE_UINT8,
       {F,  E,  F,  E,  E,  I,  I,  I,  E,  S,  I,  I,   I},     //DATATYPE_UINT16,
       {F,  E,  F,  E,  E,  E,  I,  I,  E,  E,  S,  I,   I},     //DATATYPE_UINT32,
       {F,  E,  F,  E,  E,  E,  E,  E,  E,  E,  E,  S,   S},     //DATATYPE_UINT64,
       {F,  E,  F,  E,  E,  E,  E,  E,  E,  E,  E,  S,   S}      //DATATYPE_UINT,
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

    SecrecDataType best = DATATYPE_INVALID;
    for (unsigned i = 0; i < (unsigned) NUM_DATATYPES; ++ i) {
        SecrecDataType ty = (SecrecDataType) i;
        if (latticeDataTypeLEQ (a, ty) && latticeDataTypeLEQ (b, ty)) {
            if (best == DATATYPE_INVALID || latticeDataTypeLEQ (ty, best)) {
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
    bool areLEQ = false;
    switch (getCastStyle (a, b)) {
    case CAST_EQUAL:
    case CAST_IMPLICIT:
        areLEQ = true;
    default:
        break;
    }

    return areLEQ;
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
    case DATATYPE_INT:
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
    case DATATYPE_UINT:
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
    return domain ()->name ();
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

DataTypeProcedureVoid::DataTypeProcedureVoid(const DataTypeProcedureVoid &copy)
    : DataType(copy)
{
    typedef std::vector<DataType*>::const_iterator TVCI;
    for (TVCI it(copy.m_params.begin()); it != copy.m_params.end(); it++) {
        m_params.push_back((*it)->clone());
    }
}

DataTypeProcedureVoid::~DataTypeProcedureVoid() {
    typedef std::vector<DataType*>::const_iterator TVCI;
    for (TVCI it(m_params.begin()); it != m_params.end(); it++) {
        delete (*it);
    }
}

// \todo don't use mangle() here
std::string DataTypeProcedureVoid::toString() const {
    return mangle() + " -> void";
}

// right now functions can only be overloaded by data,
// it's also possible to overload by dimensionalities, but i'm
// not sure how wise this is
std::string DataTypeProcedureVoid::mangle() const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    std::ostringstream os;
    os << "(";
    if (m_params.size() > 0) {
        os << SecrecFundDataTypeToString(m_params.at(0)->secrecDataType());
        os << m_params.at(0)->secrecDimType();
        if (m_params.size() > 1) {
            for (TVCI it(++(m_params.begin())); it != m_params.end(); it++) {
                os << ", " << SecrecFundDataTypeToString((*it)->secrecDataType());
                os << (*it)->secrecDimType();

            }
        }
    }
    os << ")";
    return os.str();
}

bool DataTypeProcedureVoid::operator==(const DataType &other) const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    if (!DataType::operator==(other)) return false;
    assert(dynamic_cast<const DataTypeProcedureVoid*>(&other) != 0);
    const DataTypeProcedureVoid &o(static_cast<const DataTypeProcedureVoid&>(other));

    if (m_params.size() != o.m_params.size()) return false;

    TVCI it(m_params.begin());
    TVCI jt(m_params.begin());
    for (; it != m_params.end(); it++, jt++) {
        if (*it != *jt) return false;
    }
    return true;
}

/*******************************************************************************
  DataTypeProcedure
*******************************************************************************/

std::string DataTypeProcedure::toString() const {
    std::ostringstream os;
    os << mangle() << " -> " << *m_ret;
    return os.str();
}

bool DataTypeProcedure::operator==(const DataType &other) const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    if (!DataTypeProcedureVoid::operator==(other)) return false;
    assert(dynamic_cast<const DataTypeProcedure*>(&other) != 0);
    const DataTypeProcedure &o(static_cast<const DataTypeProcedure&>(other));

    return *m_ret == *o.m_ret;
}


/*******************************************************************************
  TypeNonVoid
*******************************************************************************/

TypeNonVoid::TypeNonVoid(const DataType &dataType)
     : Type(false)
     , m_kind (kindToKind (dataType.kind ()))
     , m_dataType(dataType.clone())
{ }

TypeNonVoid::~TypeNonVoid() {
    delete m_dataType;
}

std::string TypeNonVoid::toString() const {
    assert(m_dataType != 0);
    std::ostringstream os;
    os << *m_dataType;
    return os.str();
}

} // namespace SecreC


/*******************************************************************************
  Global functions
*******************************************************************************/

std::ostream &operator<<(std::ostream &out, const SecrecDataType &type) {
    out << SecrecFundDataTypeToString(type);
    return out;
}
