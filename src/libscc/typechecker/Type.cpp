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
#include "log.h"

namespace SecreC {

/*******************************************************************************
 TreeNodeQuantifier
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeQuantifier* q) {
    return q->accept (*this);
}

/*******************************************************************************
 TreeNodeDomainQuantifier
*******************************************************************************/

TypeChecker::Status TreeNodeDomainQuantifier::accept(TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeDomainQuantifier* q) {
    if (q->kind ()) {
        Symbol* kindSym = findIdentifier (q->kind ());
        if (kindSym == 0)
            return E_TYPE;
        if (kindSym->symbolType () != Symbol::PKIND) {
            m_log.fatal () << "Identifier at " << q->location ()
                           << " is not a security domain kind.";
            return E_TYPE;
        }
    }

    return OK;
}

/*******************************************************************************
 TreeNodeDimQuantifier
*******************************************************************************/

TypeChecker::Status TreeNodeDimQuantifier::accept(TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeDimQuantifier* q) {
    return OK;
}

/*******************************************************************************
 TreeNodeTypeF
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeTypeF* ty) {
    return ty->accept (*this);
}

/*******************************************************************************
 TreeNodeSecTypeF
*******************************************************************************/

TypeChecker::Status TreeNodeSecTypeF::accept(TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit(TreeNodeSecTypeF * ty) {
    if (ty->cachedType () != 0)
        return OK;

    if (ty->isPublic ()) {
        ty->setCachedType (PublicSecType::get (getContext ()));
        return OK;
    }

    TreeNodeIdentifier* id = ty->identifier ();
    Symbol* s = findIdentifier (id);
    if (s == 0)
        return E_TYPE;

    if (s->symbolType () != Symbol::PDOMAIN) {
        m_log.fatalInProc(ty) << "Expecting privacy domain at " << ty->location () << '.';
        return E_TYPE;
    }

    assert (dynamic_cast<SymbolDomain*>(s) != 0);
    ty->setCachedType (static_cast<SymbolDomain*>(s)->securityType ());
    return OK;
}

void TreeNodeSecTypeF::setTypeContext (TypeContext& cxt) const {
    cxt.setContextSecType (cachedType ());
}

/*******************************************************************************
  TreeNodeDataTypeF
*******************************************************************************/

TypeChecker::Status TreeNodeDataTypeF::accept(TypeChecker&) {
    return TypeChecker::OK;
}

void TreeNodeDataTypeF::setTypeContext (TypeContext& cxt) const {
    cxt.setContextDataType (dataType ());
}

/*******************************************************************************
  TreeNodeDimTypeF
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeDimTypeF* ty) {
    return ty->accept (*this);
}

void TreeNodeDimTypeF::setTypeContext (TypeContext& cxt) const {
    cxt.setContextDimType (cachedType ());
}

/*******************************************************************************
  TreeNodeDimTypeConstF
*******************************************************************************/

TypeChecker::Status TreeNodeDimTypeConstF::accept (TypeChecker&) {
    return TypeChecker::OK;
}

/*******************************************************************************
  TreeNodeDimTypeVarF
*******************************************************************************/

TypeChecker::Status TreeNodeDimTypeVarF::accept(TypeChecker & tyChecker) {
    return tyChecker.visit (this);
}

TypeChecker::Status TypeChecker::visit (TreeNodeDimTypeVarF * ty) {
    if (ty->cachedType () != ~ SecrecDimType (0)) {
        return OK;
    }

    Symbol* s = 0;

    if ((s = findIdentifier (ty->identifier ())) == 0)
        return E_TYPE;

    if (s->symbolType () != Symbol::DIMENSIONALITY) {
        m_log.fatalInProc (ty) << "Invalid dimensionality specifier at " << ty->identifier ()->location () << ".";
        return E_TYPE;
    }

    ty->setCachedType (static_cast<SymbolDimensionality*>(s)->dimType ());
    return OK;
}

/*******************************************************************************
  TreeNodeType
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeType * _ty) {
    if (_ty->m_cachedType != 0)
        return OK;

    if (_ty->type () == NODE_TYPETYPE) {
        assert (dynamic_cast<TreeNodeTypeType*>(_ty) != 0);
        TreeNodeTypeType* tyNode = static_cast<TreeNodeTypeType*>(_ty);
        TreeNodeSecTypeF* secTyNode = tyNode->secType ();
        Status status = visit(secTyNode);
        if (status != OK)
            return status;

        status = visit (tyNode->dimType ());
        if (status != OK)
            return status;

        SecurityType* secType = secTyNode->cachedType ();
        if (secType->isPublic ()) {
            switch (tyNode->dataType ()->dataType ()) {
            case DATATYPE_XOR_UINT8:
            case DATATYPE_XOR_UINT16:
            case DATATYPE_XOR_UINT32:
            case DATATYPE_XOR_UINT64:
                m_log.fatalInProc(_ty) << "XOR types do not have public representation at "
                                       << _ty->location () << '.';
                return E_TYPE;
            default:
                break;
            }
        }

        tyNode->m_cachedType = TypeNonVoid::get (m_context,
            secType, tyNode->dataType ()->dataType (),
                     tyNode->dimType ()->cachedType ());
    }
    else {
        assert (dynamic_cast<TreeNodeTypeVoid*>(_ty) != 0);
        _ty->m_cachedType = TypeVoid::get (getContext ());
    }

    return OK;
}

} // namespace SecreC
