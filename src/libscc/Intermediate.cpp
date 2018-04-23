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

#include "Intermediate.h"

#include "Parser.h"
#include "CodeGen.h"
#include "CodeGenResult.h"
#include "Context.h"
#include "ContextImpl.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "VirtualMachine.h"

#include <boost/optional/optional.hpp>
#include <iostream>

namespace SecreC {

TreeNodeModule* ICode::parseMain (const boost::optional<std::string>& mfile) {
    int errorCode = 1;
    TreeNodeModule * parseTree = nullptr;
    StringTable& table = stringTable ();
    m_status = OK;

    if (mfile) {
        const char* name = mfile.get ().c_str ();
        FILE* h = fopen(name, "r");
        if (h != nullptr) {
            errorCode = sccparse_file(&table, name, h, &parseTree);
            fclose (h);
        }
        else {
            m_status = ERROR;
            m_log.fatal () << "Failed to open file \"" << name << "\".";
        }
    }
    else {
        errorCode = sccparse(&table, "-", &parseTree);
    }

    if (errorCode != 0) {
        m_status = ERROR;
        m_log.fatal () << "Parsing main module failed.";
    }

    return parseTree;
}

void ICode::compile (TreeNodeModule *mod, Location::PathStyle pathStyle) {
    assert (mod != nullptr);
    ICodeList code;
    CodeGen cg (code, *this, pathStyle);
    if (cg.cgMain(mod).status() != CGResult::OK) {
        m_status = ERROR;
        return;
    }

    m_status = OK;
    m_program.init (code);
}

StringTable& ICode::stringTable () {
    return context ().pImpl ()->m_stringTable;
}

std::ostream &operator<<(std::ostream &out, const ICode::Status &s) {
    switch (s) {
        case ICode::NOT_READY: out << "NOT_READY"; break;
        case ICode::OK:        out << "OK"; break;
        case ICode::ERROR:     out << "ERROR"; break;
        default:               out << "UNKNOWN"; break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const ICodeList &c) {
    unsigned i = 1;
    for (auto it = c.begin(); it != c.end (); it++, i++) {
        out << i << "  " << *it << std::endl;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const ICode &icode) {
    out << "ICode status: " << icode.status() << std::endl
        << "ICode symbols:" << std::endl << icode.symbols()    << std::endl
        << "ICode CFG:"     << std::endl << icode.program()    << std::endl
        << "ICode log:"     << std::endl << icode.compileLog() << std::endl;
    return out;
}

} // namespace SecreC
