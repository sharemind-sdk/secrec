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

#include "SyscallManager.h"

#include <libscc/SecurityType.h>

#include "VMSymbolTable.h"
#include "VMCode.h"

using namespace SecreC;

namespace SecreCC {

SyscallManager::SyscallManager(VMSymbolTable & st)
    : m_st(st)
{ }

SyscallManager::~SyscallManager() = default;

void SyscallManager::init(std::shared_ptr<VMBindingSection> sc,
                          std::shared_ptr<VMBindingSection> pd)
{
    m_scSection = std::move(sc);
    m_pdSection = std::move(pd);
}

void SyscallManager::addPd(const SecreC::SymbolDomain * sym) {
    auto const secTy = sym->securityType();
    if (secTy->isPrivate()) {
        auto const privSecTy = static_cast<const SecreC::PrivateSecType*>(secTy);
        auto const i = m_pds.find (privSecTy);
        if (i == m_pds.end ()) {
            auto const label = m_st.getUniqLabel(":PD_");
            m_pds.insert (i, std::make_pair (privSecTy, label));
            m_pdSection->addBinding (label, privSecTy->name ().str());
        }
    }
}

VMLabel* SyscallManager::getPd(const SecreC::PrivateSecType * secTy) const {
    auto const i = m_pds.find (secTy);
    assert (i != m_pds.end ());
    if (i == m_pds.end()) {
        return nullptr;
    }

    return i->second;
}

VMLabel* SyscallManager::getSyscallBinding (const SecreC::ConstantString* str) {
  return getSyscallBinding (str->value ().str ());
}

VMLabel* SyscallManager::getSyscallBinding (const std::string& name) {
    auto i = m_syscalls.find (name);
    if (i == m_syscalls.end ()) {
        auto const label = m_st.getUniqLabel(":SC_");
        i = m_syscalls.insert (i, std::make_pair (name, label));
        m_scSection->addBinding (label, name);
    }

    return i->second;
}

} // namespace SecreCC
