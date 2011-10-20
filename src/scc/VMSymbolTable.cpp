/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "VMSymbolTable.h"
#include "VMValue.h"

#include <libscc/symbol.h>

#include <map>
#include <set>

namespace {

template <typename T, typename V >
void deleteValues (typename std::map<T, V* >& kvs) {
    typename std::map<T, V* >::iterator i;
    for (i = kvs.begin (); i != kvs.end (); ++ i) {
        delete i->second;
    }

    kvs.clear ();
}

}

namespace SecreCC {

/*******************************************************************************
  VMSTImpl
*******************************************************************************/

class VMSTImpl {
public:

    VMSTImpl () { }
    ~VMSTImpl () {
        deleteValues (m_globals);
        deleteValues (m_locals);
        deleteValues (m_imms);
        deleteValues (m_labels);       
        for (std::set<VMVReg*>::iterator
             i = m_vregs.begin (); i != m_vregs.end (); ++ i) {
            VMVReg* vreg = *i;
            delete vreg;
        }
    }

    std::map<unsigned, VMReg* >                m_globals;
    std::map<unsigned, VMStack* >              m_locals;
    std::map<uint64_t, VMImm* >                m_imms;
    std::map<std::string, VMLabel* >           m_labels;
    std::set<VMVReg* >                         m_vregs;
    std::map<const SecreC::Symbol*, VMValue*>  m_mapping;
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
    m_impl = 0;
}

VMVReg* VMSymbolTable::getVReg (bool isGlobal) {
    VMVReg* vreg = new VMVReg (isGlobal);
    m_impl->m_vregs.insert (vreg);
    return vreg;
}

VMValue* VMSymbolTable::find (const SecreC::Symbol* symbol) const {
    assert (symbol != 0);
    typedef std::map<const SecreC::Symbol*, VMValue*>  SVM;
    SVM::const_iterator i = m_impl->m_mapping.find (symbol);
    if (i == m_impl->m_mapping.end ()) {
        return 0;
    }

    return i->second;
}

void VMSymbolTable::store (const SecreC::Symbol* symbol, VMValue* value) {
    assert (symbol != 0);
    assert (value != 0);
    typedef std::map<const SecreC::Symbol*, VMValue*>  SVM;
    SVM::iterator i = m_impl->m_mapping.find (symbol);
    if (i == m_impl->m_mapping.end ()) {
        i = m_impl->m_mapping.insert (i, std::make_pair (symbol, value));
    }
}

VMImm* VMSymbolTable::getImm (uint64_t value) {
    std::map<uint64_t, VMImm* >::iterator i = m_impl->m_imms.find (value);
    if (i == m_impl->m_imms.end ()) {
        i = m_impl->m_imms.insert (i, std::make_pair (value, new VMImm (value)));
    }

    return i->second;
}

VMReg* VMSymbolTable::getReg (unsigned number) {
    std::map<unsigned, VMReg* >::iterator i = m_impl->m_globals.find (number);
    if (i == m_impl->m_globals.end ()) {
        i = m_impl->m_globals.insert (i, std::make_pair (number, new VMReg (number)));
    }

    return i->second;
}

VMStack* VMSymbolTable::getStack (unsigned number) {
    std::map<unsigned, VMStack* >::iterator i = m_impl->m_locals.find (number);
    if (i == m_impl->m_locals.end ()) {
        i = m_impl->m_locals.insert (i, std::make_pair (number, new VMStack (number)));
    }

    return i->second;
}

VMLabel* VMSymbolTable::getLabel (const std::string& name) {
    std::map<std::string, VMLabel* >::iterator i = m_impl->m_labels.find (name);
    if (i == m_impl->m_labels.end ()) {
        i = m_impl->m_labels.insert (i, std::make_pair (name, new VMLabel (name)));
    }

    return i->second;
}

}
