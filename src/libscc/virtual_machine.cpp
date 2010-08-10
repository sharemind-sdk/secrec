#include "virtual_machine.h"

#include "icodelist.h"

namespace SecreC {

void VirtualMachine::run (ICodeList const& code) {

    while (true) {

        Imop const& i = *code.at(m_pc);

        switch (i.type()) {
        case Imop::ASSIGN:     assign(i.dest(), i.arg1()); break;
        case Imop::CLASSIFY:   classify(i.dest(), i.arg1()); break;
        case Imop::DECLASSIFY: declassify(i.dest(), i.arg1()); break;
        case Imop::CAST:       // not implemented, fallthrough
        case Imop::WILDCARD:   // not implemented, fallthrough
        case Imop::SUBSCRIPT:  nop(); break;
        case Imop::UNEG:       uneg(i.dest(), i.arg1()); break;
        case Imop::UMINUS:     uminus(i.dest(), i.arg1()); break;
        case Imop::MATRIXMUL:  nop(); break;
        case Imop::MUL:        mul(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::DIV:        div(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::MOD:        mod(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::ADD:        add(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::SUB:        sub(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::EQ:         eq(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::NE:         ne(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::LE:         le(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::LT:         lt(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::GE:         ge(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::GT:         gt(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::LAND:       land(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::LOR:        lor(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::CALL:       call(i.dest(), i.arg1()); break;

        case Imop::JUMP:       jump(i.dest()); break;
        case Imop::JT:         jt(i.dest(), i.arg1()); break;
        case Imop::JF:         jf(i.dest(), i.arg1()); break;
        case Imop::JE:         je(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JNE:        jne(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JLE:        jle(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JLT:        jlt(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JGE:        jge(i.dest(), i.arg1(), i.arg2()); break;
        case Imop::JGT:        jgt(i.dest(), i.arg1(), i.arg2()); break;

        case Imop::COMMENT:    nop (); break;
        case Imop::ERROR:      error(i.arg1()); return;
        case Imop::POPPARAM:   popparam(i.dest()); break;
        case Imop::PUSHPARAM:  pushparam(i.arg1()); break;
        case Imop::RETCLEAN:   nop(); break;
        case Imop::RETURNVOID: ret(); break;
        case Imop::RETURN:     ret(i.arg1()); break;
        case Imop::END:        /* intentionally empty */ return;
        default: assert (false);
        }
    }
}

std::string VirtualMachine::toString(void) {
    std::stringstream os;

    os << "Log:\n";
    os << m_log;

    os << "Store:\n";
    for (Store::const_iterator i(m_store.begin()); i != m_store.end(); ++ i) {
        Symbol const* sym = i->first;
        Value val = i->second;
        os << sym->toString() << " -> ";
        switch (sym->secrecType().secrecDataType()) {
            case DATATYPE_BOOL:
                os << val.m_bool_val;
                break;
            case DATATYPE_INT:
                os << val.m_int_val;
                break;
            case DATATYPE_UINT:
                os << val.m_uint_val;
                break;
            case DATATYPE_STRING:
                os << *val.m_str_val;
                break;
            case DATATYPE_INVALID:
                assert (false);
        }

        os << '\n';
    }

    return os.str();
}

}
