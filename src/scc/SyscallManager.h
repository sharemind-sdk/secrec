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

#ifndef SECRECC_SYSCALL_MANAGER_H
#define SECRECC_SYSCALL_MANAGER_H

#include <map>

#include <libscc/Parser.h>
#include <libscc/Constant.h>

namespace SecreC {
    class PrivateSecType;
} /* namespace SecreC { */

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
    typedef std::map<const SecreC::PrivateSecType*, VMLabel* > PDMap;

public: /* Methods: */

    SyscallManager ();
    ~SyscallManager ();

    void init (VMSymbolTable& st, VMBindingSection* sc, VMBindingSection* pd);

    VMLabel* getPD (const SecreC::PrivateSecType* secTy);

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
