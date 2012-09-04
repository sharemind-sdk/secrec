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
#include <libscc/analysis/LiveVariables.h>
#include <libscc/intermediate.h>
#include <libscc/blocks.h>
#include <libscc/constant.h>
#include <libscc/types.h>

#include "DeadVariableElimination.h"
#include "ScalarAllocPlacement.h"
#include "CopyElimination.h"
#include "SyscallManager.h"
#include "StringLiterals.h"
#include "RegisterAllocator.h"
#include "Builtin.h"
#include "VMDataType.h"

namespace SecreCC {

using namespace SecreC;

namespace /* anonymous */ {

VMDataType representationType (TypeNonVoid* tnv) {
    VMDataType ty = secrecDTypeToVMDType (tnv->secrecDataType ());
    if (tnv->secrecDimType () > 0 || tnv->secrecSecType ()->isPrivate () || tnv->secrecDataType () == DATATYPE_STRING) {
        // arrays, and private values are represented by a handle
        ty = VM_UINT64;
    }

    return ty;
}

const char* imopToVMName (const Imop& imop) {
    switch (imop.type ()) {
    case Imop::UMINUS: return "bneg";
    case Imop::UNEG  : return "bnot";
    case Imop::UINV  : return "binv";
    case Imop::MUL   : return "tmul";
    case Imop::DIV   : return "tdiv";
    case Imop::MOD   : return "tmod";
    case Imop::ADD   : return "tadd";
    case Imop::SUB   : return "tsub";
    case Imop::BAND  : return "btand";
    case Imop::LAND  : return "ltand";
    case Imop::BOR   : return "btor";
    case Imop::LOR   : return "ltor";
    case Imop::EQ    : return "teq";
    case Imop::NE    : return "tne";
    case Imop::LE    : return "tle";
    case Imop::LT    : return "tlt";
    case Imop::GE    : return "tge";
    case Imop::GT    : return "tgt";
    case Imop::XOR   : return "btxor";
    default:
        assert (false && "Not an arithmetic instruction!");
        return 0;
    }
}

bool isString (const Symbol* sym) {
    return sym->secrecType ()->secrecDataType () == DATATYPE_STRING;
}

VMDataType getVMDataType (const Symbol* sym) {
    return secrecDTypeToVMDType (sym->secrecType ()->secrecDataType ());
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
        label = getLabel (st, *symL->block ());
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

void syscallMangleImopType (std::ostream& os, Imop::Type iType) {
    switch (iType) {
    case Imop::CAST:       os << "conv";        break;
    case Imop::CLASSIFY:   os << "classify";    break;
    case Imop::DECLASSIFY: os << "declassify";  break;
    case Imop::RELEASE:    os << "delete";      break;
    case Imop::COPY:       os << "copy";        break;
    case Imop::ADD:        os << "add";         break;
    case Imop::SUB:        os << "sub";         break;
    case Imop::MUL:        os << "mul";         break;
    case Imop::MOD:        os << "mod";         break;
    case Imop::DIV:        os << "div";         break;
    case Imop::BAND:
    case Imop::LAND:       os << "and";         break;
    case Imop::BOR:
    case Imop::LOR:        os << "or";          break;
    case Imop::EQ:         os << "eq";          break;
    case Imop::GT:         os << "gt";          break;
    case Imop::GE:         os << "gte";         break;
    case Imop::LT:         os << "lt";          break;
    case Imop::LE:         os << "lte";         break;
    case Imop::STORE:      os << "store";       break;
    case Imop::LOAD:       os << "load";        break;
    case Imop::UINV:       os << "inv";         break;
    case Imop::UNEG:       os << "not";         break;
    case Imop::UMINUS:     os << "neg";         break;
    case Imop::XOR:        os << "xor";         break;
    default:                                    break;
    }
}

void syscallMangleSecrecDataType (std::ostream& os, SecrecDataType ty) {
    switch (ty) {
    case DATATYPE_BOOL:        os << "bool";         break;
    case DATATYPE_UINT8:       os << "uint8";        break;
    case DATATYPE_UINT16:      os << "uint16";       break;
    case DATATYPE_UINT32:      os << "uint32";       break;
    case DATATYPE_UINT64:      os << "uint64";       break;
    case DATATYPE_INT8:        os << "int8";         break;
    case DATATYPE_INT16:       os << "int16";        break;
    case DATATYPE_INT32:       os << "int32";        break;
    case DATATYPE_INT64:       os << "int64";        break;
    case DATATYPE_XOR_UINT8:   os << "xor_uint8";    break;
    case DATATYPE_XOR_UINT16:  os << "xor_uint16";   break;
    case DATATYPE_XOR_UINT32:  os << "xor_uint32";   break;
    case DATATYPE_XOR_UINT64:  os << "xor_uint64";   break;
    case DATATYPE_FLOAT32:     os << "float32";      break;
    case DATATYPE_FLOAT64:     os << "float64";      break;
    default:                                         break;
    }
}

void syscallMangleNamespace (std::ostream& os, TypeNonVoid* tnv) {
    if (tnv->secrecSecType ()->isPrivate ()) {
        PrivateSecType* secTy = static_cast<PrivateSecType*>(tnv->secrecSecType ());
        os << secTy->securityKind ()->name () << "::";
    }
}

bool isPrivate (const Imop& imop) {
    BOOST_FOREACH (const Symbol* sym, imop.operands ()) {
        if (sym == 0)
            continue;

        Type* ty = sym->secrecType ();
        if (ty->secrecSecType ()->isPrivate ())
            return true;
    }

    return false;
}

bool isStringRelated (const Imop& imop) {
    BOOST_FOREACH (const Symbol* sym, imop.operands ()) {
        if (sym == 0)
            continue;

        Type* ty = sym->secrecType ();
        if (ty->secrecDataType () == DATATYPE_STRING)
            return true;
    }

    return false;
}

VMLabel* getPD (SyscallManager* scm, const Symbol* sym) {
    assert (scm != 0 && sym != 0);
    SecreC::Type* ty = sym->secrecType ();
    assert (ty->secrecSecType ()->isPrivate ());
    PrivateSecType* pty = static_cast<PrivateSecType*>(ty->secrecSecType ());
    return scm->getPD (pty);
}

class SyscallName {
public: /* Methods: */
    SyscallName () { }

    SyscallName& operator << (const char* name) {
        m_os << name;
        return *this;
    }

    SyscallName& operator << (char c) {
        m_os << c;
        return *this;
    }

    SyscallName& operator << (const std::string& name) {
        m_os << name;
        return *this;
    }

    SyscallName& operator << (TypeNonVoid* tnv) {
        syscallMangleNamespace (m_os, tnv);
        return *this;
    }

    SyscallName& operator << (Imop::Type iType) {
        syscallMangleImopType (m_os, iType);
        return *this;
    }

    SyscallName& operator << (SecrecDataType ty) {
        syscallMangleSecrecDataType (m_os, ty);
        return *this;
    }

    std::string str () const {
        return m_os.str ();
    }

    static std::string arithm (TypeNonVoid* ty, Imop::Type iType);
    static std::string cast (TypeNonVoid* from, TypeNonVoid* to);
    static std::string basic (TypeNonVoid* ty, const char* name, bool needDataType = true, bool needVec = true);

private: /* Fields: */
    std::ostringstream   m_os;
};

std::string SyscallName::basic (TypeNonVoid* ty, const char* name,  bool needDataType, bool needVec) {
    SyscallName scname;
    scname << ty << name;
    if (needDataType) {
        scname << '_' << ty->secrecDataType ();
    }

    if (needVec) {
        scname << "_vec";
    }

    return scname.str ();
}

std::string SyscallName::arithm (TypeNonVoid* ty, Imop::Type iType) {
    SyscallName scname;
    scname << ty << iType << '_' << ty->secrecDataType () << "_vec";
    return scname.str ();
}

std::string SyscallName::cast (TypeNonVoid* from, TypeNonVoid* to) {
    SyscallName scname;
    scname << from << "conv_" << from->secrecDataType () << "_to_"
            << to->secrecDataType ()
            << "_vec";
    return scname.str ();
}

} // namespace anonymous

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

