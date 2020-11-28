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
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>

namespace SecreCC {
namespace {

char const * allocString(std::string const & str) {
    static std::vector<std::unique_ptr<char[]>> storage;
    auto const size = str.size() + 1u;
    auto smartPtr(std::make_unique<char[]>(size));
    auto * const ptr = smartPtr.get();
    storage.emplace_back(std::move(smartPtr));
    return static_cast<char *>(std::memcpy(ptr, str.c_str(), size));
}

} // anonymous namespace

/*******************************************************************************
  VMInstruction
*******************************************************************************/

VMInstruction& VMInstruction::arg (const StringOperand& str) {
    m_operands.emplace_back(Operand(allocString(str.m_str)));
    return *this;
}

VMInstruction& VMInstruction::arg (VMValue* val) {
    assert(val);
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
