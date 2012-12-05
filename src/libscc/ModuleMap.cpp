/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "ModuleMap.h"

#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>

#include "ModuleInfo.h"

using namespace boost;

namespace SecreC {

/*******************************************************************************
  ModuleMap
*******************************************************************************/

ModuleMap::~ModuleMap () {
    BOOST_FOREACH (ModuleInfo* modInfo, adaptors::values (m_modules)) {
        delete modInfo;
    }
}

bool ModuleMap::addModule (const std::string& name, std::auto_ptr<ModuleInfo> info) {
    assert (info.get () != 0);
    MapType::iterator it = m_modules.find (name);
    if (it != m_modules.end ())
        return false;
    m_modules.insert (it, std::make_pair (*m_stringTable.addString (name), info.get ()));
    info.release ();
    return true;
}

bool ModuleMap::addSearchPath (const std::string& pathName) {
    using namespace boost::filesystem;
    typedef std::pair<directory_iterator, directory_iterator > directory_range;

    const path p (pathName);
    if (! exists (p)) return true;
    if (! is_directory (p)) return true;
    const directory_range& range = std::make_pair (directory_iterator (p), directory_iterator ());
    BOOST_FOREACH (const directory_entry& f, range) {
        if (! is_regular_file (f)) continue;
        if (f.path ().extension () != ".sc") continue;
        std::auto_ptr<ModuleInfo> newModule (new ModuleInfo (f, m_stringTable));
        if (! addModule (f.path ().stem ().string (), newModule)) {
            return false;
        }
    }

    return true;
}

ModuleInfo* ModuleMap::findModule (StringRef name) const {
    MapType::const_iterator i = m_modules.find (name);
    if (i == m_modules.end ())
        return 0;
    return i->second;
}

} // namespace SecreC
