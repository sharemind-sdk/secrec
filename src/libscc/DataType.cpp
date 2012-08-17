#include "DataType.h"

#include "symbol.h"
#include "context.h"
#include "context_impl.h"

namespace SecreC {

namespace /* anonymous */ {

inline const char *SecrecFundDataTypeToString(SecrecDataType dataType) {
    switch (dataType) {
    case DATATYPE_UNDEFINED:     return "undefined";
    case DATATYPE_UNIT:          return "unit";
    case DATATYPE_BOOL:          return "bool";
    case DATATYPE_NUMERIC:       return "numeric";
    case DATATYPE_INT8:          return "int8";
    case DATATYPE_INT16:         return "int16";
    case DATATYPE_INT32:         return "int32";
    case DATATYPE_INT64:         return "int64";
    case DATATYPE_UINT8:         return "uint8";
    case DATATYPE_UINT16:        return "uint16";
    case DATATYPE_UINT32:        return "uint32";
    case DATATYPE_UINT64:        return "uint64";
    case DATATYPE_XOR_UINT8:     return "xor_uint8";
    case DATATYPE_XOR_UINT16:    return "xor_uint16";
    case DATATYPE_XOR_UINT32:    return "xor_uint32";
    case DATATYPE_XOR_UINT64:    return "xor_uint64";
    case DATATYPE_FLOAT32:       return "float32";
    case DATATYPE_FLOAT64:       return "float64";
    case DATATYPE_STRING:        return "string";
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

#define X CAST_FORBIDDEN
#define S CAST_EQUAL
#define I CAST_IMPLICIT
#define E CAST_EXPLICIT

const CastStyle dataTypeCasts[NUM_DATATYPES][NUM_DATATYPES] = {
    //  x   U   B   S   N   I8  I16 I32 I64 U8  U16 U32 U64 X8  X16 X32 X64 F32 F64
       {X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X},     // DATATYPE_INVALID,
       {X,  S,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X},     // DATATYPE_UNIT,
       {X,  X,  S,  X,  X,  E,  E,  E,  E,  E,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_BOOL,
       {X,  X,  X,  S,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X},     // DATATYPE_STRING,
       {X,  X,  X,  X,  S,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I},     // DATATYPE_NUMERIC,
       {X,  X,  E,  X,  X,  S,  E,  E,  E,  E,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_INT8,
       {X,  X,  E,  X,  X,  E,  S,  E,  E,  E,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_INT16,
       {X,  X,  E,  X,  X,  E,  E,  S,  E,  E,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_INT32,
       {X,  X,  E,  X,  X,  E,  E,  E,  S,  E,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_INT64,
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  S,  E,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_UINT8,
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  S,  E,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_UINT16,
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  E,  S,  E,  X,  X,  X,  X,  E,  E},     // DATATYPE_UINT32,
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  E,  E,  S,  E,  E,  E,  E,  X,  X},     // DATATYPE_UINT64,
       {X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  E,  S,  E,  E,  E,  X,  X},     // DATATYPE_XOR_UINT8
       {X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  E,  E,  S,  E,  E,  X,  X},     // DATATYPE_XOR_UINT16
       {X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  E,  E,  E,  S,  E,  X,  X},     // DATATYPE_XOR_UINT32
       {X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  E,  E,  E,  E,  S,  X,  X},     // DATATYPE_XOR_UINT64
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  E,  E,  X,  X,  X,  X,  X,  S,  E},     // DATATYPE_FLOAT32
       {X,  X,  E,  X,  X,  E,  E,  E,  E,  E,  E,  E,  X,  X,  X,  X,  X,  E,  S}      // DATATYPE_FLOAT64
};

#undef X
#undef S
#undef I
#undef E

CastStyle getCastStyle (SecrecDataType from, SecrecDataType to) {
    assert (from < NUM_DATATYPES && to < NUM_DATATYPES);
    return dataTypeCasts[from][to];
}

} // namespace anonymous

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

bool isNumericDataType (SecrecDataType dType) {
    return isSignedNumericDataType (dType) ||
           isUnsignedNumericDataType (dType);
}

bool isSignedNumericDataType (SecrecDataType dType) {
    switch (dType) {
    case DATATYPE_INT8:
    case DATATYPE_INT16:
    case DATATYPE_INT32:
    case DATATYPE_INT64:
    case DATATYPE_FLOAT32:
    case DATATYPE_FLOAT64:
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

/*******************************************************************************
  DataTypeBasic
*******************************************************************************/

std::ostream& DataTypeBasic::print (std::ostream & os) const {
    os << "(" << *m_secType << ","
       << m_dataType << ","
       << m_dimType << ")";
    return os;
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

/*******************************************************************************
  DataTypeVar
*******************************************************************************/

std::ostream& DataTypeVar::print (std::ostream & os) const {
    os << "VAR" << m_dataType;
    return os;
}

DataTypeVar* DataTypeVar::get (Context& cxt, DataType* base) {
    ContextImpl& impl = *cxt.pImpl ();
    return impl.varType (base);
}

/*******************************************************************************
  DataTypeProcedureVoid
*******************************************************************************/

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

std::ostream& DataTypeProcedureVoid::print (std::ostream & os) const {
    os << mangle () << " -> void";
    return os;
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

std::ostream& DataTypeProcedure::print (std::ostream& os) const {
    os << mangle() << " -> " << *m_ret;
    return os;
}

} // namespace SecreC

/*******************************************************************************
  Global functions
*******************************************************************************/

std::ostream &operator<<(std::ostream &out, const SecrecDataType &type) {
    out << SecreC::SecrecFundDataTypeToString(type);
    return out;
}
