#include "types.h"

#include <cassert>
#include <sstream>
#include <iostream>
#include <boost/foreach.hpp>

#include "symbol.h"
#include "context.h"
#include "context_impl.h"

namespace SecreC {

namespace /* anonymous */ {

/// \todo refactor TypeNonVoid::Kind and DataType::Kind to one.
TypeNonVoid::Kind kindToKind (DataType::Kind k) {
    switch (k) {
        case DataType::BASIC:         return TypeNonVoid::BASIC;
        case DataType::PROCEDURE:     return TypeNonVoid::PROCEDURE;
        case DataType::PROCEDUREVOID: return TypeNonVoid::PROCEDUREVOID;
    }
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
    return impl.voidType ();
}

/*******************************************************************************
  TypeNonVoid
*******************************************************************************/

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
    return TypeNonVoid::get (cxt, DATATYPE_UINT64);
}

TypeNonVoid* TypeNonVoid::getPublicBoolType (Context& cxt) {
    return TypeNonVoid::get (cxt, DATATYPE_BOOL);
}

TypeNonVoid::TypeNonVoid(DataType* dataType)
     : Type (false)
     , m_kind (kindToKind (dataType->kind ()))
     , m_dataType (dataType)
{ }

void TypeNonVoid::print (std::ostream& os) const {
    os << *m_dataType;
}

void TypeNonVoid::prettyPrint (std::ostream& os) const {
    os << DataType::PrettyPrint (m_dataType);
}

} // namespace SecreC
