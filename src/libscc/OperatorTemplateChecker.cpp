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
    else if (kind == TA_DATA) {
        // Structs are not allowed
        badType (id);
        return false;
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

    TemplateTypeVar typeVar (q->typeVariable (), quantifierKind (*q));

    if (quantifierKind (*q) == TA_SEC) {
        if (m_seenDomainVar) {
            m_log.fatal () << "Operator definition template has more than one domain variable at "
                           << q->location() << ".";
            return false;
        }
        m_seenDomainVar = true;
    } else if (quantifierKind (*q) == TA_DATA) {
        if (m_seenDataTypeVar) {
            m_log.fatal () << "Operator definition template has more than one data type variable at "
                           << q->location() << ".";
            return false;
        }
        m_seenDataTypeVar = true;
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

bool OperatorTemplateVarChecker::visitSecTypeF (TreeNodeSecTypeF* t, TypeArgumentKind kind) {
    assert (t != nullptr);

    if (! verifyKind (TA_SEC, kind, t->location ()))
        return false;

    if (t->isPublic ())
        return true;

    return visit (t->identifier (), TA_SEC);
}

bool OperatorTemplateVarChecker::visitDimTypeConstF (TreeNodeDimTypeConstF* t) {
    if (t->cachedType () != 0u && t->cachedType () != 1u) {
        badType (t);
        return false;
    }
    return true;
}

namespace {

struct TemplateType {

    TemplateType() {
        // Silence compiler
        typeConst = DATATYPE_UNDEFINED;
        isPub = true;
        isConst = true;
        dim = 0u;
    }

    bool isPub;
    StringRef domain;

    bool isConst;
    SecrecDataType typeConst;
    StringRef typeVar;

    SecrecDimType dim;
};

bool operator == (TemplateType a, TemplateType b) {
    using namespace std;

    if (a.isPub != b.isPub)
        return false;

    if (!a.isPub && a.domain != b.domain)
        return false;

    if (a.isConst != b.isConst)
        return false;

    if (a.isConst) {
        if (a.typeConst != b.typeConst)
            return false;
    } else if (a.typeVar != b.typeVar) {
        return false;
    }

    if (a.dim != b.dim)
        return false;

    return true;
}

TemplateType toTemplateType (TreeNodeType* t) {
    TemplateType res;

    if (t->secType ()->isPublic ()) {
        res.isPub = true;
    } else {
        res.isPub = false;
        res.domain = t->secType ()->identifier ()->value ();
    }

    if (t->dataType ()->type () == NODE_DATATYPE_CONST_F) {
        res.isConst = true;
        res.typeConst = static_cast<TreeNodeDataTypeConstF*> (t->dataType ())->secrecDataType ();
    } else {
        res.isConst = false;
        res.typeVar = t->dataType ()->identifier ()->value ();
    }

    assert (t->dimType ()->type () == NODE_DIMTYPE_CONST_F);

    res.dim = t->dimType ()->cachedType ();

    return res;
}

bool join (TemplateType a, TemplateType b, TemplateType& res) {
    // Domain
    if (a.isPub) {
        if (b.isPub) {
            res.isPub = true;
        }
        else {
            res.isPub = false;
            res.domain = b.domain;
        }
    }
    else {
        res.isPub = false;
        if (b.isPub || a.domain == b.domain) {
            res.domain = a.domain;
        }
        else {
            return false;
        }
    }

    // Data type
    if (a.isConst) {
        if (b.isConst) {
            if (a.typeConst == b.typeConst) {
                res.isConst = true;
                res.typeConst = a.typeConst;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        if (b.isConst) {
            return false;
        } else {
            if (a.typeVar == b.typeVar) {
                res.isConst = false;
                res.typeVar = a.typeVar;
            } else {
                return false;
            }
        }
    }

    // Dim
    if (a.dim == b.dim) {
        res.dim = a.dim;
    }
    else if (a.dim == 0u) {
        res.dim = b.dim;
    }
    else if (b.dim == 0u) {
        res.dim = a.dim;
    }
    else {
        return false;
    }

    return true;
}

} /* namespace { */

bool OperatorTemplateVarChecker::checkLUB (TreeNodeTemplate* templ) {

    TemplateType ret = toTemplateType (templ->body ()->returnType ());
    std::vector<TemplateType> args;

    for (TreeNodeStmtDecl& decl : templ->body ()->params ()) {
        args.push_back (toTemplateType (decl.varType ()));
    }

    bool bad = false;

    if (args.size () == 1u && !(args[0u] == ret))
        bad = true;

    using namespace std;

    if (args.size () == 2u) {
        TemplateType lub;
        if (!join (args[0u], args[1u], lub))
            bad = true;
        else if (!(lub == ret))
            bad = true;
    }

    if (bad) {
        m_log.fatal () << "Return type of operator definition template at "
                       << templ->location () << " is not the least upper bound of operand types.";
        return false;
    }

    return true;
}

} // namespace SecreC
