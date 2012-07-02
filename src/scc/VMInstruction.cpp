/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "VMInstruction.h"
#include "VMValue.h"

#include <cassert>
#include <ostream>
#include <sstream>

namespace SecreCC {

/*******************************************************************************
  VMInstruction
*******************************************************************************/

VMInstruction& VMInstruction::arg (const char* str) {
    m_operands.push_back (0);
    m_strings.push_back (str);
    return *this;
}

VMInstruction& VMInstruction::arg (VMValue* val) {
    assert (val != 0);
    m_operands.push_back (val);
    m_strings.push_back ("");
    return *this;
}

VMInstruction& VMInstruction::arg (unsigned n) {
    std::ostringstream ss;
    ss << "0x" << std::hex << n;
    m_operands.push_back (0);
    m_strings.push_back (ss.str ());
    return *this;
}

std::ostream& operator << (std::ostream& os, const VMInstruction& instr) {
    const size_t n = instr.m_operands.size ();
    for (size_t i = 0; i < n; ++ i) {
        if (i > 0) os << ' ';
        if (instr.m_operands[i] == 0) {
            os << instr.m_strings[i];
        }
        else {
            os << instr.m_operands[i]->toString ();
        }
    }

    return os;
}

}
