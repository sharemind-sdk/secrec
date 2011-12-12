/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECRECC_SYSCALL_MANAGER_H
#define SECRECC_SYSCALL_MANAGER_H

#include <map>

#include <libscc/parser.h>
#include <libscc/constant.h>

namespace SecreC {
    class PrivateSecType;
}

namespace SecreCC {

class VMLabel;
class VMSymbolTable;
class VMLinkingUnit;
class VMBindingSection;

/*******************************************************************************
  SyscallManager
*******************************************************************************/

class SyscallManager {
private: /* Types: */

    typedef std::map<const SecreC::ConstantString*, VMLabel*> SCMap;
    typedef std::map<SecreC::PrivateSecType*, VMLabel* > PDMap;

public: /* Methods: */

    SyscallManager ();
    ~SyscallManager ();

    /**
     * Creates required section in the VM linking unit.
     * The binding requested by the get methods will be added
     * into the apropriate sections.
     */
    void init (VMSymbolTable& st, VMLinkingUnit& vmlu);

    /// Request a label for a security domain.
    VMLabel* getPD (SecreC::PrivateSecType* secTy);

    /// Request a label for a syscall of given name.
    VMLabel* getSyscallBinding (const SecreC::ConstantString* str);

private: /* Fields: */

    VMSymbolTable*     m_st; ///< Labels are managed by the VM symbol table.
    VMBindingSection*  m_pdSection; ///< Section for security domains.
    VMBindingSection*  m_scSection; ///< Section for syscalls.
    SCMap              m_syscalls;
    PDMap              m_pds; ///< Privacy domains
};

} // namespace SecreCC

#endif
