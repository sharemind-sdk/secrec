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

#include <ostream>
#include <sharemind/Concat.h>
#include <sstream>
#include "VMValue.h"


namespace SecreCC {

using sharemind::concat;

VMInstruction & VMInstruction::operator<<(OStreamableString str) {
    m_operands.emplace_back(
                std::make_shared<OStreamableString>(std::move(str)));
    return *this;
}

VMInstruction & VMInstruction::operator<<(VMValue const & val) {
    m_operands.emplace_back(val.streamable());
    return *this;
}

VMInstruction & VMInstruction::operator<<(std::uint64_t const n) {
    struct Streamable: OStreamable {
        Streamable(std::uint64_t const value) noexcept : m_value(value) {}
        std::ostream & streamTo(std::ostream & os) const final override
        { return os << "0x" << std::hex << m_value; }
        std::uint64_t m_value;
    };

    m_operands.emplace_back(std::make_shared<Streamable>(n));
    return *this;
}

VMInstruction & VMInstruction::operator<<(VMDataType const ty) {
    struct Streamable: OStreamable {
        Streamable(VMDataType const dataType) noexcept : m_dataType(dataType) {}
        std::ostream & streamTo(std::ostream & os) const final override
        { return os << dataTypeToStr(m_dataType); }
        VMDataType m_dataType;
    };

    m_operands.emplace_back(std::make_shared<Streamable>(ty));
    return *this;
}

std::ostream & operator<<(std::ostream & os, VMInstruction const & instr) {
    auto it = instr.m_operands.begin();
    auto const end = instr.m_operands.end();
    if (it != end) {
        (*it)->streamTo(os);
        while (++it != end)
            (*it)->streamTo(os << ' ');
    }
    return os;
}

} // namespace SecreCC {
