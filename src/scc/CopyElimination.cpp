#include "CopyElimination.h"

#include <boost/range/adaptor/reversed.hpp>
#include <libscc/Intermediate.h>
#include <libscc/analysis/LiveMemory.h>
#include <libscc/analysis/ReachableReleases.h>
#include <libscc/DataflowAnalysis.h>
#include <libscc/Symbol.h>

using boost::adaptors::reverse;

namespace SecreCC {

using namespace SecreC;

namespace /* anonymous */ {

ReachableReleases::Values getReleases (const Imop* i, ReachableReleases& rr) {
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

#if 0
// For debugging
void print_graph (std::ostream& os, const std::set<const Imop*>& deadCopies) {
    unsigned count = 1;
    std::map<const Symbol*, unsigned> labels;

    os << "digraph DeadCopies {\n";
    for (const Imop* imop : deadCopies) {
        unsigned& dest = labels[imop->dest ()];
        unsigned& src = labels[imop->arg1 ()];

        if (dest == 0) {
            dest = count ++;
            os << "  ";
            os << "node_" << dest << " [label=\"" << imop->dest ()->name () << "\"];\n";
        }

        if (src == 0) {
            src = count ++;
            os << "  ";
            os << "node_" << src << " [label=\"" << imop->arg1 ()->name () << "\"];\n";
        }

        os << "  ";
        os << "node_" << src << " -> "
           << "node_" << dest << ";\n";
    }

    os << "}\n";
}
#endif

} // namespace anonymous

void eliminateRedundantCopies (ICode& code) {
    Program& program = code.program ();
    ReachableReleases reachableReleases;
    LiveMemory liveMemory;

    DataFlowAnalysisRunner ()
            .addAnalysis (reachableReleases)
            .addAnalysis (liveMemory)
            .run (program);

    std::set<const Imop*> releases;
    std::set<const Imop*> copies = liveMemory.deadCopies (program);

    // print_graph (std::cerr, copies);

    for (const Imop* copy : copies) {
        ReachableReleases::Values after = getReleases (copy, reachableReleases);
        releases += after[copy->dest ()];
        releases += after[copy->arg1 ()];
    }

    for (const Imop* imop : releases) {
        if (imop->type () == Imop::RELEASE)
            delete imop;
    }

    for (const Imop* imop : copies) {
        Imop* assign = new Imop (imop->creator (), Imop::ASSIGN, imop->dest (), imop->arg1 ());
        const_cast<Imop*>(imop)->replaceWith (*assign);
        delete imop;
    }
}

} // namespace SecreCC
