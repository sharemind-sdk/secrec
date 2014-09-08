/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "typechecker.h"

#include "treenode.h"
#include "symbol.h"

namespace SecreC {

/*
 * TODO: we are not eliminating some bad cases here. such as "x[0].y"
 */

/*******************************************************************************
  TreeNodeLValue
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeLValue* lvalue) {
    return lvalue->accept (*this);
}

/*******************************************************************************
  TreeNodeLVariable
*******************************************************************************/

TypeChecker::Status TreeNodeLVariable::accept(TypeChecker & tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeLVariable* lvar) {
    if (lvar->secrecType () != NULL)
        return OK;

    if (SymbolSymbol* sym = getSymbol (lvar->identifier ())) {
        TypeNonVoid* eType = sym->secrecType ();
        lvar->setSecrecType (eType);
        return OK;
    }

    return E_TYPE;
}

TreeNodeLValue* TreeNodeExprRVariable::makeLValueV (Location& loc) const {
    TreeNode* id = identifier ()->clone (parent ());
    TreeNodeLValue* result = new TreeNodeLVariable (location ());
    result->appendChild (id);
    return result;
}

/*******************************************************************************
  TreeNodeLSelect
*******************************************************************************/

TypeChecker::Status TreeNodeLSelect::accept(TypeChecker & tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeLSelect* lselect) {
    if (lselect->secrecType ())
        return OK;

    // Check subexpression:
    TreeNodeLValue* lval = lselect->lvalue ();
    TCGUARD (visit (lval));
    TypeNonVoid* fieldType = checkSelect (lval->location (), lval->secrecType (), lselect->identifier ());
    if (fieldType != NULL) {
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

    return NULL;
}

/*******************************************************************************
  TreeNodeLIndex
*******************************************************************************/

TypeChecker::Status TreeNodeLIndex::accept(TypeChecker & tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeLIndex* lindex) {
    if (lindex->secrecType ())
        return OK;

    TreeNodeLValue* lval = lindex->lvalue ();
    TCGUARD (visit (lval));
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

    return NULL;
}

} // namespace SecreC
