/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
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

    typedef ImopList::const_iterator InsertPoint;

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

    CodeGenState& operator = (CodeGenState state) {
        swapState (state);
        return *this;
    }

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
