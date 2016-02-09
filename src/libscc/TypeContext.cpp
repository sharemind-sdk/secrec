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

#include "TypeContext.h"

#include "DataType.h"
#include "Misc.h"
#include "SecurityType.h"
#include "Types.h"

#include <ostream>

namespace SecreC {

/*******************************************************************************
  TypeContext
*******************************************************************************/

void TypeContext::prettyPrint (std::ostream& os) const {
    bool foundAny = false;

    if (haveContextSecType ()) {
        foundAny = true;
        os << *contextSecType ();
    }

    if (haveContextDataType ()) {
        if (foundAny) {
            os << ", ";
            foundAny = true;
        }

        os << *contextDataType();
    }

    if (haveContextDimType ()) {
        if (foundAny) {
            os << ", ";
            foundAny = true;
        }

        os << contextDimType ();
    }

    if (! foundAny)
        os << '*';
}

void TypeContext::setContextIndexType (Context& cxt) {
    setContextDataType (DataTypeBuiltinPrimitive::get (cxt, DATATYPE_UINT64));
    setContextDimType (0);
    setContextSecType (PublicSecType::get (cxt));
}

void TypeContext::setContext (TypeNonVoid* ty) {
    assert (ty != nullptr);
    setContextDataType (ty->secrecDataType ());
    setContextSecType (ty->secrecSecType ());
    setContextDimType (ty->secrecDimType ());
}

bool TypeContext::matchType (TypeNonVoid* type) const {
    return matchSecType (type->secrecSecType ()) &&
           matchDataType (type->secrecDataType ()) &&
           matchDimType (type->secrecDimType ());
}

bool TypeContext::matchSecType (SecurityType* secType) const {
    assert (secType != nullptr);
    if (! haveContextSecType ()) {
        return true;
    }

    return secType == contextSecType ();
}

bool TypeContext::matchDataType (DataType* dataType) const {
    assert (dataType != nullptr);
    if (! haveContextDataType ()) {
        return true;
    }

    return dataType->equals (m_contextDataType);
}

} // namespace SecreC
