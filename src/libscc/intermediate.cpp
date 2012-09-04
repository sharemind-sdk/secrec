#include "intermediate.h"

#include <iostream>

#include "codegen.h"
#include "context.h"
#include "treenode.h"
#include "virtual_machine.h"
#include "symboltable.h"

namespace SecreC {

ICode::Status ICode::init (TreeNodeModule *mod) {
    assert(m_status == NOT_READY);
    assert (mod != 0);
    ICodeList code;
    CodeGen cg (code, *this);
    if (cg.cgMain(mod).status() != CGResult::OK)
        return m_status;

    m_status = OK;
    m_program.init (code);
    return m_status;
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
