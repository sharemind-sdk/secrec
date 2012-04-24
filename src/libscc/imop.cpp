#include "imop.h"

#include <sstream>
#include <iostream>
#include <boost/range.hpp>
#include <boost/foreach.hpp>

#include "symboltable.h"
#include "treenode.h"
#include "constant.h"

namespace {

using namespace  SecreC;

struct ImopInfoBits {
    unsigned  type;              ///< Opcode
    bool      isExpr : 1;        ///< Expression (why do we even need this?)
    bool      isJump : 1;        ///< Instruction is procedure-local jump
    bool      isTerminator : 1;  ///< Instruction terminates a basic block
    bool      writesDest : 1;    ///< Instruction may write to destination operand
    unsigned  vecArgNum;         ///< Vectorised if number of operands equals to that
    unsigned  useBegin;          ///< Points to first USE operand, ends with 0 or with last operand.
};

ImopInfoBits imopInfo [Imop::_NUM_INSTR] = {
    //{ Imop::Type,       E, J, T, W, V, U }
    // Unary operators:
      { Imop::ASSIGN,     1, 0, 0, 1, 3, 1 }
    , { Imop::CAST,       1, 0, 0, 1, 3, 1 }
    , { Imop::TOSTRING,   1, 0, 0, 1,-1, 1 }
    , { Imop::CLASSIFY,   1, 0, 0, 1, 3, 1 }
    , { Imop::DECLASSIFY, 1, 0, 0, 1, 3, 1 }
    , { Imop::UNEG,       1, 0, 0, 1, 3, 1 }
    , { Imop::UMINUS,     1, 0, 0, 1, 3, 1 }
    // Binary operators:
    , { Imop::MUL,        1, 0, 0, 1, 4, 1 }
    , { Imop::DIV,        1, 0, 0, 1, 4, 1 }
    , { Imop::MOD,        1, 0, 0, 1, 4, 1 }
    , { Imop::ADD,        1, 0, 0, 1, 4, 1 }
    , { Imop::SUB,        1, 0, 0, 1, 4, 1 }
    , { Imop::EQ,         1, 0, 0, 1, 4, 1 }
    , { Imop::NE,         1, 0, 0, 1, 4, 1 }
    , { Imop::LE,         1, 0, 0, 1, 4, 1 }
    , { Imop::LT,         1, 0, 0, 1, 4, 1 }
    , { Imop::GE,         1, 0, 0, 1, 4, 1 }
    , { Imop::GT,         1, 0, 0, 1, 4, 1 }
    , { Imop::LAND,       1, 0, 0, 1, 4, 1 }
    , { Imop::LOR,        1, 0, 0, 1, 4, 1 }
    // Array expressions:
    , { Imop::STORE,      0, 0, 0, 0,-1, 0 }
    , { Imop::LOAD,       1, 0, 0, 1,-1, 1 }
    , { Imop::ALLOC,      1, 0, 0, 1,-1, 1 }
    , { Imop::COPY,       1, 0, 0, 1,-1, 1 }
    , { Imop::RELEASE,    0, 0, 0, 0,-1, 1 }
    // Other expressions:
    , { Imop::PARAM,      1, 0, 0, 1,-1, 1 }
    , { Imop::DOMAINID,   1, 0, 0, 1,-1,-1 }
    , { Imop::CALL,       1, 0, 1, 1,-1, 1 }
    // Jumps:
    , { Imop::JUMP,       0, 1, 1, 0,-1, 1 }
    , { Imop::JT,         0, 1, 1, 0,-1, 1 }
    , { Imop::JF,         0, 1, 1, 0,-1, 1 }
    , { Imop::JE,         0, 1, 1, 0,-1, 1 }
    , { Imop::JNE,        0, 1, 1, 0,-1, 1 }
    // Terminators:
    , { Imop::ERROR,      0, 0, 1, 0,-1,-1 }
    , { Imop::RETURNVOID, 0, 0, 1, 0,-1,-1 }
    , { Imop::RETURN,     0, 0, 1, 0,-1, 1 }
    , { Imop::END,        0, 0, 1, 0,-1,-1 }
    // Other:
    , { Imop::COMMENT,    0, 0, 0, 0,-1,-1 }
    , { Imop::PRINT,      0, 0, 0, 0,-1,-1 }
    , { Imop::SYSCALL,    0, 0, 0, 0,-1, 1 }
    , { Imop::PUSH,       0, 0, 0, 0,-1, 1 }
    , { Imop::PUSHREF,    0, 0, 0, 0,-1, 1 }
    , { Imop::PUSHCREF,   0, 0, 0, 0,-1, 1 }
    , { Imop::RETCLEAN,   0, 0, 0, 0,-1,-1 }
};

const ImopInfoBits& getImopInfoBits (Imop::Type type) {
    assert (0 <= type && type < Imop::_NUM_INSTR);
    const ImopInfoBits& out = imopInfo [type];
    assert (out.type == type);
    return out;
}

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

SecreC::Symbol* getSizeSymbol (SecreC::Symbol* sym) {
    assert (sym != 0);
    assert (dynamic_cast<SecreC::SymbolSymbol* >(sym) != 0);
    return static_cast<SecreC::SymbolSymbol*>(sym)->getSizeSym ();
}

} // anonymous namespace

namespace SecreC {

#define dname  (dest() == 0 ? "_" : uniqueName(dest()))
#define tname  (dest() == 0 ? "_" : ulongToString(static_cast<const SymbolLabel*>(dest())->target()->index()))
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
    if (dest->secrecType ()->isScalar ()) {
        i = new Imop (node, iType, dest, arg1, arg2);
    }
    else {
        i = new Imop (node, iType, dest, arg1, arg2, getSizeSymbol (dest));
    }

