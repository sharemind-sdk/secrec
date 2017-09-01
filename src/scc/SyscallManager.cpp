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

#include <sstream>
#include <libscc/SecurityType.h>

#include "VMSymbolTable.h"
#include "VMCode.h"

using namespace SecreC;

namespace SecreCC {

SyscallManager::SyscallManager ()
    : m_st(nullptr)
    , m_pdSection(nullptr)
    , m_scSection(nullptr)
{ }

SyscallManager::~SyscallManager () { }

void SyscallManager::init (VMSymbolTable& st,
                           VMBindingSection* sc,
                           VMBindingSection* pd) {
    m_st = &st;
    m_scSection = sc;
    m_pdSection = pd;
}

VMLabel* SyscallManager::getPD (const SecreC::PrivateSecType* secTy) {
    PDMap::iterator i = m_pds.find (secTy);
    if (i == m_pds.end ()) {
        std::ostringstream ss;
        ss << ":PD_" << m_st->uniq ();
        VMLabel* label = m_st->getLabel (ss.str ());
        i = m_pds.insert (i, std::make_pair (secTy, label));
        m_pdSection->addBinding (label, secTy->name ().str());
    }

    return i->second;
}

VMLabel* SyscallManager::getSyscallBinding (const SecreC::ConstantString* str) {
  return getSyscallBinding (str->value ().str ());
}

VMLabel* SyscallManager::getSyscallBinding (const std::string& name) {
    SCMap::iterator i = m_syscalls.find (name);
    if (i == m_syscalls.end ()) {
        std::ostringstream ss;
        ss << ":SC_" << m_st->uniq ();
        VMLabel* label = m_st->getLabel (ss.str ());
        i = m_syscalls.insert (i, std::make_pair (name, label));
        m_scSection->addBinding (label, name);
    }

    return i->second;
}

} // namespace SecreCC
