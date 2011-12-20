/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Compiler.h"

#include <iostream>
#include <boost/foreach.hpp>

#include <libscc/treenode.h>
#include <libscc/dataflowanalysis.h>
#include <libscc/intermediate.h>
#include <libscc/blocks.h>
#include <libscc/constant.h>

#include "SyscallManager.h"
#include "StringLiterals.h"
#include "RegisterAllocator.h"
#include "Builtin.h"
#include "VMDataType.h"

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

}

namespace SecreCC {

/*******************************************************************************
  Compiler
*******************************************************************************/

Compiler::Compiler (ICode& code)
    : m_code (code)
    , m_target (0)
    , m_param (0)
    , m_funcs (0)
    , m_ra (0)
    , m_scm (0)
    , m_strLit (0)
{ }

Compiler::~Compiler () {
    delete m_funcs;
    delete m_ra;
    delete m_scm;
    delete m_strLit;
}

void Compiler::run (VMLinkingUnit& vmlu) {

    // Create and add the linking unit sections:
    VMDataSection* rodataSec = new VMDataSection (VMDataSection::RODATA);
    VMBindingSection* pdSec = new VMBindingSection ("PDBIND");
    VMBindingSection* scSec = new VMBindingSection ("BIND");
    VMCodeSection* codeSec = new VMCodeSection ();

    vmlu.addSection (pdSec);
    vmlu.addSection (scSec);
    vmlu.addSection (rodataSec);
    vmlu.addSection (codeSec);

    // Create and initialize components:
    m_funcs = new BuiltinFunctions ();
    m_ra = new RegisterAllocator ();
    m_scm = new SyscallManager ();
    m_strLit = new StringLiterals ();
    DataFlowAnalysisRunner runner;
    LiveVariables lva;

    runner.addAnalysis (&lva);
    runner.run (m_code.program ());
    m_target = codeSec;
    m_ra->init (m_st, lva);
    m_scm->init (m_st, scSec, pdSec);
    m_strLit->init (m_st, rodataSec);

    // Finally generate code:
    BOOST_FOREACH (const Procedure& proc, m_code.program ()) {
        cgProcedure (proc);
    }

    m_funcs->generateAll (*m_target, m_st);
    m_target->setNumGlobals (m_ra->globalCount ());

    // for safety, as lva variable falls out of scope here:
    m_ra->invalidateLVA ();
}

VMValue* Compiler::find (const SecreC::Symbol* sym) const {
    VMValue* const out = m_st.find (sym);
    assert (out != 0);
    return out;
}

VMValue* Compiler::loadToRegister (VMBlock &block, const Symbol *symbol) {
    VMValue* reg = 0; // this holds the value of symbol
    if (symbol->symbolType () == SecreC::Symbol::CONSTANT) {
        reg = m_ra->temporaryReg (); // temporary register
        block.push_back (
            VMInstruction () << "mov " << find (symbol) << reg
        );
    }
    else {
        reg = find (symbol);
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
    m_target->push_back (function);
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
        SecrecDataType scTy = imop.arg1 ()->secrecType ()->secrecDataType ();
        VMDataType ty = secrecDTypeToVMDType (scTy);
        assert (ty != VM_INVALID);
        instr << ty; // secrecDTypeToVMDType (imop.arg1 ()->secrecType ()->secrecDataType ());
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
    pushDef << "push" << find (imop.arg1 ());
    block.push_back (pushDef);

    VMInstruction pushSize;
    pushSize << "push" << find (imop.arg2 ());
    block.push_back (pushSize);

    VMLabel* target = 0;
    VMDataType ty = secrecDTypeToVMDType (imop.arg1 ()->secrecType ()->secrecDataType ());
    unsigned size = sizeInBytes (ty);
    {
        std::stringstream ss;
        ss << ":secrecAlloc_" << size;
        target = m_st.getLabel (ss.str ());
    }

    m_funcs->insert (target, BuiltinAlloc (size));

    block.push_back (
        VMInstruction () << "call" << target << find (imop.dest ())
    );
}

void Compiler::cgRelease (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::RELEASE);
    block.push_back (
        VMInstruction () << "free" << find (imop.arg1 ()));
}

void Compiler::cgAssign (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ASSIGN   ||
            imop.type () == Imop::CLASSIFY ||
            imop.type () == Imop::DECLASSIFY);

    VMInstruction instr;
    instr << "mov";

