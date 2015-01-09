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

#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include "Blocks.h"
#include "Context.h"
#include "Imop.h"
#include "Log.h"
#include "ModuleMap.h"
#include "SymbolTable.h"

#include <boost/optional/optional_fwd.hpp>
#include <iosfwd>
#include <string>

namespace SecreC {

class StringTable;

class ICode {
public: /* Types: */

    enum Status { NOT_READY, OK, ERROR };

public: /* Methods: */

    ICode ()
        : m_status (NOT_READY)
        , m_modules (m_context)
    { }

    ICode (const ICode&) = delete;
    ICode& operator = (const ICode&) = delete;

    TreeNodeModule* parseMain (const boost::optional<std::string>& mfile);
    void compile (TreeNodeModule *mod);

    SymbolTable& symbols () { return m_symbols; }
    const SymbolTable& symbols () const { return m_symbols; }
    Program& program () { return m_program; }
    const Program& program () const { return m_program; }
    Status status () const { return m_status; }
    CompileLog& compileLog () { return m_log; }
    const CompileLog& compileLog () const { return m_log; }
    ModuleMap& modules () { return m_modules; }
    Context& context () { return m_context; }
    StringTable& stringTable ();

private: /* Fields: */

    Status          m_status;
    Context         m_context;
    SymbolTable     m_symbols;
    ModuleMap       m_modules;
    Program         m_program;
    CompileLog      m_log;
};

std::ostream &operator<<(std::ostream &out, const ICode::Status &s);
std::ostream &operator<<(std::ostream &out, const ICodeList &c);
std::ostream &operator<<(std::ostream &out, const ICode &icode);

} // namespace SecreC

#endif // INTERMEDIATE_H
