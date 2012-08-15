/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "ModuleInfo.h"

#include <boost/filesystem.hpp>

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


bool ModuleInfo::read() {
    using namespace boost;
    assert (m_body == 0);
    FILE* f = fopen (m_location.path ().c_str (), "r");
    if (f == 0)
        return false;

    TreeNodeModule* treeNode = 0;
    int parseResult = sccparse_file (f, &treeNode);
    fclose (f);
    if (parseResult != 0 || treeNode == 0)
        return false;

    m_body = treeNode;
    return true;
}

} // namespace SecreC
