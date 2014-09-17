/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
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
    setContextDataType (DataTypePrimitive::get (cxt, DATATYPE_UINT64));
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

    return dataType == m_contextDataType;
}

} // namespace SecreC
