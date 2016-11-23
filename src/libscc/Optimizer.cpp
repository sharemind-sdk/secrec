#include "Optimizer.h"

#include "DataflowAnalysis.h"
#include "Intermediate.h"
#include "Symbol.h"
#include "analysis/ConstantFolding.h"
#include "analysis/CopyPropagation.h"
#include "analysis/LiveMemory.h"
#include "analysis/LiveVariables.h"
#include "analysis/ReachableDefinitions.h"
#include "analysis/ReachableReturns.h"
#include "analysis/ReachableUses.h"

namespace SecreC {

bool optimizeCode (ICode& code) {
    ConstantFolding cf;
    DataFlowAnalysisRunner runner;
    LiveMemory lmem;
    LiveVariables lva;
    ReachableDefinitions rd;
    ReachableReturns rr;
    ReachableUses ru;
    CopyPropagation cp;

    runner.addAnalysis (lva)
          .addAnalysis (cf)
          .addAnalysis (ru)
          .addAnalysis (rd)
          .addAnalysis (rr)
          .addAnalysis (cp)
          .addAnalysis (lmem);

    inlineCalls (code);

    bool changes = false;
    while (true) {

        if (removeUnreachableBlocks (code)) {
            removeEmptyProcedures (code);
            changes = true;
            continue;
        }

        runner.run (code.program ());

        if (eliminateConstantExpressions (cf, code) ||
            eliminateDeadVariables (lva, code) ||
            eliminateDeadStores (lmem, code) ||
            eliminateDeadAllocs (ru, code) ||
            eliminateRedundantCopies (ru, rd, rr, cp, code))
        {
            if (removeEmptyBlocks (code))
                removeEmptyProcedures (code);

            code.program ().numberInstructions ();
            code.program ().numberBlocks ();

            changes = true;
            continue;
        }

        break;
    }

    return changes;
}

} // namespace SecreCC
