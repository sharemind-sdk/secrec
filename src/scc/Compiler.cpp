/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Compiler.h"
#include "RegisterAllocator.h"
#include "Builtin.h"
#include "TargetInfo.h"

#include <libscc/treenode.h>
#include <libscc/dataflowanalysis.h>
#include <libscc/intermediate.h>
#include <libscc/blocks.h>

#include <iostream>

using namespace SecreC;

namespace {

using namespace SecreCC;

const char* imopToVMName (const Imop& imop) {
    switch (imop.type ()) {
    case Imop::UMINUS: return "bneg";
    case Imop::UNEG  : return "bnot";
    case Imop::MUL   : return "tmul";
    case Imop::DIV   : return "tdiv";
    case Imop::MOD   : return "tmod";
    case Imop::ADD   : return "tadd";
    case Imop::SUB   : return "tsub";
    case Imop::LAND  : return "ltand";
    case Imop::LOR   : return "ltor";
    case Imop::EQ    : return "teq";
    case Imop::NE    : return "tne";
    case Imop::LE    : return "tle";
    case Imop::LT    : return "tlt";
    case Imop::GE    : return "tge";
    case Imop::GT    : return "tgt";
    default:
        assert (false && "Not an arithmetic instruction!");
        return 0;
    }
}

/**
 * Functions for mapping SecreC symbols to VM values:
 */

VMLabel* getLabel (VMSymbolTable& st, const Block& block) {
    std::stringstream ss;
    ss << ":L_" << block.index ();
    return st.getLabel (ss.str ());
}

VMLabel* getLabel (VMSymbolTable& st, const Symbol* sym) {
    assert (sym != 0);
    assert (sym->symbolType () == Symbol::LABEL);
    VMValue* label = st.find (sym);
    if (label == 0) {
        const SymbolLabel* symL = static_cast<const SymbolLabel*>(sym);
        assert (symL->target () != 0);
        assert (symL->target ()->block () != 0);
        label = getLabel (st, *symL->target ()->block ());
        st.store (sym, label);
    }

    assert (dynamic_cast<VMLabel*>(label) != 0);
    return static_cast<VMLabel*>(label);
}

VMLabel* getProc (VMSymbolTable& st, const Symbol* sym) {
    assert (sym != 0);
    assert (sym->symbolType () == Symbol::PROCEDURE);
    VMValue* label = st.find (sym);
    if (label == 0) {
        std::stringstream ss;
        const SymbolProcedure* proc = static_cast<const SymbolProcedure*>(sym);
        ss << ':' << proc->decl ()->procedureName () << '_' << st.uniq ();
        label = st.getLabel (ss.str ());
        st.store (sym, label);
    }

    assert (dynamic_cast<VMLabel*>(label) != 0);
    return static_cast<VMLabel*>(label);
}

VMValue* getImm (VMSymbolTable& st, const Symbol* sym) {
    assert (sym != 0);
    assert (sym->symbolType () == Symbol::CONSTANT);
    VMValue* imm = st.find (sym);
    if (imm == 0) {
        uint64_t value = 0xdeadbeef;
        switch (sym->secrecType ().secrecDataType ()) {
            case DATATYPE_BOOL:   value = static_cast<const ConstantBool*>(sym)->value ();   break;
            case DATATYPE_INT8:   value = static_cast<const ConstantInt8*>(sym)->value ();   break;
            case DATATYPE_INT16:  value = static_cast<const ConstantInt16*>(sym)->value ();  break;
            case DATATYPE_INT32:  value = static_cast<const ConstantInt32*>(sym)->value ();  break;
            case DATATYPE_INT64:  value = static_cast<const ConstantInt64*>(sym)->value ();  break;
            case DATATYPE_INT:    value = static_cast<const ConstantInt*>(sym)->value ();    break;
            case DATATYPE_UINT8:  value = static_cast<const ConstantUInt8*>(sym)->value ();  break;
            case DATATYPE_UINT16: value = static_cast<const ConstantUInt16*>(sym)->value (); break;
            case DATATYPE_UINT32: value = static_cast<const ConstantUInt32*>(sym)->value (); break;
            case DATATYPE_UINT64: value = static_cast<const ConstantUInt64*>(sym)->value (); break;
            case DATATYPE_UINT:   value = static_cast<const ConstantUInt*>(sym)->value ();   break;
            case DATATYPE_STRING:
                assert (false && "No support for strings yet!");
            default:
                assert (false);
                break;
        }

        imm = st.getImm (value);
        st.store (sym, imm);
    }

    assert (dynamic_cast<VMImm*>(imm) != 0);
    return imm;
}

}

