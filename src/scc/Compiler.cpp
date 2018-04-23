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

#include "Compiler.h"

#include <iostream>

#include <libscc/Blocks.h>
#include <libscc/Constant.h>
#include <libscc/DataflowAnalysis.h>
#include <libscc/Intermediate.h>
#include <libscc/Optimizer.h>
#include <libscc/SecurityType.h>
#include <libscc/TreeNode.h>
#include <libscc/Types.h>
#include <libscc/analysis/LiveVariables.h>

#include "Builtin.h"
#include "RegisterAllocator.h"
#include "StringLiterals.h"
#include "SyscallManager.h"
#include "VMDataType.h"

namespace SecreCC {

using namespace SecreC;

namespace /* anonymous */ {

VMDataType representationType (const TypeNonVoid* tnv) {
    VMDataType ty = secrecDTypeToVMDType (tnv->secrecDataType ());
    if (tnv->secrecDimType () > 0 ||
            tnv->secrecSecType ()->isPrivate () ||
            tnv->secrecDataType ()->isString ())
    {
        // arrays, and private values are represented by a handle
        ty = VM_UINT64;
    }

    return ty;
}

bool isSigned (SecreC::Symbol* sym) {
    assert(sym);
    return isSignedNumericDataType (sym->secrecType ()->secrecDataType ());
}

const char* imopToVMName (const Imop& imop) {
    const bool isBool = imop.arg1()->secrecType()->secrecDataType()->isBool();

    switch (imop.type ()) {
    case Imop::UMINUS: return "bneg";
    case Imop::UNEG  : return "bnot";
    case Imop::UINV  : return isBool ? "bnot" : "binv";
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
    case Imop::SHL   : return "tshl0";
    case Imop::SHR   : return isSigned (imop.dest ()) ? "tshr" : "tshr0";
    default:
        assert (false && "Not an arithmetic instruction!");
        return nullptr;
    }
}

bool isString (const Symbol* sym) {
    return sym->secrecType ()->secrecDataType ()->isString ();
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
    assert(sym);
    assert (sym->symbolType () == SYM_LABEL);
    VMValue* label = st.find (sym);
    if (!label) {
        const SymbolLabel* symL = static_cast<const SymbolLabel*>(sym);
        label = getLabel (st, *symL->block ());
        st.store (sym, label);
    }

    assert(dynamic_cast<VMLabel *>(label));
    return static_cast<VMLabel*>(label);
}

VMLabel* getProc (VMSymbolTable& st, const Symbol* sym) {
    assert(sym);
    assert (sym->symbolType () == SYM_PROCEDURE);
    VMValue* label = st.find (sym);
    if (!label) {
        std::stringstream ss;
        const SymbolProcedure* proc = static_cast<const SymbolProcedure*>(sym);
        ss << ':' << proc->procedureName () << '_' << st.uniq ();
        label = st.getLabel (ss.str ());
        st.store (sym, label);
    }

    assert(dynamic_cast<VMLabel *>(label));
    return static_cast<VMLabel*>(label);
}

void syscallMangleImopType (std::ostream& os, Imop::Type iType) {
    switch (iType) {
    case Imop::CLASSIFY:   os << "classify";    break;
    case Imop::DECLASSIFY: os << "declassify";  break;
    case Imop::RELEASE:    os << "delete";      break;
    case Imop::COPY:       os << "copy";        break;
    case Imop::STORE:      os << "store";       break;
    case Imop::LOAD:       os << "load";        break;
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
    case DATATYPE_FLOAT32:     os << "float32";      break;
    case DATATYPE_FLOAT64:     os << "float64";      break;
    default:                                         break;
    }
}

void syscallMangleNamespace (std::ostream& os, const TypeNonVoid* tnv) {
    if (tnv->secrecSecType ()->isPrivate ()) {
        const auto secTy = static_cast<const PrivateSecType*>(tnv->secrecSecType ());
        os << secTy->securityKind ()->name () << "::";
    }
}

bool isPrivate (const Imop& imop) {
    for (const Symbol* sym : imop.operands ()) {
        if (!sym)
            continue;

        const Type* ty = sym->secrecType ();
        if (ty->secrecSecType ()->isPrivate ())
            return true;
    }

    return false;
}

bool isStringRelated (const Imop& imop) {
    for (const Symbol* sym : imop.operands ()) {
        if (!sym)
            continue;

        const Type* ty = sym->secrecType ();
        if (ty->secrecDataType ()->isString ())
            return true;
    }

    return false;
}

VMLabel* getPD (SyscallManager* scm, const Symbol* sym) {
    assert(scm);
    assert(sym);
    const SecreC::Type* ty = sym->secrecType ();
    assert (ty->secrecSecType ()->isPrivate ());
    const auto pty = static_cast<const PrivateSecType*>(ty->secrecSecType ());
    return scm->getPd(pty);
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

    SyscallName& operator << (const TypeNonVoid* tnv) {
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


    SyscallName& operator << (const DataType* ty) {
        assert(ty);
        assert (ty->isPrimitive ());

        if (ty->isBuiltinPrimitive ()) {
            syscallMangleSecrecDataType (m_os, static_cast<const DataTypeBuiltinPrimitive*>(ty)->secrecDataType ());
        }
        else {
            m_os << static_cast<const DataTypeUserPrimitive*> (ty)-> name ();
        }

        return *this;
    }

    std::string str () const {
        return m_os.str ();
    }

    static std::string tostring (SecrecDataType dType);
    static std::string tostring (const DataType* dType);
    static std::string basic (const TypeNonVoid* ty, const char* name, bool needDataType = true, bool needVec = true);

private: /* Fields: */
    std::ostringstream   m_os;
};

std::string SyscallName::tostring (SecrecDataType dType) {
    SyscallName scname;
    scname << dType << "_toString";
    return scname.str ();
}

std::string SyscallName::tostring (const DataType* dType) {
    assert(dType);
    assert (dType->isBuiltinPrimitive ());
    return tostring (static_cast<const DataTypeBuiltinPrimitive*>(dType)->secrecDataType ());
}


std::string SyscallName::basic (const TypeNonVoid* ty, const char* name,  bool needDataType, bool needVec) {
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

} // namespace anonymous

/*******************************************************************************
  Compiler
*******************************************************************************/

Compiler::Compiler (bool optimize)
    : m_target (nullptr)
    , m_param (0)
    , m_funcs (nullptr)
    , m_ra (nullptr)
    , m_scm (nullptr)
    , m_strLit (nullptr)
    , m_optimize (optimize)
{ }

Compiler::~Compiler () {
    cleanup();
}

void Compiler::cleanup() {
    delete m_funcs;
    delete m_ra;
    delete m_scm;
    delete m_strLit;

    m_funcs = nullptr;
    m_ra = nullptr;
    m_scm = nullptr;
    m_strLit = nullptr;
}

void Compiler::run (VMLinkingUnit& vmlu, SecreC::ICode& code) {

    if (m_optimize) {
        optimizeCode (code);
    }
    else {
        removeUnreachableBlocks (code);
        eliminateDeadVariables (code);
    }

    // eliminateRedundantCopies (code);

    // Create and add the linking unit sections:
    auto const rodataSec = new VMDataSection (VMDataSection::RODATA);
    auto const pdSec = new VMBindingSection ("PDBIND");
    auto const scSec = new VMBindingSection ("BIND");
    auto const codeSec = new VMCodeSection ();

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
            .run (code.program ());

    m_target = codeSec;
    m_ra->init(m_st, std::move(lv));
    m_scm->init (m_st, scSec, pdSec);
    m_strLit->init (m_st, rodataSec);

    // Register all protection domains:
    auto const & isProtectionDomainSymbol = [](SecreC::Symbol * sym) {
        assert (sym != nullptr);
        return sym->symbolType() == SYM_DOMAIN;
    };

    for (auto sym : code.symbols().findFromCurrentScope(isProtectionDomainSymbol)) {
        m_scm->addPd(static_cast<SymbolDomain *>(sym));
    }

    // Finally generate code:
    for (const Procedure& proc : code.program ()) {
        cgProcedure (proc);
    }

    m_funcs->generateAll (*m_target, m_st);
    m_target->setNumGlobals (m_ra->globalCount ());

    cleanup();
}

VMValue* Compiler::find (const SecreC::Symbol* sym) const {
    VMValue* const out = m_st.find (sym);
    assert(out);
    return out;
}

VMValue* Compiler::loadToRegister (VMBlock &block, const Symbol *symbol) {
    VMValue * reg = nullptr; // this holds the value of symbol
    if (symbol->symbolType () == SecreC::SYM_CONSTANT) {
        reg = m_ra->temporaryReg (); // temporary register
        block.push_new () << "mov" << find (symbol) << reg;
    }
    else {
        reg = find (symbol);
    }

    return reg;
}

void Compiler::pushString (VMBlock& block, const Symbol* str) {
    assert (str->secrecType ()->secrecDataType ()->isString ());
    if (str->isConstant ()) {
        assert(dynamic_cast<ConstantString const *>(str));
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
    VMLabel * name = nullptr;
    if (!blocks.name())
        name = st ().getLabel (":start"); // nullptr instead?
    else
        name = getProc (st (), blocks.name ());
    m_param = 0;
    VMFunction function (name);
    if (!blocks.name())
        function.setIsStart ();

    m_ra->enterFunction (function);
    for (const Block& block : blocks) {
        if (block.reachable ()) {
            cgBlock (function, block);
        }
    }

    m_ra->exitFunction (function);
    m_target->push_back (function);
}

void Compiler::cgBlock (VMFunction& function, const Block& block) {
    VMLabel * name = nullptr;
    if (block.hasIncomingJumps ()) {
        name = getLabel (st (), block);
    }

    VMBlock vmBlock (name, &block);
    m_ra->enterBlock (vmBlock);
    for (const Imop& imop : block) {
        cgImop (vmBlock, imop);
    }

    m_ra->exitBlock (vmBlock);
    function.push_back (vmBlock);
}

void Compiler::cgJump (VMBlock& block, const Imop& imop) {
    typedef Imop::OperandConstIterator OCI;
    assert (imop.isJump ());

    char const * name = nullptr;
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
        const DataType* scTy = imop.arg1 ()->secrecType ()->secrecDataType ();
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

void Compiler::cgDeclare (VMBlock& block, const Imop& imop) {
    assert (imop.dest ());
    if (imop.dest ()->secrecType ()->secrecSecType ()->isPrivate ()) {
        cgNewPrivateScalar (block, imop.dest ());
    }
}

void Compiler::cgAlloc (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ALLOC);

    if (isPrivate (imop)) {
        cgPrivateAlloc (block, imop);
        return;
    }

    // Public values should have default value argument
    assert (imop.nArgs () == 3);

    block.push_new () << "push" << find (imop.arg2 ());
    block.push_new () << "push" << find (imop.arg1 ());

    VMLabel * target = nullptr;
    VMDataType ty = secrecDTypeToVMDType (imop.dest ()->secrecType ()->secrecDataType ());
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
    VMLabel * target = nullptr;

    const DataType* dType = imop.arg1 ()->secrecType ()->secrecDataType ();
    if (dType->isBool ()) {
        target = m_st.getLabel (":bool_to_string__");
        m_funcs->insert (target, BuiltinBoolToString (m_strLit));

        block.push_new () << "push" << find (imop.arg1 ());
        block.push_new () << "call" << target << find (imop.dest ());
        return;
    }
    else
    if (isNumericDataType (dType)) {
        block.push_new () << "push" << find (imop.arg1 ());
        emitSyscall (block, find (imop.dest ()), SyscallName::tostring (dType));
    }
    else {
        assert (false && "Unable to convert given data type to string!");
        return;
    }
}

void Compiler::cgStrlen (VMBlock& block, const Imop& imop) {
    auto str = imop.arg1 ();
    auto dest = find (imop.dest ());
    if (str->isConstant ()) {
        assert(dynamic_cast<ConstantString const *>(str));
        auto cstr = static_cast<const ConstantString*>(str);
        auto len = m_st.getImm (cstr->value ().size ());
        block.push_new () << "mov" << len << dest;
    }
    else {
        auto arg = find (str);
        block.push_new () << "getmemsize" << arg << dest;
        block.push_new () << "udec" << VM_UINT64 << dest;
    }
}

void Compiler::cgRelease (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::RELEASE);
    if (isPrivate (imop)) {
        cgPrivateRelease (block, imop);
        return;
    }

    // We allow for public scalar to be freed (which is a nop).
    if (imop.arg1()->isArray() || imop.arg1()->isString()) {
        block.push_new () << "free" << find (imop.arg1 ());
        return;
    }
}

void Compiler::cgAssign (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ASSIGN);

    if (imop.dest ()->secrecType ()->secrecDataType ()->isString ()) {
        pushString (block, imop.arg1 ());
        VMLabel* target = m_st.getLabel (":builtin_str_dup__");
        m_funcs->insert (target, BuiltinStrDup ());
        block.push_new () << "call" << target << find (imop.dest ());
        return;
    }

    if (isPrivate (imop)) {
        cgPrivateAssign (block, imop);
        return;
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
    assert (! isPrivate (imop));

    VMDataType destTy = getVMDataType (imop.dest ());
    VMDataType srcTy = getVMDataType (imop.arg1 ());

    if (imop.isVectorized ()) {
        std::stringstream ss;

        VMLabel * target = nullptr;
        if (imop.dest ()->secrecType ()->secrecDataType ()->isBool ()) {
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
        if (imop.dest ()->secrecType ()->secrecDataType ()->isBool ()) {
            VMValue* arg = loadToRegister (block, imop.arg1 ());
            block.push_new() << "tne"
                             << srcTy << find (imop.dest ())
                             << arg << m_st.getImm (0);
        }
        else
        if (destTy != srcTy) {
            VMValue* arg = loadToRegister (block, imop.arg1 ());

            if (isFloating(srcTy) && (isSigned(destTy) || isUnsigned(destTy))) {
                std::stringstream ss;
                ss << ":cast_" << destTy << "_" << srcTy;
                VMLabel* target = m_st.getLabel(ss.str());
                m_funcs->insert (target, BuiltinFloatToInt(destTy, srcTy));
                block.push_new() << "push" << arg;
                block.push_new() << "call" << target << find(imop.dest());
            }
            else {
                block.push_new() << "convert"
                                 << srcTy << arg
                                 << destTy << find (imop.dest ());
            }
        }
        else {
            block.push_new() << "mov"
                             << find (imop.arg1 ())
                             << find (imop.dest ());
        }
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
    for (const Symbol* sym : imop.useRange ()) {
        if (isString (sym))
            pushString (block, sym);
        else
            block.push_new () << "pushcref" << find (sym);
    }

    for (const Symbol* sym : imop.defRange ()) {
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
    block.push_new ()
        << "mov cref" << (m_param ++)
        << "imm 0x0" // offset 0
        << find (imop.dest ())
        << m_st.getImm (sizeInBytes (vmty));
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
            << "ref" << (retCount ++ )
            << "imm 0x0"
            << m_st.getImm (sizeInBytes (ty));
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

    VMValue * rAddr = nullptr;
    if (imop.arg1 ()->isConstant ()) {
        assert (imop.arg1 ()->secrecType ()->secrecDataType ()->isString ());
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

    char const * opname = nullptr;
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

    assert(! isPrivate (imop));

    VMDataType ty = secrecDTypeToVMDType (imop.arg1 ()->secrecType ()->secrecDataType ());
    assert (ty != VM_INVALID);

    if (imop.isVectorized ()) {
        for (const Symbol* sym : imop.operands ()) {
            block.push_new () << "push" << find (sym);
        }

        VMLabel * target = nullptr;
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
    assert(dynamic_cast<SymbolDomain const *>(imop.arg1()));
    const SymbolDomain* dom = static_cast<const SymbolDomain*>(imop.arg1 ());
    assert(dynamic_cast<PrivateSecType const *>(dom->securityType()));
    const auto secTy = static_cast<const PrivateSecType*>(dom->securityType ());
    VMLabel* label = m_scm->getPd(secTy);
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
    assert (imop.isSyscall());

    for (auto op : imop.syscallOperands()) {
        cgSyscallOperand(block, op);
    }

    VMLabel* label = m_scm->getSyscallBinding (static_cast<const ConstantString*>(imop.arg1 ()));
    if (imop.dest ())
        block.push_new () << "syscall" << label << find (imop.dest ());
    else
        block.push_new () << "syscall" << label << "imm";
}

void Compiler::cgSyscallOperand(VMBlock& block, const SyscallOperand& op) {
    const auto arg = op.operand();
    switch (op.passingConvention()) {
    case Push:
        block.push_new () << "push" << find (arg);
        break;
    case PushRef:
        assert (arg->secrecType ()->secrecSecType ()->isPublic ());
        assert (! arg->isConstant ());

        if (arg->isArray() || isString(arg)) {
            block.push_new ()  << "pushref mem" << find (arg);
            return;
        }

        block.push_new ()
            << "pushrefpart"
            << find (arg)
            << "imm" << m_st.getImm(0)
            << sizeInBytes(representationType(arg->secrecType()));
        break;

    case PushCRef:
        assert (arg->secrecType ()->secrecSecType ()->isPublic ());
        if (isString (arg)) {
            pushString (block, arg);
            return;
        }

        if (arg->isArray()) {
            block.push_new ()  << "pushcref mem" << find (arg);
            return;
        }

        // scalars
        block.push_new ()
            << "pushcrefpart"
            << find (arg)
            << "imm" << m_st.getImm(0)
            << sizeInBytes(representationType(arg->secrecType()));
        return;
    case Return:
        /* This case is handled by Compiler::cgSyscall! */
        break;
    }
}

void Compiler::cgPrint (VMBlock& block, const Imop& imop) {
    pushString (block, imop.arg1 ());
    emitSyscall (block, "Process_logString");
}

void Compiler::cgComment(VMBlock & block, const Imop & imop) {
    assert(dynamic_cast<ConstantString const *>(imop.arg1()));
    block.push_new() << "#" <<
        VMInstruction::str (static_cast<const ConstantString*>(imop.arg1())->value().str ());
}

void Compiler::cgError (VMBlock& block, const Imop& imop) {
    assert (imop.type () == Imop::ERROR);
    pushString (block, imop.arg1 ());
    emitSyscall (block, "Process_logString");
    block.push_new () << "except 0xf00";
}

void Compiler::cgImop (VMBlock& block, const Imop& imop) {

    m_ra->getReg (imop);

    if (imop.isJump ()) {
        cgJump (block, imop);
        return;
    }

    switch (imop.type ()) {
    case Imop::DECLARE:
        cgDeclare (block, imop);
        return;
    case Imop::TOSTRING:
        cgToString (block, imop);
        return;
    case Imop::STRLEN:
        cgStrlen (block, imop);
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
        return;
    case Imop::COMMENT:
        cgComment(block, imop);
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
    const TypeNonVoid* ty = dest->secrecType ();
    block.push_new () << "push" << getPD (m_scm, dest);
    block.push_new () << "push" << m_st.getImm (1);
    emitSyscall (block, d, SyscallName::basic (ty, "new"));
}

void Compiler::cgNewPrivate (VMBlock& block, const Symbol* dest, const Symbol* size) {
    VMValue* d = find (dest);
    const TypeNonVoid* ty = dest->secrecType ();
    block.push_new () << "push" << getPD (m_scm, dest);
    block.push_new () << "push" << find (size);
    emitSyscall (block, d, SyscallName::basic (ty, "new"));
}

void Compiler::cgPrivateAssign (VMBlock& block, const Imop& imop) {
    const TypeNonVoid* ty = imop.dest ()->secrecType ();
    block.push_new () << "push" << getPD (m_scm, imop.dest ());
    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.dest ());
    emitSyscall (block, SyscallName::basic (ty, "assign"));
}

void Compiler::cgPrivateCopy (VMBlock& block, const Imop& imop) {
    const TypeNonVoid* ty = imop.dest ()->secrecType ();
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

    const TypeNonVoid* ty = imop.dest ()->secrecType ();
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
    const TypeNonVoid* ty = imop.arg1 ()->secrecType ();

    unsigned size = 0;
    if (ty->secrecDataType ()->isBuiltinPrimitive ()) {
        VMDataType dataTy = secrecDTypeToVMDType (ty->secrecDataType ());
        size = sizeInBytes (dataTy);
    }
    else if (ty->secrecDataType ()->isUserPrimitive ()) {
        assert (ty->secrecSecType ()->isPrivate ());
        auto dt = static_cast<const DataTypeUserPrimitive*> (ty->secrecDataType ());
        SymbolKind* kind =
            static_cast<const PrivateSecType*> (ty->secrecSecType ())->securityKind ();
        assert (kind->findType (dt->name ()) != nullptr);
        auto publicType = kind->findType (dt->name ())->publicType;

        if (publicType)
            size = sizeInBytes (secrecDTypeToVMDType (*publicType));
        else
            assert (false);
    }
    else {
        assert (false);
    }

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

void Compiler::cgPrivateAlloc (VMBlock& block, const Imop& imop) {
    const TypeNonVoid* ty = imop.dest ()->secrecType ();
    VMLabel* pd = getPD (m_scm, imop.dest ());
    cgNewPrivate (block, imop.dest (), imop.arg1 ());

    if (imop.nArgs () == 3) {
        // Has default value
        block.push_new () << "push" << pd;
        block.push_new () << "push" << find (imop.arg2 ());
        block.push_new () << "push" << find (imop.dest ());
        const bool privateArg = imop.arg2 ()->secrecType ()->secrecSecType ()->isPrivate ();
        emitSyscall (block, SyscallName::basic (ty, privateArg ? "fill" : "init"));
    }
}

void Compiler::cgPrivateRelease (VMBlock& block, const Imop& imop) {
    const TypeNonVoid* ty = imop.arg1 ()->secrecType ();
    block.push_new () << "push" << getPD (m_scm, imop.arg1 ());
    block.push_new () << "push" << find (imop.arg1 ());
    emitSyscall (block, SyscallName::basic (ty, "delete"));
}

void Compiler::cgPrivateLoad (VMBlock& block, const Imop& imop) {
    const TypeNonVoid* ty = imop.dest ()->secrecType ();
    block.push_new () << "push" << getPD (m_scm, imop.dest ());
    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.arg2 ());
    block.push_new () << "push" << find (imop.dest ());
    emitSyscall (block, SyscallName::basic (ty, "load"));
}

void Compiler::cgPrivateStore (VMBlock& block, const Imop& imop) {
    const TypeNonVoid* ty = imop.dest ()->secrecType ();
    block.push_new () << "push" << getPD (m_scm, imop.dest ());
    block.push_new () << "push" << find (imop.arg2 ());
    block.push_new () << "push" << find (imop.arg1 ());
    block.push_new () << "push" << find (imop.dest ());
    emitSyscall (block, SyscallName::basic (ty, "store"));
}

} // namespace SecreCC
