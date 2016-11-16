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

#include "TypeChecker.h"

#include "Log.h"
#include "SecurityType.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "Types.h"
#include "typechecker/Templates.h"

#include <boost/range.hpp>


namespace SecreC {

const TypeBasic* upperTypeBasic (const TypeBasic* a, const TypeBasic* b) {
    const SecurityType* secType = upperSecType (a->secrecSecType (), b->secrecSecType ());
    SecrecDimType dimType = upperDimType (a->secrecDimType (), b->secrecDimType ());
    const DataType* dataType = upperDataType (a, b);

    if (secType == nullptr || dimType == (~ SecrecDimType(0)) || dataType == nullptr)
        return nullptr;

    return TypeBasic::get (secType, dataType, dimType);
}

/*******************************************************************************
  TypeChecker
*******************************************************************************/

TypeChecker::TypeChecker (OperatorTable& ops, SymbolTable& st, CompileLog& log, Context& cxt)
    : m_operators (&ops)
    , m_st (&st)
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
                                             const SecurityType * need)
{
    if (need == nullptr)
        return child;

    const SecurityType * const haveSecType = child->resultType()->secrecSecType();
    assert(!(need->isPrivate() && haveSecType->isPrivate()) || need == haveSecType);

    if (need->isPublic() || haveSecType->isPrivate())
        return child;

    TreeNode * const parent = child->parent();
    const DataType* destDType = child->resultType()->secrecDataType();
    if (child->haveContextDataType() &&
        dtypeDeclassify (need, child->contextDataType()) == destDType)
    {
        destDType = child->contextDataType();
    }

    const SecrecDimType dimDType = child->resultType()->secrecDimType();
    const TypeBasic * const newTy = TypeBasic::get(need, destDType, dimDType);
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
    child->setContextSecType(PublicSecType::get());
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
        e->setContextSecType (PublicSecType::get ());
        e->setContextDataType (DataTypeBuiltinPrimitive::get (DATATYPE_BOOL));
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
            e->setContextIndexType ();
            TCGUARD (visitExpr(e));

            if (checkAndLogIfVoid(e))
                return E_TYPE;

            const auto eTy = static_cast<const TypeNonVoid*>(e->resultType());

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

bool TypeChecker::canPrintValue (const Type* ty) {
    if (ty->isVoid ())
        return false;

    if (ty->secrecSecType()->isPrivate ())
        return false;

    if (! ty->isScalar ())
        return false;

    const DataType* dType = ty->secrecDataType ();
    if (! dType->isString () &&
        ! dType->isBool () &&
        ! isNumericDataType (dType))
    {
        return false;
    }

    return true;
}

} // namespace SecreC
