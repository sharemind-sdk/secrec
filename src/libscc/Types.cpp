#include "Types.h"

#include "Context.h"
#include "ContextImpl.h"
#include "Misc.h"
#include "Symbol.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace SecreC {

namespace /* anonymous */ {

std::string mangleDataType (const Type* ty) {
    std::ostringstream os;
    os << '(';
    os << *ty->secrecSecType () << ',';
    os << *ty->secrecDataType() << ',';
    os << ty->secrecDimType();
    os << ')';
    return os.str ();
}

} // namespace anonymous

/*******************************************************************************
  TypeVoid
*******************************************************************************/

void TypeVoid::print (std::ostream & os) const {
    os << "void";
}

void TypeVoid::prettyPrint (std::ostream& os) const {
    os << "void";
}

TypeVoid* TypeVoid::get (Context& cxt) {
    ContextImpl& impl = *cxt.pImpl ();
    return &impl.m_voidType;
}

/*******************************************************************************
  TypeBasic
*******************************************************************************/

void TypeBasic::print (std::ostream& os) const {
    os << mangleDataType (this);
}

void TypeBasic::prettyPrint (std::ostream& os) const {
    if (!secrecSecType ()->isPublic())
        os << *secrecSecType () << ' ';
    os << *secrecDataType ();
    if (secrecDimType () > 0)
        os << "[[" << secrecDimType () << "]]";
}

TypeBasic* TypeBasic::get (Context& cxt, SecrecDataType dataType,
                           SecrecDimType dimType)
{
    return TypeBasic::get (cxt, PublicSecType::get (cxt), dataType, dimType);
}

TypeBasic* TypeBasic::get (Context& cxt, DataType* dataType, SecrecDimType dimType) {
    return TypeBasic::get (cxt, PublicSecType::get (cxt), dataType, dimType);
}

TypeBasic* TypeBasic::get (Context& cxt, SecurityType* secType,
                           SecrecDataType dataType,
                           SecrecDimType dimType)
{
    return TypeBasic::get (cxt, secType, DataTypePrimitive::get (cxt, dataType), dimType);
}

TypeBasic* TypeBasic::get (Context& cxt, SecurityType* secType,
                           DataType* dataType,
                           SecrecDimType dimType)
{
    typedef ContextImpl::TypeBasicMap Map;
    Map& map = cxt.pImpl ()->m_basicTypes;
    const Map::key_type index (secType, dataType, dimType);
    auto i = map.find (index);
    if (i == map.end ()) {
        i = map.insert (i, Map::value_type (index,
            new TypeBasic (secType, dataType, dimType)));
    }

    return i->second;
}

TypeBasic* TypeBasic::getIndexType (Context& cxt)
{
    return TypeBasic::get (cxt, PublicSecType::get (cxt), DATATYPE_UINT64);
}

TypeBasic* TypeBasic::getPublicBoolType (Context& cxt)
{
    return TypeBasic::get (cxt, PublicSecType::get (cxt), DATATYPE_BOOL);
}

/*******************************************************************************
  TypeProc
*******************************************************************************/

void TypeProc::print (std::ostream & os) const {
    os << mangle () << " -> " << *returnType ();
}

void TypeProc::prettyPrint (std::ostream& os) const {
    os << PrettyPrint (returnType ()) << " ()" << paramsToNormalString ();
}

std::string TypeProc::paramsToNormalString () const {
    std::ostringstream oss;
    oss << '(';
    for (auto it = m_params.begin (); it != m_params.end (); ++ it) {
        if (it != m_params.begin ())
            oss << ", ";
        oss << PrettyPrint (*it);
    }
    oss << ')';
    return oss.str();
}

std::string TypeProc::mangle () const {
    std::ostringstream os;
    os << "(";
    for (auto it = m_params.begin (); it != m_params.end (); ++ it) {
        if (it != m_params.begin ())
            os << ", ";
        os << mangleDataType (*it);
    }
    os << ")";
    return os.str();
}

TypeProc* TypeProc::get (Context& cxt,
                         const std::vector<TypeBasic*>& params,
                         Type* returnType)
{
    if (returnType == nullptr)
        return TypeProc::get (cxt, params, TypeVoid::get (cxt));

    typedef ContextImpl::TypeProcMap Map;
    Map& map = cxt.pImpl ()->m_procTypes;
    const Map::key_type index (returnType, params);
    auto i = map.find (index);
    if (i == map.end ()) {
        i = map.insert (i, Map::value_type (index,
            new TypeProc (params, returnType)));
    }

    return i->second;
}


} // namespace SecreC
