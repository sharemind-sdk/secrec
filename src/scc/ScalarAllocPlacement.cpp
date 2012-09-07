#include "ScalarAllocPlacement.h"

#include <boost/foreach.hpp>
#include <libscc/intermediate.h>
#include <libscc/analysis/LiveVariables.h>
#include <libscc/dataflowanalysis.h>
#include <libscc/symbol.h>

namespace SecreCC {

using namespace SecreC;

namespace /* anonymous */ {

bool isCandidate (const Symbol* sym) {
    return ! sym->isArray () &&
        sym->secrecType ()->secrecSecType ()->isPrivate ();
}

void releaseAfter (const Imop* imop, Symbol* sym) {
    Block* block = imop->block ();
    ImopList::const_iterator it = ImopList::s_iterator_to (*imop);
    assert (it != block->end ());
    Imop* newImop = new Imop (imop->creator (), Imop::RELEASE, 0, sym);
    block->insert (++ it, *newImop);
}

bool isDead (const LiveVariables::Symbols& liveness, const Symbol* sym) {
    return liveness.find (sym) == liveness.end ();
}

}  // namespace anonymous

AllocMap placePrivateScalarAllocs (SecreC::ICode& code) {
    Program& program = code.program ();

    LiveVariables lva;
    DataFlowAnalysisRunner ()
            .addAnalysis (lva)
            .run (program);

    typedef std::map<Symbol*, std::set<const Imop*> > LocMap;
    LocMap releases;
    AllocMap allocs;

    FOREACH_BLOCK (bi, program) {
        const Block& block = *bi;
        LiveVariables::Symbols live = lva.liveOnExit (block);
        BOOST_REVERSE_FOREACH (const Imop& imop, block) {

            // if symbol is dead before its use:
            if (imop.type () != Imop::ASSIGN && imop.type () != Imop::RELEASE && imop.type () != Imop::RETURN) {
                BOOST_FOREACH (Symbol* sym, imop.useRange ()) {
                    if (isCandidate (sym) && isDead (live, sym)) {
                        releases[sym].insert (&imop);
                    }
                }
            }

            LiveVariables::updateBackwards (imop, live);

            // if symbol is dead before it's definition:
            switch (imop.type ()) {
            case Imop::CALL:
            case Imop::ASSIGN:
                break;
            default:
                BOOST_FOREACH (Symbol* sym, imop.defRange ()) {
                    if (isCandidate (sym) && isDead (live, sym)) {
                        allocs[&imop].insert (sym);
                    }
                }

                break;
            }
        }
    }

    BOOST_FOREACH (const LocMap::value_type& v, releases) {
        BOOST_FOREACH (const Imop* imop, v.second) {
            releaseAfter (imop, v.first);
        }
    }

    return allocs;
}

} // namespace SecreCC
