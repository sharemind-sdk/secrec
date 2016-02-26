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

#include "OperatorTemplateChecker.h"

#include "Log.h"
#include "TreeNode.h"
#include "DataType.h"


namespace SecreC {

/*******************************************************************************
  OperatorTemplateVarChecker
*******************************************************************************/

const char* OperatorTemplateVarChecker::thing () {
    return m_pos == ArgReturn
        ? "return value"
        : "operand";
}

void OperatorTemplateVarChecker::badType (TreeNode* t) {
    m_log.fatal () << "Operator definition template " << thing () << " at "
                   << t->location() << " is not a vector or scalar.";
}

bool OperatorTemplateVarChecker::visit (TreeNodeIdentifier* id, TypeArgumentKind kind) {
    const StringRef name = id->value ();
    const auto it = m_vars.find (name);

    if (it != m_vars.end ()) {
        TemplateTypeVar& tv = it->second;
        if (tv.kind != kind) {
            m_log.fatal () << "Unexpected " << kindAsString (kind)
                           << " type variable \'" << name
                           << "\' at " << id->location () << ". "
                           << "Expecting " <<  kindAsString (tv.kind)
                           << " type variable.";
            return false;
        }

        tv.bound = true;
        tv.pos = m_pos;
    }

    return true;
}

bool OperatorTemplateVarChecker::visitQuantifier (TreeNodeQuantifier* q) {
    const StringRef name = q->typeVariable ()->value ();
    auto it = m_vars.find (name);
    if (it != m_vars.end ()) {
        m_log.fatal () << "Redeclaration of a type variable \'" << name << '\''
                       << " at " << q->location () << '.';
        return false;
    }

    if (quantifierKind (*q) == TA_DIM) {
        m_log.fatal() << "Operator definition template has dimension variable at "
                      << q->location() << ".";
        return false;
    }

    if (quantifierKind (*q) == TA_SEC) {
        if (m_seenDomainVar) {
            m_log.fatal () << "Operator definition template has more than one domain variable at "
                           << q->location() << ".";
            return false;
        }
        m_seenDomainVar = true;
    }

    m_vars.insert (it, std::make_pair (name, TemplateTypeVar (q->typeVariable (), quantifierKind (*q))));

    return true;
}

bool OperatorTemplateVarChecker::visitType (TreeNodeType* t) {
    assert (t != nullptr);

    if (!t->isNonVoid ()) {
        m_log.fatal () << "Operator definition template "
                       << thing () << " at " << t->location ()
                       << " has void type.";
        return false;
    }

    return visitSecTypeF (t->secType (), TA_SEC) &&
           visitDataTypeF (t->dataType (), TA_DATA) &&
           visitDimTypeF (t->dimType (), TA_DIM);
}

bool OperatorTemplateVarChecker::visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t) {
    badType (t);
    return false;
}

bool OperatorTemplateVarChecker::visitDataTypeConstF (TreeNodeDataTypeConstF* t) {
    if (t->secrecDataType () == DATATYPE_STRING) {
        badType (t);
        return false;
    }

    return true;
}

bool OperatorTemplateVarChecker::visitDimTypeConstF (TreeNodeDimTypeConstF* t) {
    if (t->cachedType () != 0u && t->cachedType () != 1u) {
        badType (t);
        return false;
    }
    return true;
}

namespace {

struct Domain {
    bool isPublic;
    StringRef name;
};

bool operator == (Domain a, Domain b) {
    if (a.isPublic != b.isPublic)
        return false;

    if (!a.isPublic && a.name != b.name)
        return false;

    return true;
}

Domain getDomain (TreeNodeType* t) {
    Domain res;

    if (t->secType ()->isPublic ()) {
        res.isPublic = true;
    } else {
        res.isPublic = false;
        res.name = t->secType ()->identifier ()->value ();
    }

    return res;
}

bool join (Domain a, Domain b, Domain& res) {
    // Domain
    if (a.isPublic) {
        if (b.isPublic) {
            res.isPublic = true;
        }
        else {
            res.isPublic = false;
            res.name = b.name;
        }
    }
    else {
        res.isPublic = false;
        if (b.isPublic || a.name == b.name) {
            res.name = a.name;
        }
        else {
            return false;
        }
    }
    return true;
}

} // namespace {

bool OperatorTemplateVarChecker::checkLUB (TreeNodeTemplate* templ) {
    Domain ret = getDomain (templ->body ()->returnType ());
    std::vector<Domain> args;

    for (TreeNodeStmtDecl& decl : templ->body ()->params ()) {
        args.push_back (getDomain (decl.varType ()));
    }

    bool bad = false;

    if (args.size () == 1u && !(args[0u] == ret))
        bad = true;

    if (args.size () == 2u) {
        Domain lub;
        if (! join (args[0u], args[1u], lub))
            bad = true;
        else if (! (lub == ret))
            bad = true;
    }

    if (bad) {
        m_log.fatal () << "Security type of return type of operator definition template at "
                       << templ->location ()
                       << " is not the least upper bound of operand security types.";
        return false;
    }

    return true;
}

} // namespace SecreC
