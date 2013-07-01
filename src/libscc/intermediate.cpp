#include "intermediate.h"

#include <boost/optional/optional.hpp>
#include <iostream>

#include "codegen.h"
#include "context.h"
#include "treenode.h"
#include "virtual_machine.h"
#include "symboltable.h"
#include "context_impl.h"

namespace SecreC {

TreeNodeModule* ICode::parseMain (const boost::optional<std::string>& mfile) {
    int errorCode = 1;
    TreeNodeModule * parseTree = 0;
    StringTable& table = stringTable ();
    m_status = OK;

    if (mfile) {
        const char* name = mfile.get ().c_str ();
        FILE* h = fopen(name, "r");
        if (h != NULL) {
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

void ICode::compile (TreeNodeModule *mod) {
    assert (mod != 0);
    ICodeList code;
    CodeGen cg (code, *this);
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
    typedef ICodeList::const_iterator CLCI;
    unsigned i = 1;

    for (CLCI it (c.begin()); it != c.end (); it++, i++) {
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
