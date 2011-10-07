#include "VMInstruction.h"

#include <cassert>
#include <ostream>

namespace SecreCC {

/*******************************************************************************
  VMInstruction
*******************************************************************************/

VMInstruction& VMInstruction::arg (const char* str) {
    m_operands.push_back (0);
    m_strings.push_back (str);
    return *this;
}

VMInstruction& VMInstruction::arg (VMImm* imm) {
    m_operands.push_back (imm);
    m_strings.push_back (0);
    return *this;
}

VMInstruction& VMInstruction::arg (VMLabel* label) {
    m_operands.push_back (label);
    m_strings.push_back (0);
    return *this;
}

VMInstruction& VMInstruction::use (VMVReg* reg) {
    m_operands.push_back (reg);
    m_strings.push_back (0);
    m_use.insert (reg);
    return *this;
}

VMInstruction& VMInstruction::def (VMVReg* reg) {
    m_operands.push_back (reg);
    m_strings.push_back (0);
    m_def.insert (reg);
    return *this;
}

std::ostream& operator << (std::ostream& os, const VMInstruction& instr) {
    const size_t n = instr.m_operands.size ();
    for (size_t i = 0; i < n; ++ i) {
        if (i > 0) os << ' ';
        if (instr.m_operands[i] == 0) {
            assert (instr.m_strings[i] != 0);
            os << instr.m_strings[i];
        }
        else {
            os << instr.m_operands[i]->toString ();
        }
    }

    return os;
}

}
