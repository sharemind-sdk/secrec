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
#include "analysis/ReachableUses.h"

#include <boost/interprocess/containers/flat_set.hpp>

namespace SecreC {

bool eliminateDeadAllocs (const ReachableUses& ru, ICode& code) {
    std::vector<std::unique_ptr<const Imop>> deadInstructions;
    Program& program = code.program ();
    FOREACH_BLOCK (bi, program) {
        const Block& block = *bi;
        SymbolReachable uses = ru.reachableOnExit (block);
        for (const Imop& imop : reverse (block)) {
            boost::container::flat_set<const Imop*> releases;

            if (imop.type () == Imop::ALLOC) {
                bool dead = true;
                for (const Imop* use : uses[imop.dest ()]) {
                    if (use->type () == Imop::RELEASE) {
                        releases.insert (use);
                    } else {
                        dead = false;
                        break;
                    }
                }

                if (dead) {
                    for (const Imop* rel : releases)
                        deadInstructions.emplace_back (rel);
                    deadInstructions.emplace_back (&imop);
                }
            }

            ReachableUses::update (imop, uses);
        }
    }

    return deadInstructions.size () > 0;
}

} // namespace SecreC
