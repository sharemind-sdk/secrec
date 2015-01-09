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

#ifndef SECREC_MODULE_MAP_H
#define SECREC_MODULE_MAP_H

#include <map>
#include <memory>
#include <string>

namespace SecreC {

class ModuleInfo;
class Context;

/*******************************************************************************
  ModuleMap
*******************************************************************************/

class ModuleMap {
    ModuleMap (ModuleMap&) = delete;
    ModuleMap& operator = (const ModuleMap&) = delete;
private: /* Types: */
    using MapType = std::map<std::string, ModuleInfo*>;
public: /* Methods: */

    explicit ModuleMap (Context& cxt) : m_cxt (cxt) { }
    ~ModuleMap ();

    bool addSearchPath (const std::string& pathName);
    bool addModule (const std::string& name, std::unique_ptr<ModuleInfo> info);
    ModuleInfo* findModule (const std::string& name) const;

private: /* Fields: */
    MapType m_modules;
    Context& m_cxt;
};

} // namespace SecreC

#endif
