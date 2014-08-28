/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TypeContext.h"

#include <ostream>
#include "misc.h"

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

} // namespace SecreC
