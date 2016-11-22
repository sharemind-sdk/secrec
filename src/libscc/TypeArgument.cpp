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

#include "TypeArgument.h"

#include <sharemind/abort.h>
#include "DataType.h"
#include "Log.h"
#include "SecurityType.h"
#include "StringRef.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Visitor.h"

namespace SecreC {


/*******************************************************************************
  TypeArgument
*******************************************************************************/

SymbolTypeVariable* TypeArgument::bind (StringRef name) const {
    switch (m_kind) {
    case TA_SEC:  return new SymbolDomain (name, secType ());
    case TA_DATA: return new SymbolDataType (name, dataType ());
    case TA_DIM:  return new SymbolDimensionality (name, dimType ());
    #ifdef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcovered-switch-default"
    #endif
    default: SHAREMIND_ABORT("TAb %d", static_cast<int>(m_kind));
    #ifdef __clang__
    #pragma GCC diagnostic pop
    #endif
    }
}

std::ostream& operator << (std::ostream& os, const TypeArgument& a) {
    switch (a.m_kind) {
    case TA_SEC:  os << *a.secType (); break;
    case TA_DATA: os << *a.dataType (); break;
    case TA_DIM:  os <<  a.dimType (); break;
    #ifdef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcovered-switch-default"
    #endif
    default: SHAREMIND_ABORT("<<TA %d", static_cast<int>(a.m_kind));
    #ifdef __clang__
    #pragma GCC diagnostic pop
    #endif
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
    const DataTypeStruct* structType = nullptr;
    TCGUARD (checkTypeApplication (t->identifier (), t->arguments (), t->location (), structType));
    assert (structType != nullptr);
    t->setTypeArgument (structType);
    return OK;
}

TypeChecker::Status TypeChecker::visitTypeArgDataTypeConst(TreeNodeTypeArgDataTypeConst* t) {
    t->setTypeArgument (DataTypeBuiltinPrimitive::get (t->secrecDataType ()));
    return OK;
}

TypeChecker::Status TypeChecker::visitTypeArgDimTypeConst(TreeNodeTypeArgDimTypeConst* t) {
    t->setTypeArgument (t->secrecDimType ());
    return OK;
}

TypeChecker::Status TypeChecker::visitTypeArgPublic(TreeNodeTypeArgPublic* t) {
    t->setTypeArgument (PublicSecType::get ());
    return OK;
}

bool operator == (const TypeArgument& a, const TypeArgument& b) {
    if (a.m_kind != b.m_kind)
        return false;

    switch (a.m_kind) {
    case TA_DIM:   return a.un_dimType  == b.un_dimType;
    case TA_SEC:   return a.un_secType  == b.un_secType;
    case TA_DATA:  return a.un_dataType->equals (b.un_dataType);
    #ifdef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcovered-switch-default"
    #endif
    default: SHAREMIND_ABORT("==TA %d", static_cast<int>(a.m_kind));
    #ifdef __clang__
    #pragma GCC diagnostic pop
    #endif
    }
}

} // namespace SecreC
