/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_MODULE_INFO_H
#define SECREC_MODULE_INFO_H

#include <string>
#include <map>
#include <boost/filesystem.hpp>

#include "codegen.h"

namespace SecreC {

/*******************************************************************************
  ModuleInfo
*******************************************************************************/

class ModuleInfo {
private:
    ModuleInfo (ModuleInfo&); // DO NOT IMPLEMENT
    void operator = (ModuleInfo&); // DO NOT IMPLEMENT

private: /* Types: */
    typedef boost::filesystem::directory_entry directory_entry;

public:
    enum CGStatus {
        CGNotStarted, ///< File is not parsed.
        CGStarted,    ///< File is parsed, generating code.
        CGDone        ///< File is parsed, code generated.
    };

public: /* Methods: */

    ModuleInfo ()
        : m_status (CGStarted)
    { }

    explicit ModuleInfo (const directory_entry& location)
        : m_location (location)
        , m_status (CGNotStarted)
    { }

    ~ModuleInfo () { }

    void setCodeGenState (const CodeGenState& state);
    directory_entry location () const { return m_location; }
    std::string fileNameStem () const;
    CGStatus status () const { return m_status; }
    void setStatus (ModuleInfo::CGStatus status) { m_status = status; }
    CodeGenState& codeGenState () { return m_cgState; }

private: /* Fields: */
    directory_entry   const  m_location;
    CGStatus                 m_status;
    CodeGenState             m_cgState;
};

} // namespace SecreC

#endif