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

#include "DataflowAnalysis.h"
#include "Intermediate.h"
#include "Optimizer.h"
#include "analysis/LiveMemory.h"

#include <memory>

namespace SecreC {

bool eliminateDeadStores (const LiveMemory& lmem, ICode& code) {
    std::vector<std::unique_ptr<const Imop>> deadInstructions;
    Program& program = code.program ();
    FOREACH_BLOCK (bi, program) {
        const Block& block = *bi;
        LiveMemory::Values values = lmem.liveOnExit (block);
        for (const Imop& imop : reverse (block)) {
            if (imop.type () == Imop::STORE &&
                (values.count (imop.dest ()) == 0 ||
                 (values.at (imop.dest ()) & LiveMemory::Read) == 0x0))
            {
                deadInstructions.emplace_back (&imop);
            }

            LiveMemory::update (imop, values);
        }
    }

    return deadInstructions.size () > 0;
}

} // namespace SecreC
