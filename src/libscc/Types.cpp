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

#include "Types.h"

#include "DataType.h"
#include "Context.h"
#include "ContextImpl.h"
#include "Misc.h"
#include "Symbol.h"

#include <cassert>
#include <iostream>
#include <sstream>

#include <boost/flyweight.hpp>
#include <boost/flyweight/key_value.hpp>
#include <boost/flyweight/no_locking.hpp>
#include <boost/flyweight/no_tracking.hpp>

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
  Type
*******************************************************************************/

bool Type::isPublicUIntScalar () const {
    if (m_kind != BASIC)
        return false;

    return secrecDataType ()->equals (DATATYPE_UINT64) &&
           secrecSecType ()->isPublic () &&
           secrecDimType () == 0;
}

bool Type::isString () const {
    if (m_kind != BASIC)
        return false;

    return secrecDataType ()->equals (DATATYPE_STRING);
}

bool Type::isFloat () const {
    if (m_kind != BASIC)
        return false;

    return secrecDataType ()->equals (DATATYPE_FLOAT32) ||
           secrecDataType ()->equals (DATATYPE_FLOAT64);
}


/*******************************************************************************
  TypeVoid
*******************************************************************************/

void TypeVoid::printPrettyV (std::ostream& os) const {
    os << "void";
}

const TypeVoid* TypeVoid::get () {
    static const TypeVoid voidType;
    return &voidType;
}

/*******************************************************************************
  TypeNonVoid
*******************************************************************************/

bool TypeNonVoid::latticeLEQ (const TypeNonVoid* other) const {
    if (kind () != other->kind ())
        return false;

    const DataType* dataType = other->secrecDataType ();
    if (other->secrecSecType ()->isPrivate () && secrecSecType ()->isPublic ()) {
        dataType = dtypeDeclassify (dataType);
    }

    return     latticeSecTypeLEQ (secrecSecType (), other->secrecSecType ())
            && latticeDataTypeLEQ (secrecDataType (), dataType)
            && latticeDimTypeLEQ (secrecDimType (), other->secrecDimType ());
}

/*******************************************************************************
  TypeBasic
*******************************************************************************/

void TypeBasic::printPrettyV (std::ostream& os) const {
    if (!secrecSecType ()->isPublic())
        os << *secrecSecType () << ' ';
    os << *secrecDataType ();
    if (secrecDimType () > 0)
        os << "[[" << secrecDimType () << "]]";
}

const TypeBasic* TypeBasic::get (SecrecDataType dataType,
                                 SecrecDimType dimType)
{
    return TypeBasic::get (PublicSecType::get (), dataType, dimType);
}

const TypeBasic* TypeBasic::get (const DataType* dataType, SecrecDimType dimType) {
    return TypeBasic::get (PublicSecType::get (), dataType, dimType);
}

const TypeBasic* TypeBasic::get (const SecurityType* secType,
                                 SecrecDataType dataType,
                                 SecrecDimType dimType)
{
    return TypeBasic::get (secType, DataTypePrimitive::get (dataType), dimType);
}

const TypeBasic* TypeBasic::get (const SecurityType* secType,
                                 const DataType* dataType,
                                 SecrecDimType dimType)
{
    using namespace ::boost::flyweights;
    using TypeBasicFlyweigh =
        flyweight<
            key_value<
                std::tuple<const SecurityType*, const DataType*, SecrecDimType>,
                TypeBasic
            >,
            no_tracking,
            no_locking
        >;

    return &TypeBasicFlyweigh{secType, dataType, dimType}.get();
}

const TypeBasic* TypeBasic::getIndexType ()
{
    return TypeBasic::get (PublicSecType::get (), DATATYPE_UINT64);
}

const TypeBasic* TypeBasic::getPublicBoolType ()
{
    return TypeBasic::get (PublicSecType::get (), DATATYPE_BOOL);
}

/*******************************************************************************
  TypeProc
*******************************************************************************/

void TypeProc::printPrettyV (std::ostream& os) const {
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

const TypeProc* TypeProc::get (const std::vector<const TypeBasic*>& params,
                               const Type* returnType)
{
    if (returnType == nullptr)
        return TypeProc::get (params, TypeVoid::get ());

    using namespace ::boost::flyweights;
    using TypeProcFlyweigh =
        flyweight<
            key_value<
                std::pair<const std::vector<const TypeBasic*>, const Type*>,
                TypeProc
            >,
            no_tracking,
            no_locking
        >;

    return &TypeProcFlyweigh{params, returnType}.get();
}


} // namespace SecreC
