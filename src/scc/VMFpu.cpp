/*
 * Copyright (C) 2016 Cybernetica
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

#include "VMFpu.h"

#include "VMCode.h"
#include "VMValue.h"
#include "VMSymbolTable.h"


namespace SecreCC {

void fpuGetState(VMBlock& block, VMStack* reg) {
    assert (reg != nullptr);
    block.push_new() << "fpu.getstate" << reg;
}

void fpuSetState(VMBlock& block, VMStack* reg) {
    assert (reg != nullptr);
    block.push_new() << "fpu.setstate" << reg;
}

void fpuSetRoundingMode(VMBlock& block, VMSymbolTable& st,
                        VMStack* reg, FpuRoundingMode mode)
{
    assert (reg != nullptr);

    auto clearBit = [&block, &st, reg](unsigned bit) {
        block.push_new() << "bbabj" << VM_UINT64 << reg << st.getImm(1u << bit);
    };

    auto setBit = [&block, &st, reg](unsigned bit) {
        block.push_new() << "bbor" << VM_UINT64 << reg << st.getImm(1u << bit);
    };

    switch(mode) {
    case FpuRoundingMode::DOWNWARD:
        clearBit(1);
        setBit(2);
        break;
    case FpuRoundingMode::TONEAREST:
        clearBit(1);
        clearBit(2);
        break;
    case FpuRoundingMode::TOWARDZERO:
        setBit(1);
        clearBit(2);
        break;
    case FpuRoundingMode::UPWARD:
        setBit(1);
        setBit(2);
        break;
    }
}

} // namespace SecreCC
