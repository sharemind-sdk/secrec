#include "constant.h"

#include <string>
#include <sstream>

#include <boost/foreach.hpp>

#include "context.h"
#include "context_impl.h"

namespace SecreC {

namespace /* anonymous */ {

std::string escape (const std::string& str) {
    std::ostringstream os;

    os << '\"';
    BOOST_FOREACH (char c, str) {
        switch (c) {
        case '\'': os << "\\\'"; break;
        case '\"': os << "\\\""; break;
        case '\?': os << "\\?";  break;
        case '\\': os << "\\\\"; break;
        case '\a': os << "\\a";  break;
        case '\b': os << "\\b";  break;
        case '\f': os << "\\f";  break;
        case '\n': os << "\\n";  break;
        case '\r': os << "\\r";  break;
        case '\t': os << "\\t";  break;
        case '\v': os << "\\v";  break;
        default:   os << c;      break;
        }
    }

    os << "\\0\"";
    return os.str ();
}

} // anonymous namesace

const char* SecrecTypeInfo<DATATYPE_BOOL>::CName = "bool";
const char* SecrecTypeInfo<DATATYPE_STRING>::CName = "string";
const char* SecrecTypeInfo<DATATYPE_INT8>::CName = "int8";
const char* SecrecTypeInfo<DATATYPE_UINT8>::CName = "uint8";
const char* SecrecTypeInfo<DATATYPE_INT16>::CName = "int16";
const char* SecrecTypeInfo<DATATYPE_UINT16>::CName = "uint16";
const char* SecrecTypeInfo<DATATYPE_INT32>::CName = "int32";
const char* SecrecTypeInfo<DATATYPE_UINT32>::CName = "uint32";
const char* SecrecTypeInfo<DATATYPE_INT64>::CName = "int64";
const char* SecrecTypeInfo<DATATYPE_UINT64>::CName = "uint64";
const char* SecrecTypeInfo<DATATYPE_FLOAT32>::CName = "float32";
const char* SecrecTypeInfo<DATATYPE_FLOAT64>::CName = "float64";


/*******************************************************************************
  Constant
*******************************************************************************/

template <SecrecDataType ty>
Constant<ty>* Constant<ty>::get (Context &cxt, const typename Constant<ty>::CType& value) {
    ContextImpl& impl = *cxt.pImpl ();
    const std::pair<SecrecDataType, uint64_t> index (ty, value);
    ContextImpl::NumericConstantMap::iterator i = impl.m_numericConstants.find (index);
    if (i == impl.m_numericConstants.end ()) {
        TypeNonVoid* tnv = TypeNonVoid::get (cxt, ty);
        i = impl.m_numericConstants.insert (i,
            make_pair (index, new Constant<ty>(value, tnv)));

        std::ostringstream os;
        os << "{const " << SecrecTypeInfo<ty>::CName << "}" << static_cast<uint64_t>(value);
        i->second->setName (os.str ());
    }

    assert (dynamic_cast<Constant<ty>* >(i->second) != 0);
    return static_cast<Constant<ty>* >(i->second);
}

template <SecrecDataType ty>
void Constant<ty>::print (std::ostream& os) const {
    os << static_cast<uint64_t>(m_value);
}

/*******************************************************************************
  ConstantBool
*******************************************************************************/

template <>
ConstantBool* ConstantBool::get (Context& cxt, const bool& value) {
    ContextImpl& impl = *cxt.pImpl ();

    if (impl.m_trueConstant == 0) {
        impl.m_trueConstant = new ConstantBool (true,
            TypeNonVoid::get (cxt, DATATYPE_BOOL));
        impl.m_trueConstant->setName ("{const bool}true");
    }

    if (impl.m_falseConstant == 0) {
        impl.m_falseConstant = new ConstantBool (false,
            TypeNonVoid::get (cxt, DATATYPE_BOOL));
        impl.m_falseConstant->setName ("{const bool}false");
    }

    return value ? impl.m_trueConstant : impl.m_falseConstant;
}

template <>
void ConstantBool::print (std::ostream& os) const {
    os << (m_value ? "true" : "false");
}

/*******************************************************************************
  ConstantString
*******************************************************************************/

template <>
ConstantString* ConstantString::get (Context &cxt, const std::string& value) {
    ContextImpl& impl = *cxt.pImpl ();
    std::map<std::string, ConstantString*>::iterator
            i = impl.m_stringLiterals.find (value);
    if (i == impl.m_stringLiterals.end ()) {
        TypeNonVoid* tnv = TypeNonVoid::get (cxt, DATATYPE_STRING);
        i = impl.m_stringLiterals.insert (i,
            std::make_pair (value, new ConstantString (value, tnv)));
        std::ostringstream os;
        os << "{const string}" << escape (value);
        i->second->setName (os.str ());
    }

    return i->second;
}

template <>
void ConstantString::print (std::ostream& os) const {
    os << m_value;
}

/*******************************************************************************
  ConstantFloat32
*******************************************************************************/

template <>
void ConstantFloat32::print (std::ostream& os) const {
    os << m_value;
}

/*******************************************************************************
  ConstantFloat64
*******************************************************************************/

template <>
void ConstantFloat64::print (std::ostream& os) const {
    os << m_value;
}

SymbolConstant* defaultConstant (Context& cxt, SecrecDataType ty) {
    switch (ty) {
    case DATATYPE_BOOL:   return ConstantBool::get (cxt, false); break;
    case DATATYPE_STRING: return ConstantString::get (cxt, ""); break;
    default:              return numericConstant (cxt, ty, 0); break;
    }
}

SymbolConstant* numericConstant (Context& cxt, SecrecDataType ty, uint64_t value) {
    switch (ty) {
    case DATATYPE_INT8: return ConstantInt8::get (cxt, value); break;
    case DATATYPE_UINT8: return ConstantUInt8::get (cxt, value); break;
    case DATATYPE_INT16: return ConstantInt16::get (cxt, value); break;
    case DATATYPE_UINT16: return ConstantUInt16::get (cxt, value); break;
    case DATATYPE_INT32: return ConstantInt32::get (cxt, value); break;
    case DATATYPE_UINT32: return ConstantUInt32::get (cxt, value); break;
    case DATATYPE_INT64: return ConstantInt64::get (cxt, value); break;
    case DATATYPE_UINT64: return ConstantUInt64::get (cxt, value); break;
    case DATATYPE_XOR_UINT8: return ConstantUInt8::get (cxt, value); break;
    case DATATYPE_XOR_UINT16: return ConstantUInt16::get (cxt, value); break;
    case DATATYPE_XOR_UINT32: return ConstantUInt32::get (cxt, value); break;
    case DATATYPE_XOR_UINT64: return ConstantUInt64::get (cxt, value); break;
    case DATATYPE_FLOAT32: {
            uint32_t i_val;
            const float f_val = static_cast<float>(value);
            memcpy (&i_val, &f_val, sizeof (float));
            return ConstantFloat32::get (cxt, i_val);
        }
    case DATATYPE_FLOAT64: {
            uint64_t i_val;
            const double f_val = static_cast<double>(value);
            memcpy (&i_val, &f_val, sizeof (double));
            return ConstantFloat64::get (cxt, i_val);
        }
    default:
        assert (false && "Not numeric constant");
        return 0;
    }
}

template class Constant<DATATYPE_STRING>;
template class Constant<DATATYPE_BOOL>;
template class Constant<DATATYPE_INT8>;
template class Constant<DATATYPE_UINT8>;
template class Constant<DATATYPE_INT16>;
template class Constant<DATATYPE_UINT16>;
template class Constant<DATATYPE_INT32>;
template class Constant<DATATYPE_UINT32>;
template class Constant<DATATYPE_INT64>;
template class Constant<DATATYPE_UINT64>;
template class Constant<DATATYPE_FLOAT32>;
template class Constant<DATATYPE_FLOAT64>;

/******************************************************************
  ConstantVector
******************************************************************/

template <SecrecDataType ty>
ConstantVector<ty>* ConstantVector<ty>::get (Context& cxt, const std::vector<SymbolConstant*>& values) {
    ContextImpl& impl = *cxt.pImpl ();
    ContextImpl::ConstantVectorMap::iterator i = impl.m_constantVectors.find (values);
    if (i == impl.m_constantVectors.end ()) {
        TypeNonVoid* tnv = TypeNonVoid::get (cxt, ty, 1);
        ConstantVector<ty>* newVec = new ConstantVector<ty>(tnv, values);
        i = impl.m_constantVectors.insert (i, std::make_pair (values, newVec));

        std::ostringstream ss;
        ss << "{const " << value_trait::CName << " vec}";
        newVec->print (ss);
        newVec->setName (ss.str ());
    }

    return static_cast<ConstantVector<ty>*>(i->second);
}

template <SecrecDataType ty>
void ConstantVector<ty>::print (std::ostream& os) const {
    os << "{";
    for (size_t i = 0; i < m_values.size (); ++ i) {
        if (i != 0)
            os << ", ";

        os << *at (i);
    }

    os << '}';
}


template class ConstantVector<DATATYPE_BOOL>;
template class ConstantVector<DATATYPE_INT8>;
template class ConstantVector<DATATYPE_UINT8>;
template class ConstantVector<DATATYPE_INT16>;
template class ConstantVector<DATATYPE_UINT16>;
template class ConstantVector<DATATYPE_INT32>;
template class ConstantVector<DATATYPE_UINT32>;
template class ConstantVector<DATATYPE_INT64>;
template class ConstantVector<DATATYPE_UINT64>;
template class ConstantVector<DATATYPE_FLOAT32>;
template class ConstantVector<DATATYPE_FLOAT64>;

} // namespace SecreC
