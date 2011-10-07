#include "Compiler.h"
#include "RegisterAllocator.h"

#include <libscc/treenode.h>

#include <iostream>

using namespace SecreC;

namespace {

using namespace SecreCC;

const char* secrecDTypeToVMDType (SecrecDataType dtype) {
    switch (dtype) {
        case DATATYPE_INT8:   return "int8";
        case DATATYPE_INT16:  return "int16";
        case DATATYPE_INT32:  return "int32";
        case DATATYPE_INT64:  return "int64";
        case DATATYPE_INT:    return "int64";
        case DATATYPE_UINT8:  return "uint8";
        case DATATYPE_UINT16: return "uint16";
        case DATATYPE_UINT32: return "uint32";
        case DATATYPE_UINT64: return "uint64";
        case DATATYPE_UINT:   return "uint64";
        case DATATYPE_BOOL:   return "uint64";
        default:
            assert (false);
            break;
    }

    return 0;
}

unsigned secrecDTypeSize (SecrecDataType dtype) {
    switch (dtype) {
        case DATATYPE_INT8:   return 1;
        case DATATYPE_INT16:  return 2;
        case DATATYPE_INT32:  return 4;
        case DATATYPE_INT64:  return 8;
        case DATATYPE_INT:    return 8;
        case DATATYPE_UINT8:  return 1;
        case DATATYPE_UINT16: return 2;
        case DATATYPE_UINT32: return 4;
        case DATATYPE_UINT64: return 8;
        case DATATYPE_UINT:   return 8;
        case DATATYPE_BOOL:   return 8;
        default:
            assert (false);
            break;
    }

    return 0;
}

/**
 * Functions for mapping SecreC symbols to VM values:
 */

VMImm* getImm (VMSymbolTable& st, const Symbol* sym) {
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
    return static_cast<VMImm*>(imm);
}

VMVReg* getReg (VMSymbolTable& st, const Symbol* sym) {
    assert (sym != 0);
    assert (sym->symbolType () == Symbol::SYMBOL ||
            sym->symbolType () == Symbol::TEMPORARY);
    VMValue* vreg = st.find (sym);
    if (vreg == 0) {
        vreg = st.getVReg ();
        st.store (sym, vreg);
    }

    assert (dynamic_cast<VMVReg*>(vreg) != 0);
    return static_cast<VMVReg*>(vreg);
}

VMLabel* getLabel (VMSymbolTable& st, const Block* block) {
    assert (block != 0);
    std::stringstream ss;
    ss << ":L_" << block->index ();
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
        label = getLabel (st, symL->target ()->block ());
        st.store (sym, label);
    }

    assert (dynamic_cast<VMLabel*>(label) != 0);
    return static_cast<VMLabel*>(label);
}

VMLabel* getProc (VMSymbolTable& st, unsigned& uniq, const Symbol* sym) {
    assert (sym != 0);
    assert (sym->symbolType () == Symbol::PROCEDURE);
    VMValue* label = st.find (sym);
    if (label == 0) {
        std::stringstream ss;
        const SymbolProcedure* proc = static_cast<const SymbolProcedure*>(sym);
        ss << ':' << proc->decl ()->procedureName () << '_' << (uniq ++);
        label = st.getLabel (ss.str ());
        st.store (sym, label);
    }

    assert (dynamic_cast<VMLabel*>(label) != 0);
    return static_cast<VMLabel*>(label);
}

/**
 * Emit operand that is an USE. Meaning either register or an immediate.
 */
VMInstruction& emitDef (VMInstruction& i, VMSymbolTable& st, const Symbol* sym) {
    switch (sym->symbolType ()) {
    case Symbol::SYMBOL:
    case Symbol::TEMPORARY:
        return i.def (getReg (st, sym));
    case Symbol::CONSTANT:
    case Symbol::LABEL:
    case Symbol::PROCEDURE:
        assert (false);
        return i;
    }
}

/**
 * Emit operand DEF, only registers allowed.
 */
VMInstruction& emitUse (VMInstruction& i, VMSymbolTable& st, const Symbol* sym) {
    switch (sym->symbolType ()) {
    case Symbol::CONSTANT:
        return i.arg ("imm").arg (getImm (st, sym));
    case Symbol::SYMBOL:
    case Symbol::TEMPORARY:
        return i.use (getReg (st, sym));
    case Symbol::LABEL:
    case Symbol::PROCEDURE:
        assert (false);
        return i;
    }
}

