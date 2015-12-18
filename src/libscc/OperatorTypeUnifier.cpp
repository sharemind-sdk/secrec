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

#include "OperatorTypeUnifier.h"

#include "DataType.h"
#include "SecurityType.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Types.h"
#include "Visitor.h"


#define OTUGUARD(expr) \
    do { \
        if (! (expr)) \
            return false; \
    } while (false)

namespace SecreC {

/*******************************************************************************
  OperatorTypeUnifier
*******************************************************************************/

OperatorTypeUnifier::OperatorTypeUnifier (Context& context,
                                          const std::vector<TypeBasic*>& argTypes,
                                          SymbolTemplate* sym)
    : m_sym (sym)
{
    TypeNonVoid* tynv = argTypes.size () == 1u
        ? argTypes[0u]
        : upperTypeNonVoid (context, argTypes[0u], argTypes[1u]);

    TypeBasic* ty = static_cast<TypeBasic*> (tynv);
    m_securityType = ty->secrecSecType ();
    m_dataType = ty->secrecDataType ();
    m_domainVar = nullptr;

    for (TreeNodeQuantifier& quant : m_sym->decl ()-> quantifiers ()) {
        if (quant.type () == NODE_TEMPLATE_QUANTIFIER_DOMAIN) {
            m_domainVar = static_cast<TreeNodeQuantifierDomain*> (&quant);
            m_typeArgs.push_back (TypeArgument (m_securityType));
        } else if (quant.type () == NODE_TEMPLATE_QUANTIFIER_DATA) {
            m_typeArgs.push_back (TypeArgument (m_dataType));
        } else {
            assert(false);
        }
    }
}

bool OperatorTypeUnifier::kindMatches () {

    // Check if m_securityType kind matches the template's domain kind
    if (m_domainVar != nullptr) {
        TreeNodeIdentifier* kind = m_domainVar->kind ();
        if (kind != nullptr) {
            const std::string& k = static_cast<PrivateSecType*> (m_securityType)->securityKind ()->name ();

            if (m_securityType->isPublic ())
                return false;

            if (std::string (kind->value ().data ()) != k)
                return false;
        }
    }

    return true;
}

/*******************************************************************************
  TreeNodeType
*******************************************************************************/

bool OperatorTypeUnifier::visitType (TreeNodeType* t, Type* type) {
    assert (t != nullptr);
    assert (type != nullptr);

    OTUGUARD (!type->isVoid ());

    const auto tnv = static_cast<TypeNonVoid*>(type);

    {
        assert (t->secType () != nullptr);
        assert (tnv->secrecSecType () != nullptr);

        TreeNodeSecTypeF* tsec = t->secType ();
        SecurityType* secType = tnv->secrecSecType ();

        if (tsec->isPublic () && ! secType->isPublic ())
            return false;

        if (! secType->isPublic ()) {
            StringRef expectedPD = static_cast<PrivateSecType*> (secType)->name ();
            StringRef templPD = tsec->identifier ()->value ();

            // OK when: template param pd is D or == expected pd

            if ((m_domainVar == nullptr || templPD != m_domainVar->typeVariable ()->value ()) &&
                templPD != expectedPD)
            {
                return false;
            }
        }
    }

    {
        TreeNodeDataTypeF* tdata = t->dataType ();
        DataType* dataType = tnv->secrecDataType ();

        assert (tdata->type () != NODE_DATATYPE_TEMPLATE_F);

        if (tdata->type () == NODE_DATATYPE_CONST_F) {
            assert (dynamic_cast<DataTypePrimitive*> (dataType) != nullptr);
            assert (dynamic_cast<TreeNodeDataTypeConstF*> (tdata) != nullptr);

            SecrecDataType a = static_cast<DataTypePrimitive*> (dataType)->secrecDataType ();
            SecrecDataType b = static_cast<TreeNodeDataTypeConstF*> (tdata)->secrecDataType ();

            if (! t->secType ()->isPublic () &&
                tnv->secrecSecType ()->isPublic ())
            {
                b = dtypeDeclassify (b);
            }

            if (a != b)
                return false;
        }
    }

    OTUGUARD (visitDimTypeF (t->dimType (), tnv->secrecDimType ()));

    return true;
}

/*******************************************************************************
  TreeNodeDimTypeF
*******************************************************************************/

bool OperatorTypeUnifier::visitDimTypeF (TreeNodeDimTypeF* t, SecrecDimType dimType) {
    return dispatchDimTypeF (*this, t, dimType);
}

bool OperatorTypeUnifier::visitDimTypeConstF (TreeNodeDimTypeConstF* t, SecrecDimType dimType) {
    if (dimType == 0u)
        return true;

    SecrecDimType templateDim = t->cachedType ();
    if (dimType > templateDim && templateDim == 0u)
        return false;

    return true;
}

bool OperatorTypeUnifier::visitDimTypeVarF (TreeNodeDimTypeVarF*, SecrecDimType) {
    // Dim variables are not allowed in operator definitions so this
    // should never happen.
    assert(false);
    return false;
}

bool OperatorTypeUnifier::checkSecLUB () {
    // This is the equivalent of the sec(lub(exprA, exprB)) ==
    // sec(definition return type) check of non-templated
    // definitions. Note that we compute the LUB of the expression's
    // operands to bind template variables. It's also required that
    // LUB(templaten operands)==sec(template return type). So when a
    // domain variable exists, it's definitely in the return type and
    // the condition becomes sec(lub(exprA, exprB)) == sec(lub(exprA,
    // exprB)) so there's nothing to check.

    if (m_domainVar != nullptr)
        return true;

    TreeNodeType* retTy = m_sym->decl ()->body ()->returnType ();

    if (m_securityType->isPublic ())
        return retTy->secType ()->isPublic ();

    StringRef templPD = retTy->secType ()->identifier ()->value ();

    if (templPD != static_cast<PrivateSecType*> (m_securityType)->name ())
        return false;

    return true;
}

void OperatorTypeUnifier::getTypeArguments (std::vector<TypeArgument>& params) {
    params = m_typeArgs;
}

} // namespace SecreC
