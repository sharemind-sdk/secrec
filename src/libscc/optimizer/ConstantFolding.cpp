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

#include "../analysis/ConstantFolding.h"
#include "../ContextImpl.h"
#include "../Constant.h"
#include "../Context.h"
#include "../DataType.h"
#include "../Imop.h"
#include "../Optimizer.h"
#include "../SecurityType.h"
#include "../StringTable.h"
#include "../SymbolTable.h"
#include "../Types.h"
#include "../Intermediate.h"


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
