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

#include "DataType.h"
#include "Log.h"
#include "SecurityType.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Types.h"
#include "Visitor.h"

namespace SecreC {

/*******************************************************************************
 TreeNodeQuantifier
*******************************************************************************/

TypeChecker::Status TypeChecker::visitQuantifier (TreeNodeQuantifier* q) {
    return dispatchQuantifier (*this, q);
}

TypeChecker::Status TypeChecker::visitQuantifierDomain(TreeNodeQuantifierDomain* q) {
    if (q->kind ()) {
        if (findIdentifier (SYM_KIND, q->kind ()) == nullptr)
            return E_TYPE;
    }

    return OK;
}

TypeChecker::Status TypeChecker::visitQuantifierDim (TreeNodeQuantifierDim*) {
    return OK;
}

TypeChecker::Status TypeChecker::visitQuantifierData(TreeNodeQuantifierData*) {
    return OK;
}

/*******************************************************************************
 TreeNodeTypeF
*******************************************************************************/

TypeChecker::Status TypeChecker::visitTypeF(TreeNodeTypeF* ty) {
    return dispatchTypeF (*this, ty);
}

TypeChecker::Status TypeChecker::visitTypeVarF(TreeNodeTypeVarF* ty) {
    if (ty->typeVariable ())
        return OK;

    const StringRef name = ty->identifier ()->value ();
    SymbolTypeVariable* symType = m_st->find<SYM_TYPE>(name);
    SymbolTypeVariable* symDomain = m_st->find<SYM_DOMAIN>(name);
    if (symType == nullptr && symDomain == nullptr) {
        m_log.fatalInProc(ty) << "Identifier '" << name
                              << "' at " << ty->identifier ()->location()
                              << " not in scope.";
        return E_TYPE;
    }

    ty->setTypeVariable (symType ? symType : symDomain);
    return OK;
}

void TreeNodeTypeVarF::setTypeContext (TypeContext& cxt) const {
    assert (typeVariable () != nullptr);
    typeVariable ()->setTypeContext (cxt);
}

/*******************************************************************************
 TreeNodeSecTypeF
*******************************************************************************/

TypeChecker::Status TypeChecker::visitSecTypeF(TreeNodeSecTypeF * ty) {
    if (ty->cachedType () != nullptr)
        return OK;

    if (ty->isPublic ()) {
        ty->setCachedType (PublicSecType::get ());
        return OK;
    }

    TreeNodeIdentifier* id = ty->identifier ();
    SymbolDomain* s = findIdentifier<SYM_DOMAIN>(id);
    if (s == nullptr)
        return E_TYPE;

    ty->setCachedType (s->securityType ());
    return OK;
}

void TreeNodeSecTypeF::setTypeContext (TypeContext& cxt) const {
    cxt.setContextSecType (cachedType ());
}

/*******************************************************************************
  TreeNodeDataTypeF
*******************************************************************************/

TypeChecker::Status TypeChecker::visitDataTypeF (TreeNodeDataTypeF *ty) {
    return dispatchDataTypeF (*this, ty);
}

TypeChecker::Status TypeChecker::visitDataTypeConstF (TreeNodeDataTypeConstF *ty) {
    if (ty->cachedType () != nullptr) {
        return OK;
    }

    ty->setCachedType (DataTypePrimitive::get (ty->secrecDataType ()));
    return OK;
}

void TreeNodeDataTypeF::setTypeContext (TypeContext& cxt) const {
    cxt.setContextDataType (cachedType ());
}

TypeChecker::Status TypeChecker::visitDataTypeVarF (TreeNodeDataTypeVarF* ty) {
    if (ty->cachedType () != nullptr) {
        return OK;
    }

    SymbolDataType* s = nullptr;
    if ((s = findIdentifier<SYM_TYPE>(ty->identifier ())) == nullptr)
        return E_TYPE;

    ty->setCachedType (s->dataType ());
    return OK;
}

/*******************************************************************************
  TreeNodeDimTypeF
*******************************************************************************/

TypeChecker::Status TypeChecker::visitDimTypeF(TreeNodeDimTypeF* ty) {
    return dispatchDimTypeF (*this, ty);
}

void TreeNodeDimTypeF::setTypeContext (TypeContext& cxt) const {
    cxt.setContextDimType (cachedType ());
}

TypeChecker::Status TypeChecker::visitDimTypeConstF (TreeNodeDimTypeConstF*) {
    return OK;
}

TypeChecker::Status TypeChecker::visitDimTypeVarF (TreeNodeDimTypeVarF * ty) {
    if (ty->cachedType () != ~ SecrecDimType (0)) {
        return OK;
    }

    Symbol* s = nullptr;
    if ((s = findIdentifier (SYM_DIM, ty->identifier ())) == nullptr)
        return E_TYPE;

    ty->setCachedType (static_cast<SymbolDimensionality*>(s)->dimType ());
    return OK;
}

/*******************************************************************************
  TreeNodeType
*******************************************************************************/

TypeChecker::Status TypeChecker::visitType (TreeNodeType * _ty) {
    if (_ty->m_cachedType != nullptr)
        return OK;

    if (_ty->type () == NODE_TYPETYPE) {
        assert (dynamic_cast<TreeNodeTypeType*>(_ty) != nullptr);
        TreeNodeTypeType* tyNode = static_cast<TreeNodeTypeType*>(_ty);
        TreeNodeSecTypeF* secTyNode = tyNode->secType ();
        TCGUARD (visitSecTypeF (secTyNode));
        TCGUARD (visitDimTypeF (tyNode->dimType ()));
        TCGUARD (visitDataTypeF (tyNode->dataType ()));

        const SecurityType* const secType = secTyNode->cachedType ();
        const DataType* const dataType = tyNode->dataType ()->cachedType ();
        const SecrecDimType dimType = tyNode->dimType ()->cachedType ();

        if (dataType->isPrimitive ()) {
            SecrecDataType secrecDataType = static_cast<const DataTypePrimitive*>(dataType)->secrecDataType ();
            if (secType->isPublic ()) {
                switch (secrecDataType) {
                case DATATYPE_XOR_UINT8:
                case DATATYPE_XOR_UINT16:
                case DATATYPE_XOR_UINT32:
                case DATATYPE_XOR_UINT64:
                    m_log.fatalInProc(_ty) << "XOR types do not have public representation at "
                                           << _ty->location () << '.';
                    return E_TYPE;
                default:
                    break;
                }
            }
        }
        else {
            if (secType->isPrivate () || dimType > 0) {
                m_log.fatal () << "Non-primitive types may not be private or non-scalar. Error at " << _ty->location () << ".";
                return E_TYPE;
            }
        }

        tyNode->m_cachedType = TypeBasic::get (secType, dataType, dimType);
    }
    else {
        assert (dynamic_cast<TreeNodeTypeVoid*>(_ty) != nullptr);
        _ty->m_cachedType = TypeVoid::get();
    }

    return OK;
}

/*******************************************************************************
  TreeNodeDataTypeTemplateF
*******************************************************************************/

TypeChecker::Status TypeChecker::visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t) {
    assert (t != nullptr);
    if (t->cachedType () != nullptr)
        return OK;

    const DataTypeStruct* structType = nullptr;
    TCGUARD (checkTypeApplication (t->identifier (), t->arguments (), t->location (), structType));
    assert (structType != nullptr);
    t->setCachedType (structType);
    return OK;
}

