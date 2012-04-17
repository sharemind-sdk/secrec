#include "constant.h"

#include <string>
#include <sstream>

#include <boost/foreach.hpp>

#include "context.h"
#include "context_impl.h"

namespace {

using namespace SecreC;
using namespace std;

template <SecrecDataType ty>
Constant<ty>* getNumeric (Context& cxt,
                          const typename SecrecTypeInfo<ty>::CType& value) {
    ContextImpl& impl = *cxt.pImpl ();
    const pair<SecrecDataType, uint64_t> index (ty, value);
    map<pair<SecrecDataType, uint64_t>, Symbol*>::iterator
        i = impl.m_numericConstants.find (index);
    if (i == impl.m_numericConstants.end ()) {
        TypeNonVoid* tnv = TypeNonVoid::get (cxt, ty);
        i = impl.m_numericConstants.insert (i,
            make_pair (index, new Constant<ty>(value, tnv)));
        ostringstream os;
        os << "{const" << SecrecTypeInfo<ty>::CName << "}" << static_cast<uint64_t>(value);
        i->second->setName (os.str ());
    }

    assert (dynamic_cast<Constant<ty>* >(i->second) != 0);
    return static_cast<Constant<ty>* >(i->second);
}

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

namespace SecreC {

const char* SecrecTypeInfo<DATATYPE_BOOL>::CName = "Bool";
const char* SecrecTypeInfo<DATATYPE_STRING>::CName = "String";
const char* SecrecTypeInfo<DATATYPE_INT8>::CName = "Int8";
const char* SecrecTypeInfo<DATATYPE_UINT8>::CName = "UInt8";
const char* SecrecTypeInfo<DATATYPE_INT16>::CName = "Int16";
const char* SecrecTypeInfo<DATATYPE_UINT16>::CName = "UInt16";
const char* SecrecTypeInfo<DATATYPE_INT32>::CName = "Int32";
const char* SecrecTypeInfo<DATATYPE_UINT32>::CName = "UInt32";
const char* SecrecTypeInfo<DATATYPE_INT64>::CName = "Int64";
const char* SecrecTypeInfo<DATATYPE_UINT64>::CName = "UInt64";


/*******************************************************************************
  SymbolConstantBool
*******************************************************************************/

template <>
ConstantBool* ConstantBool::get (Context& cxt, const bool& value) {
    ContextImpl& impl = *cxt.pImpl ();

    if (impl.m_trueConstant == 0) {
        impl.m_trueConstant = new ConstantBool (true,
            TypeNonVoid::get (cxt, DATATYPE_BOOL));
        impl.m_trueConstant->setName ("{constBool}true");
    }

    if (impl.m_falseConstant == 0) {
        impl.m_falseConstant = new ConstantBool (false,
            TypeNonVoid::get (cxt, DATATYPE_BOOL));
        impl.m_falseConstant->setName ("{constBool}false");
    }

    return value ? impl.m_trueConstant : impl.m_falseConstant;
}

/*******************************************************************************
  SymbolConstantInt8
*******************************************************************************/

template <>
ConstantInt8* ConstantInt8::get (Context &cxt, const CType &value) {
    return getNumeric<DATATYPE_INT8> (cxt, value);
}

/*******************************************************************************
  SymbolConstantInt16
*******************************************************************************/

template <>
ConstantInt16* ConstantInt16::get (Context &cxt, const CType &value) {
    return getNumeric<DATATYPE_INT16> (cxt, value);
}

/*******************************************************************************
  SymbolConstantInt32
*******************************************************************************/

template <>
ConstantInt32* ConstantInt32::get (Context &cxt, const CType &value) {
    return getNumeric<DATATYPE_INT32> (cxt, value);
}

/*******************************************************************************
  SymbolConstantInt64
*******************************************************************************/

template <>
ConstantInt64* ConstantInt64::get (Context &cxt, const CType &value) {
    return getNumeric<DATATYPE_INT64> (cxt, value);
}

/*******************************************************************************
  SymbolConstantString
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
        os << "{constString}" << escape (value);
        i->second->setName (os.str ());
    }

    return i->second;
}

/*******************************************************************************
  SymbolConstantUInt8
*******************************************************************************/

template <>
ConstantUInt8* ConstantUInt8::get (Context &cxt, const CType &value) {
    return getNumeric<DATATYPE_UINT8> (cxt, value);
}

/*******************************************************************************
  SymbolConstantUInt16
*******************************************************************************/

template <>
ConstantUInt16* ConstantUInt16::get (Context &cxt, const CType &value) {
    return getNumeric<DATATYPE_UINT16> (cxt, value);
}

/*******************************************************************************
  SymbolConstantUInt32
*******************************************************************************/

template <>
ConstantUInt32* ConstantUInt32::get (Context &cxt, const CType &value) {
    return getNumeric<DATATYPE_UINT32> (cxt, value);
}

/*******************************************************************************
  SymbolConstantUInt64
*******************************************************************************/

template <>
ConstantUInt64* ConstantUInt64::get (Context &cxt, const CType &value) {
    return getNumeric<DATATYPE_UINT64> (cxt, value);
}


Symbol* defaultConstant (Context& cxt, SecrecDataType ty) {
    switch (ty) {
    case DATATYPE_BOOL:   return ConstantBool::get (cxt, false); break;
    case DATATYPE_STRING: return ConstantString::get (cxt, ""); break;
    default:              return  numericConstant (cxt, ty, 0); break;
    }
}

Symbol* numericConstant (Context& cxt, SecrecDataType ty, uint64_t value) {
    switch (ty) {
    case DATATYPE_INT8:   return ConstantInt8::get (cxt, value); break;
    case DATATYPE_UINT8:  return ConstantUInt8::get (cxt, value); break;
    case DATATYPE_INT16:  return ConstantInt16::get (cxt, value); break;
    case DATATYPE_UINT16: return ConstantUInt16::get (cxt, value); break;
    case DATATYPE_INT32:  return ConstantInt32::get (cxt, value); break;
    case DATATYPE_UINT32: return ConstantUInt32::get (cxt, value); break;
    case DATATYPE_INT64:  return ConstantInt64::get (cxt, value); break;
    case DATATYPE_UINT64: return ConstantUInt64::get (cxt, value); break;
    case DATATYPE_XOR_UINT8: return ConstantUInt8::get (cxt, value); break;
    case DATATYPE_XOR_UINT16: return ConstantUInt16::get (cxt, value); break;
    case DATATYPE_XOR_UINT32: return ConstantUInt32::get (cxt, value); break;
    case DATATYPE_XOR_UINT64: return ConstantUInt64::get (cxt, value); break;
    default:
        assert (false && "Not numeric constant");
        return 0;
    }
}

}
