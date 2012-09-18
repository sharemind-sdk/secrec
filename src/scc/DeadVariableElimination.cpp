#include "DeadVariableElimination.h"

#include <boost/foreach.hpp>
#include <libscc/intermediate.h>
#include <libscc/analysis/LiveVariables.h>
#include <libscc/dataflowanalysis.h>

namespace SecreCC {

using namespace SecreC;

namespace /* anonymous */ {

/**
 * CALL instructions can't be eliminated as they may perform side effects
 * PARAM instructions can't be eliminated as it will mess up passed parameters
 */
inline bool mayEliminate (const Imop& imop) {
    return imop.isExpr () && imop.type () != Imop::CALL && imop.type () != Imop::PARAM;
}

} // namespace anonymous

void eliminateDeadVariables (ICode& code) {
    Program& program = code.program ();

    LiveVariables lva;
    DataFlowAnalysisRunner runner;
    std::vector<const Imop*> deadInstructions;

    runner.addAnalysis(lva);

    bool changed = true;
    while (changed) {
        changed = false;
        runner.run(program);
        deadInstructions.clear();
        FOREACH_BLOCK (bi, code.program ()) {
            const Block& block = *bi;
            LiveVariables::Symbols live = lva.liveOnExit (block);
            BOOST_REVERSE_FOREACH (const Imop& imop, block) {
                if (mayEliminate (imop) && live.count (imop.dest ()) == 0) {
                    deadInstructions.push_back (&imop);
                }
                else {
                    LiveVariables::updateBackwards (imop, live);
                }
            }
        }

        changed = deadInstructions.size () > 0;
        BOOST_FOREACH (const Imop* imop, deadInstructions) {
            delete imop;
        }
    }
}

} // namespace SecreCC
