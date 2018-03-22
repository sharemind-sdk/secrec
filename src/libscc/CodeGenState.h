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

#ifndef SECREC_CODEGENSTATE_H
#define SECREC_CODEGENSTATE_H

#include "ICodeList.h"

namespace SecreC {

class SymbolTable;
class TreeNode;
class ScopedSetNode;

/*******************************************************************************
  CodeGenState
*******************************************************************************/

struct ScopedSetSymbolTable;

/**
 * The part of code generator that needs to be stored and resumed on demand.
 * For example, we might pause code generation at some scope to type check and
 * generate code for some imported module. After the code generation previous
 * state needs to be resumed.
 */
class CodeGenState {
    friend class CodeGen;
    friend class ScopedSetNode;
    friend struct ScopedSetSymbolTable;
protected: /* Types: */

    using InsertPoint = ImopList::const_iterator;

public: /* Methods: */

    CodeGenState ()
        : m_st (nullptr)
        , m_node (nullptr)
    { }

    CodeGenState (InsertPoint it, SymbolTable* st)
        : m_insertPoint (std::move(it))
        , m_st (st)
        , m_node (nullptr)
    { }

    CodeGenState(CodeGenState &&) noexcept = default;
    CodeGenState(CodeGenState const &) noexcept = default;

    CodeGenState & operator=(CodeGenState &&) noexcept = default;
    CodeGenState & operator=(CodeGenState const &) noexcept = default;

    SymbolTable* st () const { return m_st; }

private:

    void swapState (CodeGenState& st) {
        std::swap (m_insertPoint, st.m_insertPoint);
        std::swap (m_st, st.m_st);
        std::swap (m_node, st.m_node);
    }

private: /* Fields: */
    InsertPoint   m_insertPoint;  ///< Location before which to insert instructions.
    SymbolTable*  m_st;           ///< Pointer to symbol table of current scope.
    TreeNode*     m_node;         ///< Current tree node.
};

} // namespace SecreC

#endif // SECREC_CODEGENSTATE_H
