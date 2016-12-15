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

#include <iostream>
#include <boost/range/adaptors.hpp>
#include <boost/range/iterator_range.hpp>

using namespace boost;

namespace SecreC {

/*******************************************************************************
  ModuleMap
*******************************************************************************/

ModuleMap::ModuleMap (Context& cxt)
    : m_cxt (cxt)
{ }

ModuleMap::~ModuleMap() { }

bool ModuleMap::addModule (const std::string& name, std::unique_ptr<ModuleInfo> info) {
    assert (info.get () != nullptr);
    auto it = m_modules.find (name);
    if (it != m_modules.end ())
        return false;
    m_modules.insert (it, std::make_pair (name, std::move(info)));
    return true;
}

void ModuleMap::addSearchPath (const std::string& pathName, bool verbose) {
    using namespace boost::filesystem;

    const path p (pathName);

    try  {
        if (! exists(p)) {
            if (verbose) {
                std::cerr << "Search path " << p << " does not exist."
                          << std::endl;
            }

            return;
        }

        if (! is_directory (p)) {
            if (verbose) {
                std::cerr << "Invalid search path " << p << "."
                          << " Not a directory."
                          << std::endl;
            }

            return;
        }

        if (verbose) {
            std::cerr << "Searching module files from " << p << "." << std::endl;
        }

        for (auto f : make_iterator_range (directory_iterator (p),
                                           directory_iterator ())) {
            if (f.path ().extension () != ".sc")
                continue;

            if (! is_regular_file (f))
                continue;

            if (verbose) {
                std::cerr << "Using module " << f.path() << std::endl;
            }

            if (! addModule (f.path ().stem ().string (),
                    std::unique_ptr<ModuleInfo>(new ModuleInfo (f, m_cxt))))
            {
                if (verbose) {
                    std::cerr << "    Ignoring. File with same name already found."
                              << std::endl;
                }

                continue;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception " << e.what()
                  << " thrown when adding search path for " << pathName
                  << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception thrown when adding search path for "
                  << pathName << std::endl;
    }
}

ModuleInfo* ModuleMap::findModule (const std::string& name) const {
    auto i = m_modules.find (name);
    if (i == m_modules.end ())
        return nullptr;
    return i->second.get();
}

} // namespace SecreC
