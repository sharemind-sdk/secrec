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
