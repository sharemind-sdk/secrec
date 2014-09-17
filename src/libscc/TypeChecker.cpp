/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TypeChecker.h"

#include "Log.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "typechecker/Templates.h"
#include "SecurityType.h"

#include <boost/range.hpp>

namespace SecreC {

/*******************************************************************************
  TypeChecker
*******************************************************************************/

TypeChecker::TypeChecker (SymbolTable& st, CompileLog& log, Context& cxt)
    : m_st (&st)
    , m_log (log)
    , m_context (cxt)
    , m_instantiator (new TemplateInstantiator ())
{ }

TypeChecker::~TypeChecker () {
    delete m_instantiator;
}

bool TypeChecker::getForInstantiation (InstanceInfo& info) {
    return m_instantiator->getForInstantiation (info);
}

Symbol* TypeChecker::findIdentifier (SymbolCategory type, const TreeNodeIdentifier* id) const {
    Symbol* s = m_st->find (type, id->value ());
    if (s == nullptr) {
        m_log.fatalInProc(id) << "Identifier '" << id->value()
                              << "' at " << id->location()
                              << " not in scope.";
    }

    return s;
}

SymbolSymbol* TypeChecker::getSymbol (TreeNodeIdentifier *id) {
    SymbolSymbol *s = m_st->find<SYM_SYMBOL>(id->value ());
    if (s == nullptr) {
        m_log.fatalInProc(id) << "Undeclared identifier '" << id->value ()
                              << "' at " << id->location() << '.';
        return nullptr;
    }

    return s;
}

// Potentially replaces the child in parent list. Does not invalidate iterators.
TreeNodeExpr * TypeChecker::classifyIfNeeded(TreeNodeExpr * child,
                                             SecurityType * need)
{
    if (need == nullptr)
        return child;

    SecurityType * const haveSecType = child->resultType()->secrecSecType();
    assert(!(need->isPrivate() && haveSecType->isPrivate()) || need == haveSecType);

    if (need->isPublic() || haveSecType->isPrivate())
        return child;

    TreeNode * const parent = child->parent();
    DataType* destDType = child->resultType()->secrecDataType();
    if (child->haveContextDataType()) {
        if (dtypeDeclassify (getContext (), child->contextDataType()) == destDType)
            destDType = child->contextDataType();
    }

    const SecrecDimType dimDType = child->resultType()->secrecDimType();
    TypeBasic * const newTy = TypeBasic::get(getContext(), need, destDType, dimDType);
    const auto ec = new TreeNodeExprClassify(need, child->location());
    ec->appendChild(child);
    ec->resetParent(parent);
    ec->setResultType(newTy);
    for (TreeNode *& n : parent->children()) {
        if (n == child) {
            n = ec;
            break;
        }
    }

    // patch up context types just in case
    ec->setContext(child);
    child->setContextSecType(PublicSecType::get(getContext()));
    child->setContextDataType(destDType);
    child = ec;
    return child;
}

bool TypeChecker::checkAndLogIfVoid (TreeNodeExpr* e) {
    assert (e->haveResultType());
    if (e->resultType()->isVoid()) {
        m_log.fatalInProc(e) << "Subexpression has type void at "
                             << e->location() << '.';
        return true;
    }

    return false;
}

TypeChecker::Status TypeChecker::checkPublicBooleanScalar (TreeNodeExpr * e) {
    assert (e != nullptr);
    if (! e->haveResultType ()) {
        e->setContextSecType (PublicSecType::get (getContext ()));
        e->setContextDataType (DataTypePrimitive::get (getContext (), DATATYPE_BOOL));
        e->setContextDimType (0);

        if (visitExpr (e) != OK)
            return E_TYPE;

        if (!e->havePublicBoolType())
            return E_TYPE;
    }

    return OK;
}

TypeChecker::Status TypeChecker::checkIndices(TreeNode * node,
                                              SecrecDimType & destDim)
{
    assert (node->type() == NODE_SUBSCRIPT);
    destDim = 0;
    for (TreeNode* tNode : node->children ()) {
        switch (tNode->type()) {
        case NODE_INDEX_SLICE:
            ++ destDim;
        case NODE_INDEX_INT:
            break;
        default:
            assert (false && "Reached an index that isn't int or a slice.");
            return E_TYPE;
        }

        for (TreeNode* j : tNode->children ()) {
            if (j->type() == NODE_EXPR_NONE) {
                continue;
            }

            assert (dynamic_cast<TreeNodeExpr*>(j) != nullptr);
            TreeNodeExpr* e = static_cast<TreeNodeExpr*>(j);
            e->setContextIndexType (getContext ());
            TCGUARD (visitExpr(e));

            if (checkAndLogIfVoid(e))
                return E_TYPE;

            TypeNonVoid* eTy = static_cast<TypeNonVoid*>(e->resultType());

            if (! eTy->isPublicUIntScalar ()) {
                m_log.fatalInProc(node) << "Invalid type for index at "
                                        << e->location()
                                        << ". Expecting 'uint', got "
                                        << *eTy << '.';
                return E_TYPE;
            }
        }
    }

    return OK;
}

bool TypeChecker::canPrintValue (Type* ty) {
    if (ty->isVoid ())
        return false;

    if (ty->secrecSecType()->isPrivate ())
        return false;

    if (! ty->isScalar ())
        return false;

    DataType* dType = ty->secrecDataType ();
    if (! dType->isString () &&
        ! dType->isBool () &&
        ! isNumericDataType (dType))
    {
        return false;
    }

    return true;
}

} // namespace SecreC
