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

#include "Optimizer.h"

#include "DataflowAnalysis.h"
#include "Intermediate.h"
#include "Symbol.h"
#include "analysis/ConstantFolding.h"
#include "analysis/LiveVariables.h"
#include "analysis/LiveMemory.h"
#include "analysis/ReachableReleases.h"
#include "analysis/ReachableUses.h"

namespace SecreC {

bool optimizeCode (ICode& code) {
    ConstantFolding cf;
    DataFlowAnalysisRunner runner;
    LiveMemory lmem;
    LiveVariables lva;
    ReachableReleases rr;
    ReachableUses ru;

    runner.addAnalysis (lva)
          .addAnalysis (cf)
          .addAnalysis (lmem)
          .addAnalysis (rr)
          .addAnalysis (ru);

    inlineCalls (code);

    bool changes = false;
    while (true) {
        if (removeUnreachableBlocks (code)) {
            changes = true;
            continue;
        }

        runner.run (code.program ());

        if (eliminateConstantExpressions (cf, code) ||
            eliminateDeadVariables (lva, code) ||
            eliminateDeadStores (lmem, code) ||
            eliminateRedundantCopies (rr, ru, lmem, code))
        {
            removeEmptyBlocks (code);
            changes = true;
            continue;
        }

        break;
    }

    return changes;
}


} // namespace SecreCC
