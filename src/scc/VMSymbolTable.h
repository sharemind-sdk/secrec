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

#ifndef VM_SYMBOL_TABLE_H
#define VM_SYMBOL_TABLE_H

#include <stdint.h>
#include <string>

namespace SecreC {
    class Symbol;
}

namespace SecreCC {

class VMValue;
class VMImm;
class VMReg;
class VMStack;
class VMLabel;
class VMVReg;
class VMSTImpl;

/*******************************************************************************
  VMSymbolTable
*******************************************************************************/

class VMSymbolTable {

private: /* Not copyable/assignable: */

    VMSymbolTable (const VMSymbolTable&) = delete;
    VMSymbolTable& operator = (const VMSymbolTable&) = delete;

public: /* Methods: */

    VMSymbolTable ();
    ~VMSymbolTable ();

    VMValue* find (const SecreC::Symbol* symbol) const;
    void store (const SecreC::Symbol* symbol, VMValue*);

    unsigned uniq () { return m_uniq ++; }

    VMLabel* getUniqLabel ();
    VMImm*   getImm (uint64_t value);
    VMReg*   getReg (unsigned number);
    VMStack* getStack (unsigned number);
    VMLabel* getLabel (const std::string& name);
    VMVReg*  getVReg (bool isGlobal);

private: /* Fields: */

    VMSTImpl*  m_impl;
    unsigned   m_uniq;
};

}

#endif
