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

#include "ModuleMap.h"

#include "ModuleInfo.h"

#include <boost/range/adaptors.hpp>
#include <boost/range/iterator_range.hpp>

using namespace boost;

namespace SecreC {

/*******************************************************************************
  ModuleMap
*******************************************************************************/

ModuleMap::~ModuleMap () {
    for (ModuleInfo* modInfo : adaptors::values (m_modules)) {
        delete modInfo;
    }
}

bool ModuleMap::addModule (const std::string& name, std::unique_ptr<ModuleInfo> info) {
    assert (info.get () != nullptr);
    auto it = m_modules.find (name);
    if (it != m_modules.end ())
        return false;
    m_modules.insert (it, std::make_pair (name, info.get ()));
    info.release ();
    return true;
}

bool ModuleMap::addSearchPath (const std::string& pathName) {
    using namespace boost::filesystem;

    const path p (pathName);

    try  {
        if (! exists (p))
            return false;

        if (! is_directory (p))
            return false;

        for (auto f : make_iterator_range (directory_iterator (p), directory_iterator ())) {
            if (! is_regular_file (f))
                continue;

            if (f.path ().extension () != ".sc")
                continue;

            std::unique_ptr<ModuleInfo> newModule (new ModuleInfo (f, m_cxt));
            if (! addModule (f.path ().stem ().string (), std::move (newModule)))
                return false;
        }
    }
    catch (...) {
        return false;
    }

    return true;
}

ModuleInfo* ModuleMap::findModule (const std::string& name) const {
    auto i = m_modules.find (name);
    if (i == m_modules.end ())
        return nullptr;
    return i->second;
}

} // namespace SecreC
