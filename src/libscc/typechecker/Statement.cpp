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

#include "Log.h"
#include "Misc.h"
#include "SecurityType.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Types.h"

#include <boost/range.hpp>

namespace SecreC {

/*******************************************************************************
  TypeChecker
*******************************************************************************/

TypeChecker::Status TypeChecker::checkVarInit(const TypeNonVoid * ty,
                                              TreeNodeVarInit * varInit)
{
    SecrecDimType shapeExpressions = 0;

    if (m_st->findFromCurrentScope (SYM_SYMBOL, varInit->variableName ()).size () > 0) {
        m_log.fatalInProc(varInit) << "Redeclaration of variable at "
                                   << varInit->location () << '.';
        return E_TYPE;
    }

    for (TreeNodeExpr& e : varInit->shape()) {
        e.setContextIndexType();
        TCGUARD (visitExpr(&e));
        if (checkAndLogIfVoid(&e))
            return E_TYPE;
        if (!e.resultType ()->isPublicUIntScalar()) {
            m_log.fatalInProc(varInit) << "Expecting \'uint\' at "
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

    if (varInit->rightHandSide() != nullptr) {
        TreeNodeExpr * e = varInit->rightHandSide();
        e->setContext(ty);
        TCGUARD (visitExpr(e));
        if (checkAndLogIfVoid(e))
            return E_TYPE;
        if (!static_cast<const TypeNonVoid*>(e->resultType())->latticeLEQ (ty)) {
            m_log.fatalInProc(varInit) << "Illegal assignment at "
                                       << varInit->location() << '.';
            m_log.fatal() << "Got " << (*e->resultType())
                          << " expecting " << *ty << '.';
            return E_TYPE;
        }

        classifyIfNeeded(e, ty->secrecSecType());
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtIf
*******************************************************************************/

TypeChecker::Status TypeChecker::visitStmtIf(TreeNodeStmtIf * stmt) {
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

TypeChecker::Status TypeChecker::visitStmtWhile(TreeNodeStmtWhile * stmt) {
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

TypeChecker::Status TypeChecker::visitStmtDoWhile(TreeNodeStmtDoWhile * stmt) {
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
TypeChecker::Status TypeChecker::visitStmtDecl(TreeNodeStmtDecl * decl) {
    if (decl->haveResultType ())
        return OK;

    TreeNodeType *type = decl->varType ();
    TCGUARD (visitType(type));
    assert (! type->secrecType()->isVoid());
    assert (dynamic_cast<const TypeNonVoid*>(type->secrecType()) != nullptr);
    decl->setResultType (static_cast<const TypeNonVoid*>(type->secrecType()));

    if (decl->procParam ()) {
        // some sanity checks that parser did its work correctly.
        assert (decl->initializers ().size () == 1);
        assert (decl->initializer () != nullptr);
        assert (decl->shape ().empty ());
        assert (decl->initializer ()->rightHandSide () == nullptr);
    }

    return OK;
}

/*******************************************************************************
  TreeNodeStmtPrint
*******************************************************************************/

TypeChecker::Status TypeChecker::visitStmtPrint(TreeNodeStmtPrint * stmt) {
    for (TreeNodeExpr& e : stmt->expressions ()) {
        e.setContextSecType (PublicSecType::get ());
        e.setContextDimType (0);

        TCGUARD (visitExpr(&e));

        e.instantiateDataType ();

        if (checkAndLogIfVoid(&e))
            return E_TYPE;

        if (! canPrintValue (e.resultType ())) {
            m_log.fatalInProc(stmt) << "Invalid argument to \"print\" statement. "
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

TypeChecker::Status TypeChecker::visitStmtReturn(TreeNodeStmtReturn * stmt) {
    const TypeProc* procType = stmt->containingProcedure ()->procedureType ();
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
        const auto resultType = static_cast<const TypeBasic*>(e->resultType ());
        const auto returnType = static_cast<const TypeBasic*>(procType->returnType ());
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

TypeChecker::Status TypeChecker::visitStmtSyscall(TreeNodeStmtSyscall * stmt) {
    TreeNodeExprString* e = stmt->name ();
    TCGUARD (visitExprString (e));

    if (! e->isConstant ()) {
        m_log.fatalInProc (stmt) << "Syscall name at " << e->location ()
                              << " is not a static string.";
        return E_TYPE;
    }

    bool hasReturn = false;

    for (TreeNodeSyscallParam& param : stmt->params ()) {
        TreeNodeExpr* e = param.expression ();
        if (param.type () != NODE_PUSH) {
            e->setContextSecType (PublicSecType::get ());
        }

        TCGUARD (visitExpr (e));

        e->instantiateDataType ();

        if (param.type () == NODE_PUSH && e->type () == NODE_LITE_STRING) {
            if (static_cast<TreeNodeExprString*>(e)->isConstant ()) {
                m_log.fatalInProc(stmt) << "Passing string literal to syscall via stack at "
                                        << param.location () << ". "
                                        << "Try via __cref instead.";
                return E_TYPE;
            }
        }

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

TypeChecker::Status TypeChecker::visitStmtAssert(TreeNodeStmtAssert * stmt) {
    TreeNodeExpr* e = stmt->expression ();
    if (checkPublicBooleanScalar (e) != OK) {
        if (e->haveResultType ()) {
            m_log.fatalInProc(stmt) << "Invalid expression of type "
                                    << PrettyPrint (e->resultType())
                                    << " passed to assert statement at " << e->location() << '.'
                                    << " Expecting 'bool'.";
        }

        return E_TYPE;
    }

    return OK;
}

} // namespace SecreC
