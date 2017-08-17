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

#ifndef SECREC_VM_FPU_H
#define SECREC_VM_FPU_H

class VMBlock;
class VMStack;
class VMSymbolTable;

namespace SecreCC {

enum class FpuRoundingMode {
    DOWNWARD,
    TONEAREST,
    TOWARDZERO,
    UPWARD
};

void fpuGetState(VMBlock& block, VMStack* reg)
        __attribute__((visibility("internal")));

void fpuSetState(VMBlock& block, VMStack* reg)
        __attribute__((visibility("internal")));

void fpuSetRoundingMode(VMBlock& block, VMSymbolTable& st,
                        VMStack* reg, FpuRoundingMode mode)
        __attribute__((visibility("internal")));

} // namespace SecreCC

#endif /* SECREC_VM_FPU_H */
