#include "intermediate.h"

#include <iostream>
#include "treenode.h"
#include "virtual_machine.h"


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

void ICode::run (VirtualMachine& vm) const {

    assert (m_status == ICode::OK);

    while (true) {

        Imop const& i = *m_code.at(vm.pc());

        switch (i.type()) {
        case Imop::ASSIGN:     vm.assign(i.dest(), i.arg1()); break;
        case Imop::CLASSIFY:   vm.classify(i.dest(), i.arg1()); break;
        case Imop::DECLASSIFY: vm.declassify(i.dest(), i.arg1()); break;
        case Imop::CAST:       // not implemented, fallthrough
        case Imop::WILDCARD:   // not implemented, fallthrough
        case Imop::SUBSCRIPT:  vm.nop(); break;
        case Imop::UNEG:       vm.uneg(i.dest(), i.arg1()); break;
        case Imop::UMINUS:     vm.uminus(i.dest(), i.arg1()); break;
        case Imop::MATRIXMUL:  vm.nop(); break;
        case Imop::MUL:        vm.mul(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::DIV:        vm.div(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::MOD:        vm.mod(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::ADD:        vm.add(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::SUB:        vm.sub(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::EQ:         vm.eq(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::NE:         vm.ne(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::LE:         vm.le(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::LT:         vm.lt(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::GE:         vm.ge(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::GT:         vm.gt(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::LAND:       vm.land(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::LOR:        vm.lor(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::CALL:       vm.call(i.dest(), i.arg1()); break;

        case Imop::JUMP:       vm.jump(i.dest()); break;
        case Imop::JT:         vm.jt(i.dest(), i.arg1()); break;
        case Imop::JF:         vm.jf(i.dest(), i.arg1()); break;
        case Imop::JE:         vm.je(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JNE:        vm.jne(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JLE:        vm.jle(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JLT:        vm.jlt(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JGE:        vm.jge(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JGT:        vm.jgt(i.dest(), i.arg1(), i.arg2()); break;

        case Imop::COMMENT:    vm.nop (); break;
        case Imop::ERROR:      vm.error(i.arg1()); return;
        case Imop::POPPARAM:   vm.popparam(i.dest()); break;
        case Imop::PUSHPARAM:  vm.pushparam(i.arg1()); break;
        case Imop::RETCLEAN:   vm.nop(); break;
        case Imop::RETURNVOID: vm.ret(); break;
        case Imop::RETURN:     vm.ret(i.arg1()); break;
        case Imop::END:        /* intentionally empty */ return;
        default: assert (false);
        }
    }
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
