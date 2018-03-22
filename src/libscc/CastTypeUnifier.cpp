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

#include "CastTypeUnifier.h"

#include "DataType.h"
#include "Misc.h"
#include "SecurityType.h"
#include "TreeNode.h"
#include "Visitor.h"


namespace SecreC {

/*******************************************************************************
  CastTypeUnifier
*******************************************************************************/

CastTypeUnifier::CastTypeUnifier (const TypeBasic* argType,
                                  SymbolTable* st,
                                  SymbolTemplate* sym)
    : AbstractOperatorTypeUnifier {st, sym}
{
    m_securityType = argType->secrecSecType ();

    m_domainVar = nullptr;

    for (TreeNodeQuantifier& quant : m_sym->decl ()-> quantifiers ()) {
        if (quant.isDomainQuantifier ()) {
            m_domainVar = static_cast<TreeNodeQuantifierDomain*> (&quant);
            bind (quant.typeVariable ()->value (), m_securityType);
            break;
        }
    }
}

/*******************************************************************************
  TreeNodeSecTypeF
*******************************************************************************/

bool CastTypeUnifier::visitSecTypeF (TreeNodeSecTypeF* t, const SecurityType* secType) {
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

    return s->securityType () == secType;
}

/*******************************************************************************
  TreeNodeDimTypeF
*******************************************************************************/

bool CastTypeUnifier::visitDimTypeConstF (TreeNodeDimTypeConstF*, SecrecDimType) {
    return true;
}

} /* namespace SecreC */