    if (imop.isVectorized ()) {
        VMValue* rSize = m_ra->temporaryReg ();
        VMValue* rNum = find (imop.arg2 ());
        VMDataType ty = secrecDTypeToVMDType (imop.arg1 ()->secrecType ()->secrecDataType ());
        assert (ty != VM_INVALID);
        const unsigned elemSize = sizeInBytes (ty);
        block.push_back (
            VMInstruction ()
                << "tmul uint64" << rSize << rNum
                << m_st.getImm (elemSize)
        );

        instr << "mem" << find (imop.arg1 ()) << "imm 0x0";
        instr << "mem" << find (imop.dest ()) << "imm 0x0";
        instr << rSize;
    }
    else {
        instr << find (imop.arg1 ()) << find (imop.dest ());
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
            VMInstruction () << "pushcref" << find (*it)
        );
    }

    assert (it != itEnd && *it == 0 &&
        "Malformed CALL instruction!");

    for (++ it; it != itEnd; ++ it) {
        block.push_back (
            VMInstruction () << "pushref" << find (*it)
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
    VMDataType ty = secrecDTypeToVMDType (imop.dest ()->secrecType ()->secrecDataType ());
    assert (ty != VM_INVALID);
    block.push_back (
        VMInstruction ()
            << "mov cref"
            << m_st.getImm (m_param ++)
            << "0x0" // offset 0
            << find (imop.dest ())
            << m_st.getImm (sizeInBytes (ty))
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
        VMDataType ty = secrecDTypeToVMDType ((*it)->secrecType ()->secrecDataType ());
        assert (ty != VM_INVALID);
        movI << "mov"
             << find (*it);
        movI << "ref"
             << m_st.getImm (retCount ++ )
             << "0x0"
             << m_st.getImm (sizeInBytes (ty));
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
    tmpInstr << "mov" << find (imop.arg2 ()) << rOffset;
    block.push_back (tmpInstr);

    VMDataType ty = secrecDTypeToVMDType (imop.dest ()->secrecType ()->secrecDataType ());
    assert (ty != VM_INVALID);
    const unsigned size = sizeInBytes (ty);
    VMInstruction mulInstr;
    mulInstr << "bmul uint64" << rOffset << m_st.getImm (size);
    block.push_back (mulInstr);

    VMInstruction movInstr;
    movInstr << "mov mem";
    movInstr << find (imop.arg1 ());
    movInstr << rOffset;
    movInstr << find (imop.dest ());
    movInstr << m_st.getImm (size);
    block.push_back (movInstr);
}

void Compiler::cgStore (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::STORE);
    VMValue* rOffset = m_ra->temporaryReg ();

    VMInstruction tmpInstr;
    tmpInstr << "mov" << find (imop.arg1 ());
    tmpInstr << rOffset;
    block.push_back (tmpInstr);

    VMDataType ty = secrecDTypeToVMDType (imop.arg2 ()->secrecType ()->secrecDataType ());
    assert (ty != VM_INVALID);
    const unsigned size = sizeInBytes (ty);
    VMInstruction mulInstr;
    mulInstr << "bmul uint64" << rOffset << m_st.getImm (size);
    block.push_back (mulInstr);

    VMInstruction movInstr;
    movInstr << "mov" << find (imop.arg2 ());
    movInstr << "mem" << find (imop.dest ()) << rOffset;
    movInstr << m_st.getImm (size);
    block.push_back (movInstr);
}

void Compiler::cgArithm (VMBlock& block, const Imop& imop) {
    assert (imop.isExpr ());
    VMDataType ty = secrecDTypeToVMDType (imop.dest ()->secrecType ()->secrecDataType ());
    assert (ty != VM_INVALID);

    if (imop.isVectorized ()) {
        for (Imop::OperandConstIterator i = imop.operandsBegin (), e = imop.operandsEnd (); i != e; ++ i) {
            VMInstruction instr;
            instr << "push" << find (*i);
            block.push_back (instr);
        }

        VMLabel* target = 0;
        {
            std::stringstream ss;
            ss << ':' << imopToVMName (imop) << "_vec_" << ty;
            target = st ().getLabel (ss.str ());
        }

        m_funcs->insert (target, BuiltinVArith (&imop));
        block.push_back (VMInstruction () << "call" << target << "imm");
        return;
    }

    const char* name = imopToVMName (imop);
    VMInstruction instr;
    instr << name << ty << find (imop.dest ());

    Imop::OperandConstIterator 
        i = imop.operandsBegin (),
        e = imop.operandsEnd ();
    for (++ i; i != e; ++ i) {
        instr << loadToRegister (block, *i);
    }

    block.push_back (instr);
}

void Compiler::cgDomainID (VMBlock& block, const Imop& imop) {
    assert (dynamic_cast<const SymbolDomain*>(imop.arg1 ()) != 0);
    const SymbolDomain* dom = static_cast<const SymbolDomain*>(imop.arg1 ());
    assert (dynamic_cast<PrivateSecType*>(dom->securityType ()) != 0);
    PrivateSecType* secTy = static_cast<PrivateSecType*>(dom->securityType ());
    VMLabel* label = m_scm->getPD (secTy);
    block.push_back (VMInstruction () << "mov" << label << find (imop.dest ()));
}

void Compiler::cgSyscall (VMBlock& block, const Imop& imop) {
    VMLabel* label = m_scm->getSyscallBinding (static_cast<const ConstantString*>(imop.arg1 ()));
    block.push_back (VMInstruction () << "syscall" << label << "imm");
}

void Compiler::cgPush (VMBlock& block, const Imop& imop) {
    const char* name = 0;
    switch (imop.type ()) {
    case Imop::PUSH: name = "push"; break;
    case Imop::PUSHREF: name = "pushref"; break;
    case Imop::PUSHCREF: name = "pushcref"; break;
    default: assert (false); break;
    }

    assert (find (imop.arg1 ()) != 0);
    block.push_back (VMInstruction () << name << find (imop.arg1 ()));
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
        case Imop::RELEASE:
            cgRelease (block, imop);
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
        case Imop::DOMAINID:
            cgDomainID (block, imop);
            return;
        case Imop::SYSCALL:
            cgSyscall (block, imop);
            return;
        case Imop::PUSH:
        case Imop::PUSHREF:
        case Imop::PUSHCREF:
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

    std::cerr << imop.toString () << std::endl;
    
    assert (false && "Unable to handle instruction!");
}

} // namespac SecreC
