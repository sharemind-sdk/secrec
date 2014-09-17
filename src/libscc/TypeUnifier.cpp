/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TypeUnifier.h"

#include "DataType.h"
#include "SecurityType.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "Types.h"
#include "Visitor.h"

#define TUGUARD(expr) \
    do { \
        if (! (expr)) \
            return false; \
    } while (false)

namespace SecreC {

/*******************************************************************************
  TypeUnifier
*******************************************************************************/

bool TypeUnifier::bind (StringRef name, const TypeArgument& arg) {
    auto it = m_names.find (name);
    if (it != m_names.end () && it->second != arg)
        return false;
    m_names.insert (it, std::make_pair (name, arg));
    return true;
}

bool TypeUnifier::findName (StringRef name, TypeArgument& arg) const {
    auto it = m_names.find (name);
    if (it == m_names.end ())
        return false;

    arg = it->second;
    return true;
}

/*******************************************************************************
  TreeNodeType
*******************************************************************************/

bool TypeUnifier::visitType (TreeNodeType* t, Type* type) {
    assert (t != nullptr);
    assert (type != nullptr);

    TUGUARD (t->isNonVoid () != type->isVoid ());

    if (t->isNonVoid ()) {
        const auto tnv = static_cast<TypeNonVoid*>(type);
        TUGUARD (visitSecTypeF (t->secType (), tnv->secrecSecType ()));
        TUGUARD (visitDataTypeF (t->dataType (), tnv->secrecDataType ()));
        TUGUARD (visitDimTypeF (t->dimType (), tnv->secrecDimType ()));
    }

    return true;
}

/*******************************************************************************
  TreeNodeSecTypeF
*******************************************************************************/

// TODO: figure out why we have retained a domain symbols in the current
// scope. This should definitely not happen.
bool TypeUnifier::visitSecTypeF (TreeNodeSecTypeF* t, SecurityType* secType) {
    assert (t != nullptr);
    assert (secType != nullptr);

    if (t->isPublic ()) {
        return secType->isPublic ();
    }

    const StringRef name = t->identifier ()->value ();
//    if (SymbolDomain* s = m_st->find<SYM_DOMAIN>(name)) {
//        return s->securityType () == secType;
//    }

    return bind (name, secType);
}

/*******************************************************************************
  TreeNodeDataTypeF
*******************************************************************************/

bool TypeUnifier::visitDataTypeF (TreeNodeDataTypeF* t, DataType* dataType) {
    return dispatchDataTypeF (*this, t, dataType);
}

bool TypeUnifier::visitDataTypeConstF (TreeNodeDataTypeConstF* t, DataType* dataType) {
    assert (dataType != nullptr);
    return dataType->equals (t->secrecDataType ());
}

bool TypeUnifier::visitDataTypeVarF (TreeNodeDataTypeVarF* t, DataType* dataType) {
    assert (dataType != nullptr);
    const StringRef name = t->identifier ()->value ();
//    if (SymbolDataType* sym = m_st->find<SYM_TYPE>(name)) {
//        return dataType->equals (sym->dataType ());
//    }

    return bind (name, dataType);
}

bool TypeUnifier::visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t, DataType* dataType) {
    assert (dataType != nullptr);

    TUGUARD (dataType->isComposite ());

    DataTypeStruct* structType = static_cast<DataTypeStruct*>(dataType);
    const StringRef name = t->identifier ()-> value ();
    auto args = t->arguments ();
    const auto& expectedArgs = structType->typeArgs ();

    TUGUARD (name == structType->name ());
    TUGUARD (args.size () == expectedArgs.size ());
    for (size_t i = 0; i < args.size (); ++ i) {
        TreeNodeTypeArg& arg = args[i];
        TypeArgument expectedArg = expectedArgs[i];
        TUGUARD (visitTypeArg (&arg, expectedArg));
    }

    return true;
}

/*******************************************************************************
  TreeNodeDimTypeF
*******************************************************************************/

bool TypeUnifier::visitDimTypeF (TreeNodeDimTypeF* t, SecrecDimType dimType) {
    return dispatchDimTypeF (*this, t, dimType);
}

bool TypeUnifier::visitDimTypeConstF (TreeNodeDimTypeConstF* t, SecrecDimType dimType) {
    return t->cachedType () == dimType;
}

bool TypeUnifier::visitDimTypeVarF (TreeNodeDimTypeVarF* t, SecrecDimType dimType) {
    assert (t != nullptr);
    const StringRef name = t->identifier ()->value ();
//    if (SymbolDimensionality* sym = m_st->find<SYM_DIM>(name)) {
//        return sym->dimType () == dimType;
//    }

    return bind (name, dimType);
}

/*******************************************************************************
  TreeNodeTypeArg
*******************************************************************************/

bool TypeUnifier::visitTypeArg (TreeNodeTypeArg* t, const TypeArgument& arg) {
    return dispatchTypeArg (*this, t, arg);
}

bool TypeUnifier::visitTypeArgVar (TreeNodeTypeArgVar* t, const TypeArgument& arg) {
    return bind (t->identifier ()->value (), arg);
}

bool TypeUnifier::visitTypeArgTemplate (TreeNodeTypeArgTemplate* t, const TypeArgument& arg) {
    TUGUARD (arg.isDataType ());
    TUGUARD (arg.dataType ()->isComposite ());

    const auto structType = static_cast<DataTypeStruct*>(arg.dataType ());
    auto args = t->arguments ();
    const auto& expectedArgs = structType->typeArgs ();
    TUGUARD (structType->name () == t->identifier ()->value ());

    // we don't have to verify the number of arguments because this
    // is already guaranteed to be correct by the type checking of the argument.
    assert (args.size () == expectedArgs.size () &&
            "ICE: This should have bee guaranteed by the type checker!");

    for (size_t i = 0; i < args.size (); ++ i) {
        TreeNodeTypeArg& arg = args[i];
        TypeArgument expectedArg = expectedArgs[i];
        TUGUARD (visitTypeArg (&arg, expectedArg));
    }

    return true;
}

bool TypeUnifier::visitTypeArgDataTypeConst (TreeNodeTypeArgDataTypeConst* t, const TypeArgument& arg) {
    return arg.equals (t->secrecDataType ());
}

bool TypeUnifier::visitTypeArgDimTypeConst (TreeNodeTypeArgDimTypeConst* t, const TypeArgument& arg) {
    return arg.equals (t->secrecDimType ());
}

bool TypeUnifier::visitTypeArgPublic (TreeNodeTypeArgPublic*, const TypeArgument& arg) {
    TUGUARD (arg.kind () == TA_SEC);
    return arg.secType ()->isPublic ();
}

} // namespace SecreC
