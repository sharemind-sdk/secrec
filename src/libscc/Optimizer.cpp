/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Optimizer.h"

#include "DataflowAnalysis.h"
#include "Intermediate.h"
#include "Symbol.h"
#include "analysis/ConstantFolding.h"
#include "analysis/LiveVariables.h"


namespace SecreC {

bool optimizeCode (ICode& code) {
    DataFlowAnalysisRunner runner;
    LiveVariables lva;
    ConstantFolding cf;

    runner.addAnalysis (lva)
          .addAnalysis (cf);

    bool changes = false;
    while (true) {

        if (removeUnreachableBlocks (code)) {
            changes = true;
            continue;
        }

        runner.run (code.program ());

        if (eliminateConstantExpressions (cf, code) ||
            eliminateDeadVariables (lva, code))
        {
            changes = true;
            continue;
        }

        break;
    }

    return changes;
}


} // namespace SecreCC
