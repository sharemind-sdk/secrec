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

#include "CastTemplateChecker.h"

#include "Log.h"
#include "TreeNode.h"
#include "TypeChecker.h"


namespace SecreC {

/*******************************************************************************
  CastTemplateVarChecker
*******************************************************************************/

void CastTemplateVarChecker::badType (TreeNode* t) {
    m_log.fatal () << "Cast definition template " << thing () << " at "
                   << t->location() << " is not a vector or scalar.";
}

bool CastTemplateVarChecker::visitQuantifier (TreeNodeQuantifier* q) {
    sharemind::StringView const name = q->typeVariable()->value();
    auto it = m_vars.find (name);
    if (it != m_vars.end ()) {
        m_log.fatal () << "Redeclaration of a type variable \'" << name << '\''
                       << " at " << q->location () << '.';
        return false;
    }

    if (quantifierKind (*q) == TA_DIM) {
        m_log.fatal() << "Cast definition template has dimension variable at "
                      << q->location() << ".";
        return false;
    }

    if (quantifierKind (*q) == TA_SEC) {
        if (m_seenDomainVar) {
            m_log.fatal () << "Cast definition template has more than one domain variable at "
                           << q->location() << ".";
            return false;
        }
        m_seenDomainVar = true;
    }

    m_vars.insert (it, std::make_pair (name, TemplateTypeVar (q->typeVariable (), quantifierKind (*q))));

    return true;
}

bool CastTemplateVarChecker::visitType (TreeNodeType* t) {
    assert (t != nullptr);

    if (!t->isNonVoid ()) {
        m_log.fatal () << "Cast definition template "
                       << thing () << " at " << t->location ()
                       << " has void type.";
        return false;
    }

    return visitSecTypeF (t->secType (), TA_SEC) &&
           visitDataTypeF (t->dataType (), TA_DATA) &&
           visitDimTypeF (t->dimType (), TA_DIM);
}

bool CastTemplateVarChecker::visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t) {
    badType (t);
    return false;
}

bool CastTemplateVarChecker::visitDataTypeConstF (TreeNodeDataTypeConstF* t) {
    if (t->secrecDataType () == DATATYPE_STRING) {
        badType (t);
        return false;
    }

    return true;
}

bool CastTemplateVarChecker::visitDimTypeConstF (TreeNodeDimTypeConstF* t) {
    if (m_typeChecker.visitDimTypeConstF(t) != TypeChecker::OK) {
        return false;
    }

    if (t->cachedType () != 1u) {
        badType (t);
        return false;
    }
    return true;
}

} /* namespace SecreC */
