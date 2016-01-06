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

TypeVoid* TypeVoid::get (Context& cxt) {
    ContextImpl& impl = *cxt.pImpl ();
    return &impl.m_voidType;
}

/*******************************************************************************
  TypeNonVoid
*******************************************************************************/

bool TypeNonVoid::latticeLEQ (Context& cxt, const TypeNonVoid* other) const {
    if (kind () != other->kind ())
        return false;

    const DataType* dataType = other->secrecDataType ();
    if (other->secrecSecType ()->isPrivate () && secrecSecType ()->isPublic ()) {
        dataType = dtypeDeclassify (cxt, dataType);
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

const TypeBasic* TypeBasic::get (Context& cxt, SecrecDataType dataType,
                                 SecrecDimType dimType)
{
    return TypeBasic::get (cxt, PublicSecType::get (cxt), dataType, dimType);
}

const TypeBasic* TypeBasic::get (Context& cxt, const DataType* dataType, SecrecDimType dimType) {
    return TypeBasic::get (cxt, PublicSecType::get (cxt), dataType, dimType);
}

const TypeBasic* TypeBasic::get (Context& cxt, const SecurityType* secType,
                                 SecrecDataType dataType,
                                 SecrecDimType dimType)
{
    return TypeBasic::get (cxt, secType, DataTypePrimitive::get (cxt, dataType), dimType);
}

const TypeBasic* TypeBasic::get (Context& cxt, const SecurityType* secType,
                                 const DataType* dataType,
                                 SecrecDimType dimType)
{
    auto& map = cxt.pImpl ()->m_basicTypes;
    const auto index = std::make_tuple (secType, dataType, dimType);
    auto i = map.find (index);
    if (i == map.end ()) {
        i = map.insert (i, std::make_pair (index, new TypeBasic (secType, dataType, dimType)));
    }

    return i->second;
}

const TypeBasic* TypeBasic::getIndexType (Context& cxt)
{
    return TypeBasic::get (cxt, PublicSecType::get (cxt), DATATYPE_UINT64);
}

const TypeBasic* TypeBasic::getPublicBoolType (Context& cxt)
{
    return TypeBasic::get (cxt, PublicSecType::get (cxt), DATATYPE_BOOL);
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

const TypeProc* TypeProc::get (Context& cxt,
                               const std::vector<const TypeBasic*>& params,
                               const Type* returnType)
{
    if (returnType == nullptr)
        return TypeProc::get (cxt, params, TypeVoid::get (cxt));

    auto& map = cxt.pImpl ()->m_procTypes;
    const auto index = std::make_pair (returnType, params);
    auto i = map.find (index);
    if (i == map.end ()) {
        i = map.insert (i, std::make_pair (index, new TypeProc (params, returnType)));
    }

    return i->second;
}


} // namespace SecreC
