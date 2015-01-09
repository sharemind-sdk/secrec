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

#ifndef SECREC_VISITOR_H
#define SECREC_VISITOR_H

#include "TreeNodeFwd.h"
#include "ParserEnums.h"

/*
 * Here we provide some dispatch callbacks for various abstract syntax tree
 * nodes.  These are intended to be used by AST visitor classes. For each
 * purely virtual class we have a method that dispatches the call to derived
 * classes.
 */

namespace SecreC {

#define O(ENUM, CLASS) \
    case NODE_##ENUM: \
        return visitor.visit##CLASS (static_cast<TreeNode##CLASS *>(t), std::forward<Args>(args)...);

#define DEFAULT(MSG) \
    default: \
        assert (false && (MSG)); \
        return typename V::result_type ();

/**
 * \brief TreeNodeTypeArg visitor
 */
template <typename V, typename... Args>
inline typename V::result_type dispatchTypeArg (V& visitor, TreeNodeTypeArg* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    TYPE_ARG_LIST
    DEFAULT("Unexpected TreeNodeTypeArg!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchDataTypeF (V& visitor, TreeNodeDataTypeF* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DATA_TYPE_LIST
    DEFAULT("Unexpected TreeNodeDataTypeF!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchDimTypeF (V& visitor, TreeNodeDimTypeF* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DIM_TYPE_LIST
    DEFAULT("Unexpected TreeNodeDataTypeF!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchTypeF (V& visitor, TreeNodeTypeF* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DATATYPE_LIST
    DEFAULT("Unexpected TreeNodeTypeF!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchExpr (V& visitor, TreeNodeExpr* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    EXPR_LIST
    DEFAULT("Unexpected TreeNodeExpr!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchStmt (V& visitor, TreeNodeStmt* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    STMT_LIST
    DEFAULT("Unexpected TreeNodeStmt!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchLValue(V& visitor, TreeNodeLValue* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    LVALUE_LIST
    DEFAULT("Unexpected TreeNodeStmt!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchQuantifier (V& visitor, TreeNodeQuantifier* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    QUANTIFIER_LIST
    DEFAULT("Unexpected TreeNodeQuantifier!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchStringPart (V& visitor, TreeNodeStringPart* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    STRING_PART_LIST
    DEFAULT("Unexpected TreeNodeStringPart!")
    }
}

#undef O
#undef DEFAULT

} /* namespace SecreC { */

#endif /* SECREC_VISITOR_H */
