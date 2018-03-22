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

#ifndef SECREC_SYSCALL_H
#define SECREC_SYSCALL_H

#include <cassert>
#include <vector>

namespace SecreC {

class Symbol;

enum SyscallAttribute {
    None     = 0x00,
    ReadOnly = 0x01,
};


enum SyscallPassingConvention {
    Return,
    Push,
    PushRef,
    PushCRef,
};

// TODO: make m_attributeSet type safe!
class SyscallOperand {
public: /* Methods: */

    SyscallOperand(Symbol* arg,
                   SyscallPassingConvention convention,
                   SyscallAttribute attr = None)
        : m_operand(arg)
        , m_convention(convention)
        , m_attributeSet(static_cast<unsigned>(attr))
    {
        assert (arg != nullptr);
    }


    SyscallOperand(Symbol* arg,
                   SyscallPassingConvention convention,
                   unsigned attributeSet)
        : m_operand(arg)
        , m_convention(convention)
        , m_attributeSet(attributeSet)
    {
        assert (arg != nullptr);
    }


    Symbol* operand() const { return m_operand; }
    SyscallPassingConvention passingConvention() const { return m_convention; }
    unsigned attributeSet() const { return m_attributeSet; }

    bool isReadOnly() const {
        if (m_convention == PushCRef)
            return true;

        if (m_attributeSet & (static_cast<unsigned>(ReadOnly) != 0))
            return true;

        return false;
    }

private: /* Fields: */
    Symbol* const                  m_operand;
    const SyscallPassingConvention m_convention;
    const unsigned                 m_attributeSet;
};

using SyscallOperands = std::vector<SyscallOperand>;

} /* namespace SecreC { */

#endif /* SECREC_SYSCALL_H */