    eliminateDeadVariables (m_code);
    eliminateRedundantCopies (m_code);
    m_allocs = placePrivateScalarAllocs (m_code);

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

    RegisterAllocator::LVPtr lv (new LiveVariables ());
    DataFlowAnalysisRunner ()
            .addAnalysis (*lv.get ())
            .run (m_code.program ());

    m_target = codeSec;
    m_ra->init (m_st, lv);
    m_scm->init (m_st, scSec, pdSec);
    m_strLit->init (m_st, rodataSec);

    // Finally generate code:
    BOOST_FOREACH (const Procedure& proc, m_code.program ()) {
        cgProcedure (proc);
    }

    m_funcs->generateAll (*m_target, m_st);
    m_target->setNumGlobals (m_ra->globalCount ());
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
        block.push_new () << "mov" << find (symbol) << reg;
    }
    else {
        reg = find (symbol);
    }

    return reg;
}

void Compiler::pushString (VMBlock& block, const Symbol* str) {
    assert (str->secrecType ()->secrecDataType () == DATATYPE_STRING);
    if (str->isConstant ()) {
        assert (dynamic_cast<const ConstantString*>(str) != 0);
        const ConstantString* cstr = static_cast<const ConstantString*>(str);
        StringLiterals::LiteralInfo info = m_strLit->insert (cstr);
        VMLabel* offset = info.label;
        VMVReg* temp = m_ra->temporaryReg ();
        block.push_new () << "mov imm :RODATA" << temp;
        block.push_new () << "pushcrefpart mem" << temp << offset << m_st.getImm (info.size);
    }
    else {
        block.push_new () << "pushcref mem" << find (str);
    }
}

