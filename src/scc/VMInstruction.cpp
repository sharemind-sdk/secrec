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
