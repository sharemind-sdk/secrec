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
#include "analysis/LiveVariables.h"

#include <boost/range/adaptor/reversed.hpp>

using boost::adaptors::reverse;

namespace SecreC {

namespace /* anonymous */ {

/**
 * CALL instructions can't be eliminated as they may perform side effects
 * PARAM instructions can't be eliminated as it will mess up passed parameters
 */
inline bool mayEliminate (const Imop& imop) {
    switch (imop.type ()) {
    case Imop::CALL:
    case Imop::PARAM:
    case Imop::SYSCALL:
        return false;
    default:
        return imop.isExpr ();
    }
}

} // namespace anonymous

bool eliminateDeadVariables (const LiveVariables& lva, ICode& code) {
    std::vector<std::unique_ptr<const Imop>> deadInstructions;
    Program& program = code.program ();
    FOREACH_BLOCK (bi, program) {
        const Block& block = *bi;
        LiveVariables::Symbols live = lva.liveOnExit (block);
        for (const Imop& imop : reverse (block)) {
            if (mayEliminate (imop) && live.count (imop.dest ()) == 0)
                deadInstructions.emplace_back (&imop);
            else
                LiveVariables::updateBackwards (imop, live);
        }
    }

    return deadInstructions.size () > 0;
}

bool eliminateDeadVariables (ICode& code) {
    LiveVariables lva;
    DataFlowAnalysisRunner runner;
    runner.addAnalysis(lva);

    bool changes = false;
    while (true) {
        runner.run (code.program ());
        if (! eliminateDeadVariables (lva, code))
            break;

        changes = true;
    }

    return changes;
}

} // namespace SecreCC