void Compiler::paramString (VMBlock& block, const Symbol* dest) {
    VMLabel* target = m_st.getLabel (":builtin_str_dup__");
    block.push_new () << "pushcref cref" << m_param;
    m_funcs->insert (target, BuiltinStrDup ());
    block.push_new () << "call" << target << find (dest);
    ++ m_param;
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
    BOOST_FOREACH (const Block& block, blocks) {
        if (block.reachable ()) {
            cgBlock (function, block);
        }
    }

    m_ra->exitFunction (function);
    m_target->push_back (function);
}

void Compiler::cgBlock (VMFunction& function, const Block& block) {
    VMLabel* name = 0;
    if (block.hasIncomingJumps ()) {
        name = getLabel (st (), block);
    }

    VMBlock vmBlock (name, &block);
    m_ra->enterBlock (vmBlock);
    BOOST_FOREACH (const Imop& imop, block) {
        cgImop (vmBlock, imop);
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
        instr << ty;
    }

    // arguments
    OCI i = imop.operandsBegin (),
        e = imop.operandsEnd ();
    for (++ i; i != e; ++ i)
        instr << loadToRegister (block, *i);

    block.push_back (instr);
}

void Compiler::cgAlloc (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ALLOC);

    if (isPrivate (imop)) {
        cgPrivateAlloc (block, imop);
        return;
    }

    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.arg2 ());

    VMLabel* target = 0;
    VMDataType ty = secrecDTypeToVMDType (imop.arg1 ()->secrecType ()->secrecDataType ());
    unsigned size = sizeInBytes (ty);
    {
        std::stringstream ss;
        ss << ":secrecAlloc_" << size;
        target = m_st.getLabel (ss.str ());
    }

    m_funcs->insert (target, BuiltinAlloc (size));

    block.push_new () << "call" << target << find (imop.dest ());
}

void Compiler::cgToString (VMBlock& block, const Imop& imop) {
    VMLabel* target = 0;

    SecrecDataType dType = imop.arg1 ()->secrecType ()->secrecDataType ();
    if (dType == DATATYPE_BOOL) {
        target = m_st.getLabel (":bool_to_string__");
        m_funcs->insert (target, BuiltinBoolToString (m_strLit));

        block.push_new () << "push" << find (imop.arg1 ());
        block.push_new () << "call" << target << find (imop.dest ());
        return;
    }
    else
    if (dType == DATATYPE_FLOAT32) {
        block.push_new () << "push" << find (imop.arg1 ());
        emitSyscall (block, find (imop.dest ()), "miner_float32_to_string");
        return;
    }
    else
    if (isNumericDataType (dType)) {
        VMDataType vmDType = secrecDTypeToVMDType (dType);
        target = m_st.getLabel (":numeric_to_string__");
        m_funcs->insert (target, BuiltinNumericToString (m_strLit));

        if (vmDType != VM_UINT64) {
            VMValue* rTmp = m_ra->temporaryReg ();
            VMValue* arg = loadToRegister (block, imop.arg1 ());
            block.push_new () << "convert"
                              << vmDType << arg
                              << VM_UINT64 << rTmp;
            block.push_new () << "push" << rTmp;
        }
        else {
            block.push_new () << "push" << find (imop.arg1 ());
        }

        block.push_new () << "call" << target << find (imop.dest ());
        return;
    }
    else {
        assert (false && "Unable to convert given data type to string!");
        return;
    }
}

