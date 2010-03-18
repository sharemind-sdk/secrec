#include "intermediate.h"

#include <iostream>
#include "secrec/treenodeprogram.h"


namespace SecreC {

std::string Imop::toString() const {
    std::ostringstream os;
    switch (m_type) {
        case ASSIGN:       /*   d = arg1;                        */
            os << m_dest->name() << " = " << m_arg1->name();
            break;
        case CAST:         /*   d = (arg1) arg2;                 */
            os << "TODO CAST";
            break;
        case PUTPARAM:     /* PUTPARAM arg1;                     */
            os << "PUTPARAM " << m_arg1->name();
            break;
        case FUNCALL:      /*   d = arg1(PARAMS);                */
            os << m_dest->name() << " = CALL " << m_arg1->name();
            break;
        case WILDCARD:     /*   d = arg1[*];                     */
            os << m_dest->name() << " = " << m_arg1->name() << "[*]";
            break;
        case SUBSCRIPT:    /*   d = arg1[arg2];                  */
            os << m_dest->name() << " = " << m_arg1->name() << "[" << m_arg2->name() << "]";
            break;
        case UNEG:         /*   d = !arg1;                       */
            os << m_dest->name() << " = !" << m_arg1->name();
            break;
        case UMINUS:       /*   d = -arg1;                       */
            os << m_dest->name() << " = -" << m_arg1->name();
            break;
        case MATRIXMUL:    /*   d = arg1 #  arg2;                */
            os << m_dest->name() << " = " << m_arg1->name() << " # " << m_arg2->name();
            break;
        case MUL:          /*   d = arg1 *  arg2;                */
            os << m_dest->name() << " = " << m_arg1->name() << " * " << m_arg2->name();
            break;
        case DIV:          /*   d = arg1 /  arg2;                */
            os << m_dest->name() << " = " << m_arg1->name() << " / " << m_arg2->name();
            break;
        case MOD:          /*   d = arg1 %  arg2;                */
            os << m_dest->name() << " = " << m_arg1->name() << " % " << m_arg2->name();
            break;
        case ADD:          /*   d = arg1 +  arg2;                */
            os << m_dest->name() << " = " << m_arg1->name() << " + " << m_arg2->name();
            break;
        case SUB:          /*   d = arg1 -  arg2;                */
            os << m_dest->name() << " = " << m_arg1->name() << " - " << m_arg2->name();
            break;
        case EQ:           /*   d = arg1 == arg2;                */
            os << m_dest->name() << " = (" << m_arg1->name() << " == " << m_arg2->name() << ")";
            break;
        case NE:           /*   d = arg1 != arg2;                */
            os << m_dest->name() << " = (" << m_arg1->name() << " != " << m_arg2->name() << ")";
            break;
        case LE:           /*   d = arg1 <= arg2;                */
            os << m_dest->name() << " = (" << m_arg1->name() << " <= " << m_arg2->name() << ")";
            break;
        case LT:           /*   d = arg1 <  arg2;                */
            os << m_dest->name() << " = (" << m_arg1->name() << " < " << m_arg2->name() << ")";
            break;
        case GE:           /*   d = arg1 >= arg2;                */
            os << m_dest->name() << " = (" << m_arg1->name() << " >= " << m_arg2->name() << ")";
            break;
        case GT:           /*   d = arg1 >  arg2;                */
            os << m_dest->name() << " = (" << m_arg1->name() << " > " << m_arg2->name() << ")";
            break;
        case LAND:         /*   d = arg1 && arg2;                */
            os << m_dest->name() << " = (" << m_arg1->name() << " && " << m_arg2->name() << ")";
            break;
        case LOR:          /*   d = arg1 || arg2;                */
            os << m_dest->name() << " = (" << m_arg1->name() << " || " << m_arg2->name() << ")";
            break;
        case JT:           /* if (arg1) GOTO d;                  */
            os << "if (" << m_arg1->name() << ") GOTO " << m_dest->name();
            break;
        case JF:           /* if (!arg1) GOTO d;                 */
            os << "if (!" << m_arg1->name() << ") GOTO " << m_dest->name();
            break;
        case JE:           /* if (arg1 == arg2) GOTO d;          */
            os << "if (" << m_arg1->name() << " == " << m_arg2->name() << ") GOTO " << m_dest->name();
            break;
        case JNE:          /* if (arg1 != arg2) GOTO d;          */
            os << "if (" << m_arg1->name() << " != " << m_arg2->name() << ") GOTO " << m_dest->name();
            break;
        case JLE:          /* if (arg1 <= arg2) GOTO d;          */
            os << "if (" << m_arg1->name() << " <= " << m_arg2->name() << ") GOTO " << m_dest->name();
            break;
        case JLT:          /* if (arg1 <  arg2) GOTO d;          */
            os << "if (" << m_arg1->name() << " < " << m_arg2->name() << ") GOTO " << m_dest->name();
            break;
        case JGE:          /* if (arg1 >= arg2) GOTO d;          */
            os << "if (" << m_arg1->name() << " >= " << m_arg2->name() << ") GOTO " << m_dest->name();
            break;
        case JGT:          /* if (arg1 >  arg2) GOTO d;          */
            os << "if (" << m_arg1->name() << " > " << m_arg2->name() << ") GOTO " << m_dest->name();
            break;
        case JUMP:         /* GOTO d;                            */
            os << "GOTO " << m_dest->name();
            break;
        case RETURN:       /* RETURN;                            */
            if (m_arg1 == 0) {
                os << "RETURN";
            } else {       /* RETURN arg1;                       */
                os << "RETURN " << m_arg1->name();
            }
        case END:          /* END PROGRAM                        */
            os << "END PROGRAM";
        default:
            os << "TODO";
    }
    return os.str();
}

ICode::ICode(TreeNodeProgram *program) {
    m_status = program->generateCode(m_code, m_symbols, m_errorStream);
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Imop &i) {
    (void) i;
    out << i.toString() << std::endl;
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
    // if (icode.status() == SecreC::ICode::OK) {
        out << "ICode symbols:"  << std::endl << icode.symbols()  << std::endl
            << "ICode code:"     << std::endl << icode.code()     << std::endl
            << "ICode messages:" << std::endl << icode.messages() << std::endl;
    // }
    return out;
}
