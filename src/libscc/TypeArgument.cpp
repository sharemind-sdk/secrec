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
#include "Symbol.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "SymbolTable.h"
#include "Log.h"

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

std::ostream& operator << (std::ostream& os, const TypeArgument& a) {
    switch (a.m_kind) {
    case TA_UNDEF: os << "TA_UNDEF"; break;
    case TA_SEC:   os << *a.secType (); break;
    case TA_DATA:  os << *a.dataType (); break;
    case TA_DIM:   os <<  a.dimType (); break;
    }

    return os;
}


TypeArgumentKind quantifierKind (const TreeNodeQuantifier& quant) {
    switch (quant.type ()) {
    case NODE_TEMPLATE_DOMAIN_QUANT: return TA_SEC;
    case NODE_TEMPLATE_DATA_QUANT: return TA_DATA;
    case NODE_TEMPLATE_DIM_QUANT: return TA_DIM;
    default: return TA_UNDEF;
    }
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
    assert (t != NULL);
    DataTypeStruct* structType = NULL;
    TCGUARD (checkTypeApplication (t->identifier (), t->arguments (), t->location (), structType));
    assert (structType != NULL);
    t->setTypeArgument (structType);
    return OK;
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
