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

/**
 * Class to manage mapping system calls and protection domains to labels.
 */
class SyscallManager {
private: /* Types: */

    typedef std::map<std::string, VMLabel*> SCMap;
    typedef std::map<SecreC::PrivateSecType*, VMLabel* > PDMap;

public: /* Methods: */

    SyscallManager ();
    ~SyscallManager ();

    void init (VMSymbolTable& st, VMBindingSection* sc, VMBindingSection* pd);

    VMLabel* getPD (SecreC::PrivateSecType* secTy);

    VMLabel* getSyscallBinding (const SecreC::ConstantString* str);
    VMLabel* getSyscallBinding (const std::string& name);

private: /* Fields: */

    VMSymbolTable*     m_st; ///< Labels are managed by the VM symbol table.
    VMBindingSection*  m_pdSection; ///< Section for security domains.
    VMBindingSection*  m_scSection; ///< Section for syscalls.
    SCMap              m_syscalls; ///< Sharemin system calls.
    PDMap              m_pds; ///< Privacy domains
};

} // namespace SecreCC

#endif
