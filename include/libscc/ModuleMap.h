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

#include <memory>
#include <string>
#include <map>

namespace SecreC {

class ModuleInfo;

/*******************************************************************************
  ModuleMap
*******************************************************************************/

class ModuleMap {
    ModuleMap (ModuleMap&); // DO NOT IMPLEMENT
    void operator = (ModuleMap&); // DO NOT IMPLEMENT
private: /* Types: */
    typedef std::map<std::string, ModuleInfo* > MapType;
public: /* Methods: */
    ModuleMap () { }
    ~ModuleMap ();

    bool addSearchPath (const std::string& pathName);

    /// Takes ownership, or frees a ModuleInfo object
    bool addModule (const std::string& name, std::auto_ptr<ModuleInfo>& info);

    ModuleInfo* findModule (const std::string& name) const;

private: /* Fields: */
    MapType m_modules;
};

} // namespace SecreC

#endif
