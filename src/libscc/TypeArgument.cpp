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

/*******************************************************************************
  TreeNodeTypeArgVar
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgVar::accept(TypeChecker & tyChecker) {
    assert (false && "TODO");
    return TypeChecker::E_TYPE;
}

/*******************************************************************************
  TreeNodeTypeArgTemplate
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgTemplate::accept(TypeChecker & tyChecker) {
    assert (false && "TODO");
    return TypeChecker::E_TYPE;
}

/*******************************************************************************
  TreeNodeTypeArgDataTypeConst
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgDataTypeConst::accept(TypeChecker & tyChecker) {
    assert (false && "TODO");
    return TypeChecker::E_TYPE;
}

/*******************************************************************************
  TreeNodeTypeArgDimTypeConst
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgDimTypeConst::accept(TypeChecker & tyChecker) {
    assert (false && "TODO");
    return TypeChecker::E_TYPE;
}

/*******************************************************************************
  TreeNodeTypeArgPublic
*******************************************************************************/

TypeChecker::Status TreeNodeTypeArgPublic::accept(TypeChecker & tyChecker) {
    assert (false && "TODO");
    return TypeChecker::E_TYPE;
}

} // namespace SecreC