VMVReg* loadToRegister (VMBlock &block, VMSymbolTable& st, const SecreC::Symbol *symbol) {
    VMVReg* reg = 0; // this holds the value of symbol
    if (symbol->symbolType () == SecreC::Symbol::CONSTANT) {
        reg = st.getVReg (); // temporary register
        block.push_back (VMInstruction ()
                         .arg ("mov imm")
                         .arg (getImm (st, symbol))
                         .def (reg));
    }
    else {
        reg = getReg (st, symbol);
    }

    return reg;
}

}

namespace SecreCC {

/*******************************************************************************
  Compiler
*******************************************************************************/

Compiler::Compiler (const ICode& code)
    : m_code (code)
    , m_uniq (0)
    , m_param (0)
{ }

Compiler::~Compiler () { }

void Compiler::run () {
    m_cfg.init (m_code.code ());
    typedef Blocks::ProcMap::iterator PMI;
    for (PMI i = m_cfg.beginProc (), e = m_cfg.endProc (); i != e; ++ i) {
        cgProcedure (i->first, i->second);
    }

    allocRegisters (m_target, m_st);
}

void Compiler::cgProcedure (const SymbolProcedure* proc,
                            const std::list<Block*>& blocks) {
    typedef std::list<Block*>::const_iterator BI;
    VMLabel* name = 0; 
    if (proc == 0)
        name = st ().getLabel (":start"); // NULL instead?
    else
        name = getProc (st (), m_uniq, proc);
    m_param = 0;
    VMFunction function (name);
    if (proc == 0)
        function.setIsStart ();

    for (BI i = blocks.begin (), e = blocks.end (); i != e; ++ i) {
        Block* block = *i;
        if (block->reachable ()) {
            cgBlock (function, block);
        }
    }

    m_target.push_back (function);
}

void Compiler::cgBlock (VMFunction& function, const Block* block) {
    typedef Block::const_iterator BCI;
    VMLabel* name = getLabel (st (), block);
    VMBlock vmBlock (name, block);
    for (BCI i = block->begin (), e = block->end (); i != e; ++ i) { 
        cgImop (vmBlock, *i);
    }

    function.push_back (vmBlock);
}

void Compiler::cgJump (VMBlock& block, const Imop* imop) {
    typedef Imop::OperandConstIterator OCI;
    assert ((imop->type () & Imop::JUMP_MASK) != 0);

    const char* name = 0;
    switch (imop->type ()) {
        case Imop::JUMP: name = "jmp"; break;
        case Imop::JT  : name = "jnz"; break;
        case Imop::JF  : name = "jz";  break;
        case Imop::JE  : name = "je";  break;
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
    instr.arg (name)
         .arg ("imm")
         .arg (getLabel (st (), imop->jumpDest ()));

    // type of arguments (if needed)
    if (imop->type () != Imop::JUMP) {
        instr.arg (secrecDTypeToVMDType (imop->arg1 ()->secrecType ().secrecDataType ()));
    }

    // arguments
    OCI i = imop->operandsBegin (),
        e = imop->operandsEnd ();
    for (++ i; i != e; ++ i) {
        const Symbol* symbol = *i;
        VMVReg* reg = loadToRegister (block, st (), symbol);
        instr.use (reg);
    }

    block.push_back (instr);
}

/// Assignments can accept immediates
void Compiler::cgAssign (VMBlock& block, const Imop* imop) {
    assert (imop->type () == Imop::ASSIGN   ||
            imop->type () == Imop::CLASSIFY ||
            imop->type () == Imop::DECLASSIFY);

    VMInstruction instr;
    instr.arg ("mov");
    emitUse (instr, st (), imop->arg1 ());
    emitDef (instr, st (), imop->dest ());
    block.push_back (instr);
}

void Compiler::cgCall (VMBlock& block, const Imop* imop) {
    assert (imop->type () == Imop::CALL);
    Imop::OperandConstIterator it, itBegin, itEnd;

    itBegin = imop->operandsBegin ();
    itEnd = imop->operandsEnd ();
    it = itBegin;

    // compute destinations for calls
    const Symbol* dest = *itBegin;

    // push arguments
    for (++ it; it != itEnd && *it != 0; ++ it) {
        block.push_back (
            emitUse (VMInstruction ().arg ("pushcref"), st (), *it)
        );
    }

    assert (it != itEnd && *it == 0 &&
        "Malformed CALL instruction!");

    for (++ it; it != itEnd; ++ it) {
        block.push_back (
            // \todo It may or may not be def... hmms...
            emitDef (VMInstruction ().arg ("pushref"), st (), *it)
        );
    }

    // CALL
    block.push_back (
        VMInstruction ()
            .arg ("call imm")
            .arg (getProc (st (), m_uniq, dest))
            .arg ("imm")
    );
}

void Compiler::cgParam (VMBlock& block, const Imop* imop) {
    assert (imop->type () == Imop::PARAM);    
    block.push_back (
        VMInstruction ()
            .arg ("mov cref_imm")
            .arg (st ().getImm (m_param ++))
            .arg ("0x0") // offset 0
            .def (getReg (st (), imop->dest ()))
            .arg ("imm")
            .arg (st ().getImm (secrecDTypeSize (imop->dest ()->secrecType ().secrecDataType ())))
    );
}

void Compiler::cgReturn (VMBlock& block, const Imop* imop) {
    assert (imop->type () == Imop::RETURN);

    typedef Imop::OperandConstIterator OCI;

    assert (imop->operandsBegin () != imop->operandsEnd () &&
            "Malformed RETURN instruction!");

    OCI it = imop->operandsBegin ();
    OCI itEnd = imop->operandsEnd ();
    unsigned retCount = 0;
    for (++ it; it != itEnd; ++ it) {
        VMInstruction movI;
        movI.arg ("mov");
        emitUse (movI, st (), *it);
        movI.arg ("ref imm")
            .arg (st ().getImm (retCount ++ ))
            .arg ("0x0")
            .arg ("imm")
            .arg (st ().getImm (secrecDTypeSize ((*it)->secrecType ().secrecDataType ())));
        block.push_back (movI);
    }

    block.push_back (
        VMInstruction ()
            .arg ("return imm 0x0")
    );
}

void Compiler::cgArithm (VMBlock& block, const Imop* imop) {
    assert ((imop->type () & Imop::EXPR_MASK) != 0);

    const char* name = 0;
    switch (imop->type ()) {
        case Imop::UMINUS: name = "bneg";  break; 
        case Imop::MUL   : name = "tmul";  break; 
        case Imop::DIV   : name = "tdiv";  break; 
        case Imop::MOD   : name = "tmod";  break; 
        case Imop::ADD   : name = "tadd";  break; 
        case Imop::SUB   : name = "tsub";  break; 
        case Imop::UNEG  : name = "bnot";  break; 
        case Imop::LAND  : name = "ltand"; break; 
        case Imop::LOR   : name = "ltor";  break; 
        case Imop::EQ    : name = "teq";   break;
        case Imop::NE    : name = "tne";   break;
        case Imop::LE    : name = "tle";   break;
        case Imop::LT    : name = "tlt";   break;
        case Imop::GE    : name = "tge";   break;
        case Imop::GT    : name = "tgt";   break;
        default:
            std::cout << imop->toString () << std::endl;
            assert (false && "Not an arithmetic instruction!");
            return;
    }

    VMInstruction instr;
    instr.arg (name)
         .arg (secrecDTypeToVMDType (imop->dest ()->secrecType ().secrecDataType ()))
         .def (getReg (st (), imop->dest ()));

    Imop::OperandConstIterator 
        i = imop->operandsBegin (),
        e = imop->operandsEnd ();
    for (++ i; i != e; ++ i) {
        VMVReg* reg = loadToRegister (block, st (), *i);
        instr.use (reg);
    }

    block.push_back (instr);
}

void Compiler::cgImop (VMBlock& block, const Imop* imop) {
    if ((imop->type () & Imop::JUMP_MASK) != 0) {
        cgJump (block, imop);
        return;
    }

    switch (imop->type ()) {
        case Imop::CLASSIFY:   // \todo
        case Imop::DECLASSIFY: // \todo
        case Imop::ASSIGN:
            cgAssign (block, imop);
            return;
        case Imop::CALL:
            cgCall (block, imop);
            return;
        case Imop::PARAM:
            cgParam (block, imop);
            return;
        case Imop::RETURNVOID:
            block.push_back (VMInstruction ().arg ("return imm 0x0"));
            return;
        case Imop::RETURN:
            cgReturn (block, imop);
            return;
        case Imop::END:
            block.push_back (VMInstruction ().arg ("halt imm 0x0"));
            return;
        case Imop::ERROR:
            block.push_back (VMInstruction ().arg ("halt imm 0xff"));
            return;
        case Imop::RETCLEAN:
        case Imop::COMMENT:
            return;
        default:
            break;
    }

    if ((imop->type () & Imop::EXPR_MASK) != 0) {
        cgArithm (block, imop);
        return;
    }
    
    assert (false && "Unable to handle instruction!");
}

} // namespac SecreC
