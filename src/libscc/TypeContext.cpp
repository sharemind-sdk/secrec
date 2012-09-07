/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TypeContext.h"

#include <sstream>
#include "misc.h"


namespace SecreC {

/*******************************************************************************
  TypeContext
*******************************************************************************/

std::string TypeContext::toString () const {
    std::ostringstream os;
    os << '(';
    if (haveContextSecType ())
        os << *contextSecType ();
    else
        os << '*';
    os << ',';
    if (haveContextDataType ())
        os << SecrecFundDataTypeToString(contextDataType());
    else
        os << '*';
    os << ',';
    if (haveContextDimType ())
        os << contextDimType ();
    else
        os << '*';
    os << ')';
    return os.str ();
}

} // namespace SecreC