void Compiler::cgRelease (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::RELEASE);
    if (isPrivate (imop)) {
        cgPrivateRelease (block, imop);
        return;
    }

    block.push_new () << "free" << find (imop.arg1 ());
}

void Compiler::cgAssign (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ASSIGN);

    if (imop.dest ()->secrecType ()->secrecDataType () == DATATYPE_STRING) {
        pushString (block, imop.arg1 ());
        VMLabel* target = m_st.getLabel (":builtin_str_dup__");
        m_funcs->insert (target, BuiltinStrDup ());
        block.push_new () << "call" << target << find (imop.dest ());
        return;
    }

    if (isPrivate (imop)) {
        if (imop.isVectorized ()) {
            cgPrivateAssign (block, imop);
            return;
        }
    }

    VMInstruction instr;
    instr << "mov";
    if (imop.isVectorized ()) {
        VMValue* rSize = m_ra->temporaryReg ();
        VMValue* rNum = find (imop.arg2 ());
        VMDataType ty = secrecDTypeToVMDType (imop.arg1 ()->secrecType ()->secrecDataType ());
        assert (ty != VM_INVALID);
        const unsigned elemSize = sizeInBytes (ty);

        block.push_new ()
            << "tmul" << VM_UINT64 << rSize << rNum
            << m_st.getImm (elemSize);

        instr << "mem" << find (imop.arg1 ()) << "imm 0x0";
        instr << "mem" << find (imop.dest ()) << "imm 0x0";
        instr << rSize;
    }
    else {
        instr << find (imop.arg1 ()) << find (imop.dest ());
    }

    block.push_back (instr);
}

void Compiler::cgCast (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::CAST);

    if (isPrivate (imop)) {
        cgPrivateCast (block, imop);
        return;
    }

    VMDataType destTy = getVMDataType (imop.dest ());
    VMDataType srcTy = getVMDataType (imop.arg1 ());

    if (imop.isVectorized ()) {
        std::stringstream ss;

        VMLabel* target = 0;
        if (imop.dest ()->secrecType ()->secrecDataType () == DATATYPE_BOOL) {
            block.push_new () << "push" << find (imop.dest ());
            block.push_new () << "push" << find (imop.arg1 ());
            block.push_new () << "push" << find (imop.arg2 ());
            ss << ":vec_cast_bool_" << srcTy;
            target = m_st.getLabel (ss.str ());
            m_funcs->insert (target, BuiltinVBoolCast (srcTy));
            block.push_new () << "call" << target << "imm";
        }
        else
        if (destTy != srcTy) {
            block.push_new () << "push" << find (imop.dest ());
            block.push_new () << "push" << find (imop.arg1 ());
            block.push_new () << "push" << find (imop.arg2 ());
            ss << ":vec_cast_" << destTy << "_" << srcTy;
            target = m_st.getLabel (ss.str ());
            m_funcs->insert (target, BuiltinVCast (destTy, srcTy));
            block.push_new () << "call" << target << "imm";
        }
        else {
            block.push_new ()
                  << "mov"
                  << "mem" << find (imop.arg1 ()) << "imm 0x0"
                  << "mem" << find (imop.dest ()) << "imm 0x0"
                  << find (imop.arg2 ());
        }
    }
    else {
        VMInstruction instr;
        if (imop.dest ()->secrecType ()->secrecDataType () == DATATYPE_BOOL) {
            VMValue* arg = loadToRegister (block, imop.arg1 ());
            instr << "tgt" << destTy << find (imop.dest ())
                  << arg << m_st.getImm (0);
        }
        else
        if (destTy != srcTy) {
            VMValue* arg = loadToRegister (block, imop.arg1 ());
            instr << "convert"
                  << srcTy << arg
                  << destTy << find (imop.dest ());
        }
        else {
            instr << "mov" << find (imop.arg1 ()) << find (imop.dest ());
        }

        block.push_back (instr);
    }
}