    return i;
}

Imop* newUnary (TreeNode* node, Imop::Type iType, Symbol *dest, Symbol *arg1) {
    Imop* i = 0;
    if (dest->secrecType ()->isScalar ()) {
        i = new Imop (node, iType, dest, arg1);
    }
    else {
        i = new Imop (node, iType, dest, arg1, getSizeSymbol (dest));
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
    if (dest->secrecType ()->isScalar ()) {
        i = new Imop (node, iType, dest);
    }
    else {
        i = new Imop (node, iType, dest, getSizeSymbol (dest));
    }

    return i;
}

Imop::~Imop() {
    typedef std::set<Imop*>::const_iterator ISCI;
    typedef std::vector<Symbol const* >::iterator SVI;
    for (SVI it (m_args.begin ()); it != m_args.end (); ++ it) {
        *it = 0;
    }
}

bool Imop::isJump () const {
    return getImopInfoBits (m_type).isJump;
}

bool Imop::isCondJump() const {
    return isJump () && (m_type != JUMP);
}

bool Imop::isExpr() const {
    return getImopInfoBits (m_type).isExpr;
}

bool Imop::isTerminator (void) const {
    return getImopInfoBits (m_type).isTerminator;
}

bool Imop::isVectorized () const {
    const size_t argNum = m_args.size ();
    return getImopInfoBits (m_type).vecArgNum == argNum;
}

Imop::OperandConstRange Imop::useRange () const {
    size_t off = 0;
    if (! isVectorized ()) {
        off = getImopInfoBits (m_type).useBegin;
        if (off > m_args.size ())
            off = m_args.size ();
    }

    OperandConstIterator i = m_args.begin () + off;
    OperandConstIterator e = i;
    for (; e != m_args.end () && *e != 0; ++ e);
    return std::make_pair (i, e);
}

Imop::OperandConstRange Imop::defRange () const {
    OperandConstIterator i = operandsBegin ();
    const OperandConstIterator e = operandsEnd ();

    // vectorised operantions don't DEF any operands.
    if (isVectorized ()) return std::make_pair (i, i);
    if (! getImopInfoBits (m_type).writesDest) {
        return std::make_pair (i, i);
    }

    if (type () == CALL) {
        for  (++ i ; *i != 0 && i != e; ++ i);
        if (i != m_args.end () && *i == 0) ++ i;
        return std::make_pair (i, e);
    }

    return std::make_pair (i, i + 1);
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
    assert (dest != 0);
    assert (isJump ());
    setDest (dest);
}

void Imop::setCallDest(SymbolProcedure *proc) {
    assert (proc != 0);
    assert (m_type == CALL);
    assert (proc->target () != 0);
    setDest (proc);
}

void Imop::setReturnDestFirstImop (SymbolLabel *label) {
    assert (label != 0);
    assert (label->target () != 0);
    assert (label->target ()->m_type == COMMENT);
    setDest (label);
}

std::string Imop::toString() const {
    std::ostringstream os;
    switch (m_type) {
        case ASSIGN:       /*   d = arg1;                        */
            os << dname << " = " << a1name;
            if (nArgs() == 3) os <<  "(" << a2name << ")";
            break;
        case CAST:         /* d = CAST (arg1 {, arg2})           */
            os << dname << " = CAST(" << a1name;
            if (nArgs() == 3) os << ", " << a2name;
            os << ")";
            break;
        case TOSTRING:       /*   d = TOSTRING arg1;             */
            os << dname << " = TOSTRING " << a1name;
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
        case COPY:
            os << dname << " = COPY " << a1name << " " << a2name;
            break;
        case RELEASE:
            os << "RELEASE " << a1name;
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
        case MUL:          /*   d = arg1 * arg2;                 */
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

                bool isFirst = true;
                BOOST_FOREACH (const Symbol* sym, defRange ()) {
                    if (! isFirst) {
                        os << ", ";
                        isFirst = false;
                    }

                    os << symToString (sym);

                }

                if (! isFirst)
                    os << " = ";

                os << "CALL " << symToString (m_args[0]) << " (";
                isFirst = true;
                BOOST_FOREACH (const Symbol* sym, useRange ()) {
                    if (! isFirst) {
                        os << ", ";
                        isFirst = false;
                    }

                    os << symToString (sym);
                }

                os << ");";
            }
            break;
        case PARAM:
            os << ".PARAM " << dname;
            break;
        case DOMAINID:
            os << dname << " = DOMAINID " << a1name;
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
        case COMMENT:      /* // arg1                            */
            assert (arg1 () != 0);
            assert (dynamic_cast<const ConstantString*>(arg1()) != 0);
            os << "// " << static_cast<const ConstantString*>(arg1())->value ();
            break;
        case ERROR:        /* // arg1                            */
            os << "ERROR " <<  a1name;
            break;
        case PRINT:
            os << "PRINT " << a1name;
            break;
        case SYSCALL:
            assert (arg1 () != 0);
            assert (dynamic_cast<const ConstantString*>(arg1()) != 0);
            os << "__SYSCALL \"" << static_cast<const ConstantString*>(arg1())->value () << "\"";
            break;
        case PUSH:
            os << "__PUSH " << a1name;
            break;
        case PUSHREF:
            os << "__PUSHREF " << a1name;
            break;
        case PUSHCREF:
            os << "__PUSHCREF " << a1name;
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

    return os.str();
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Imop &i) {
    (void) i;
    out << i.toString();
    return out;
}

