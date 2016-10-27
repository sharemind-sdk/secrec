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

#include "DataflowAnalysis.h"
#include "Intermediate.h"
#include "Optimizer.h"
#include "Symbol.h"
#include "analysis/LiveMemory.h"
#include "analysis/ReachableReleases.h"
#include "analysis/ReachableUses.h"

#include <boost/range/adaptor/reversed.hpp>

using boost::adaptors::reverse;

namespace SecreC {

namespace /* anonymous */ {

ReachableReleases::Values getReleases (const Imop* i, const ReachableReleases& rr) {
    const Block& block = *i->block ();
    ReachableReleases::Values after = rr.releasedOnExit (block);
    for (const Imop& imop : reverse (block)) {
        if (&imop == i) {
            break;
        }

        ReachableReleases::update (imop, after);
    }

    return after;
}

ReachableUses::SymbolUses getUses (const Imop* i, const ReachableUses& ru) {
    const Block& block = *i->block ();
    ReachableUses::SymbolUses after = ru.usesOnExit (block);
    for (const Imop& imop : reverse (block)) {
        if (&imop == i)
            break;

        ReachableUses::update (imop, after);
    }

    return after;
}

} // namespace anonymous

bool compareImop (const Imop* a, const Imop* b) {
    return (a->block ()->dfn () < b->block ()->dfn () ||
            a->index () < b->index ());
}

bool eliminateRedundantCopies (const ReachableReleases& rr,
                               const ReachableUses& ru,
                               const LiveMemory& lmem,
                               ICode& code)
{
    Program& program = code.program ();
    std::set<const Imop*> releases;
    std::set<const Imop*> copySet = lmem.deadCopies (program);
    std::vector<const Imop*> copies (copySet.begin (), copySet.end ());

    std::sort (copies.begin (), copies.end (), compareImop);

    for (const Imop* copy : copies) {
        ReachableReleases::Values after = getReleases (copy, rr);
        releases += after[copy->dest ()];
        releases += after[copy->arg1 ()];
    }

    size_t changes = 0;

    for (const Imop* copy : copies) {
        const Symbol* dest = copy->dest ();
        Symbol* newArg = copy->arg1 ();
        ReachableUses::SymbolUses uses = getUses (copy, ru);

        for (Imop* use : uses[dest]) {
            if (use->type () == Imop::RELEASE)
                continue;

            for (unsigned i = 0; i < use->nArgs (); ++i) {
                Symbol* arg = use->arg (i);
                if (arg != nullptr && arg == dest) {
                    use->setArg (i, newArg);
                }
            }
        }
    }

    for (const Imop* copy : copies) {
        copy->block ()->erase (blockIterator (*copy));
        delete copy;
        ++ changes;
    }

    for (const Imop* imop : releases) {
        if (imop->type () == Imop::RELEASE) {
            delete imop;
            ++ changes;
        }
    }

    return changes > 0;
}

bool eliminateRedundantCopies (ICode& code) {
    Program& program = code.program ();
    ReachableReleases reachableReleases;
    ReachableUses reachableUses;
    LiveMemory liveMemory;

    DataFlowAnalysisRunner ()
            .addAnalysis (reachableReleases)
            .addAnalysis (reachableUses)
            .addAnalysis (liveMemory)
            .run (program);

    return eliminateRedundantCopies (reachableReleases, reachableUses, liveMemory, code);
}

} // namespace SecreCC