void Compiler::cgCopy (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::COPY);

    if (isPrivate (imop)) {
        cgPrivateCopy (block, imop);
        return;
    }

    VMDataType ty = secrecDTypeToVMDType (imop.arg1 ()->secrecType ()->secrecDataType ());
    assert (ty != VM_INVALID);
    const unsigned elemSize = sizeInBytes (ty);
    VMValue* rNum = find (imop.arg2 ());
    VMValue* rSize = m_ra->temporaryReg ();
    block.push_new ()
        << "tmul" << VM_UINT64 << rSize << rNum
        << m_st.getImm (elemSize);
    block.push_new () << "uinc" << VM_UINT64 << rSize; // fehh! no 0 sized memory regions
    block.push_new () << "alloc" << find (imop.dest ()) << rSize;
    block.push_new ()
        << "mov"
        << "mem" << find (imop.arg1 ()) << "imm 0x0"
        << "mem" << find (imop.dest ()) << "imm 0x0"
        << rSize;
}


void Compiler::cgCall (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::CALL);

    // compute destinations for calls
    const Symbol* dest = imop.dest ();

    // push arguments
    BOOST_FOREACH (const Symbol* sym, imop.useRange ()) {
        if (isString (sym))
            pushString (block, sym);
        else
            block.push_new () << "pushcref" << find (sym);
    }

    BOOST_FOREACH (const Symbol* sym, imop.defRange ()) {
        block.push_new () << "pushref" << find (sym);
    }

    // CALL
    block.push_new () << "call" << getProc (st (), dest) << "imm";
}

void Compiler::cgParam (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::PARAM);

    if (isString (imop.dest ())) {
        paramString (block, imop.dest ());
        return;
    }

    VMDataType vmty = representationType (imop.dest ()->secrecType ());
    assert (vmty != VM_INVALID);

    if (isPrivate (imop) && ! imop.dest ()->isArray ()) {
        VMValue* temp = m_ra->temporaryReg ();
        VMValue* d = find (imop.dest ());

        block.push_new ()
            << "mov cref"
            << m_st.getImm (m_param ++)
            << "0x0" // offset 0
            << temp
            << m_st.getImm (sizeInBytes (vmty));

        TypeNonVoid* ty = imop.dest ()->secrecType ();
        block.push_new () << "push" << getPD (m_scm, imop.dest ());
        block.push_new () << "push" << temp;
        block.push_new () << "push" << d;
        emitSyscall (block, SyscallName::basic (ty, "assign"));
    }
    else {
        block.push_new ()
            << "mov cref"
            << m_st.getImm (m_param ++)
            << "0x0" // offset 0
            << find (imop.dest ())
            << m_st.getImm (sizeInBytes (vmty));
    }
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
        VMDataType ty = representationType ((*it)->secrecType ());
        assert (ty != VM_INVALID);
        block.push_new ()
            << "mov" << find (*it)
            << "ref" << m_st.getImm (retCount ++ )
            << "0x0" << m_st.getImm (sizeInBytes (ty));
    }

    block.push_new () << "return imm 0x0";
}

void Compiler::cgLoad (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::LOAD);

    if (isPrivate (imop)) {
        cgPrivateLoad (block, imop);
        return;
    }
    VMDataType ty = secrecDTypeToVMDType (imop.dest ()->secrecType ()->secrecDataType ());
    assert (ty != VM_INVALID);
    const unsigned size = sizeInBytes (ty);

    VMValue* rOffset = m_ra->temporaryReg ();
    block.push_new () << "mov" << find (imop.arg2 ()) << rOffset;
    block.push_new () << "bmul" << VM_UINT64 << rOffset << m_st.getImm (size);

    VMValue* rAddr = 0;
    if (imop.arg1 ()->isConstant ()) {
        assert (imop.arg1 ()->secrecType ()->secrecDataType () == DATATYPE_STRING);
        rAddr = m_ra->temporaryReg ();
        block.push_new () << "mov imm :RODATA" << rAddr;

        const ConstantString* cstr = static_cast<const ConstantString*>(imop.arg1 ());
        StringLiterals::LiteralInfo info = m_strLit->insert (cstr);
        block.push_new () << "badd" << VM_UINT64 << rOffset << info.label;
    }
    else {
        rAddr = find (imop.arg1 ());
    }

    block.push_new ()
        << "mov mem"
        << rAddr
        << rOffset
        << find (imop.dest ())
        << m_st.getImm (size);
}