namespace SecreCC {

class Compiler::SyscallManager {
private: /* Types: */

    typedef std::map<const ConstantString*, VMLabel*> SCMap;

public: /* Methods: */

    SyscallManager () : m_st (0) { }
    ~SyscallManager () { }

    void init (VMSymbolTable& st) {
        m_st = &st;
    }

    void emitBindings (VMCode& code) {
        for (SCMap::iterator i = m_syscalls.begin (); i != m_syscalls.end (); ++ i) {
            code.addBinding (i->second, i->first->value ());
        }
    }

    VMLabel* getSyscallBinding (const ConstantString* name) {
        SCMap::iterator i = m_syscalls.find (name);
        if (i == m_syscalls.end ()) {
            std::ostringstream ss;
            ss << ":SC_" << m_st->uniq ();
            VMLabel* label = m_st->getLabel (ss.str ());
            i = m_syscalls.insert (i, std::make_pair (name, label));
        }

        return i->second;
    }

private: /* Fields: */

    VMSymbolTable* m_st;
    SCMap m_syscalls;
};

/*******************************************************************************
  Compiler
*******************************************************************************/

Compiler::Compiler (ICode& code)
    : m_code (code)
    , m_param (0)
    , m_funcs (new BuiltinFunctions ())
    , m_ra (new RegisterAllocator ())
    , m_scm (new Compiler::SyscallManager ())
{ }

Compiler::~Compiler () {
    delete m_funcs;
    delete m_ra;
    delete m_scm;
}

void Compiler::run () {
    DataFlowAnalysisRunner runner;
    LiveVariables lv;
    runner.addAnalysis (&lv);
    runner.run (m_code.program ());
    m_ra->init (m_st, lv);
    m_scm->init (m_st);
    for (Program::iterator i = m_code.program ().begin (), e = m_code.program ().end (); i != e; ++ i) {
        cgProcedure (*i);
    }

    m_funcs->generateAll (m_target, m_st);
    m_target.setNumGlobals (m_ra->globalCount ());
    m_scm->emitBindings (m_target);
}


VMValue* Compiler::loadToRegister (VMBlock &block, const Symbol *symbol) {
    VMValue* reg = 0; // this holds the value of symbol
    if (symbol->symbolType () == SecreC::Symbol::CONSTANT) {
        reg = m_ra->temporaryReg (); // temporary register
        block.push_back (
            VMInstruction () << "mov " << m_st.find (symbol) << reg
        );
    }
    else {
        reg = m_st.find (symbol);
    }

    return reg;
}

void Compiler::cgProcedure (const Procedure& blocks) {
    VMLabel* name = 0; 
    if (blocks.name () == 0)
        name = st ().getLabel (":start"); // NULL instead?
    else
        name = getProc (st (), blocks.name ());
    m_param = 0;
    VMFunction function (name);
    if (blocks.name () == 0)
        function.setIsStart ();

    m_ra->enterFunction (function);
    for (Procedure::const_iterator i = blocks.begin (), e = blocks.end (); i != e; ++ i) {
        const Block& block = *i;
        if (block.reachable ()) {
            cgBlock (function, block);
        }
    }

    m_ra->exitFunction (function);
    m_target.push_back (function);
}

void Compiler::cgBlock (VMFunction& function, const Block& block) {
    typedef Block::const_iterator BCI;
    VMLabel* name = getLabel (st (), block);
    VMBlock vmBlock (name, &block);
    m_ra->enterBlock (vmBlock);
    for (BCI i = block.begin (), e = block.end (); i != e; ++ i) {
        cgImop (vmBlock, *i);
    }
    m_ra->exitBlock (vmBlock);
    function.push_back (vmBlock);
}

void Compiler::cgJump (VMBlock& block, const Imop& imop) {
    typedef Imop::OperandConstIterator OCI;
    assert (imop.isJump ());

    const char* name = 0;
    switch (imop.type ()) {
        case Imop::JUMP: name = "jmp"; break;
        case Imop::JT  : name = "jnz"; break;
        case Imop::JF  : name = "jz";  break;
        case Imop::JE  : name = "jeq"; break;
        case Imop::JNE : name = "jne"; break;
        case Imop::JLE : name = "jle"; break;
        case Imop::JLT : name = "jlt"; break;
        case Imop::JGE : name = "jge"; break;
        case Imop::JGT : name = "jgt"; break;
        default:
            assert (false && "Unknown jump instruction");
            break;
    }

    // jump target
    VMInstruction instr;
    instr << name << getLabel (st (), imop.jumpDest ());

    // type of arguments (if needed)
    if (imop.type () != Imop::JUMP) {
        instr << secrecDTypeToVMDType (imop.arg1 ()->secrecType ().secrecDataType ());
    }

    // arguments
    OCI i = imop.operandsBegin (),
        e = imop.operandsEnd ();
//    if (i != e)
        for (++ i; i != e; ++ i)
            instr << loadToRegister (block, *i);

    block.push_back (instr);
}

void Compiler::cgAlloc (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ALLOC);

