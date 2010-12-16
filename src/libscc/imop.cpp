#include "imop.h"

#include <sstream>
#include <iostream>
#include "symboltable.h"
#include "treenode.h"


namespace {

std::string ulongToString(unsigned long n) {
    std::ostringstream os;
    os << n;
    return os.str();
}

std::string uniqueName(const SecreC::Symbol *s) {
    assert(s != 0);
    if (s->symbolType() == SecreC::Symbol::CONSTANT) return s->name();
    std::ostringstream os;
    os << s->name() << '{' << s << '}';
    return os.str();
}

} // anonymous namespace

namespace SecreC {

#define dname  (dest() == 0 ? "_" : uniqueName(dest()))
#define tname  (dest() == 0 ? "_" : ulongToString(((SecreC::Imop*) dest())->index()) )
#define a1name (arg1() == 0 ? "_" : uniqueName(arg1()))
#define a2name (arg2() == 0 ? "_" : uniqueName(arg2()))
#define a3name (arg3() == 0 ? "_" : uniqueName(arg3()))
#define cImop  (arg1() == 0 ? "_" : ulongToString(static_cast<const SymbolProcedure*>(arg1())->decl()->firstImop()->index()))

Imop::~Imop() {
    typedef std::set<Imop*>::const_iterator ISCI;
    typedef std::vector<Symbol const* >::iterator SVI;

    if (m_type == COMMENT || m_type == ERROR) {
        delete (std::string*) arg1();
    } else if ((m_type & JUMP_MASK) != 0x0) {
        if (dest() != 0) {
            ((Imop*) dest())->removeIncoming(this);
        }
    } else if (m_type == CALL) {
        if (arg2() != 0) {
            ((Imop*) arg2())->removeIncomingCall(this);
        }
    }

    for (ISCI it(m_incoming.begin()); it != m_incoming.end(); it++) {
        (*it)->setDest(0);
    }

    for (SVI it(m_args.begin()); it != m_args.end(); ++ it) {
        *it = 0;
    }
}

const Imop *Imop::callDest() const {
    assert(m_type == CALL);
    assert(arg1()->symbolType() == Symbol::PROCEDURE);
    assert(dynamic_cast<const SymbolProcedure*>(arg1()) != 0);

    return static_cast<const SymbolProcedure*>(arg1())->decl()->firstImop();
}

void Imop::setCallDest(SymbolProcedure *proc, Imop *clean) {
    assert(proc != 0);
    assert(clean != 0);
    assert(m_type == CALL);
    assert(clean->m_type == RETCLEAN);
    assert(proc->decl()->firstImop() != 0);
    setArg1(proc);
    setArg2((SecreC::Symbol*) clean);
    proc->setTarget(proc->decl()->firstImop());
    proc->decl()->firstImop()->addIncomingCall(this);
    clean->setArg2((SecreC::Symbol*) this);
}

/// \todo make this pretty
std::string Imop::toString() const {
    std::ostringstream os;
    switch (m_type) {
        case ASSIGN:       /*   d = arg1;                        */
            os << dname << " = " << a1name;
            if (nArgs() == 3) os <<  "(" << a2name << ")";
            break;
        case CLASSIFY:     /*   d = CLASSIFY(arg1);              */
            os << dname << " = CLASSIFY(" << a1name;
            if (nArgs() == 3) os << ", " << a2name;
            os << ")";
            break;
        case DECLASSIFY:   /*   d = DECLASSIFY(arg1);            */
            os << dname << " = DECLASSIFY(" << a1name;
            if (nArgs() == 3) os << ", " << a2name;
            os << ")";
            break;
        case FILL:
            os << dname << " = FILL " << a1name << " " << a2name;
            break;
        case STORE:
            os << dname << "[" << a1name << "] = " << a2name;
            break;
        case LOAD:
            os << dname << " = " << a1name << "[" << a2name << "]";
            break;
        case UNEG:         /*   d = !arg1;                       */
            os << dname << " = !" << a1name;
            if (nArgs() == 3) os << " (" << a2name << ")";
            break;
        case UMINUS:       /*   d = -arg1;                       */
            os << dname << " = -" << a1name;
            if (nArgs() == 3) os << " (" << a2name << ")";
            break;
        case MUL:          /*   d = arg1 *  arg2;                */
            os << dname << " = " << a1name << " * " << a2name;
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case DIV:          /*   d = arg1 /  arg2;                */
            os << dname << " = " << a1name << " / " << a2name;
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case MOD:          /*   d = arg1 %  arg2;                */
            os << dname << " = " << a1name << " % " << a2name;
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case ADD:          /*   d = arg1 +  arg2;                */
            os << dname << " = " << a1name << " + " << a2name;
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case SUB:          /*   d = arg1 -  arg2;                */
            os << dname << " = " << a1name << " - " << a2name;
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case EQ:           /*   d = arg1 == arg2;                */
            os << dname << " = (" << a1name << " == " << a2name << ")";
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case NE:           /*   d = arg1 != arg2;                */
            os << dname << " = (" << a1name << " != " << a2name << ")";
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case LE:           /*   d = arg1 <= arg2;                */
            os << dname << " = (" << a1name << " <= " << a2name << ")";
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case LT:           /*   d = arg1 <  arg2;                */
            os << dname << " = (" << a1name << " < " << a2name << ")";
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case GE:           /*   d = arg1 >= arg2;                */
            os << dname << " = (" << a1name << " >= " << a2name << ")";
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case GT:           /*   d = arg1 >  arg2;                */
            os << dname << " = (" << a1name << " > " << a2name << ")";
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case LAND:         /*   d = arg1 && arg2;                */
            os << dname << " = (" << a1name << " && " << a2name << ")";
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case LOR:          /*   d = arg1 || arg2;                */
            os << dname << " = (" << a1name << " || " << a2name << ")";
            if (nArgs() == 4) os << " (" << a3name << ")";
            break;
        case CALL:         /*   d = arg1(PARAMS);   (Imop *arg2) */
            if (dest() != 0) {
                os << dname << " = ";
            }
            os << "CALL " << a1name << " @ " << cImop;
            break;
        case JUMP:         /* GOTO d;                            */
            os << "GOTO " << tname;
            break;
        case JT:           /* if (arg1) GOTO d;                  */
            os << "IF (" << a1name << ") GOTO " << tname;
            break;
        case JF:           /* if (!arg1) GOTO d;                 */
            os << "if (!" << a1name << ") GOTO " << tname;
            break;
        case JE:           /* if (arg1 == arg2) GOTO d;          */
            os << "IF (" << a1name << " == " << a2name << ") GOTO " << tname;
            break;
        case JNE:          /* if (arg1 != arg2) GOTO d;          */
            os << "IF (" << a1name << " != " << a2name << ") GOTO " << tname;
            break;
        case JLE:          /* if (arg1 <= arg2) GOTO d;          */
            os << "IF (" << a1name << " <= " << a2name << ") GOTO " << tname;
            break;
        case JLT:          /* if (arg1 <  arg2) GOTO d;          */
            os << "IF (" << a1name << " < " << a2name << ") GOTO " << tname;
            break;
        case JGE:          /* if (arg1 >= arg2) GOTO d;          */
            os << "IF (" << a1name << " >= " << a2name << ") GOTO " << tname;
            break;
        case JGT:          /* if (arg1 >  arg2) GOTO d;          */
            os << "IF (" << a1name << " > " << a2name << ") GOTO " << tname;
            break;
        case COMMENT:      /* // arg1                            */
            os << "// " << *((std::string*) arg1());
            break;
        case ERROR:        /* // arg1                            */
            os << "ERROR \"" <<  *((std::string*) arg1()) << "\"";
            break;
        case POP:     /* POP d;                        */
            os << "POP " << dname << ";";
            if (nArgs() == 2) os << " (" << a1name << ")";
            break;
        case PUSH:    /* PUSH arg1;                    */
            os << "PUSH " << a1name;
            if (nArgs() == 3) os << " (" << a2name << ")";
            break;
        case PRINT:
            os << "PRINT " << a1name << ";";
            break;
        case RETCLEAN:     /* RETCLEAN;       (clean call stack) */
            os << "RETCLEAN;";
            break;
        case RETURNVOID:   /* RETURN;                            */
            os << "RETURN";
            break;
        case RETURN:       /* RETURN arg1;                       */
            os << "RETURN " << a1name;
            break;
        case END:          /* END PROGRAM                        */
            os << "END";
            break;
        default:
            os << "TODO";
    }

    typedef std::set<Imop*>::const_iterator ISCI;
    typedef std::set<unsigned long>::const_iterator ULSCI;
    if (!m_incoming.empty()) {
        std::set<unsigned long> is;
        for (ISCI it(m_incoming.begin()); it != m_incoming.end(); it++) {
            is.insert((*it)->index());
        }
        os << "    IN[";
        for (ULSCI it(is.begin()); it != is.end(); it++) {
            if (it != is.begin()) {
                os << ", ";
            }
            os << (*it);
        }
        os << "]";
    }
    if (!m_incomingCalls.empty()) {
        std::set<unsigned long> is;
        for (ISCI it(m_incomingCalls.begin()); it != m_incomingCalls.end(); it++) {
            is.insert((*it)->index());
        }
        os << "    CALLEDBY[";
        for (ULSCI it(is.begin()); it != is.end(); it++) {
            if (it != is.begin()) {
                os << ", ";
            }
            os << (*it);
        }
        os << "]";
    }
    if ((m_type == RETURN || m_type == RETURNVOID) && arg2() != 0) {
        Imop *i = (Imop*) arg2();
        if (!i->m_incomingCalls.empty()) {
            std::set<unsigned long> is;
            for (ISCI it(i->m_incomingCalls.begin()); it != i->m_incomingCalls.end(); it++) {
                is.insert((*it)->index());
            }
            os << "    RETURNSTO[";
            for (ULSCI it(is.begin()); it != is.end(); it++) {
                if (it != is.begin()) {
                    os << ", ";
                }
                os << (*it);
            }
            os << "]";
        }
    }
    return os.str();
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Imop &i) {
    (void) i;
    out << i.toString();
    return out;
}

