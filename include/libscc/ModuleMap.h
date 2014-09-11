/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
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
    typedef std::map<std::string, ModuleInfo*> MapType;
public: /* Methods: */

    explicit ModuleMap (Context& cxt) : m_cxt (cxt) { }
    ~ModuleMap ();

    bool addSearchPath (const std::string& pathName);
    bool addModule (const std::string& name, std::auto_ptr<ModuleInfo> info);
    ModuleInfo* findModule (const std::string& name) const;

private: /* Fields: */
    MapType m_modules;
    Context& m_cxt;
};

} // namespace SecreC

#endif
