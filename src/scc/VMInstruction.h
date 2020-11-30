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

#ifndef VMINSTRUCTION_H
#define VMINSTRUCTION_H

#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>
#include "OStreamable.h"
#include "VMDataType.h"


namespace SecreCC {

class VMValue;

/******************************************************************
  VMInstruction
******************************************************************/

class __attribute__ ((visibility("internal"))) VMInstruction {

public: /* Methods: */

    VMInstruction & operator<<(OStreamableString str);
    VMInstruction & operator<<(VMValue const & val);
    VMInstruction & operator<<(uint64_t const n);
    VMInstruction & operator<<(VMDataType const ty);

    VMInstruction & operator<<(VMValue const * const val) {
        assert(val);
        return *this << *val;
    }

private: /* Methods: */

    friend std::ostream & operator<<(std::ostream & o,
                                     VMInstruction const & instr);

private: /* Fields: */

    std::vector<std::shared_ptr<OStreamable const>> m_operands;
};

std::ostream& operator << (std::ostream& o, const VMInstruction& instr)
        __attribute__((visibility("internal")));

}

#endif
