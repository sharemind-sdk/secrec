/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef VMINSTRUCTION_H
#define VMINSTRUCTION_H

#include <vector>
#include <ostream>
#include <string>

#include "VMDataType.h"

namespace SecreCC {

class VMValue;

/******************************************************************
  VMInstruction
******************************************************************/

/*
 * TODO: this is very inefficient
 */

class VMInstruction {

public: /* Methods: */

    VMInstruction () { }
    ~VMInstruction () { }

    VMInstruction& operator << (const char* str) { return arg (str); }
    VMInstruction& operator << (VMValue* val) { return arg (val); }
    VMInstruction& operator << (unsigned n) { return arg (n); }
    VMInstruction& operator << (VMDataType ty) { return arg (dataTypeToStr (ty)); }

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
