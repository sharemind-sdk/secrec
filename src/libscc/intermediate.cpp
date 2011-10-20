#include "intermediate.h"

#include <iostream>
#include "treenode.h"
#include "virtual_machine.h"
#include "codegen.h"


namespace SecreC {

ICode::Status ICode::init(TreeNodeProgram *program) {
    ICodeList code;
    CodeGen cg (code, m_symbols, m_log);
    m_status = program->codeGenWith (cg);
    if (m_status != OK) {
        return m_status;
    }

    m_blocks.init (code);
    return OK;
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::ICode::Status &s) {
    switch (s) {
        case SecreC::ICode::OK:                out << "OK"; break;
        case SecreC::ICode::E_NOT_IMPLEMENTED: out << "NOT IMPLEMENTED"; break;
        case SecreC::ICode::E_EMPTY_PROGRAM:   out << "EMPTY PROGRAM"; break;
        case SecreC::ICode::E_NO_MAIN:         out << "NO MAIN"; break;
        case SecreC::ICode::E_TYPE:            out << "TYPE ERROR"; break;
        case SecreC::ICode::E_OTHER:           out << "OTHER ERROR"; break;
        case SecreC::ICode::E_NO_MEM:          out << "OUT OF MEMORY"; break;
        default:                               out << "UNKNOWN"; break;
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
    out << "ICode status: " << icode.status() << std::endl;
    // if (icode.status() == SecreC::ICode::OK) {
        out << "ICode symbols:" << std::endl << icode.symbols()  << std::endl
            << "ICode blocks:"  << std::endl << icode.blocks()     << std::endl
            << "ICode log:"     << std::endl << icode.compileLog() << std::endl;
    // }
    return out;
}
