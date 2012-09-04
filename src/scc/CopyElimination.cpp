#include "CopyElimination.h"

#include <boost/foreach.hpp>
#include <libscc/intermediate.h>
#include <libscc/analysis/LiveMemory.h>
#include <libscc/analysis/ReachableReleases.h>
#include <libscc/dataflowanalysis.h>
#include <libscc/symbol.h>

namespace SecreCC {

using namespace SecreC;

namespace /* anonymous */ {

ReachableReleases::Values getReleases (const Imop* i, ReachableReleases& rr) {
    const Block& block = *i->block ();
    ReachableReleases::Values after = rr.releasedOnExit (block);
    BOOST_REVERSE_FOREACH (const Imop& imop, block) {
        if (&imop == i) {
            break;
        }

        ReachableReleases::update (imop, after);
    }

    return after;
}

void print_graph (std::ostream& os, const std::set<const Imop*>& deadCopies) {
    unsigned count = 1;
    std::map<const Symbol*, unsigned> labels;

    os << "digraph DeadCopies {\n";
    BOOST_FOREACH (const Imop* imop, deadCopies) {
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

    BOOST_FOREACH (const Imop* copy, copies) {
        ReachableReleases::Values after = getReleases (copy, reachableReleases);
        releases += after[copy->dest ()];
        releases += after[copy->arg1 ()];
    }

    BOOST_FOREACH (const Imop* imop, releases) {
        if (imop->type () == Imop::RELEASE)
            delete imop;
    }

    BOOST_FOREACH (const Imop* imop, copies) {
        Imop* assign = new Imop (imop->creator (), Imop::ASSIGN, imop->dest (), imop->arg1 ());
        const_cast<Imop*>(imop)->replaceWith (*assign);
        delete imop;
    }
}

} // namespace SecreCC
