#include "DataflowAnalysis.h"
#include "Intermediate.h"
#include "Optimizer.h"
#include "Symbol.h"
#include "analysis/LiveMemory.h"
#include "analysis/ReachableReleases.h"

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

} // namespace anonymous

bool eliminateRedundantCopies (const ReachableReleases& rr,
                               const LiveMemory& lmem,
                               ICode& code)
{
    Program& program = code.program ();
    std::set<const Imop*> releases;
    std::set<const Imop*> copies = lmem.deadCopies (program);

    for (const Imop* copy : copies) {
        ReachableReleases::Values after = getReleases (copy, rr);
        releases += after[copy->dest ()];
        releases += after[copy->arg1 ()];
    }

    size_t changes = 0;

    for (const Imop* imop : releases) {
        if (imop->type () == Imop::RELEASE) {
            delete imop;
            ++ changes;
        }
    }

    for (const Imop* imop : copies) {
        Imop* assign = new Imop (imop->creator (), Imop::ASSIGN, imop->dest (), imop->arg1 ());
        const_cast<Imop*>(imop)->replaceWith (*assign);
        delete imop;
        ++ changes;
    }

    return changes > 0;
}


bool eliminateRedundantCopies (ICode& code) {
    Program& program = code.program ();
    ReachableReleases reachableReleases;
    LiveMemory liveMemory;

    DataFlowAnalysisRunner ()
            .addAnalysis (reachableReleases)
            .addAnalysis (liveMemory)
            .run (program);

    return eliminateRedundantCopies (reachableReleases, liveMemory, code);
}

} // namespace SecreCC
