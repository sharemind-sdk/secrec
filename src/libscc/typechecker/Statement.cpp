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

    if (m_st->findFromCurrentScope (SYM_SYMBOL, varInit->variableName ()).size () > 1) {
        m_log.fatalInProc(varInit) << "Redeclaration of variable at "
                                   << varInit->location () << '.';
        return E_TYPE;
    }

    BOOST_FOREACH (TreeNodeExpr& e, varInit->shape()) {
        e.setContextIndexType(getContext());
        TCGUARD (visitExpr(&e));
        if (checkAndLogIfVoid(&e))
            return E_TYPE;
        if (!e.resultType ()->isPublicUIntScalar()) {
            m_log.fatalInProc(varInit) << "Expecting public unsigned integer scalar at "
                                       << e.location() << '.';
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
        TCGUARD (visitExpr(e));
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        if (!static_cast<TypeNonVoid*>(e->resultType())->latticeLEQ (ty)) {
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
    if (decl->haveResultType ())
        return OK;

    TreeNodeType *type = decl->varType ();
    TCGUARD (visit(type));
    assert (! type->secrecType()->isVoid());
    assert (dynamic_cast<TypeNonVoid*>(type->secrecType()) != 0);
    TypeNonVoid* justType = static_cast<TypeNonVoid*>(type->secrecType());
    decl->setResultType (justType);

    if (decl->procParam ()) {
        // some sanity checks that parser did its work correctly.
        assert (decl->initializers ().size () == 1);
        assert (decl->initializer () != 0);
        assert (decl->shape ().empty ());
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

        TCGUARD (visitExpr(&e));

        e.instantiateDataType (getContext ());

        if (checkAndLogIfVoid(&e))
            return E_TYPE;

        if (! canPrintValue (e.resultType ())) {
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
    TypeProc* procType = stmt->containingProcedure ()->procedureType ();
    if (!stmt->hasExpression()) { // stmt = return;
        if (! procType->returnType ()->isVoid ()) {
            m_log.fatalInProc(stmt) << "Cannot return from non-void procedure "
                                       "without value at " << stmt->location() << '.';
            return E_TYPE;
        }
    }
    else { // stmt = return e;
        TreeNodeExpr * e = stmt->expression ();

        if (procType->returnType ()->isVoid ()) {
            m_log.fatalInProc(stmt) << "Cannot return value from void procedure at"
                                    << stmt->location() << '.';
            return E_TYPE;
        }

        e->setContext (procType);
        TCGUARD (visitExpr(e));
        e = classifyIfNeeded(e, procType->secrecSecType());
        TypeBasic* resultType = static_cast<TypeBasic*>(e->resultType ());
        TypeBasic* returnType = static_cast<TypeBasic*>(procType->returnType ());
        if (! resultType->latticeLEQ (returnType) ||
              resultType->secrecDimType () != returnType->secrecDimType ())
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
    TCGUARD (visit (e));

    if (! e->isConstant ()) {
        m_log.fatalInProc (stmt) << "Syscall name at " << e->location ()
                              << " is not a static string.";
        return E_TYPE;
    }

    bool hasReturn = false;

    BOOST_FOREACH (TreeNodeSyscallParam& param, stmt->params ()) {
        TreeNodeExpr* e = param.expression ();
        if (param.type () != NODE_PUSH) {
            e->setContextSecType (PublicSecType::get (getContext ()));
        }

        TCGUARD (visitExpr (e));

        e->instantiateDataType (getContext ());

        if (param.type () != NODE_PUSH) {
            if (e->resultType ()->secrecSecType ()->isPrivate ()) {
                m_log.fatalInProc(stmt) << "Passing reference to a private value at "
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
    if (checkPublicBooleanScalar (e) != OK) {
        if (e->haveResultType ()) {
            m_log.fatalInProc(stmt) << "Invalid expression of type "
                                    << Type::PrettyPrint (e->resultType())
                                    << " passed to assert statement at " << e->location() << '.'
                                    << " Expecting public boolean scalar.";
        }

        return E_TYPE;
    }

    return OK;
}

} // namespace SecreC
