/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "Imop.h"

#include "Blocks.h"
#include "Constant.h"
#include "SecurityType.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "Types.h"

#include <algorithm>
#include <boost/range.hpp>
#include <iostream>
#include <sstream>

namespace SecreC {

namespace /* anonymous*/ {

#ifndef UD
#define UD (~static_cast<unsigned>(0))
#endif

struct ImopInfoBits {
    unsigned  type;              ///< Opcode
    bool      isExpr : 1;        ///< Expression (why do we even need this?)
    bool      defIfPrivate : 1;  ///< Is destination DEF if it's private.
    bool      isJump : 1;        ///< Instruction is procedure-local jump
    bool      isTerminator : 1;  ///< Instruction terminates a basic block
    bool      writesDest : 1;    ///< Instruction may write to destination operand
    unsigned  vecArgNum;         ///< Vectorised if number of operands equals to that
    unsigned  useBegin;          ///< Points to first USE operand, ends with 0 or with last operand.
};

ImopInfoBits imopInfo [Imop::_NUM_INSTR] = {
    //{ Imop::Type,       E, D, J, T, W, V, U }
    // Unary operators:
      { Imop::DECLARE,    1, 1, 0, 0, 1,UD, 1 }
    , { Imop::ASSIGN,     1, 0, 0, 0, 1, 3, 1 }
    , { Imop::CAST,       1, 0, 0, 0, 1, 3, 1 }
    , { Imop::CLASSIFY,   1, 0, 0, 0, 1, 3, 1 }
    , { Imop::DECLASSIFY, 1, 0, 0, 0, 1, 3, 1 }
    , { Imop::UINV,       1, 0, 0, 0, 1, 3, 1 }
    , { Imop::UNEG,       1, 0, 0, 0, 1, 3, 1 }
    , { Imop::UMINUS,     1, 0, 0, 0, 1, 3, 1 }
    , { Imop::TOSTRING,   1, 0, 0, 0, 0,UD, 1 }
    , { Imop::STRLEN,     1, 0, 0, 0, 1,UD, 1 }
    // Binary operators:
    , { Imop::MUL,        1, 0, 0, 0, 1, 4, 1 }
    , { Imop::DIV,        1, 0, 0, 0, 1, 4, 1 }
    , { Imop::MOD,        1, 0, 0, 0, 1, 4, 1 }
    , { Imop::ADD,        1, 0, 0, 0, 1, 4, 1 }
    , { Imop::SUB,        1, 0, 0, 0, 1, 4, 1 }
    , { Imop::EQ,         1, 0, 0, 0, 1, 4, 1 }
    , { Imop::NE,         1, 0, 0, 0, 1, 4, 1 }
    , { Imop::LE,         1, 0, 0, 0, 1, 4, 1 }
    , { Imop::LT,         1, 0, 0, 0, 1, 4, 1 }
    , { Imop::GE,         1, 0, 0, 0, 1, 4, 1 }
    , { Imop::GT,         1, 0, 0, 0, 1, 4, 1 }
    , { Imop::LAND,       1, 0, 0, 0, 1, 4, 1 }
    , { Imop::LOR,        1, 0, 0, 0, 1, 4, 1 }
    , { Imop::BAND,       1, 0, 0, 0, 1, 4, 1 }
    , { Imop::BOR,        1, 0, 0, 0, 1, 4, 1 }
    , { Imop::XOR,        1, 0, 0, 0, 1, 4, 1 }
    , { Imop::SHL,        1, 0, 0, 0, 1, 4, 1 }
    , { Imop::SHR,        1, 0, 0, 0, 1, 4, 1 }
    // Array expressions:
    , { Imop::STORE,      0, 0, 0, 0, 1,UD, 0 }
    , { Imop::LOAD,       1, 0, 0, 0, 0,UD, 1 }
    , { Imop::ALLOC,      1, 1, 0, 0, 0,UD, 1 }
    , { Imop::COPY,       1, 1, 0, 0, 0,UD, 1 }
    , { Imop::RELEASE,    0, 0, 0, 0, 0,UD, 1 }
    // Other expressions:
    , { Imop::PARAM,      1, 1, 0, 0, 0,UD, 1 }
    , { Imop::DOMAINID,   1, 0, 0, 0, 0,UD,UD }
    , { Imop::CALL,       1, 1, 0, 1, 0,UD, 1 }
    , { Imop::GETFPUSTATE,1, 0, 0, 0, 1,UD, 1 }
    , { Imop::SETFPUSTATE,0, 0, 0, 0, 0,UD, 1 }
    // Jumps:
    , { Imop::JUMP,       0, 0, 1, 1, 0,UD, 1 }
    , { Imop::JT,         0, 0, 1, 1, 0,UD, 1 }
    , { Imop::JF,         0, 0, 1, 1, 0,UD, 1 }
    // Terminators:
    , { Imop::ERROR,      0, 0, 0, 1, 0,UD,UD }
    , { Imop::RETURN,     0, 0, 0, 1, 0,UD, 1 }
    , { Imop::END,        0, 0, 0, 1, 0,UD,UD }
    // Other:
    , { Imop::COMMENT,    0, 0, 0, 0, 0,UD,UD }
    , { Imop::PRINT,      0, 0, 0, 0, 0,UD,UD }
    , { Imop::SYSCALL,    1, 0, 0, 0, 0,UD, 1 } // DEF if private?
    , { Imop::RETCLEAN,   0, 0, 0, 0, 0,UD,UD }
};

const ImopInfoBits& getImopInfoBits (Imop::Type type) {
    assert (0 <= type && type < Imop::_NUM_INSTR);
    const ImopInfoBits& out = imopInfo [type];
    assert (out.type == type);
    return out;
}

SecreC::Symbol* getSizeSymbol (SecreC::Symbol* sym) {
    assert (sym != nullptr);
    assert (dynamic_cast<SecreC::SymbolSymbol* >(sym) != nullptr);
    return static_cast<SecreC::SymbolSymbol*>(sym)->getSizeSym ();
}

class SymbolWrapperBase {
protected: /* Methods: */
    SymbolWrapperBase (const SecreC::Symbol * symbol) : m_symbol (symbol) { }
protected: /* Fields: */
    const SecreC::Symbol * const m_symbol;
};

struct SymbolOstreamWrapper : public SymbolWrapperBase {

