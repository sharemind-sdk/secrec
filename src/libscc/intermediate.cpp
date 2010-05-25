#include "intermediate.h"

#include <iostream>
#include "treenode.h"


namespace SecreC {

ICode::Status ICode::init(TreeNodeProgram *program) {
    ICode::Status s = program->generateCode(m_code, m_symbols, m_log);
    if (s != OK) {
        m_status = s;
        return s;
    }

    m_status = OK;
    return m_status;
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

    for (CLCI it(c.begin()); it != c.end(); it++, i++) {
        out << i << "  " << **it << std::endl;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const SecreC::ICode &icode) {
    out << "ICode status: " << icode.status() << std::endl;
    // if (icode.status() == SecreC::ICode::OK) {
        out << "ICode symbols:" << std::endl << icode.symbols()  << std::endl
            << "ICode code:"    << std::endl << icode.code()     << std::endl
            << "ICode log:"     << std::endl << icode.compileLog() << std::endl;
    // }
    return out;
}