    VMInstruction pushDef;
    pushDef << "push" << m_st.find (imop.arg1 ());
    block.push_back (pushDef);

    VMInstruction pushSize;
    pushSize << "push" << m_st.find (imop.arg2 ());
    block.push_back (pushSize);

    VMLabel* target = 0;
    unsigned size = secrecDTypeSize (imop.arg1 ()->secrecType ().secrecDataType ());
    {
        std::stringstream ss;
        ss << ":secrecAlloc_" << size;
        target = m_st.getLabel (ss.str ());
    }

    m_funcs->insert (target, BuiltinAlloc (size));

    block.push_back (
        VMInstruction ()
            << "call"
            << target
            << m_st.find (imop.dest ())
    );
}

void Compiler::cgAssign (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ASSIGN   ||
            imop.type () == Imop::CLASSIFY ||
            imop.type () == Imop::DECLASSIFY);

    VMInstruction instr;
    instr << "mov";

    if (imop.isVectorized ()) {
        VMValue* rSize = m_ra->temporaryReg ();
        VMValue* rNum = m_st.find (imop.arg2 ());
        const unsigned elemSize = secrecDTypeSize (imop.arg1 ()->secrecType ().secrecDataType ());
        block.push_back (
            VMInstruction ()
                << "tmul uint64" << rSize << rNum
                << m_st.getImm (elemSize)
        );

        instr << "mem" << m_st.find (imop.arg1 ()) << "imm 0x0";
        instr << "mem" << m_st.find (imop.dest ()) << "imm 0x0";
        instr << rSize;
    }
    else {
        instr << m_st.find (imop.arg1 ()) << m_st.find (imop.dest ());
    }

    block.push_back (instr);
}

void Compiler::cgCall (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::CALL);
    Imop::OperandConstIterator it, itBegin, itEnd;

    itBegin = imop.operandsBegin ();
    itEnd = imop.operandsEnd ();
    it = itBegin;

    // compute destinations for calls
    const Symbol* dest = *itBegin;

    // push arguments
    for (++ it; it != itEnd && *it != 0; ++ it) {
        block.push_back (
            VMInstruction () << "pushcref" << m_st.find (*it)
        );
    }

    assert (it != itEnd && *it == 0 &&
        "Malformed CALL instruction!");

    for (++ it; it != itEnd; ++ it) {
        block.push_back (
            VMInstruction () << "pushref" << m_st.find (*it)
        );
    }

    // CALL
    block.push_back (
        VMInstruction ()
            << "call" << getProc (st (), dest) << "imm"
    );
}

void Compiler::cgParam (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::PARAM);
    block.push_back (
        VMInstruction ()
            << "mov cref"
            << m_st.getImm (m_param ++)
            << "0x0" // offset 0
            << m_st.find (imop.dest ())
            << m_st.getImm (secrecDTypeSize (imop.dest ()->secrecType ().secrecDataType ()))
    );
}

void Compiler::cgReturn (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::RETURN);

    typedef Imop::OperandConstIterator OCI;

    assert (imop.operandsBegin () != imop.operandsEnd () &&
            "Malformed RETURN instruction!");

    OCI it = imop.operandsBegin ();
    OCI itEnd = imop.operandsEnd ();
    unsigned retCount = 0;
    for (++ it; it != itEnd; ++ it) {
        VMInstruction movI;
        movI << "mov"
             << m_st.find (*it);
        movI << "ref"
             << m_st.getImm (retCount ++ )
             << "0x0"
             << m_st.getImm (secrecDTypeSize ((*it)->secrecType ().secrecDataType ()));
        block.push_back (movI);
    }

    block.push_back (
        VMInstruction () << "return imm 0x0"
    );
}

