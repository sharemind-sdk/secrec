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

#include "Constant.h"
#include "DataflowAnalysis.h"
#include "Intermediate.h"
#include "Optimizer.h"
#include "Symbol.h"
#include "TreeNode.h"
#include "analysis/AbstractReachable.h"
#include "analysis/CopyPropagation.h"
#include "analysis/ReachableDefinitions.h"
#include "analysis/ReachableReturns.h"
#include "analysis/ReachableUses.h"

#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>

using boost::adaptors::reverse;

namespace SecreC {

namespace /* anonymous */ {

template <typename Analysis, typename State, typename Get>
State getInfo (const Imop* i, Get get, bool forward=true) {
    const Block& block = *i->block ();
    State state = get (block);

    if (forward) {
        for (const Imop& imop : block) {
            if (&imop == i) break;
            Analysis::update (imop, state);
        }
    } else {
        for (const Imop& imop : reverse (block)) {
            if (&imop == i) break;
            Analysis::update (imop, state);
        }
    }

    return state;
}

CopyPropagation::Copies getCopies (const Imop* i, const CopyPropagation& cp) {
    return getInfo<CopyPropagation, CopyPropagation::Copies>
        (i, [cp](const Block& block) { return cp.getCopies (block); });
}

SymbolReachable getUses (const Imop* i, const ReachableUses& ru) {
    return getInfo<ReachableUses, SymbolReachable>
        (i, [ru](const Block& block) { return ru.reachableOnExit (block); }, false);
}

SymbolReachable getDefinitions (const Imop* i, const ReachableDefinitions& rd)
{
    return getInfo<ReachableDefinitions, SymbolReachable>
        (i, [rd](const Block& block) { return rd.reachableOnExit (block); }, false);
}

ReachableReturns::Returns getReturns (const Imop* i,
                                      const ReachableReturns& rr)
{
    return getInfo<ReachableReturns, ReachableReturns::Returns>
        (i, [rr](const Block& block) { return rr.returnsOnExit (block); }, false);
}

} // namespace anonymous

bool compareImop (const Imop* a, const Imop* b) {
    return (a->block ()->dfn () < b->block ()->dfn () ||
            a->index () < b->index ());
}

bool eliminateRedundantCopies (const ReachableUses& ru,
                               const ReachableDefinitions& rd,
                               const ReachableReturns& rr,
                               const CopyPropagation& cp,
                               ICode& code)
{
    Program& program = code.program ();
    boost::container::flat_set<const Imop*> releases;
    boost::container::flat_set<const Imop*> copySet;

    for (const Procedure& proc : program) {
        for (const Block& block : proc) {
            for (const Imop& imop : block) {
                if (imop.type () == Imop::COPY)
                    copySet.insert (&imop);
            }
        }
    }

    std::vector<const Imop*> copies (copySet.begin (), copySet.end ());
    std::map<const Imop*, SymbolReachable> useMaps;
    std::sort (copies.begin (), copies.end (), compareImop);

    for (const Imop* copy : copies) {
        useMaps.emplace(std::make_pair (copy, getUses (copy, ru)));
    }

    // Check if copy can be propagated to uses
    std::set<const Imop*> badCopies;
    for (const Imop* copy : copies) {
        const Symbol* dest = copy->dest ();
        const Symbol* arg = copy->arg1 ();

        for (const Imop* use : useMaps[copy][dest]) {
            CopyPropagation::Copies propCopies = getCopies (use, cp);
            if (propCopies.count (copy) == 0) {
                badCopies.insert (copy);
                break;
            }
        }

        for (const Imop* use : useMaps[copy][arg]) {
            CopyPropagation::Copies propCopies = getCopies (use, cp);
            if (propCopies.count (copy) == 0) {
                badCopies.insert (copy);
                break;
            }
        }
    }

    std::vector<const Imop*> filtered;
    std::copy_if (copies.begin (), copies.end (), std::inserter (filtered, filtered.end ()),
                  [&badCopies](const Imop* copy) { return badCopies.count (copy) == 0; });
    copies = filtered;

    // Find releases
    for (const Imop* copy : copies) {
        SymbolReachable& after = useMaps[copy];

        for (Imop* use : after[copy->dest ()]) {
            if (use->type () == Imop::RELEASE) {
                releases.insert (use);
            }
        }

        for (Imop* use : after[copy->arg1 ()]) {
            if (use->type () == Imop::RELEASE) {
                releases.insert (use);
            }
        }
    }

    size_t changes = 0;

    // Replace y -> x if y = COPY x
    for (const Imop* copy : copies) {
        const Symbol* dest = copy->dest ();
        Symbol* newArg = copy->arg1 ();
        SymbolReachable& uses = useMaps[copy];

        // Replace copied variable
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

    // Find kills for each copy
    std::map<Symbol*, boost::container::flat_set<Imop*>> kills;
    for (const Imop* copy : copies) {
        Symbol* newArg = copy->arg1 ();

        SymbolReachable defs = getDefinitions (copy, rd);
        for (Imop* def : defs[newArg]) {
            if (copySet.count (def) != 0)
                continue;

            kills[newArg].insert (def);
        }

        ReachableReturns::Returns rets = getReturns (copy, rr);
        for (const Imop* ret : rets) {
            bool dontRelease = false;

            for (Symbol* sym : ret->useRange ()) {
                if (sym == newArg) {
                    dontRelease = true;
                    break;
                }
            }

            if (dontRelease)
                continue;

            kills[newArg].insert (const_cast<Imop*> (ret));
        }
    }

    // Release before each kill
    for (auto it = kills.begin (); it != kills.end (); ++it) {
        Symbol* arg = it->first;
        for (Imop* kill : it->second) {
            Imop* release = new Imop (nullptr, Imop::RELEASE, nullptr, arg);
            kill->block ()->insert (blockIterator (*kill), *release);
            release->setBlock (kill->block ());
        }
    }

    ConstantString* copyComment =
        ConstantString::get (code.context (), "copy removed by copy elimination");
    for (const Imop* copy : copies) {
        //copy->block ()->erase (blockIterator (*copy));
        //delete copy;
        Imop* newImop = new Imop (copy->creator (), Imop::COMMENT, nullptr, copyComment);
        const_cast<Imop*> (copy)->replaceWith (*newImop);
        ++ changes;
    }

    ConstantString* releaseComment =
        ConstantString::get (code.context (), "release removed by copy elimination");
    for (const Imop* imop : releases) {
        if (imop->type () == Imop::RELEASE) {
            //delete imop;
            Imop* newImop = new Imop (imop->creator (), Imop::COMMENT, nullptr, releaseComment);
            const_cast<Imop*> (imop)->replaceWith (*newImop);
            ++ changes;
        }
    }

    return changes > 0;
}

bool eliminateRedundantCopies (ICode& code) {
    Program& program = code.program ();
    ReachableDefinitions reachableDefinitions;
    ReachableReturns reachableReturns;
    ReachableUses reachableUses;
    CopyPropagation copyPropagation;

    DataFlowAnalysisRunner ()
            .addAnalysis (reachableDefinitions)
            .addAnalysis (reachableUses)
            .addAnalysis (reachableReturns)
            .addAnalysis (copyPropagation)
            .run (program);

    return eliminateRedundantCopies (reachableUses, reachableDefinitions,
                                     reachableReturns, copyPropagation, code);
}

} // namespace SecreCC
