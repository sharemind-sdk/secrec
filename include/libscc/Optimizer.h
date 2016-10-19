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

#ifndef SECREC_OPTIMIZER_H
#define SECREC_OPTIMIZER_H

namespace SecreC {

class ConstantFolding;
class ICode;
class LiveMemory;
class LiveVariables;
class Procedure;
class ReachableReleases;
class ReachableUses;
class SymbolTable;

bool eliminateDeadVariables (const LiveVariables& lva, ICode& code);
bool eliminateConstantExpressions (const ConstantFolding& cf, ICode& code);
bool eliminateRedundantCopies (const ReachableReleases& rr,
                               const ReachableUses& ru,
                               const LiveMemory& lmem,
                               ICode& code);

bool eliminateRedundantCopies (ICode& code);
bool eliminateDeadVariables (ICode& code);
bool eliminateConstantExpressions (ICode& code);
bool removeUnreachableBlocks (ICode& code);
bool removeEmptyBlocks (ICode& code);

void inlineCalls (ICode& code);

bool optimizeCode (ICode& code);

} /* namespace SecreCC { */

#endif /* SECREC_OPTIMIZER_H */
