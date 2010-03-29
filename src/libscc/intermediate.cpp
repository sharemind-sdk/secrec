#include "intermediate.h"

#include <iostream>
#include "secrec/treenode.h"


namespace SecreC {

#define dname  (m_dest == 0 ? "_" : m_dest->name())
#define a1name (m_arg1 == 0 ? "_" : m_arg1->name())
#define a2name (m_arg2 == 0 ? "_" : m_arg2->name())

std::string Imop::toString() const {
    std::ostringstream os;
    switch (m_type) {
        case ASSIGN:       /*   d = arg1;                        */
            os << dname << " = " << a1name;
            break;
        case CAST:         /*   d = (arg1) arg2;                 */
            os << "TODO CAST";
            break;
        case PUTPARAM:     /* PUTPARAM arg1;                     */
            os << "PUTPARAM " << a1name;
            break;
        case FUNCALL:      /*   d = arg1(PARAMS);                */
            os << dname << " = CALL " << a1name;
            break;
        case WILDCARD:     /*   d = arg1[*];                     */
            os << dname << " = " << a1name << "[*]";
            break;
        case SUBSCRIPT:    /*   d = arg1[arg2];                  */
            os << dname << " = " << a1name << "[" << a2name << "]";
            break;
        case UNEG:         /*   d = !arg1;                       */
            os << dname << " = !" << a1name;
            break;
        case UMINUS:       /*   d = -arg1;                       */
            os << dname << " = -" << a1name;
            break;
        case MATRIXMUL:    /*   d = arg1 #  arg2;                */
            os << dname << " = " << a1name << " # " << a2name;
            break;
        case MUL:          /*   d = arg1 *  arg2;                */
            os << dname << " = " << a1name << " * " << a2name;
            break;
        case DIV:          /*   d = arg1 /  arg2;                */
            os << dname << " = " << a1name << " / " << a2name;
            break;
        case MOD:          /*   d = arg1 %  arg2;                */
            os << dname << " = " << a1name << " % " << a2name;
            break;
        case ADD:          /*   d = arg1 +  arg2;                */
            os << dname << " = " << a1name << " + " << a2name;
            break;
        case SUB:          /*   d = arg1 -  arg2;                */
            os << dname << " = " << a1name << " - " << a2name;
            break;
        case EQ:           /*   d = arg1 == arg2;                */
            os << dname << " = (" << a1name << " == " << a2name << ")";
            break;
        case NE:           /*   d = arg1 != arg2;                */
            os << dname << " = (" << a1name << " != " << a2name << ")";
            break;
        case LE:           /*   d = arg1 <= arg2;                */
            os << dname << " = (" << a1name << " <= " << a2name << ")";
            break;
        case LT:           /*   d = arg1 <  arg2;                */
            os << dname << " = (" << a1name << " < " << a2name << ")";
            break;
        case GE:           /*   d = arg1 >= arg2;                */
            os << dname << " = (" << a1name << " >= " << a2name << ")";
            break;
        case GT:           /*   d = arg1 >  arg2;                */
            os << dname << " = (" << a1name << " > " << a2name << ")";
            break;
        case LAND:         /*   d = arg1 && arg2;                */
            os << dname << " = (" << a1name << " && " << a2name << ")";
            break;
        case LOR:          /*   d = arg1 || arg2;                */
            os << dname << " = (" << a1name << " || " << a2name << ")";
            break;
        case JT:           /* if (arg1) GOTO d;                  */
            os << "if (" << a1name << ") GOTO " << dname;
            break;
        case JF:           /* if (!arg1) GOTO d;                 */
            os << "if (!" << a1name << ") GOTO " << dname;
            break;
        case JE:           /* if (arg1 == arg2) GOTO d;          */
            os << "if (" << a1name << " == " << a2name << ") GOTO " << dname;
            break;
        case JNE:          /* if (arg1 != arg2) GOTO d;          */
            os << "if (" << a1name << " != " << a2name << ") GOTO " << dname;
            break;
        case JLE:          /* if (arg1 <= arg2) GOTO d;          */
            os << "if (" << a1name << " <= " << a2name << ") GOTO " << dname;
            break;
        case JLT:          /* if (arg1 <  arg2) GOTO d;          */
            os << "if (" << a1name << " < " << a2name << ") GOTO " << dname;
            break;
        case JGE:          /* if (arg1 >= arg2) GOTO d;          */
            os << "if (" << a1name << " >= " << a2name << ") GOTO " << dname;
            break;
        case JGT:          /* if (arg1 >  arg2) GOTO d;          */
            os << "if (" << a1name << " > " << a2name << ") GOTO " << dname;
            break;
        case JUMP:         /* GOTO d;                            */
            os << "GOTO " << dname;
            break;
        case RETURN:       /* RETURN;                            */
            if (m_arg1 == 0) {
                os << "RETURN";
            } else {       /* RETURN arg1;                       */
                os << "RETURN " << a1name;
            }
        case END:          /* END PROGRAM                        */
            os << "END PROGRAM";
        default:
            os << "TODO";
    }
    return os.str();
}

ICode::CodeList::~CodeList() {
    for (const_iterator it(begin()); it != end(); it++) {
        delete *it;
    }
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
