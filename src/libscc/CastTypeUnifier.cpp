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


#define CTUGUARD(expr) \
    do { \
        if (! (expr)) \
            return false; \
    } while (false)

namespace SecreC {

/*******************************************************************************
  CastTypeUnifier
*******************************************************************************/

CastTypeUnifier::CastTypeUnifier (const TypeBasic* argType,
                                  SymbolTable* st,
                                  SymbolTemplate* sym,
                                  Context& cxt)
    : m_st {st}
    , m_sym {sym}
    , m_cxt {cxt}
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

bool CastTypeUnifier::bind (StringRef name, const TypeArgument& arg) {
    auto it = m_names.find (name);
    if (it != m_names.end () && it->second != arg) {
        return false;
    }
    m_names.insert (it, std::make_pair (name, arg));
    return true;
}

/*******************************************************************************
  TreeNodeType
*******************************************************************************/

bool CastTypeUnifier::visitType (TreeNodeType* t, Type* type) {
    assert (t != nullptr);
    assert (type != nullptr);

    CTUGUARD (! type->isVoid ());

    const auto tnv = static_cast<TypeNonVoid*>(type);

    CTUGUARD (visitSecTypeF (t->secType (), tnv->secrecSecType ()));
    CTUGUARD (visitDataTypeF (t, tnv));
    CTUGUARD (visitDimTypeF (t->dimType (), tnv->secrecDimType ()));

    return true;
}

/*******************************************************************************
  TreeNodeSecTypeF
*******************************************************************************/

bool CastTypeUnifier::visitSecTypeF (TreeNodeSecTypeF* t, SecurityType* secType) {
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
  TreeNodeDataTypeF
*******************************************************************************/

bool CastTypeUnifier::visitDataTypeF (TreeNodeType* t, TypeNonVoid* type) {
    return dispatchDataTypeF (*this, t->dataType (), t, type);
}

bool CastTypeUnifier::visitDataTypeConstF (TreeNodeDataTypeConstF* tconst,
                                           TreeNodeType* t,
                                           TypeNonVoid* type)
{
    SecrecDataType treeSc = tconst->secrecDataType ();
    DataType* argData = type->secrecDataType ();

    if (! t->secType ()->isPublic () &&
        type->secrecSecType ()->isPublic ())
    {
        treeSc = dtypeDeclassify (treeSc);
    }

    if (argData->isBuiltinPrimitive ()) {
        SecrecDataType argSc = static_cast<DataTypeBuiltinPrimitive*> (argData)->secrecDataType ();
        if (treeSc != argSc)
            return false;
    }
    else if (argData->isUserPrimitive ()) {
        assert (type->secrecSecType ()->isPrivate ());

        if (! static_cast<DataTypeUserPrimitive*> (argData)->equals (treeSc))
            return false;
    }
    else {
        // No structs in cast definitions
        assert (false);
    }

    return true;
}

bool CastTypeUnifier::visitDataTypeVarF (TreeNodeDataTypeVarF* tvar,
                                         TreeNodeType* t,
                                         TypeNonVoid* type)
{
    auto dataQuants = m_sym->dataTypeQuantifiers ();
    StringRef var = tvar->identifier ()->value ();
    DataType* argData = type->secrecDataType ();

    if (dataQuants.find (var) != dataQuants.end ()) {
        // bind template quantifier variable
        CTUGUARD (bind (var, argData));
    }
    else if (m_securityType->isPrivate ()) {
        // Check if the protection domain has this type
        SymbolKind* kind = static_cast<PrivateSecType*> (m_securityType)->securityKind ();
        DataTypeUserPrimitive* tyPrim = kind->findType (var);
        DataType* ty;

        CTUGUARD (tyPrim != nullptr);

        if (! t->secType ()->isPublic () &&
            type->secrecSecType ()->isPublic ())
        {
            auto publicType = tyPrim->publicType (kind);
            if (publicType)
                ty = *publicType;
            else
                ty = tyPrim;
        }
        else {
            ty = tyPrim;
        }

        if (argData->isPrimitive ()) {
            CTUGUARD (ty->equals (argData));
        }
        else {
            // No structs in operator definitions
            assert (false);
        }
    }
    else {
        // The variable is a struct which is not supported
        return false;
    }

    return true;
}

bool CastTypeUnifier::visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* ttemplate,
                                              TreeNodeType* t,
                                              TypeNonVoid* type)
{
    (void) ttemplate;
    (void) t;
    (void) type;
    assert (false);
}

/*******************************************************************************
  TreeNodeDimTypeF
*******************************************************************************/

bool CastTypeUnifier::visitDimTypeF (TreeNodeDimTypeF* t, SecrecDimType dimType) {
    return dispatchDimTypeF (*this, t, dimType);
}

bool CastTypeUnifier::visitDimTypeConstF (TreeNodeDimTypeConstF* t, SecrecDimType dimType) {
    (void) t;
    (void) dimType;
    return true;
}

bool CastTypeUnifier::visitDimTypeVarF (TreeNodeDimTypeVarF*, SecrecDimType) {
    // Dim variables are not allowed in cast definitions so this
    // should never happen.
    assert(false);
    return false;
}

bool CastTypeUnifier::checkKind () {
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

void CastTypeUnifier::getTypeArguments (std::vector<TypeArgument>& params) {
    params.clear ();
    for (TreeNodeQuantifier& quant : m_sym->decl ()-> quantifiers ()) {
        auto it = m_names.find (quant.typeVariable ()->value ());
        assert (it != m_names.end ());
        params.push_back (it->second);
    }
}

} /* namespace SecreC */