void Compiler::cgStore (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::STORE);

    if (isPrivate (imop)) {
        cgPrivateStore (block, imop);
        return;
    }

    VMValue* rOffset = m_ra->temporaryReg ();

    block.push_new () << "mov" << find (imop.arg1 ()) << rOffset;

    VMDataType ty = secrecDTypeToVMDType (imop.arg2 ()->secrecType ()->secrecDataType ());
    assert (ty != VM_INVALID);
    const unsigned size = sizeInBytes (ty);
    block.push_new () << "bmul" << VM_UINT64 << rOffset << m_st.getImm (size);
    block.push_new ()
        << "mov" << find (imop.arg2 ())
        << "mem" << find (imop.dest ()) << rOffset
        << m_st.getImm (size);
}

void Compiler::cgStringAppend (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ADD);
    pushString (block, imop.arg1 ());
    pushString (block, imop.arg2 ());
    VMLabel* target = m_st.getLabel (":builtin_str_append__");
    m_funcs->insert (target, BuiltinStrAppend ());
    block.push_new () << "call" << target << find (imop.dest ());
}

void Compiler::cgStringCmp (VMBlock& block, const Imop& imop) {
    pushString (block, imop.arg1 ());
    pushString (block, imop.arg2 ());
    VMLabel* target = m_st.getLabel (":builtin_str_cmp__");
    m_funcs->insert (target, BuiltinStringCmp ());
    VMValue* dest = find (imop.dest ());
    block.push_new () << "call" << target << dest;

    const char* opname = 0;
    switch (imop.type ()) {
    case Imop::EQ: opname = "beq"; break;
    case Imop::NE: opname = "bne"; break;
    case Imop::LE: opname = "ble"; break;
    case Imop::LT: opname = "blt"; break;
    case Imop::GE: opname = "bge"; break;
    case Imop::GT: opname = "bgt"; break;
    default: assert (false); break;
    }

    block.push_new () << opname << VM_INT64 << dest << m_st.getImm (0) ;
    find (imop.dest ());
}

void Compiler::cgArithm (VMBlock& block, const Imop& imop) {
    assert (imop.isExpr ());

    if (isStringRelated (imop)) {
        switch (imop.type ()) {
        case Imop::ADD:
            cgStringAppend (block, imop);
            break;
        case Imop::EQ:
        case Imop::NE:
        case Imop::LE:
        case Imop::LT:
        case Imop::GE:
        case Imop::GT:
            cgStringCmp (block, imop);
            break;
        default:
            assert (false && "Invalid string related arithemtic operation.");
            break;
        }

        return;
    }

    VMDataType ty = secrecDTypeToVMDType (imop.arg1 ()->secrecType ()->secrecDataType ());
    assert (ty != VM_INVALID);

    if (isPrivate (imop)) {
        cgPrivateArithm (block, imop);
        return;
    }

    if (imop.isVectorized ()) {
        BOOST_FOREACH (const Symbol* sym, imop.operands ()) {
            block.push_new () << "push" << find (sym);
        }

        VMLabel* target = 0;
        {
            std::stringstream ss;
            ss << ':' << imopToVMName (imop) << "_vec_" << ty;
            target = st ().getLabel (ss.str ());
        }

        m_funcs->insert (target, BuiltinVArith (&imop));
        block.push_new () << "call" << target << "imm";
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
    block.push_new () << "mov" << label << find (imop.dest ());
}

void Compiler::emitSyscall (VMBlock& block, const std::string& name) {
    VMLabel* label = m_scm->getSyscallBinding (name);
    block.push_new () << "syscall" << label << "imm";
}

void Compiler::emitSyscall (VMBlock& block, VMValue* dest, const std::string& name) {
    VMLabel* label = m_scm->getSyscallBinding (name);
    block.push_new () << "syscall" << label << dest;
}

void Compiler::cgSyscall (VMBlock& block, const Imop& imop) {
    VMLabel* label = m_scm->getSyscallBinding (static_cast<const ConstantString*>(imop.arg1 ()));
    block.push_new () << "syscall" << label << "imm";
}

void Compiler::cgPush (VMBlock& block, const Imop& imop) {
    assert (imop.arg1 () != 0);

    const Symbol* arg = imop.arg1 ();

    if (imop.type () == Imop::PUSH) {
        block.push_new () << "push" << find (arg);
        return;
    }

    if (imop.type () == Imop::PUSHREF) {
        assert (arg->secrecType ()->secrecSecType ()->isPublic ());
        assert (! arg->isConstant ());
        block.push_new ()
            << ((arg->isArray () || isString (arg)) ? "pushref mem" : "pushref")
            << find (arg);
        return;
    }

    if (imop.type () == Imop::PUSHCREF) {
        assert (arg->secrecType ()->secrecSecType ()->isPublic ());
        if (isString (arg)) {
            pushString (block, arg);
            return;
        }

        block.push_new ()
            << (arg->isArray () ? "pushcref mem" : "pushcref")
            << find (arg);
        return;
    }
}

void Compiler::cgPrint (VMBlock& block, const Imop& imop) {
    pushString (block, imop.arg1 ());
    emitSyscall (block, "miner_log_string");
}

void Compiler::cgError (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ERROR);
    pushString (block, imop.arg1 ());
    emitSyscall (block, "miner_log_string");
    block.push_new () << "halt imm 0xff";
}

