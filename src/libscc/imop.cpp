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

std::string symToString (const SecreC::Symbol* s) {
    return s == 0 ? "_" : uniqueName (s);
}

} // anonymous namespace

namespace SecreC {

#define dname  (dest() == 0 ? "_" : uniqueName(dest()))
#define tname  (dest() == 0 ? "_" : ulongToString(static_cast<const SymbolLabel*>(dest())->target()->index()) )
#define a1name (arg1() == 0 ? "_" : uniqueName(arg1()))
#define a2name (arg2() == 0 ? "_" : uniqueName(arg2()))
#define a3name (arg3() == 0 ? "_" : uniqueName(arg3()))
#define cImop  (arg1() == 0 ? "_" : ulongToString(static_cast<const SymbolProcedure*>(arg1())->target ()->index()))

Imop* newError (TreeNode* node, ConstantString* msg) {
    Imop* imop = new Imop (node, Imop::ERROR, (Symbol*) 0, msg);
    return imop;
}

Imop* newAssign (TreeNode* node, Symbol* dest, Symbol* from) {
    return newUnary (node, Imop::ASSIGN, dest, from);
}

Imop* newBinary (TreeNode* node, Imop::Type iType, Symbol *dest, Symbol *arg1, Symbol *arg2) {
    Imop* i = 0;
    if (dest->secrecType ().isScalar ()) {
        i = new Imop (node, iType, dest, arg1, arg2);
    }
    else {
        i = new Imop (node, iType, dest, arg1, arg2, dest->getSizeSym ());
    }

    return i;
}

Imop* newUnary (TreeNode* node, Imop::Type iType, Symbol *dest, Symbol *arg1) {
    Imop* i = 0;
    if (dest->secrecType ().isScalar ()) {
        i = new Imop (node, iType, dest, arg1);
    }
    else {
        i = new Imop (node, iType, dest, arg1, dest->getSizeSym ());
    }

    return i;
}

Imop* newCall (TreeNode* node) {
   Imop* out = new Imop (node, Imop::CALL, (Symbol*) 0, (Symbol*) 0);
   return out;
}


Imop* newNullary (TreeNode* node, Imop::Type iType, Symbol *dest) {
    assert (dest != 0);
    Imop* i = 0;
    if (dest->secrecType ().isScalar ()) {
        i = new Imop (node, iType, dest);
    }
    else {
        i = new Imop (node, iType, dest, dest->getSizeSym ());
    }

    return i;
}

Imop::~Imop() {
    typedef std::set<Imop*>::const_iterator ISCI;
    typedef std::vector<Symbol const* >::iterator SVI;

    if (m_type == COMMENT) {
        delete (std::string*) arg1();
    }

    for (SVI it(m_args.begin()); it != m_args.end(); ++ it) {
        *it = 0;
    }
}

const Imop *Imop::callDest() const {
    assert(m_type == CALL);
    assert(dest()->symbolType() == Symbol::PROCEDURE);
    assert(dynamic_cast<const SymbolProcedure*>(dest()) != 0);

    return static_cast<const SymbolProcedure*>(dest())->target ();
}

SymbolLabel const* Imop::jumpDest() const {
    Symbol const* sym = dest ();
    assert (dynamic_cast<SymbolLabel const*>(sym) != 0);
    return static_cast<SymbolLabel const*>(sym);
}

void Imop::setJumpDest (SymbolLabel *dest) {
    assert(dest != 0);
    assert((m_type & JUMP_MASK) != 0x0);
    setDest(dest);
}

void Imop::setCallDest(SymbolProcedure *proc) {
//void Imop::setCallDest(SymbolProcedure *proc, SymbolLabel* clean) {
    assert (proc != 0);
    assert (m_type == CALL);
//    assert (clean->target()->m_type == RETCLEAN);
    assert (proc->target () != 0);
    setDest (proc);
//    setArg1 (proc);
    // setArg2 (clean);
//    proc->setTarget (proc->target ());
    proc->target ()->addIncomingCall (this);
}

void Imop::setReturnDestFirstImop (SymbolLabel *label) {
    assert (label != 0);
    assert (label->target () != 0);
    assert (label->target ()->m_type == COMMENT);
//    setArg2(label);
    setDest (label);
}

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
        case ALLOC:
            os << dname << " = ALLOC " << a1name << " " << a2name;
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
            {
                std::list<const Symbol* > argList, retList;
                OperandConstIterator it, itBegin, itEnd;
                itBegin = m_args.begin ();
                itEnd = m_args.end ();
                it = itBegin;
                const Symbol* callDest = *it;
                for (++ it; it != itEnd && *it != 0; ++ it)
                    argList.push_back (*it);

                for (++ it; it != itEnd; ++ it)
                    retList.push_back (*it);

                // print destination
                for (std::list<const Symbol*>::const_iterator jt = retList.begin (); jt != retList.end (); ++ jt) {
                    const Symbol* sym = *jt;
                    if (jt != retList.begin ())
                        os << ", ";
                    os << symToString (sym);
                }

                if (!retList.empty ())
                    os << " = ";

                os << "CALL " << symToString (callDest) << " (";
                for (std::list<const Symbol*>::const_iterator jt = argList.begin (); jt != argList.end (); ++ jt) {
                    const Symbol* sym = *jt;
                    if (jt != argList.begin ())
                        os << ", ";
                    os << symToString (sym);
                }

                os << ");";
            }
            break;
        case PARAM:
            os << ".PARAM " << dname;
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
            os << "ERROR \"" <<  a1name << "\"";
            break;
        case PRINT:
            os << "PRINT \"" << a1name << "\";";
            break;
        case FREAD:
            os << "FREAD \"" << a1name << "\";";
            break;
        case RETCLEAN:     /* RETCLEAN;       (clean call stack) */
            os << "RETCLEAN;";
            break;
        case RETURNVOID:   /* RETURN;                            */
            os << "RETURN";
            break;
        case RETURN:       /* RETURN arg1;                       */
            os << "RETURN (";
            {
                const OperandConstIterator itBegin = m_args.begin () + 1; // !
                const OperandConstIterator itEnd = m_args.end ();
                assert (itBegin != itEnd && "Only RETURNVOID can return nothing.");
                for (OperandConstIterator it = itBegin; it != itEnd; ++ it) {
                    if (it != itBegin) {
                        os << ", ";
                    }

                    const Symbol* sym = *it;
                    os << symToString (sym);
                }
            }
            os << ");";
            break;
        case END:          /* END PROGRAM                        */
            os << "END";
            break;
        default:
            os << "TODO";
            break;
    }

    typedef std::set<Imop*>::const_iterator ISCI;
    typedef std::set<unsigned long>::const_iterator ULSCI;
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
    if ((m_type == RETURN || m_type == RETURNVOID) && dest () != 0) {
        assert (dynamic_cast<const SymbolLabel*>(dest ()) != 0);
        Imop const* i = static_cast<const SymbolLabel*>(dest ())->target ();
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