TypeChecker::Status TypeChecker::checkTypeApplication (TreeNodeIdentifier* id,
                                                       TreeNodeSeqView<TreeNodeTypeArg> args,
                                                       const Location& loc,
                                                       const DataTypeStruct*& result)
{
    assert (id != nullptr);

    SymbolStruct* sym = m_st->find<SYM_STRUCT>(id->value ());
    if (sym == nullptr) {
        m_log.fatal () << "Structure name \'" << id->value () << "\' not in scope at "
                       << id->location () << ".";
        return E_TYPE;
    }

    TreeNodeStructDecl* structDecl = sym->decl ();
    TreeNodeSeqView<TreeNodeQuantifier> quants = structDecl->quantifiers ();
    if (quants.size () != args.size ()) {
        m_log.fatal () << "Mismatching number of type arguments at " << loc << ".";
        m_log.fatal () << "Expected " << quants.size () << " got " << args.size () << ".";
        return E_TYPE;
    }

    std::vector<TypeArgument> typeArgs;
    typeArgs.reserve (args.size ());
    for (size_t i = 0; i < args.size (); ++ i) {
        TreeNodeTypeArg& arg = args[i];
        TCGUARD (visitTypeArg (&arg));
        TreeNodeQuantifier& quant = quants[i];
        const TypeArgument& typeArg = arg.typeArgument ();
        bool mismatch = false;
        if (typeArg.kind () != quantifierKind (quant)) {
            mismatch = true;
        }
        else
        if (quant.type () == NODE_TEMPLATE_QUANTIFIER_DOMAIN) {
            auto kind = static_cast<TreeNodeQuantifierDomain&>(quant).kind ();
            if (kind != nullptr) {
                if (typeArg.secType ()->isPublic ()) {
                    mismatch = true;
                }
                else {
                    const auto kindSym = m_st->find<SYM_KIND>(kind->value ());
                    assert (kindSym != nullptr);
                    const auto privateSecType = static_cast<const PrivateSecType*>(typeArg.secType ());
                    if (privateSecType->securityKind () != kindSym) {
                        mismatch = true;
                    }
                }
            }
        }

        if (mismatch) {
            m_log.fatal () << "Mismatching type argument at " << arg.location () << ".";
            m_log.fatal () << "Respective quantifier defined at " << quant.location () << ".";
            return E_TYPE;
        }

        typeArgs.push_back (typeArg);
    }

    return checkStruct (structDecl, loc, result, typeArgs);
}

} // namespace SecreC