void Compiler::cgImop (VMBlock& block, const Imop& imop) {

    m_ra->getReg (imop);

    AllocMap::iterator it = m_allocs.find (&imop);
    if (it != m_allocs.end ()) {
        BOOST_FOREACH (const Symbol* dest, it->second) {
            cgNewPrivateScalar (block, dest);
        }

        m_allocs.erase (it);
    }

    if (imop.isJump ()) {
        cgJump (block, imop);
        return;
    }

    switch (imop.type ()) {
    case Imop::TOSTRING:
        cgToString (block, imop);
        return;
    case Imop::CLASSIFY:
        cgClassify (block, imop);
        return;
    case Imop::DECLASSIFY:
        cgDeclassify (block, imop);
        return;
    case Imop::ASSIGN:
        cgAssign (block, imop);
        return;
    case Imop::CAST:
        cgCast (block, imop);
        return;
    case Imop::COPY:
        cgCopy (block, imop);
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
        block.push_new () << "halt imm 0x0";
        return;
    case Imop::ERROR:
        cgError (block, imop);
        return;
    case Imop::PRINT:
        cgPrint (block, imop);
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

/**
 * Private operations:
 */


void Compiler::cgNewPrivateScalar (VMBlock& block, const Symbol* dest) {
    VMValue* d = find (dest);
    TypeNonVoid* ty = dest->secrecType ();
    block.push_new () << "push" << getPD (m_scm, dest);
    block.push_new () << "push" << m_st.getImm (1);
    emitSyscall (block, d, SyscallName::basic (ty, "new"));
}

void Compiler::cgNewPrivate (VMBlock& block, const Symbol* dest, const Symbol* size) {
    VMValue* d = find (dest);
    TypeNonVoid* ty = dest->secrecType ();
    block.push_new () << "push" << getPD (m_scm, dest);
    block.push_new () << "push" << find (size);
    emitSyscall (block, d, SyscallName::basic (ty, "new"));
}

void Compiler::cgPrivateAssign (VMBlock& block, const Imop& imop) {
    TypeNonVoid* ty = imop.dest ()->secrecType ();
    block.push_new () << "push" << getPD (m_scm, imop.dest ());
    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.dest ());
    emitSyscall (block, SyscallName::basic (ty, "assign"));
}

void Compiler::cgPrivateCopy (VMBlock& block, const Imop& imop) {
    TypeNonVoid* ty = imop.dest ()->secrecType ();
    cgNewPrivate (block, imop.dest (), imop.arg2 ());
    block.push_new () << "push" << getPD (m_scm, imop.dest ());
    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.dest ());
    emitSyscall (block, SyscallName::basic (ty, "assign"));
}

void Compiler::cgClassify (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::CLASSIFY);
    assert (imop.dest ()->secrecType ()->secrecSecType ()->isPrivate ());
    assert (imop.arg1 ()->secrecType ()->secrecSecType ()->isPublic ());

    TypeNonVoid* ty = imop.dest ()->secrecType ();
    if (imop.isVectorized ()) {
        block.push_new () << "push" << getPD (m_scm, imop.dest ());
        block.push_new () << "push" << find (imop.dest ());
        block.push_new () << "pushcref" << "mem" << find (imop.arg1 ());
        emitSyscall (block, SyscallName::basic (ty, "classify"));
    }
    else {
        block.push_new () << "push" << getPD (m_scm, imop.dest ());
        block.push_new () << "push" << find (imop.arg1 ());
        block.push_new () << "push" << find (imop.dest ());
        emitSyscall (block, SyscallName::basic (ty, "init"));
    }
}

