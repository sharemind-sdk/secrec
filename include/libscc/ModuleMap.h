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
#include <map>
#include "StringRef.h"

namespace SecreC {

class ModuleInfo;
class StringTable;

/*******************************************************************************
  ModuleMap
*******************************************************************************/

class ModuleMap {
    ModuleMap (ModuleMap&); // DO NOT IMPLEMENT
    void operator = (ModuleMap&); // DO NOT IMPLEMENT
private: /* Types: */
    typedef std::map<StringRef, ModuleInfo*, StringRef::FastCmp> MapType;
public: /* Methods: */

    explicit ModuleMap (StringTable& stringTable)
        : m_stringTable (stringTable)
    { }

    ~ModuleMap ();

    bool addSearchPath (const std::string& pathName);

    /// Takes ownership, or frees a ModuleInfo object
    bool addModule (const std::string& name, std::auto_ptr<ModuleInfo> info);

    ModuleInfo* findModule (StringRef name) const;

private: /* Fields: */
    MapType m_modules;
    StringTable& m_stringTable;
};

} // namespace SecreC

#endif
