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

boost::ptr_vector<std::string> VMInstruction::m_allocStrings;

/*******************************************************************************
  VMInstruction
*******************************************************************************/

VMInstruction& VMInstruction::arg (const StringOperand& str) {
    std::string* copy = new std::string (str.m_str);
    m_allocStrings.push_back (copy);
    m_operands.push_back (Operand (copy->c_str()));
    return *this;
}

VMInstruction& VMInstruction::arg (VMValue* val) {
    assert (val != 0);
    m_operands.push_back (Operand (*val));
    return *this;
}

VMInstruction& VMInstruction::arg (uint64_t n) {
    m_operands.push_back (Operand (n));
    return *this;
}

VMInstruction& VMInstruction::arg (const char* cstr) {
    m_operands.push_back (Operand (cstr));
    return *this;
}

void VMInstruction::print (std::ostream& os) const {
    bool first = true;
    for (const Operand& op : m_operands) {
        if (! first)
            os << ' ';
        else
            first = false;

        switch (op.m_type) {
        case Operand::Value:  os << *op.un_value;                     break;
        case Operand::Number: os << "0x" << std::hex << op.un_number; break;
        case Operand::String: os << op.un_string;                     break;
        }
    }
}

std::ostream& operator << (std::ostream& os, const VMInstruction& instr) {
    instr.print (os);
    return os;
}

}
