/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "ModuleInfo.h"

#include "Context.h"
#include "ContextImpl.h"
#include "TreeNode.h"
#include "Parser.h"

#include <boost/filesystem.hpp>

using namespace boost;

namespace SecreC {

/*******************************************************************************
  ModuleInfo
*******************************************************************************/

ModuleInfo::~ModuleInfo() {
    delete m_body;
}

std::string ModuleInfo::fileNameStem () const {
    return m_location.path ().stem ().string ();
}

void ModuleInfo::setCodeGenState (const CodeGenState& state) {
    m_cgState = state;
}

bool ModuleInfo::read() {
    using namespace boost;
    assert (m_body == nullptr);
    const char* fname = m_location.path ().c_str ();
    FILE* f = fopen (fname, "r");
    if (f == nullptr) {
        std::cerr << "Was not able to open file \"" << fname << "\"." << std::endl;
        return false;
    }

    ContextImpl* pImpl = m_cxt.pImpl ();
    TreeNodeModule* treeNode = nullptr;
    int parseResult = sccparse_file(&pImpl->stringTable (), m_location.path().c_str (), f, &treeNode);
    fclose (f);
    if (parseResult != 0 || treeNode == nullptr) {
        return false;
    }

    m_body = treeNode;
    return true;
}

} // namespace SecreC
