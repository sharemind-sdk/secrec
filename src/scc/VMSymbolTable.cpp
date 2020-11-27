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

#include "VMSymbolTable.h"

#include <cassert>
#include <libscc/Symbol.h>
#include <map>
#include <set>
#include <sstream>

#include "VMValue.h"

namespace SecreCC {

namespace /* anonymous */ {

template <typename MapContainer >
void deleteValues (MapContainer& kvs) {
    typename MapContainer::iterator i;
    for (i = kvs.begin (); i != kvs.end (); ++ i) {
        delete i->second;
    }

    kvs.clear ();
}

template <typename K, typename T>
T * insertNew(std::map<K, T *> & map, K const & key) {
    auto const it = map.find(key);
    if (it != map.end())
        return it->second;
    return map.emplace_hint(it, key, new T(key))->second;
}

} // namespace anonymous

/*******************************************************************************
  VMSTImpl
*******************************************************************************/

class __attribute__ ((visibility("internal"))) VMSTImpl {
public: /* Types: */

    typedef std::map<std::string, VMLabel* > LabelMap;
    typedef std::map<uint64_t, VMImm* > ImmMap;
    typedef std::map<std::size_t, VMStack* > LocalMap;
    typedef std::map<std::size_t, VMReg* > GlobalMap;
    typedef std::set<VMVReg* > VRegSet;
    typedef std::map<const SecreC::Symbol*, VMValue*> SymbolMap;

public: /* Methods: */

    ~VMSTImpl () {
        deleteValues (m_globals);
        deleteValues (m_locals);
        deleteValues (m_imms);
        deleteValues (m_labels);       
        for (VRegSet::iterator i = m_vregs.begin (); i != m_vregs.end (); ++ i) {
            VMVReg* vreg = *i;
            delete vreg;
        }
    }

public: /* Fields: */

    GlobalMap   m_globals;
    LocalMap    m_locals;
    ImmMap      m_imms;
    LabelMap    m_labels;
    VRegSet     m_vregs;
    SymbolMap   m_mapping;
};

/*******************************************************************************
  VMSymbolTable
*******************************************************************************/

VMSymbolTable::VMSymbolTable () 
    : m_impl (new VMSTImpl ())
    , m_uniq (0)
{ } 

VMSymbolTable::~VMSymbolTable () {
    delete m_impl;
    m_impl = nullptr;
}

VMVReg* VMSymbolTable::getVReg (bool isGlobal) {
    VMVReg* vreg = new VMVReg (isGlobal);
    m_impl->m_vregs.insert (vreg);
    return vreg;
}

VMValue* VMSymbolTable::find (const SecreC::Symbol* symbol) const {
    assert (symbol != nullptr);
    typedef VMSTImpl::SymbolMap SVM;
    SVM::const_iterator i = m_impl->m_mapping.find (symbol);
    if (i == m_impl->m_mapping.end ()) {
        return nullptr;
    }

    return i->second;
}

void VMSymbolTable::store (const SecreC::Symbol* symbol, VMValue* value) {
    assert (symbol != nullptr);
    assert (value != nullptr);
    typedef VMSTImpl::SymbolMap  SVM;
    SVM::iterator i = m_impl->m_mapping.find (symbol);
    if (i == m_impl->m_mapping.end ()) {
        i = m_impl->m_mapping.insert (i, std::make_pair (symbol, value));
    }
}

VMImm * VMSymbolTable::getImm(uint64_t const value)
{ return insertNew(m_impl->m_imms, value); }

VMReg * VMSymbolTable::getReg (std::size_t const number)
{ return insertNew(m_impl->m_globals, number); }

VMStack * VMSymbolTable::getStack(std::size_t const number)
{ return insertNew(m_impl->m_locals, number); }

VMLabel * VMSymbolTable::getLabel(std::string const & name)
{ return insertNew(m_impl->m_labels, name); }

VMLabel* VMSymbolTable::getUniqLabel () {
    std::ostringstream os;
    os << ":LU_" << uniq ();
    return getLabel (os.str ());
}

}
