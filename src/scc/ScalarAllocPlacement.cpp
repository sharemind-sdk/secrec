#include "ScalarAllocPlacement.h"

#include <boost/range/adaptor/reversed.hpp>
#include <libscc/Intermediate.h>
#include <libscc/analysis/LiveVariables.h>
#include <libscc/DataflowAnalysis.h>
#include <libscc/Symbol.h>
#include <libscc/SecurityType.h>
#include <libscc/Types.h>

using boost::adaptors::reverse;

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
    return liveness.count (sym) == 0;
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
        for (const Imop& imop : reverse (block)) {

            // if symbol is dead before its use:
            if (imop.type () != Imop::RELEASE && imop.type () != Imop::RETURN) {
                for (Symbol* sym : imop.useRange ()) {
                    if (isCandidate (sym) && isDead (live, sym)) {
                        releases[sym].insert (&imop);
                    }
                }
            }

            LiveVariables::updateBackwards (imop, live);

            // if symbol is dead before it's definition:
            switch (imop.type ()) {
            case Imop::CALL:
                break;
            default:
                for (Symbol* sym : imop.defRange ()) {
                    if (isCandidate (sym) && isDead (live, sym)) {
                        allocs[&imop].insert (sym);
                    }
                }

                break;
            }
        }
    }

    for (const auto& v : releases) {
        for (const Imop* imop : v.second) {
            releaseAfter (imop, v.first);
        }
    }

    return allocs;
}

} // namespace SecreCC