void Compiler::cgDeclassify (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::DECLASSIFY);
    assert (imop.dest ()->secrecType ()->secrecSecType ()->isPublic ());
    assert (imop.arg1 ()->secrecType ()->secrecSecType ()->isPrivate ());
    TypeNonVoid* ty = imop.arg1 ()->secrecType ();
    VMDataType dataTy = secrecDTypeToVMDType (ty->secrecDataType ());
    unsigned size = sizeInBytes (dataTy);
    block.push_new () << "push" << getPD (m_scm, imop.arg1 ());
    block.push_new () << "push" << find (imop.arg1 ());
    if (imop.isVectorized ()) {
        block.push_new () << "pushref" << "mem" << find (imop.dest ());
    }
    else {
        block.push_new () << "pushrefpart" << find (imop.dest ()) << m_st.getImm (0) << m_st.getImm (size);
    }

    emitSyscall (block, SyscallName::basic (ty, "declassify"));
}

void Compiler::cgPrivateNE (VMBlock& block, const Imop& imop) {
    TypeNonVoid* ty = imop.arg1 ()->secrecType ();
    VMValue* pd = getPD (m_scm, imop.dest ());
    VMValue* arg1 = find (imop.arg1 ());
    VMValue* arg2 = find (imop.arg2 ());
    VMValue* dest = find (imop.dest ());

    block.push_new () << "push" << pd;
    block.push_new () << "push" << arg1;
    block.push_new () << "push" << arg2;
    block.push_new () << "push" << dest;
    emitSyscall (block, SyscallName::basic (ty, "eq"));

    block.push_new () << "push" << pd;
    block.push_new () << "push" << dest;
    block.push_new () << "push" << dest;
    emitSyscall (block, SyscallName::basic (imop.dest()->secrecType(), "not"));
}

void Compiler::cgPrivateArithm (VMBlock& block, const Imop& imop) {

    if (imop.type () == Imop::NE) {
        cgPrivateNE (block, imop);
        return;
    }

    TypeNonVoid* ty = imop.arg1 ()->secrecType ();
    block.push_new () << "push" << getPD (m_scm, imop.dest ());
    block.push_new () << "push" << find (imop.arg1 ());
    switch (imop.type ()) {
    case Imop::UINV:
    case Imop::UNEG:
    case Imop::UMINUS:
        break;
    default:
        block.push_new () << "push" << find (imop.arg2 ());
        break;
    }
    block.push_new () << "push" << find (imop.dest ());
    emitSyscall (block, SyscallName::arithm (ty, imop.type ()));
}

void Compiler::cgPrivateAlloc (VMBlock& block, const Imop& imop) {
    TypeNonVoid* ty = imop.dest ()->secrecType ();
    VMLabel* pd = getPD (m_scm, imop.dest ());
    cgNewPrivate (block, imop.dest (), imop.arg2 ());
    block.push_new () << "push" << pd;
    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.dest ());
    const bool privateArg = imop.arg1 ()->secrecType ()->secrecSecType ()->isPrivate ();
    emitSyscall (block, SyscallName::basic (ty, privateArg ? "fill" : "init"));
}

void Compiler::cgPrivateRelease (VMBlock& block, const Imop& imop) {
    TypeNonVoid* ty = imop.arg1 ()->secrecType ();
    block.push_new () << "push" << getPD (m_scm, imop.arg1 ());
    block.push_new () << "push" << find (imop.arg1 ());
    emitSyscall (block, SyscallName::basic (ty, "delete"));
}

void Compiler::cgPrivateCast (VMBlock& block, const Imop& imop) {
    block.push_new () << "push" << getPD (m_scm, imop.dest ());
    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.dest ());
    emitSyscall (block, SyscallName::cast (imop.arg1 ()->secrecType (), imop.dest ()->secrecType ()));
}

void Compiler::cgPrivateLoad (VMBlock& block, const Imop& imop) {
    TypeNonVoid* ty = imop.dest ()->secrecType ();
    block.push_new () << "push" << getPD (m_scm, imop.dest ());
    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.arg2 ());
    block.push_new () << "push" << find (imop.dest ());
    emitSyscall (block, SyscallName::basic (ty, "load"));
}

void Compiler::cgPrivateStore (VMBlock& block, const Imop& imop) {
    TypeNonVoid* ty = imop.dest ()->secrecType ();
    block.push_new () << "push" << getPD (m_scm, imop.dest ());
    block.push_new () << "push" << find (imop.arg2 ());
    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.dest ());
    emitSyscall (block, SyscallName::basic (ty, "store"));
}

} // namespac SecreC
