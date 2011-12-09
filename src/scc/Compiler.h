/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "VMCode.h"
#include "VMSymbolTable.h"

namespace SecreC {
class Procedure;
class Block;
class Imop;
class ICode;
}


namespace SecreCC {

class BuiltinFunctions;
class RegisterAllocator;

/*******************************************************************************
  Compiler
*******************************************************************************/

class Compiler {
private:

    Compiler (const Compiler&); // DO NOT IMPLEMENT
    void operator = (const Compiler&); // DO NOT IMPLEMENT

public: /* Methods: */

    Compiler (SecreC::ICode& code);
    ~Compiler ();

    void run (VMLinkingUnit& vmlu);

    VMSymbolTable& st () { return m_st; }

protected:

    void cgProcedure (const SecreC::Procedure& blocks);
    void cgBlock (VMFunction& func, const SecreC::Block& block);
    void cgImop (VMBlock& block, const SecreC::Imop& imop);

    void cgJump (VMBlock& block, const SecreC::Imop& imop);
    void cgAssign (VMBlock& block, const SecreC::Imop& imop);
    void cgCall (VMBlock& block, const SecreC::Imop& imop);
    void cgParam (VMBlock& block, const SecreC::Imop& imop);
    void cgReturn (VMBlock& block, const SecreC::Imop& imop);
    void cgArithm (VMBlock& block, const SecreC::Imop& imop);
    void cgAlloc (VMBlock& block, const SecreC::Imop& imop);
    void cgStore (VMBlock& block, const SecreC::Imop& imop);
    void cgLoad (VMBlock& block, const SecreC::Imop& imop);
    void cgSyscall (VMBlock& block, const SecreC::Imop& imop);
    void cgDomainID (VMBlock& block, const SecreC::Imop& imop);
    void cgPush (VMBlock& block, const SecreC::Imop& imop);

    VMValue* loadToRegister (VMBlock& block, const SecreC::Symbol* symbol);
    void emitAny (VMInstruction& instr, const SecreC::Symbol* symbol);

private: /* Fields: */

    class SyscallManager;

    SecreC::ICode&        m_code;    ///< SecreC intermediate code
    VMCodeSection*        m_target; ///< Target code
    VMSymbolTable         m_st;      ///< VM symbol table
    unsigned              m_param;   ///< Current param count
    BuiltinFunctions*     m_funcs;   ///< Bult-in functions
    RegisterAllocator*    m_ra;      ///< Instance of register allocator
    SyscallManager*       m_scm;
};

} // namespace SecreCC

#endif
