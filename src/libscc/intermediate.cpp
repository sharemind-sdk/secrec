#include "intermediate.h"

#include <iostream>
#include "secrec/treenodeprogram.h"


namespace SecreC {

ICode::ICode(TreeNodeProgram *program) {
    m_status = program->generateCode(m_code, m_symbols, m_errorStream);
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Imop &i) {
    (void) i;
    out << "{Imop}"; /// \todo
    return out;
}

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

std::ostream &operator<<(std::ostream &out, const SecreC::ICode::CodeList &c) {
    typedef SecreC::ICode::CodeList::const_iterator CLCI;

    for (CLCI it(c.begin()); it != c.end(); it++) {
        out << **it << std::endl;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const SecreC::ICode &icode) {
    out << "ICode:" << icode.status() << std::endl;
    if (icode.status() == SecreC::ICode::OK) {
        out << "ICode symbols:" << std::endl << icode.symbols() << std::endl
            << "ICode code:"    << std::endl << icode.code()    << std::endl;
    }
    return out;
}