void Compiler::cgLoad (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::LOAD);
    VMValue* rOffset = m_ra->temporaryReg ();

    VMInstruction tmpInstr;
    tmpInstr << "mov" << m_st.find (imop.arg2 ()) << rOffset;
    block.push_back (tmpInstr);

    const unsigned size = secrecDTypeSize (imop.arg1()->secrecType ().secrecDataType ());
    VMInstruction mulInstr;
    mulInstr << "bmul uint64" << rOffset << m_st.getImm (size);
    block.push_back (mulInstr);

    VMInstruction movInstr;
    movInstr << "mov mem";
    movInstr << m_st.find (imop.arg1 ());
    movInstr << rOffset;
    movInstr << m_st.find (imop.dest ());
    movInstr << m_st.getImm (size);
    block.push_back (movInstr);
}

void Compiler::cgStore (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::STORE);
    VMValue* rOffset = m_ra->temporaryReg ();

    VMInstruction tmpInstr;
    tmpInstr << "mov" << m_st.find (imop.arg1 ());
    tmpInstr << rOffset;
    block.push_back (tmpInstr);

    const unsigned size = secrecDTypeSize (imop.arg2()->secrecType ().secrecDataType ());
    VMInstruction mulInstr;
    mulInstr << "bmul uint64" << rOffset << m_st.getImm (size);
    block.push_back (mulInstr);

    VMInstruction movInstr;
    movInstr << "mov" << m_st.find (imop.arg2 ());
    movInstr << "mem" << m_st.find (imop.dest ()) << rOffset;
    movInstr << m_st.getImm (size);
    block.push_back (movInstr);
}

void Compiler::cgArithm (VMBlock& block, const Imop& imop) {
    assert (imop.isExpr ());

    if (imop.isVectorized ()) {
        for (Imop::OperandConstIterator i = imop.operandsBegin (), e = imop.operandsEnd (); i != e; ++ i) {
            VMInstruction instr;
            instr << "push" << m_st.find (*i);
            block.push_back (instr);
        }

        VMLabel* target = 0;
        {
            std::stringstream ss;
            ss << ':' << imopToVMName (imop) << "_vec_"
                      << secrecDTypeSize (imop.arg1 ()->secrecType ().secrecDataType ());
            target = st ().getLabel (ss.str ());
        }

        m_funcs->insert (target, BuiltinVArith (&imop));
        block.push_back (VMInstruction () << "call" << target << "imm");
        return;
    }

    const char* name = imopToVMName (imop);
    VMInstruction instr;
    instr << name
          << secrecDTypeToVMDType (imop.dest ()->secrecType ().secrecDataType ())
          << m_st.find (imop.dest ());

    Imop::OperandConstIterator 
        i = imop.operandsBegin (),
        e = imop.operandsEnd ();
    for (++ i; i != e; ++ i) {
        instr << loadToRegister (block, *i);
    }

    block.push_back (instr);
}

void Compiler::cgSyscall (VMBlock& block, const Imop& imop) {
    VMLabel* label = m_scm->getSyscallBinding (static_cast<const ConstantString*>(imop.arg1 ()));
    block.push_back (VMInstruction () << "syscall" << label << "imm");
}

void Compiler::cgPush (VMBlock& block, const Imop& imop) {
    assert (m_st.find (imop.arg1 ()) != 0);
    block.push_back (VMInstruction () << "push" << m_st.find (imop.arg1 ()));
}

void Compiler::cgImop (VMBlock& block, const Imop& imop) {

    m_ra->getReg (imop);

    if (imop.isJump ()) {
        cgJump (block, imop);
        return;
    }

    switch (imop.type ()) {
        case Imop::CLASSIFY:   // \todo
        case Imop::DECLASSIFY: // \todo
        case Imop::ASSIGN:
            cgAssign (block, imop);
            return;
        case Imop::ALLOC:
            cgAlloc (block, imop);
            return;
        case Imop::LOAD:
            cgLoad (block, imop);
            return;
        case Imop::STORE:
            cgStore (block, imop);
            return;
        case Imop::CALL:
            cgCall (block, imop);
            return;
        case Imop::PARAM:
            cgParam (block, imop);
            return;
        case Imop::RETURNVOID:
            block.push_back (VMInstruction () << "return imm 0x0");
            return;
        case Imop::RETURN:
            cgReturn (block, imop);
            return;
        case Imop::SYSCALL:
            cgSyscall (block, imop);
            return;
        case Imop::PUSH:
            cgPush (block, imop);
            return;
        case Imop::END:
            block.push_back (VMInstruction () << "halt imm 0x0");
            return;
        case Imop::ERROR:
            block.push_back (VMInstruction () << "halt imm 0xff");
            return;
        case Imop::RETCLEAN:
        case Imop::COMMENT:
            return;
        default:
            break;
    }

    if (imop.isExpr ()) {
        cgArithm (block, imop);
        return;
    }
    
    assert (false && "Unable to handle instruction!");
}

} // namespac SecreC
