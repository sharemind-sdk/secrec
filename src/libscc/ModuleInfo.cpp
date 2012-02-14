/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "ModuleInfo.h"

using namespace boost;

namespace SecreC {

/*******************************************************************************
  ModuleInfo
*******************************************************************************/

std::string ModuleInfo::fileNameStem () const {
    return m_location.path ().stem ().string ();
}

void ModuleInfo::setCodeGenState (const CodeGenState& state) {
    m_cgState = state;
}


} // namespace SecreC
