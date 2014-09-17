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
#include "Visitor.h"
#include "SecurityType.h"

namespace SecreC {


/*******************************************************************************
  TypeArgument
*******************************************************************************/

SymbolTypeVariable* TypeArgument::bind (StringRef name) const {
    switch (m_kind) {
    case TA_SEC:  return new SymbolDomain (name, secType ());
    case TA_DATA: return new SymbolDataType (name, dataType ());
    case TA_DIM:  return new SymbolDimensionality (name, dimType ());
    }
}

std::ostream& operator << (std::ostream& os, const TypeArgument& a) {
    switch (a.m_kind) {
    case TA_SEC:  os << *a.secType (); break;
    case TA_DATA: os << *a.dataType (); break;
    case TA_DIM:  os <<  a.dimType (); break;
    }

    return os;
}

TypeArgumentKind quantifierKind (const TreeNodeQuantifier& quant) {
    switch (quant.type ()) {
    case NODE_TEMPLATE_QUANTIFIER_DOMAIN: return TA_SEC;
    case NODE_TEMPLATE_QUANTIFIER_DATA: return TA_DATA;
    case NODE_TEMPLATE_QUANTIFIER_DIM: return TA_DIM;
    default:
        assert (false && "ICE: invalid quantifier (probably parser error).");
        return TA_DATA;
    }
}

bool TypeArgument::equals (SecrecDataType dataType) const {
    if (m_kind != TA_DATA)
        return false;
    return un_dataType->equals (dataType);
}

/*******************************************************************************
  TreeNodeTypeArg
*******************************************************************************/

TypeChecker::Status TypeChecker::visitTypeArg (TreeNodeTypeArg *t) {
    assert (t != nullptr);
    if (t->hasTypeArgument ())
        return OK;
    return dispatchTypeArg (*this, t);
}

TypeChecker::Status TypeChecker::visitTypeArgVar(TreeNodeTypeArgVar* t) {
    const StringRef name = t->identifier ()->value ();
    SymbolDomain* symDom = m_st->find<SYM_DOMAIN>(name);
    SymbolDataType* symTy = m_st->find<SYM_TYPE>(name);
    SymbolDimensionality* symDim = m_st->find<SYM_DIM>(name);

    // If the name is not symbol, type or dimensionality type variable:
    if (symDom == nullptr && symTy == nullptr && symDim == nullptr) {
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

    if (symDom != nullptr)
        t->setTypeArgument (symDom->securityType ());
    else
    if (symTy != nullptr)
        t->setTypeArgument (symTy->dataType ());
    else
    if (symDim != nullptr)
        t->setTypeArgument (symDim->dimType ());

    return OK;
}

TypeChecker::Status TypeChecker::visitTypeArgTemplate(TreeNodeTypeArgTemplate* t) {
    DataTypeStruct* structType = nullptr;
    TCGUARD (checkTypeApplication (t->identifier (), t->arguments (), t->location (), structType));
    assert (structType != nullptr);
    t->setTypeArgument (structType);
    return OK;
}

TypeChecker::Status TypeChecker::visitTypeArgDataTypeConst(TreeNodeTypeArgDataTypeConst* t) {
    t->setTypeArgument (DataTypePrimitive::get (getContext (), t->secrecDataType ()));
    return OK;
}

TypeChecker::Status TypeChecker::visitTypeArgDimTypeConst(TreeNodeTypeArgDimTypeConst* t) {
    t->setTypeArgument (t->secrecDimType ());
    return OK;
}

TypeChecker::Status TypeChecker::visitTypeArgPublic(TreeNodeTypeArgPublic* t) {
    t->setTypeArgument (PublicSecType::get (getContext ()));
    return OK;
}


} // namespace SecreC
