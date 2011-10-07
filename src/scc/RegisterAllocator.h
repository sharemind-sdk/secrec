#ifndef REGISTER_ALLOCATOR_H
#define REGISTER_ALLOCATOR_H

namespace SecreCC {

class VMSymbolTable;
class VMCode;

// sets actual registers for all VMVRegs.
void allocRegisters (VMCode& code, VMSymbolTable& st);

}

#endif
