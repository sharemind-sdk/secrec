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

void SyscallManager::init (VMSymbolTable& st,
                           VMBindingSection* sc,
                           VMBindingSection* pd) {
    m_st = &st;
    m_scSection = sc;
    m_pdSection = pd;
}

VMLabel* SyscallManager::getPD (SecreC::PrivateSecType* secTy) {
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
  return getSyscallBinding (str->value ());
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
