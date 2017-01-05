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

#ifndef CODEGEN_H
#define CODEGEN_H

#include "VMCode.h"
#include "VMSymbolTable.h"

#include <set>
#include <map>

namespace SecreC {
class Procedure;
class Block;
class Imop;
class ICode;
class SyscallOperand;
}


namespace SecreCC {

class BuiltinFunctions;
class RegisterAllocator;
class SyscallManager;
class StringLiterals;

/*******************************************************************************
  Compiler
*******************************************************************************/

class Compiler {
private:

    Compiler (const Compiler&) = delete;
    Compiler& operator = (const Compiler&) = delete;

public: /* Methods: */

    explicit Compiler (bool optimize);
    ~Compiler ();

    void run (VMLinkingUnit& vmlu, SecreC::ICode& code);

    VMSymbolTable& st () { return m_st; }

private:

    void cleanup();

    void cgProcedure (const SecreC::Procedure& blocks);
    void cgBlock (VMFunction& func, const SecreC::Block& block);
    void cgImop (VMBlock& block, const SecreC::Imop& imop);

    /**
     * Code generation for various intermediate instructions:
     */
    void cgDeclare (VMBlock& block, const SecreC::Imop& imop);
    void cgJump (VMBlock& block, const SecreC::Imop& imop);
    void cgAssign (VMBlock& block, const SecreC::Imop& imop);
    void cgCast (VMBlock& block, const SecreC::Imop& imop);
    void cgClassify (VMBlock& block, const SecreC::Imop& imop);
    void cgDeclassify (VMBlock& block, const SecreC::Imop& imop);
    void cgCopy (VMBlock& block, const SecreC::Imop& imop);
    void cgCall (VMBlock& block, const SecreC::Imop& imop);
    void cgParam (VMBlock& block, const SecreC::Imop& imop);
    void cgReturn (VMBlock& block, const SecreC::Imop& imop);
    void cgArithm (VMBlock& block, const SecreC::Imop& imop);
    void cgAlloc (VMBlock& block, const SecreC::Imop& imop);
    void cgRelease (VMBlock& block, const SecreC::Imop& imop);
    void cgStore (VMBlock& block, const SecreC::Imop& imop);
    void cgLoad (VMBlock& block, const SecreC::Imop& imop);
    void cgSyscall (VMBlock& block, const SecreC::Imop& imop);
    void cgSyscallOperand (VMBlock& block, const SecreC::SyscallOperand& op);
    void cgDomainID (VMBlock& block, const SecreC::Imop& imop);
    void cgPush (VMBlock& block, const SecreC::Imop& imop);
    void cgError (VMBlock& block, const SecreC::Imop& imop);
    void cgPrint (VMBlock& block, const SecreC::Imop& imop);
    void cgComment(VMBlock & block, const SecreC::Imop & imop);
    void cgToString (VMBlock& block, const SecreC::Imop& imop);
    void cgStrlen (VMBlock& block, const SecreC::Imop& imop);


    /**
     * Operations performed through syscalls:
     */
    void cgNewPrivate (VMBlock& block, const SecreC::Symbol* dest, const SecreC::Symbol* size);
    void cgNewPrivateScalar (VMBlock& block, const SecreC::Symbol* dest);
    void emitSyscall (VMBlock& block, VMValue* dest, const std::string& name);
    void emitSyscall (VMBlock& block, const std::string& name);
    void cgPrivateAssign (VMBlock& block, const SecreC::Imop& imop);
    void cgPrivateCopy (VMBlock& block, const SecreC::Imop& imop);
    void cgPrivateNE (VMBlock& block, const SecreC::Imop& imop);
    void cgPrivateArithm (VMBlock& block, const SecreC::Imop& imop);
    void cgPrivateAlloc (VMBlock& block, const SecreC::Imop& imop);
    void cgPrivateRelease (VMBlock& block, const SecreC::Imop& imop);
    void cgPrivateCast (VMBlock& block, const SecreC::Imop& imop);
    void cgPrivateLoad (VMBlock& block, const SecreC::Imop& imop);
    void cgPrivateStore (VMBlock& block, const SecreC::Imop& imop);

    /**
     * Convenience operations:
     */
    VMValue* loadToRegister (VMBlock& block, const SecreC::Symbol* symbol);
    void pushString (VMBlock& block, const SecreC::Symbol* str);
    void paramString (VMBlock& block, const SecreC::Symbol* str);
    VMValue* find (const SecreC::Symbol* sym) const;
    void cgStringAppend (VMBlock& block, const SecreC::Imop& imop);
    void cgStringCmp (VMBlock& block, const SecreC::Imop& imop);


private: /* Fields: */

    VMCodeSection*        m_target;   ///< Target code
    VMSymbolTable         m_st;       ///< VM symbol table
    unsigned              m_param;    ///< Current param count
    BuiltinFunctions*     m_funcs;    ///< Bult-in functions
    RegisterAllocator*    m_ra;       ///< Register allocator
    SyscallManager*       m_scm;      ///< The syscall manager
    StringLiterals*       m_strLit;   ///< String literals
    bool                  m_optimize; ///< If we need to optimize the code.
};

} // namespace SecreCC

#endif
