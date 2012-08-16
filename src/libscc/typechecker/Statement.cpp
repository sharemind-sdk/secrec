/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "symboltable.h"
#include "treenode.h"
#include "typechecker.h"

#include <boost/range.hpp>

namespace SecreC {

/*******************************************************************************
  TypeChecker
*******************************************************************************/

TypeChecker::Status TypeChecker::checkVarInit(TypeNonVoid * ty,
                                              TreeNodeVarInit * varInit)
{
    SecrecDimType n = 0;

    if (m_st->findFromCurrentScope (varInit->variableName ()) != 0) {
        m_log.fatal () << "Redeclaration of variable at " << varInit->location () << ".";
        return E_TYPE;
    }

    BOOST_FOREACH (TreeNode* node, varInit->shape ()->children ()) {
        assert (dynamic_cast<TreeNodeExpr*>(node) != 0);
        TreeNodeExpr* e = static_cast<TreeNodeExpr*>(node);
        e->setContextIndexType (getContext ());
        Status s = visitExpr (e);
        if (s != OK)
            return s;
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        if (! e->resultType ()->isPublicIntScalar ()) {
            m_log.fatal() << "Expecting public unsigned integer scalar at "
                          << e->location() << ".";
            return E_TYPE;
        }

        ++ n;
    }

    if (n > 0 && n != ty->secrecDimType()) {
        m_log.fatal() << "Mismatching number of shape components in declaration at "
                      << varInit->location() << ".";
        return E_TYPE;
    }

    if (varInit->rightHandSide () != 0) {
        TreeNodeExpr *e = varInit->rightHandSide ();
        e->setContext (ty);
        Status s = visitExpr(e);
        if (s != OK)
            return s;
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        e = classifyIfNeeded (e, ty->secrecSecType ());
        if (! ty->canAssign (e->resultType ())) {
            m_log.fatal () << "Illegal assignment at " << varInit->location () << ".";
            m_log.fatal () << "Got " << *e->resultType () << " expected " << *ty << ".";
            return E_TYPE;
        }
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtDecl
*******************************************************************************/

// Note that declarations are type checked very lazility, checks of
// individual variable initializations will be requested by the code
// generator (see CodeGen::cgVarInit).
TypeChecker::Status TypeChecker::visit(TreeNodeStmtDecl * decl) {
    typedef DataTypeBasic DTB;
    typedef TypeNonVoid TNV;

    if (decl->m_type != 0)
        return OK;

    TreeNodeType *type = decl->varType ();
    Status s = visit(type);
    if (s != OK)
        return s;

    assert (!type->secrecType()->isVoid());
    assert (dynamic_cast<TNV*>(type->secrecType()) != 0);
    TNV* justType = static_cast<TNV*>(type->secrecType());

    assert(justType->kind() == TNV::BASIC);
    assert(dynamic_cast<DTB*>(justType->dataType()) != 0);
    DTB* dataType = static_cast<DTB*>(justType->dataType());

    decl->m_type = TypeNonVoid::get (getContext (),
        DataTypeVar::get (getContext (), dataType));

    if (decl->procParam ()) {
        // some sanity checks that parser did its work correctly.
        assert (boost::size (decl->initializers ()) == 1);
        assert (decl->initializer () != 0);
        assert (decl->shape ()->children ().empty ());
        assert (decl->initializer ()->rightHandSide () == 0);
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtPrint
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeStmtPrint * stmt) {
    BOOST_FOREACH (TreeNode* node, stmt->expressions ()) {
        assert (dynamic_cast<TreeNodeExpr*>(node) != 0);
        TreeNodeExpr* e = static_cast<TreeNodeExpr*>(node);
        e->setContextSecType (PublicSecType::get (getContext ()));
        e->setContextDimType (0);

        Status s = visitExpr(e);
        e->instantiateDataType (getContext ());
        if (s != OK)
            return s;

        if (checkAndLogIfVoid(e))
            return E_TYPE;

        bool isLegalType = true;
        if (  e->resultType()->secrecSecType()->isPrivate ()  ||
            ! e->resultType()->isScalar ()) {
            isLegalType = false;
        }

        SecrecDataType dType = e->resultType ()->secrecDataType ();
        if (  dType != DATATYPE_STRING && dType != DATATYPE_BOOL && ! isNumericDataType (dType)) {
            isLegalType = false;
        }

        if (! isLegalType) {
            m_log.fatal () << "Invalid argument to \"print\" statement."
                           << "Got " << *e->resultType() << " at " << stmt->location() << ". "
                           << "Expected public scalar or string.";
            return E_TYPE;
        }
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtReturn
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeStmtReturn * stmt) {
    TypeNonVoid* procType = stmt->containingProcedure ()->procedureType ();
    TreeNodeExpr *e = stmt->expression ();
    if (e == 0) {
        if (procType->kind () == TypeNonVoid::PROCEDURE) {
            m_log.fatal() << "Cannot return from non-void function without value "
                             "at " << stmt->location() << ".";
            return E_TYPE;
        }
    }
    else {

        if (procType->kind () == TypeNonVoid::PROCEDUREVOID) {
            m_log.fatal () << "Cannot return value from void function at"
                           << stmt->location() << ".";
            return E_TYPE;
        }

        e->setContext (procType);
        Status s = visitExpr(e);
        if (s != OK)
            return s;
        e = classifyIfNeeded (e, procType->secrecSecType ());
        if (!procType->canAssign (e->resultType ()) ||
             procType->secrecDimType () != e->resultType ()->secrecDimType ())
        {
            m_log.fatal () << "Cannot return value of type " << *e->resultType ()
                           << " from function with type "
                           << *procType << " at "
                           << stmt->location () << ".";
            return E_TYPE;
        }
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtPush
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeStmtPush * stmt) {
    TreeNodeExpr* e = stmt->expression ();
    Status s = visitExpr (e);
    if (s != OK)
        return s;

    e->instantiateDataType (getContext ());
    return OK;
}


} // namespace SecreC
