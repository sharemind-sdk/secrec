/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_OPTIMIZER_H
#define SECREC_OPTIMIZER_H

namespace SecreC {

class ConstantFolding;
class ICode;
class LiveMemory;
class LiveVariables;
class ReachableReleases;

bool eliminateDeadVariables (const LiveVariables& lva, ICode& code);
bool eliminateConstantExpressions (const ConstantFolding& cf, ICode& code);
bool eliminateRedundantCopies (const ReachableReleases& rr,
                               const LiveMemory& lmem,
                               ICode& code);

bool eliminateRedundantCopies (ICode& code);
bool eliminateDeadVariables (ICode& code);
bool eliminateConstantExpressions (ICode& code);
bool removeUnreachableBlocks (ICode& code);

bool optimizeCode (ICode& code);

} /* namespace SecreCC { */

#endif /* SECREC_OPTIMIZER_H */
