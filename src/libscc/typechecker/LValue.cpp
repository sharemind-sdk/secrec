/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TypeChecker.h"

#include "Symbol.h"
#include "TreeNode.h"
#include "Types.h"
#include "Visitor.h"

namespace SecreC {

/*
 * TODO: we are not eliminating some bad cases here. such as "x[0].y"
 */

/*******************************************************************************
  TreeNodeLValue
*******************************************************************************/

TypeChecker::Status TypeChecker::visitLValue (TreeNodeLValue* lvalue) {
    return dispatchLValue (*this, lvalue);
}

/*******************************************************************************
  TreeNodeLVariable
*******************************************************************************/

TypeChecker::Status TypeChecker::visitLVariable (TreeNodeLVariable* lvar) {
    if (lvar->secrecType () != nullptr)
        return OK;

    if (SymbolSymbol* sym = getSymbol (lvar->identifier ())) {
        TypeNonVoid* eType = sym->secrecType ();
        lvar->setSecrecType (eType);
        return OK;
    }

    return E_TYPE;
}

TreeNodeLValue* TreeNodeExprRVariable::makeLValueV (Location&) const {
    TreeNode* id = identifier ()->clone (parent ());
    TreeNodeLValue* result = new TreeNodeLVariable (location ());
    result->appendChild (id);
    return result;
}

/*******************************************************************************
  TreeNodeLSelect
*******************************************************************************/

TypeChecker::Status TypeChecker::visitLSelect (TreeNodeLSelect* lselect) {
    if (lselect->secrecType ())
        return OK;

    // Check subexpression:
    TreeNodeLValue* lval = lselect->lvalue ();
    TCGUARD (visitLValue (lval));
    TypeNonVoid* fieldType = checkSelect (lval->location (), lval->secrecType (), lselect->identifier ());
    if (fieldType != nullptr) {
        lselect->setSecrecType (fieldType);
        return OK;
    }

    return E_TYPE;
}

TreeNodeLValue* TreeNodeExprSelection::makeLValueV (Location& loc) const {
    if (TreeNodeLValue* lval = expression ()->makeLValue (loc)) {
        TreeNode* id = identifier ()->clone (parent ());
        TreeNodeLValue* result = new TreeNodeLSelect (location ());
        result->appendChild (lval);
        result->appendChild (id);
        return result;
    }

    return nullptr;
}

/*******************************************************************************
  TreeNodeLIndex
*******************************************************************************/

TypeChecker::Status TypeChecker::visitLIndex(TreeNodeLIndex* lindex) {
    if (lindex->secrecType ())
        return OK;

    TreeNodeLValue* lval = lindex->lvalue ();
    TCGUARD (visitLValue (lval));
    TypeNonVoid* lvalType = lval->secrecType ();
    SecrecDimType destDim = 0;
    TCGUARD (checkIndices(lindex->indices (), destDim));
    lindex->setSecrecType (TypeBasic::get (getContext (),
        lvalType->secrecSecType (), lvalType->secrecDataType (), destDim));
    return OK;
}

TreeNodeLValue* TreeNodeExprIndex::makeLValueV (Location& loc) const {
    if (TreeNodeLValue* lval = expression ()->makeLValue (loc)) {
        TreeNode* idx = indices ()->clone (parent ());
        TreeNodeLValue* result = new TreeNodeLIndex (location ());
        result->appendChild (lval);
        result->appendChild (idx);
        return result;
    }

    return nullptr;
}

} // namespace SecreC
