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

#include "../analysis/LiveVariables.h"
#include "../Constant.h"
#include "../DataflowAnalysis.h"
#include "../Intermediate.h"
#include "../Optimizer.h"

#include <boost/range/adaptor/reversed.hpp>
#include <memory>


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
    Program& program = code.program ();
    std::vector<std::pair<std::unique_ptr<Imop>, Imop*>> replace;

    FOREACH_BLOCK (bi, program) {
        const Block& block = *bi;
        LiveVariables::Symbols live = lva.liveOnExit (block);
        for (const Imop& imop : reverse (block)) {
            if (mayEliminate (imop) && live.count (imop.dest ()) == 0) {
                Imop* newImop = new Imop (imop.creator (), Imop::COMMENT, nullptr,
                                          ConstantString::get (code.context (),
                                                               "dead variable eliminated"));
                replace.emplace_back (std::unique_ptr<Imop> (const_cast<Imop*> (&imop)), newImop);
            } else {
                LiveVariables::updateBackwards (imop, live);
            }
        }
    }

    for (auto& p : replace)
        p.first->replaceWith (*p.second);

    return replace.size ();
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

} // namespace SecreC
