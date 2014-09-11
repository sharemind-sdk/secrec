/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "CodeGenResult.h"

#include "Intermediate.h"
#include "Symbol.h"

#include <boost/foreach.hpp>

namespace SecreC {

namespace /* anonymous */ {

inline void patchList(PatchList & list, SymbolLabel * dest) {
    BOOST_FOREACH (Imop * imop, list) {
        imop->setDest(dest);
    }

    list.clear();
}

} // namespace anonymous

/*******************************************************************************
  CGResult
*******************************************************************************/

void CGResult::patchNextList(SymbolLabel * dest) {
    patchList(m_nextList, dest);
}

/*******************************************************************************
  CGBranchResult
*******************************************************************************/

void CGBranchResult::patchTrueList(SymbolLabel * dest) {
    patchList(m_trueList, dest);
}

void CGBranchResult::patchFalseList(SymbolLabel * dest) {
    patchList(m_falseList, dest);
}

/*******************************************************************************
  CGStmtResult
*******************************************************************************/

void CGStmtResult::patchBreakList(SymbolLabel * dest) {
    patchList(m_breakList, dest);
}

void CGStmtResult::patchContinueList(SymbolLabel * dest) {
    patchList(m_continueList, dest);
}

} // namespace SecreC
