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
#include "Misc.h"
#include "SecurityType.h"
#include "TreeNode.h"
#include "TypeChecker.h"
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

OperatorTypeUnifier::OperatorTypeUnifier (const std::vector<const TypeBasic*>& argTypes,
                                          SymbolTable* st,
                                          SymbolTemplate* sym)
    : AbstractOperatorTypeUnifier {st, sym}
{
    m_securityType = argTypes.size () == 1u
        ? argTypes[0u]->secrecSecType ()
        : upperSecType (argTypes[0u]->secrecSecType (),
                        argTypes[1u]->secrecSecType ());

    m_domainVar = nullptr;

    for (TreeNodeQuantifier& quant : m_sym->decl ()-> quantifiers ()) {
        if (quant.isDomainQuantifier ()) {
            m_domainVar = static_cast<TreeNodeQuantifierDomain*> (&quant);
            bind (quant.typeVariable ()->value (), m_securityType);
            break;
        }
    }
}

bool OperatorTypeUnifier::bind (StringRef name, const TypeArgument& arg, const SecurityType* sec) {
    auto it = m_names.find (name);
    if (it != m_names.end ()) {
        TypeArgument& bound = it->second;
        if (! bound.isDataType ())
            return bound == arg;

        /*
         * In the case of data type variables, the final bound data
         * type is the least upper bound of the different data types
         * bound to the variable. LUB=top is an error. Note that we
         * don't keep the PDK of the bound data type, it is
         * m_securityType (if DataTypeUserPrimitive) or public because
         * m_securityType is the LUB of the domains of the provided
         * arguments.
         */

        OTUGUARD (arg.isDataType ());
        assert (sec != nullptr);
        const DataType* boundDT = bound.dataType ();
        const DataType* argDT = arg.dataType ();

        const SecurityType* boundSec = boundDT->isUserPrimitive ()
            ? m_securityType
            : PublicSecType::get ();

        const TypeBasic* boundTy = TypeBasic::get (boundSec, boundDT);
        const TypeBasic* argTy = TypeBasic::get (sec, argDT);
        const TypeBasic* upper = upperTypeBasic (boundTy, argTy);
        OTUGUARD (upper != nullptr);
        m_names.insert (it, std::make_pair (name, TypeArgument (upper->secrecDataType ())));
    }
    m_names.insert (it, std::make_pair (name, arg));
    return true;
}

/*******************************************************************************
  TreeNodeSecTypeF
*******************************************************************************/

bool OperatorTypeUnifier::visitSecTypeF (TreeNodeSecTypeF* t, const SecurityType* secType) {
    assert (secType != nullptr);
    assert (t != nullptr);

    if (t->isPublic ()) {
        return secType->isPublic ();
    }

    const StringRef name = t->identifier ()->value ();
    if (m_domainVar != nullptr && m_domainVar->typeVariable ()->value () == name) {
        // Domain variable, not domain
        return true;
    }

    SymbolDomain* s = m_st->find<SYM_DOMAIN>(name);
    assert (s != nullptr); // TemplateVarChecker checks that domains exists

    return secType->isPublic () || (s->securityType () == secType);
}

/*******************************************************************************
  TreeNodeDimTypeF
*******************************************************************************/

bool OperatorTypeUnifier::visitDimTypeConstF (TreeNodeDimTypeConstF* t, SecrecDimType dimType) {
    if (dimType == 0u)
        return true;

    SecrecDimType templateDim = t->cachedType ();
    if (dimType > templateDim && templateDim == 0u)
        return false;

    return true;
}

bool OperatorTypeUnifier::visitDimTypeZeroF(TreeNodeDimTypeZeroF*, SecrecDimType) {
    return true;
}

bool OperatorTypeUnifier::checkReturnDataType (TreeNodeType* retNodeTy,
                                               const DataType* dt)
{
    // Domain variable is bound differently, we can use
    // m_securityType here
    const SecurityType* sec = m_securityType;
    // Dimension variables are not allowed in operators
    assert (retNodeTy->dimType ()->type () == NODE_DIMTYPE_CONST_F ||
            retNodeTy->dimType ()->type () == NODE_DIMTYPE_ZERO_F);
    SecrecDimType dim = retNodeTy->dimType ()->cachedType ();
    const TypeBasic* type = TypeBasic::get (sec, dt, dim);
    OTUGUARD (visitType (retNodeTy, type));
    return true;
}

bool OperatorTypeUnifier::checkDomainQuantifier () {
    OTUGUARD (m_securityType != nullptr);
    OTUGUARD (checkKind ());
    return true;
}

bool OperatorTypeUnifier::checkSecLUB () {
    // This is the equivalent of the sec(lub(exprA, exprB)) ==
    // sec(definition return type) check of non-templated
    // definitions.

    // Note that we compute the LUB of the expression's operands to
    // bind the template domain variable. It's also required that
    // LUB(template operands)==sec(template return type). So when a
    // domain variable exists, it's definitely in the return type and
    // the condition becomes sec(lub(exprA, exprB)) == sec(lub(exprA,
    // exprB)) so there's nothing to check.
    if (m_domainVar != nullptr)
        return true;

    TreeNodeType* retTy = m_sym->decl ()->body ()->returnType ();

    if (m_securityType->isPublic ())
        return retTy->secType ()->isPublic ();

    StringRef templPD = retTy->secType ()->identifier ()->value ();

    return templPD == static_cast<const PrivateSecType*> (m_securityType)->name ();
}

} // namespace SecreC
