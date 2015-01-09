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

#include "CodeGenResult.h"

#include "Intermediate.h"
#include "Symbol.h"

namespace SecreC {

namespace /* anonymous */ {

inline void patchList(PatchList & list, SymbolLabel * dest) {
    for (Imop * imop : list) {
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
