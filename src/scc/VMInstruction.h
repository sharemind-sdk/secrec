#ifndef VMINSTRUCTION_H
#define VMINSTRUCTION_H

#include <vector>
#include <ostream>
#include <string>

namespace SecreCC {

class VMValue;

/******************************************************************
  VMInstruction
******************************************************************/

class VMInstruction {

public: /* Methods: */

    VMInstruction () { }
    ~VMInstruction () { }

    VMInstruction& operator << (const char* str) { return arg (str); }
    VMInstruction& operator << (VMValue* val) { return arg (val); }
    VMInstruction& operator << (unsigned n) { return arg (n); }

protected:

    VMInstruction& arg (const char* str);
    VMInstruction& arg (VMValue* val);
    VMInstruction& arg (unsigned n);

    friend std::ostream& operator << (std::ostream& o, const VMInstruction& instr);

private: /* Fields: */

    std::vector<VMValue* >     m_operands;
    std::vector<std::string >  m_strings;
};


std::ostream& operator << (std::ostream& o, const VMInstruction& instr);

}

#endif