    SymbolOstreamWrapper (const SecreC::Symbol * symbol)
        : SymbolWrapperBase (symbol)
    { }

    inline void print (std::ostream& os) const {
        if (! m_symbol) {
            os << '_';
            return;
        }

        os << *m_symbol;
    }
};

struct LabelOstreamWrapper : public SymbolWrapperBase  {

    LabelOstreamWrapper (const SecreC::Symbol * symbol)
        : SymbolWrapperBase (symbol)
    { }

    inline void print (std::ostream& os) const {
        if (! m_symbol) {
            os << '_';
            return;
        }

        assert (dynamic_cast<const SymbolLabel*>(m_symbol) != nullptr);
        const auto dest = static_cast<const SymbolLabel*>(m_symbol);
        if (dest->block ())
            os << "[Block " << dest->block ()->index () << "]";
        else
            os << "[" << dest->target ()->index () << "]";
    }
};

std::ostream& operator << (std::ostream & os, const SymbolOstreamWrapper & wrapper) { wrapper.print (os); return os; }
std::ostream& operator << (std::ostream & os, const LabelOstreamWrapper & wrapper) { wrapper.print (os); return os; }

#define dname  (SymbolOstreamWrapper (imop.dest()))
#define tname  (LabelOstreamWrapper (imop.dest()))
#define a1name (SymbolOstreamWrapper (imop.arg1()))
#define a2name (SymbolOstreamWrapper (imop.arg2()))
#define a3name (SymbolOstreamWrapper (imop.arg3()))

void printBinaryArith (std::ostream& os, const Imop& imop, const char* opname) {
    os << dname << " = " << a1name << ' ' << opname << ' ' << a2name;
    if (imop.nArgs () == 4)
        os << " (" << a3name << ")";
}

void printUnaryArith (std::ostream& os, const Imop& imop, const char* opname) {
    os << dname << " = " << opname << ' ' << a1name;
    if (imop.nArgs () == 3)
        os << " (" << a2name << ")";
}

// Has private destination that needs to be skipped as DEF.
bool hasUSEPrivateDest (const Imop& imop) {
    return
        imop.isExpr () &&
        imop.nArgs () > 0 &&
        imop.dest () &&
        ! getImopInfoBits (imop.type ()).defIfPrivate &&
        imop.dest ()->secrecType ()->secrecSecType ()->isPrivate ();
}

} // anonymous namespace


Imop::Imop(TreeNode *creator, Type type)
    : m_creator(creator)
    , m_type(type)
    , m_args()
{ }

Imop::Imop(TreeNode *creator, Type type, Symbol *dest)
    : m_creator(creator)
    , m_type(type)
    , m_args(1, dest)
{ }

Imop::Imop(TreeNode *creator, Type type, Symbol *dest,
                     Symbol *arg1)
    : m_creator(creator)
    , m_type(type)
    , m_args(2)
{
    m_args[0] = dest;
    m_args[1] = arg1;
}

Imop::Imop(TreeNode *creator, Type type, Symbol *dest,
                     Symbol *arg1, Symbol *arg2)
    : m_creator(creator)
    , m_type(type)
    , m_args(3)
{
    m_args[0] = dest;
    m_args[1] = arg1;
    m_args[2] = arg2;
}

Imop::Imop(TreeNode *creator, Type type, Symbol *dest,
                     Symbol *arg1, Symbol *arg2, Symbol *arg3)
    : m_creator(creator)
    , m_type(type)
    , m_args(4)
{
    m_args[0] = dest;
    m_args[1] = arg1;
    m_args[2] = arg2;
    m_args[3] = arg3;
}

Imop::Imop(TreeNode* creator, Type type, OperandList args)
    : m_creator (creator)
    , m_type (type)
    , m_args (std::move(args))
{ }

Imop::Imop(TreeNode *creator, ConstantString *name, SyscallOperands operands)
    : m_creator(creator)
    , m_type(Imop::SYSCALL)
    , m_syscallOperands(sharemind::inPlace, std::move(operands))
{
    m_args.reserve(2 + m_syscallOperands->size());
    m_args.push_back(nullptr);
    m_args.push_back(name);
    for (const SyscallOperand& operand : *m_syscallOperands) {
        const auto sym = operand.operand();
        if (operand.passingConvention() == Return) {
            assert (m_args[0] == nullptr);
            if (m_args[0] != nullptr) {
                // TODO: throw
                exit(EXIT_FAILURE);
            }

             m_args[0] = sym;
        }
        else {
            m_args.push_back(sym);
        }
    }
}

Imop* newError (TreeNode* node, ConstantString* msg) {
    return new Imop(node, Imop::ERROR, static_cast<Symbol *>(nullptr), msg);
}

Imop* newAssign (TreeNode* node, Symbol* dest, Symbol* from) {
    return newUnary (node, Imop::ASSIGN, dest, from);
}

Imop* newBinary (TreeNode* node, Imop::Type iType, Symbol *dest, Symbol *arg1, Symbol *arg2) {
    Imop* i = nullptr;
    if (dest->secrecType ()->isScalar ()) {
        i = new Imop (node, iType, dest, arg1, arg2);
    }
    else {
        i = new Imop (node, iType, dest, arg1, arg2, getSizeSymbol (dest));
    }

    return i;
}

Imop* newUnary (TreeNode* node, Imop::Type iType, Symbol *dest, Symbol *arg1) {
    Imop* i = nullptr;
    if (dest->secrecType ()->isScalar ()) {
        i = new Imop (node, iType, dest, arg1);
    }
    else {
        i = new Imop (node, iType, dest, arg1, getSizeSymbol (dest));
    }

    return i;
}

Imop* newCall (TreeNode* node) {
   return new Imop(node,
                   Imop::CALL,
                   static_cast<Symbol *>(nullptr),
                   static_cast<Symbol *>(nullptr));
}


Imop* newNullary (TreeNode* node, Imop::Type iType, Symbol *dest) {
    assert (dest != nullptr);
    Imop* i = nullptr;
    if (dest->secrecType ()->isScalar ()) {
        i = new Imop (node, iType, dest);
    }
    else {
        i = new Imop (node, iType, dest, getSizeSymbol (dest));
    }

    return i;
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

bool Imop::isComment () const {
    return m_type == COMMENT;
}

bool Imop::writesDest () const {
    if (! isVectorized ()) {
        return m_type == Imop::STORE;
    }

    return getImopInfoBits (m_type).writesDest;
}

bool Imop::isSyscall() const {
    return static_cast<bool>(m_syscallOperands);
}

const SyscallOperands& Imop::syscallOperands() const {
    assert (isSyscall());
    return *m_syscallOperands;
}

Imop::OperandConstRange Imop::useRange () const {
    size_t off = 0;

    if (! isVectorized ()) {
        off = std::min (static_cast<unsigned>(m_args.size ()),
                        getImopInfoBits (m_type).useBegin);
    }

    if (hasUSEPrivateDest (*this)) {
        off = 0;
    }

    const OperandConstIterator i = m_args.begin () + off;
    OperandConstIterator e = i;
    for (; e != m_args.end () && *e != nullptr; ++ e);
    return OperandConstRange (i, e);
}

Imop::OperandConstRange Imop::defRange () const {
    auto i = operandsBegin ();
    const OperandConstIterator e = operandsEnd ();

    if (type () == SYSCALL) {
        return dest () ? OperandConstRange (i, i + 1) : OperandConstRange (e, e);
    }

    if (type () == CALL) {
        for  (++ i ; *i != nullptr && i != e; ++ i);
        if (i != m_args.end () && *i == nullptr) ++ i;
        return OperandConstRange (i, e);
    }

    if (hasUSEPrivateDest (*this)) {
        return OperandConstRange (e, e);
    }

    // vectorised operations don't DEF any operands.
    if (isVectorized () || ! getImopInfoBits (m_type).isExpr) {
        return OperandConstRange (e, e);
    }

    return OperandConstRange (i, i + 1);
}

const Imop *Imop::callDest() const {
    assert(m_type == CALL);
    assert(dest()->symbolType() == SYM_PROCEDURE);
    assert(dynamic_cast<const SymbolProcedure*>(dest()) != nullptr);
    assert(static_cast<const SymbolProcedure*>(dest())->target () != nullptr);
    return static_cast<const SymbolProcedure*>(dest())->target ();
}

SymbolLabel* Imop::jumpDest() const {
    Symbol* sym = dest ();
    assert (dynamic_cast<SymbolLabel*>(sym) != nullptr);
    return static_cast<SymbolLabel*>(sym);
}

void Imop::print(std::ostream & os) const {
    const Imop& imop = *this;
    switch (m_type) {
    case DECLARE:
        os << dname << " = UNDEF;";
        break;
    case ASSIGN:       /*   d = arg1;                        */
        os << dname << " <- " << a1name;
        if (nArgs() == 3) os <<  " (" << a2name << ")";
        break;
    case TOSTRING:       /*   d = TOSTRING arg1;             */
        os << dname << " = TOSTRING " << a1name;
        break;
    case ALLOC:
        if (nArgs () == 3) {
            os << dname << " = ALLOC " << a1name << " " << a2name;
        } else {
            os << dname << " = ALLOC " << a1name << " UNDEF";
        }
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
    case CAST: printUnaryArith (os, imop, "CAST"); break;
    case CLASSIFY: printUnaryArith (os, imop, "CLASSIFY"); break;
    case DECLASSIFY: printUnaryArith (os, imop, "DECLASSIFY"); break;
    case UINV: printUnaryArith (os, imop, "~"); break;
    case UNEG: printUnaryArith (os, imop, "!"); break;
    case UMINUS: printUnaryArith (os, imop, "-"); break;
    case MUL: printBinaryArith (os, imop, "*"); break;
    case DIV: printBinaryArith (os, imop, "/"); break;
    case MOD: printBinaryArith (os, imop, "%"); break;
    case ADD: printBinaryArith (os, imop, "+"); break;
    case SUB: printBinaryArith (os, imop, "-"); break;
    case EQ: printBinaryArith (os, imop, "==");  break;
    case NE: printBinaryArith (os, imop, "!=");  break;
    case LE: printBinaryArith (os, imop, "<=");  break;
    case LT: printBinaryArith (os, imop, "<");  break;
    case GE: printBinaryArith (os, imop, ">=");  break;
    case GT: printBinaryArith (os, imop, ">");  break;
    case LAND: printBinaryArith (os, imop, "&&"); break;
    case LOR: printBinaryArith (os, imop, "||"); break;
    case BAND: printBinaryArith (os, imop, "&"); break;
    case BOR: printBinaryArith (os, imop, "|"); break;
    case XOR: printBinaryArith (os, imop, "^"); break;
    case SHL: printBinaryArith (os, imop, "<<"); break;
    case SHR: printBinaryArith (os, imop, ">>"); break;
    case CALL:
    {

        bool isFirst = true;
        for (const Symbol* sym : defRange ()) {
            if (! isFirst) {
                os << ", ";
            }

            os << SymbolOstreamWrapper (sym);
            isFirst = false;
        }

        if (! isFirst)
            os << " = ";

        os << "CALL \"" << m_args[0]->name () << "\" ";
        isFirst = true;
        for (const Symbol* sym : useRange ()) {
            if (! isFirst) {
                os << " ";
            }

            os << SymbolOstreamWrapper (sym);
            isFirst = false;
        }
    }
        break;
    case PARAM:
        os << "PARAM " << dname;
        break;
    case DOMAINID:
        os << dname << " = DOMAINID " << *static_cast<SymbolDomain*>(imop.arg1 ())->securityType ();
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
    case COMMENT:      /* // arg1                            */
        assert (arg1 () != nullptr);
        assert (dynamic_cast<const ConstantString*>(arg1()) != nullptr);
        os << "// " << static_cast<const ConstantString*>(arg1())->value ();
        break;
    case ERROR:        /* // arg1                            */
        os << "ERROR " <<  a1name;
        break;
    case PRINT:
        os << "PRINT " << a1name;
        break;
    case SYSCALL:
        assert (arg1 () != nullptr);
        assert (dynamic_cast<const ConstantString*>(arg1()) != nullptr);
        os << "__SYSCALL \"" << static_cast<const ConstantString*>(arg1())->value () << "\"";
        for (auto op : imop.syscallOperands()) {
            const auto sym = op.operand();
            switch(op.passingConvention()) {
            case Return:
                os << " __return " << SymbolOstreamWrapper(sym);
                break;
            case Push:
                os << " __push " << SymbolOstreamWrapper(sym);
                break;
            case PushRef:
                os << " __pushref " << SymbolOstreamWrapper(sym);
                break;
            case PushCRef:
                os << " __pushcref " << SymbolOstreamWrapper(sym);
                break;
            }
        }
        break;
    case STRLEN:
        os << dname << " = STRLEN " << a1name;
        break;
    case RETCLEAN:     /* RETCLEAN;       (clean call stack) */
        os << "RETCLEAN";
        break;
    case RETURN:       /* RETURN arg1;                       */
        os << "RETURN ";
    {
        const OperandConstIterator itBegin = m_args.begin () + 1; // !
        const OperandConstIterator itEnd = m_args.end ();
        for (OperandConstIterator it = itBegin; it != itEnd; ++ it) {
            if (it != itBegin) {
                os << ", ";
            }

            const Symbol* sym = *it;
            os << SymbolOstreamWrapper (sym);
        }
    }
        break;
    case END:          /* END PROGRAM                        */
        os << "END";
        break;
    case GETFPUSTATE:
        os << dname << " = __FPU_STATE";
        break;
    case SETFPUSTATE:
        os << "__SET_FPU_STATE(" << a1name << ')';
        break;
    default:
        os << "TODO";
        break;
    }
}

} // namespace SecreC
