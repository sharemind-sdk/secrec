#include "SyscallManager.h"

#include "VMSymbolTable.h"
#include "VMCode.h"

using namespace SecreC;

namespace SecreCC {

SyscallManager::SyscallManager ()
    : m_st (0)
    , m_pdSection (0)
    , m_scSection (0)
{ }

SyscallManager::~SyscallManager () { }

/// Creates required sections.
void SyscallManager::init (VMSymbolTable& st, VMLinkingUnit& vmlu) {
    m_st = &st;
    m_scSection = new VMBindingSection ("BIND");
    m_pdSection = new VMBindingSection ("PDBIND");
    vmlu.addSection (m_scSection);
    vmlu.addSection (m_pdSection);
}

VMLabel* SyscallManager::getPD (SecreC::PrivateSecType* secTy) {
    PDMap::iterator i = m_pds.find (secTy);
    if (i == m_pds.end ()) {
        std::ostringstream ss;
        ss << ":PD_" << m_st->uniq ();
        VMLabel* label = m_st->getLabel (ss.str ());
        i = m_pds.insert (i, std::make_pair (secTy, label));
        m_pdSection->addBinding (label, secTy->name ());
    }

    return i->second;
}

VMLabel* SyscallManager::getSyscallBinding (const SecreC::ConstantString* str) {
    SCMap::iterator i = m_syscalls.find (str);
    if (i == m_syscalls.end ()) {
        std::ostringstream ss;
        ss << ":SC_" << m_st->uniq ();
        VMLabel* label = m_st->getLabel (ss.str ());
        i = m_syscalls.insert (i, std::make_pair (str, label));
        m_scSection->addBinding (label, str->name ());
    }

    return i->second;
}

} // namespace SecreCC
