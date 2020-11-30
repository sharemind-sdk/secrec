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

#include "VMValue.h"

#include <cassert>
#include <iomanip>
#include <sstream>


#define DEFINE_STREAMABLE(Class,T,...) \
    struct Class: OStreamable { \
        Class(T value) : m_value(std::move(value)) {} \
        std::ostream & streamTo(std::ostream & os) const final override \
        { return os << __VA_ARGS__; } \
        T m_value; \
    }
#define DEFINE_PREFIXED_HEX_STREAMABLE_VALUE(Class,T,prefix) \
    Class::Class(T value) \
        : VMValue( \
            [v = std::move(value)] { \
                DEFINE_STREAMABLE(Streamable, \
                                  T, \
                                  prefix " 0x" << std::hex << m_value); \
                return std::make_shared<Streamable>(std::move(v)); \
            }()) \
    {}

namespace SecreCC {
namespace {

struct LabelStreamable: OStreamable {
    LabelStreamable(std::string label)
        : m_nameStreamable(std::move(label))
    {}

    std::ostream & streamTo(std::ostream & os) const final override
    { return m_nameStreamable.streamTo(os << "imm "); }

    OStreamableString m_nameStreamable;
};

} // anonymous namespace

VMValue::~VMValue() = default;

DEFINE_PREFIXED_HEX_STREAMABLE_VALUE(VMImm, std::uint64_t, "imm")
DEFINE_PREFIXED_HEX_STREAMABLE_VALUE(VMStack, unsigned, "stack")
DEFINE_PREFIXED_HEX_STREAMABLE_VALUE(VMReg, unsigned, "reg")

VMLabel::VMLabel(std::string name)
    : VMValue(std::make_shared<LabelStreamable>(std::move(name)))
{}

std::shared_ptr<OStreamable> VMLabel::nameStreamable() const noexcept {
    auto const & s = streamable();
    return std::shared_ptr<OStreamable>(
                s,
                &static_cast<LabelStreamable &>(*s).m_nameStreamable);
}

std::string const & VMLabel::name() const noexcept {
    return static_cast<LabelStreamable &>(
                *streamable()).m_nameStreamable.m_value;
}

VMVReg::VMVReg(bool const isGlobal)
    : VMValue(std::make_shared<OStreamableString>(std::string()))
    , m_isGlobal(isGlobal)
{}

void VMVReg::setActualReg(VMValue const & reg) {
    std::ostringstream oss;
    assert(reg.streamable());
    reg.streamable()->streamTo(oss);
    static_cast<OStreamableString &>(*streamable()).m_value = oss.str();
}

} // namespace SecreCC {

#undef DEFINE_PREFIXED_HEX_STREAMABLE_VALUE
#undef DEFINE_STREAMABLE
