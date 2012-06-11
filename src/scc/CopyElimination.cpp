#include "CopyElimination.h"

#include <boost/foreach.hpp>
#include <libscc/intermediate.h>
#include <libscc/analysis/LiveMemory.h>
#include <libscc/analysis/ReachableReleases.h>
#include <libscc/dataflowanalysis.h>

namespace SecreCC {

using namespace SecreC;

void eliminateRedundantCopies (ICode& code) {
    Program& program = code.program ();
    ReachableReleases reachableReleases;
    LiveMemory liveMemory;

    DataFlowAnalysisRunner ()
            .addAnalysis (&reachableReleases)
            .addAnalysis (&liveMemory)
            .run (program);

    std::set<const Imop*> releases;
    std::set<const Imop*> copies = liveMemory.deadCopies (program);

    BOOST_FOREACH (const Imop* copy, copies) {
        const Block& block = *copy->block ();
        ReachableReleases::Values vs = reachableReleases.releasedOnExit (block);
        BOOST_REVERSE_FOREACH (const Imop& imop, block) {
            if (&imop == copy) {
                const ReachableReleases::Domain& dom = vs[imop.arg1 ()];
                if (dom.empty ()) {
                    BOOST_FOREACH (const Imop* release, vs[imop.dest ()]) {
                        releases.insert (release);
                    }
                }
                else {
                    BOOST_FOREACH (const Imop* release, vs[imop.arg1 ()]) {
                        releases.insert (release);
                    }
                }

                break;
            }

            ReachableReleases::update (imop, vs);
        }
    }

    BOOST_FOREACH (const Imop* imop, releases) {
        delete imop;
    }

    BOOST_FOREACH (const Imop* imop, copies) {
        Imop* assign = new Imop (imop->creator (), Imop::ASSIGN, imop->dest (), imop->arg1 ());
        const_cast<Imop*>(imop)->replaceWith (*assign);
        delete imop;
    }
}

} // namespace SecreCC
