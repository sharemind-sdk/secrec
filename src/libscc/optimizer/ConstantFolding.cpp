/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Optimizer.h"

#include "Constant.h"
#include "Context.h"
#include "ContextImpl.h"
#include "DataType.h"
#include "Imop.h"
#include "SecurityType.h"
#include "StringTable.h"
#include "SymbolTable.h"
#include "Types.h"
#include "Intermediate.h"
#include "analysis/ConstantFolding.h"

namespace SecreC {

bool eliminateConstantExpressions (const ConstantFolding& cf, ICode& code) {
    size_t replaced = 0;
    auto& prog = code.program ();
    auto& cxt = code.context ();
    auto& st = code.stringTable ();

    for (auto& proc : prog) {
        for (auto& block : proc) {
            replaced += cf.optimizeBlock (cxt, st, block);
        }
    }

    return replaced > 0;
}

bool eliminateConstantExpressions (ICode& code) {
    ConstantFolding cf;
    DataFlowAnalysisRunner runner;
    runner.addAnalysis (cf);

    bool changes = false;
    while (true) {
        runner.run (code.program ());
        if (! eliminateConstantExpressions (cf, code))
            break;

        changes = true;
    }

    return changes;
}

} // namespace SecreC
