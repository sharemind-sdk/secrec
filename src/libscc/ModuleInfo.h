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

#ifndef SECREC_MODULE_INFO_H
#define SECREC_MODULE_INFO_H

#include <string>
#include <map>
#include <boost/filesystem.hpp>

#include "CodeGenState.h"
#include "TreeNodeFwd.h"

namespace SecreC {

class TreeNodeProgram;
class Context;

/*******************************************************************************
  ModuleInfo
*******************************************************************************/

class ModuleInfo {
private:
    ModuleInfo (ModuleInfo&) = delete;
    ModuleInfo& operator = (const ModuleInfo&) = delete;

private: /* Types: */
    using directory_entry = boost::filesystem::directory_entry;

public:
    enum CGStatus {
        CGNotStarted, ///< File is not parsed.
        CGStarted,    ///< File is parsed, generating code.
        CGDone        ///< File is parsed, code generated.
    };

public: /* Methods: */

    explicit ModuleInfo (Context& cxt)
        : m_status (CGNotStarted)
        , m_body (nullptr)
        , m_cxt (cxt)
    { }

    explicit ModuleInfo (directory_entry location, Context& cxt)
        : m_location (std::move(location))
        , m_status (CGNotStarted)
        , m_body (nullptr)
        , m_cxt (cxt)
    { }

    ~ModuleInfo ();

    void setCodeGenState (const CodeGenState& state);
    directory_entry location () const { return m_location; }
    std::string fileNameStem () const;
    CGStatus status () const { return m_status; }
    void setStatus (ModuleInfo::CGStatus status) { m_status = status; }
    CodeGenState& codeGenState () { return m_cgState; }
    TreeNodeModule* body () const { return m_body; }
    void setBody (TreeNodeModule* body) { m_body = body; }
    bool read();

private: /* Fields: */
    directory_entry   const  m_location;
    CGStatus                 m_status;
    CodeGenState             m_cgState;
    TreeNodeModule*          m_body;
    Context&                 m_cxt;
};

} // namespace SecreC

#endif
