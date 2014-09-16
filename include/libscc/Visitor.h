/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_VISITOR_H
#define SECREC_VISITOR_H

#include "TreeNodeFwd.h"
#include "ParserEnums.h"

/*
 * Here we provide some dispatch callbacks for various abstract syntax tree
 * nodes.  These are intended to be used by AST visitor classes.
 */

#define DISPATCH(ENUM,CLASS) \
    case NODE_##ENUM: \
        return visitor.visit##CLASS (static_cast<TreeNode##CLASS *>(t), std::forward<Args>(args)...);

#define DEFAULT(MSG) \
    default: \
        assert (false && (MSG)); \
        return typename V::result_type ();

namespace SecreC {

/**
 * \brief TreeNodeTypeArg visitor
 */
template <typename V, typename... Args>
inline typename V::result_type dispatchTypeArg (V& visitor, TreeNodeTypeArg* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(TYPE_ARG_DATA_TYPE_CONST, TypeArgDataTypeConst)
    DISPATCH(TYPE_ARG_DIM_TYPE_CONST, TypeArgDimTypeConst)
    DISPATCH(TYPE_ARG_PUBLIC, TypeArgPublic)
    DISPATCH(TYPE_ARG_TEMPLATE, TypeArgTemplate)
    DISPATCH(TYPE_ARG_VAR, TypeArgVar)
    DEFAULT("Unexpected TreeNodeTypeArg!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchDataTypeF (V& visitor, TreeNodeDataTypeF* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(DATATYPE_TEMPLATE_F, DataTypeTemplateF)
    DISPATCH(DATATYPE_CONST_F, DataTypeConstF)
    DISPATCH(DATATYPE_VAR_F, DataTypeVarF)
    DEFAULT("Unexpected TreeNodeDataTypeF!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchDimTypeF (V& visitor, TreeNodeDimTypeF* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(DIMTYPE_CONST_F, DimTypeConstF)
    DISPATCH(DIMTYPE_VAR_F, DimTypeVarF)
    DEFAULT("Unexpected TreeNodeDataTypeF!")
    }
}

template <typename V, typename... Args>
inline typename V::result_type dispatchTypeF (V& visitor, TreeNodeTypeF* t, Args&&... args) {
    assert (t != nullptr);
    switch (t->type ()) {
    DISPATCH(TYPEVAR, TypeVarF)
    DISPATCH(DATATYPE_CONST_F, DataTypeConstF)
    DISPATCH(DATATYPE_VAR_F, DataTypeVarF)
    DISPATCH(DATATYPE_TEMPLATE_F, DataTypeTemplateF)
    DISPATCH(DIMTYPE_CONST_F, DimTypeConstF)
    DISPATCH(DIMTYPE_VAR_F, DimTypeVarF)
    DEFAULT("Unexpected TreeNodeTypeF!")
    }
}

#undef DISPATCH
#undef DEFAULT

} /* namespace SecreC { */

#endif /* SECREC_VISITOR_H */
