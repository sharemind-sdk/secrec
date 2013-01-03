/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "log.h"
#include "misc.h"
#include "symboltable.h"
#include "treenode.h"
#include "typechecker.h"

#include <boost/range.hpp>
#include <boost/foreach.hpp>

namespace SecreC {

/*******************************************************************************
  TypeChecker
*******************************************************************************/

TypeChecker::Status TypeChecker::checkVarInit(TypeNonVoid * ty,
                                              TreeNodeVarInit * varInit)
{
    SecrecDimType shapeExpressions = 0;

    if (m_st->findFromCurrentScope (varInit->variableName ()) != 0) {
        m_log.fatalInProc(varInit) << "Redeclaration of variable at "
                                   << varInit->location () << '.';
        return E_TYPE;
    }

    BOOST_FOREACH (TreeNode * const node, varInit->shape()->children()) {
        assert(dynamic_cast<TreeNodeExpr *>(node) != 0);
        TreeNodeExpr * const e = static_cast<TreeNodeExpr *>(node);
        e->setContextIndexType(getContext());
        Status s = visitExpr(e);
        if (s != OK)
            return s;
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        if (!e->resultType ()->isPublicIntScalar()) {
            m_log.fatalInProc(varInit) << "Expecting public unsigned integer scalar at "
                                       << e->location() << '.';
            return E_TYPE;
        }

        ++shapeExpressions;
    }

    if (shapeExpressions > 0 && shapeExpressions != ty->secrecDimType()) {
        m_log.fatalInProc(varInit) << "Mismatching number of shape components "
                                      "in declaration at "
                                   << varInit->location() << '.';
        return E_TYPE;
    }

    if (varInit->rightHandSide() != 0) {
        TreeNodeExpr * e = varInit->rightHandSide();
        e->setContext(ty);
        Status s = visitExpr(e);
        if (s != OK)
            return s;
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        if (!ty->canAssign(e->resultType())) {
            m_log.fatalInProc(varInit) << "Illegal assignment at "
                                       << varInit->location() << '.';
            m_log.fatal() << "Got " << (*e->resultType())
                          << " expected " << *ty << '.';
            return E_TYPE;
        }

        classifyIfNeeded(e, ty->secrecSecType());
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtIf
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeStmtIf * stmt) {
    TreeNodeExpr *e = stmt->conditional ();
    if (checkPublicBooleanScalar (e) != OK) {
        m_log.fatalInProc(stmt) << "Conditional expression in if statement must be of "
                        "type public bool in " << e->location ();
        return E_TYPE;
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtWhile
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeStmtWhile * stmt) {
    TreeNodeExpr *e = stmt->conditional ();
    if (checkPublicBooleanScalar (e) != OK) {
        m_log.fatalInProc(stmt) << "Conditional expression in while statement must be of "
                       "type public bool in " << e->location();
        return E_TYPE;
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtDoWhile
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeStmtDoWhile * stmt) {
    TreeNodeExpr *e = stmt->conditional ();
    if (checkPublicBooleanScalar (e) != OK) {
        m_log.fatalInProc(stmt) << "Conditional expression in do-while statement must be of "
                       "type public bool in " << e->location();
        return E_TYPE;
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
        assert (decl->initializers ().size () == 1);
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
    BOOST_FOREACH (TreeNodeExpr& e, stmt->expressions ()) {
        e.setContextSecType (PublicSecType::get (getContext ()));
        e.setContextDimType (0);

        Status s = visitExpr(&e);
        if (s != OK)
            return s;

        e.instantiateDataType (getContext ());

        if (checkAndLogIfVoid(&e))
            return E_TYPE;

        bool isLegalType = true;
        if (  e.resultType()->secrecSecType()->isPrivate ()  ||
            ! e.resultType()->isScalar ()) {
            isLegalType = false;
        }

        SecrecDataType dType = e.resultType ()->secrecDataType ();
        if (  dType != DATATYPE_STRING && dType != DATATYPE_BOOL && ! isNumericDataType (dType)) {
            isLegalType = false;
        }

        if (! isLegalType) {
            m_log.fatalInProc(stmt) << "Invalid argument to \"print\" statement."
                           << "Got " << *e.resultType() << " at " << stmt->location() << ". "
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
    if (!stmt->hasExpression()) {
        if (procType->kind () == TypeNonVoid::PROCEDURE) {
            m_log.fatalInProc(stmt) << "Cannot return from non-void procedure "
                                       "without value at " << stmt->location() << '.';
            return E_TYPE;
        }
    } else {
        TreeNodeExpr * e = stmt->expression ();

        if (procType->kind () == TypeNonVoid::PROCEDUREVOID) {
            m_log.fatalInProc(stmt) << "Cannot return value from void procedure at"
                                    << stmt->location() << '.';
            return E_TYPE;
        }

        e->setContext (procType);
        Status s = visitExpr(e);
        if (s != OK)
            return s;
        e = classifyIfNeeded(e, procType->secrecSecType());
        if (!procType->canAssign (e->resultType ()) ||
             procType->secrecDimType () != e->resultType ()->secrecDimType ())
        {
            m_log.fatalInProc(stmt) << "Cannot return value of type "
                                    << *e->resultType ()
                                    << " from procedure with type "
                                    << *procType << " at "
                                    << stmt->location () << '.';
            return E_TYPE;
        }
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtSyscall
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeStmtSyscall * stmt) {
    TreeNodeExprString* e = stmt->name ();
    Status s = visit (e);
    if (s != OK)
        return s;

    bool hasReturn = false;

    BOOST_FOREACH (TreeNodeSyscallParam& param, stmt->params ()) {
        TreeNodeExpr* e = param.expression ();
        if (param.type () != NODE_PUSH) {
            e->setContextSecType (PublicSecType::get (getContext ()));
        }

        s = visitExpr (e);
        if (s != OK)
            return s;

        e->instantiateDataType (getContext ());

        if (param.type () != NODE_PUSH) {
            if (e->resultType ()->secrecSecType ()->isPrivate ()) {
                m_log.fatalInProc(stmt) << "Passing reference to private value at "
                                        << param.location () << '.';
                return E_TYPE;
            }
        }

        if (param.type () == NODE_SYSCALL_RETURN) {
            if (hasReturn) {
                m_log.fatalInProc (stmt) << "Multiple return values specified for syscall at "
                                         << stmt->location () << ".";
                return E_TYPE;
            }

            hasReturn = true;
        }
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtAssert
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeStmtAssert * stmt) {
    TreeNodeExpr* e = stmt->expression ();
    Status s = visitExpr(e);
    if (s != OK)
        return s;

    if (checkPublicBooleanScalar (e) != OK) {
        assert(e->haveResultType());
        m_log.fatalInProc(stmt) << "Invalid expression of type "
                                << Type::PrettyPrint (e->resultType())
                                << " given for assert statement at " << e->location() << '.';
        return E_TYPE;
    }

    return OK;
}

} // namespace SecreC
