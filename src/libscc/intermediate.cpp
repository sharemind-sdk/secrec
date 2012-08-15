#include "intermediate.h"

#include <iostream>

#include "treenode.h"
#include "virtual_machine.h"
#include "codegen.h"
#include "context.h"

namespace SecreC {

ICode::Status ICode::init(Context& cxt, TreeNodeModule *mod) {
    assert(m_status == NOT_READY);
    assert (mod != 0);
    ICodeList code;
    CodeGen cg (cxt, code, *this);
    if (cg.cgMain(mod).status() != CGResult::OK)
        return m_status;

    m_status = OK;
    m_program.init (code);
    return m_status;
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::ICode::Status &s) {
    switch (s) {
        case SecreC::ICode::NOT_READY: out << "NOT_READY"; break;
        case SecreC::ICode::OK:        out << "OK"; break;
        case SecreC::ICode::ERROR:     out << "ERROR"; break;
        default:                       out << "UNKNOWN"; break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const SecreC::ICodeList &c) {
    typedef SecreC::ICodeList::const_iterator CLCI;
    unsigned i = 1;

    for (CLCI it (c.begin()); it != c.end (); it++, i++) {
        out << i << "  " << *it << std::endl;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const SecreC::ICode &icode) {
    out << "ICode status: " << icode.status() << std::endl
        << "ICode symbols:" << std::endl << icode.symbols()    << std::endl
        << "ICode CFG:"     << std::endl << icode.program()    << std::endl
        << "ICode log:"     << std::endl << icode.compileLog() << std::endl;
    return out;
}
