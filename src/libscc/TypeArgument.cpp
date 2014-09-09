/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TypeArgument.h"

#include "StringRef.h"
#include "symbol.h"
#include "treenode.h"
#include "typechecker.h"
#include "symboltable.h"
#include "log.h"

namespace SecreC {


/*******************************************************************************
  TypeArgument
*******************************************************************************/

SymbolTypeVariable* TypeArgument::bind (StringRef name) const {
    switch (m_kind) {
    case TA_UNDEF: return NULL;
    case TA_SEC:   return new SymbolDomain (name, secType ());
    case TA_DATA:  return new SymbolDataType (name, dataType ());
    case TA_DIM:   return new SymbolDimensionality (name, dimType ());
    }
}

TypeChecker::Status TypeChecker::visit(TreeNodeTypeArg* t) {
    assert (t != NULL);
    if (t->hasTypeArgument ())
        return OK;

    return t->accept (*this);
}

/*******************************************************************************
  TreeNodeTypeArgVar
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgVar::accept(TypeChecker & tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeTypeArgVar* t) {
    assert (t != NULL);

    const StringRef name = t->identifier ()->value ();
    SymbolDomain* symDom = m_st->find<SYM_DOMAIN>(name);
    SymbolDataType* symTy = m_st->find<SYM_TYPE>(name);
    SymbolDimensionality* symDim = m_st->find<SYM_DIM>(name);

    // If the name is not symbol, type or dimensionality type variable:
    if (symDom == NULL && symTy == NULL && symDim == NULL) {
        m_log.fatalInProc (t) << "Type variable \'" << name
                              << "\' at " << t->location () << " not in scope.";
        return E_TYPE;
    }

    // If more than one type variable is defined:
    if ((!!symDom + !!symTy + !!symDim) != 1) {
        m_log.fatalInProc (t) << "Ambiguous use of a type variable \'" << name
                              << "\' at " << t->location () << ".";
        return E_TYPE;
    }

    if (symDom != NULL)
        t->setTypeArgument (symDom->securityType ());
    else
    if (symTy != NULL)
        t->setTypeArgument (symTy->dataType ());
    else
    if (symDim != NULL)
        t->setTypeArgument (symDim->dimType ());

    return OK;
}

/*******************************************************************************
  TreeNodeTypeArgTemplate
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgTemplate::accept(TypeChecker & tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeTypeArgTemplate* t) {
    assert (false && "TODO");
    return E_TYPE;
}

/*******************************************************************************
  TreeNodeTypeArgDataTypeConst
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgDataTypeConst::accept(TypeChecker & tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeTypeArgDataTypeConst* t) {
    assert (t != NULL);
    t->setTypeArgument (DataTypePrimitive::get (getContext (), t->secrecDataType ()));
    return OK;
}

/*******************************************************************************
  TreeNodeTypeArgDimTypeConst
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgDimTypeConst::accept(TypeChecker & tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeTypeArgDimTypeConst* t) {
    assert (t != NULL);
    t->setTypeArgument (t->secrecDimType ());
    return OK;
}

/*******************************************************************************
  TreeNodeTypeArgPublic
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgPublic::accept(TypeChecker & tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeTypeArgPublic* t) {
    assert (t != NULL);
    t->setTypeArgument (PublicSecType::get (getContext ()));
    return OK;
}


} // namespace SecreC
