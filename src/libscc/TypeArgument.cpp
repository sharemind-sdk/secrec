/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TypeArgument.h"

#include "StringRef.h"
#include "symbol.h"

namespace SecreC {


/*******************************************************************************
  TypeArgument
*******************************************************************************/

SymbolTypeVariable* TypeArgument::bind (StringRef name) const {
    switch (m_kind) {
    case TA_UNDEF: return NULL;
    case TA_SEC:   return new SymbolDomain (name, secType ());
    case TA_DATA:  return new SymbolDataType (name, dataType ());
    case TA_DIM:   return new SymbolDimensionality (name, dimType ());
    }
}

} // namespace SecreC
