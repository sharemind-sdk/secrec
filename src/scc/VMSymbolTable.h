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

    VMSymbolTable (const VMSymbolTable&); // DO NOT IMPLEMENT
    void operator = (const VMSymbolTable&); // DO NOT IMPLEMENT

public: /* Methods: */

    VMSymbolTable ();
    ~VMSymbolTable ();

    VMValue* find (const SecreC::Symbol* symbol) const;
    void store (const SecreC::Symbol* symbol, VMValue*);

    VMImm*   getImm (uint64_t value);
    VMReg*   getReg (unsigned number);
    VMStack* getStack (unsigned number);
    VMVReg*  getVReg ();
    VMLabel* getLabel (const std::string& name);

private: /* Fields: */

    VMSTImpl*  m_impl;
};

}

#endif
