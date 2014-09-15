/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TypeUnifier.h"

#include "TreeNode.h"
#include "Symbol.h"
#include "SymbolTable.h"

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

bool TypeUnifier::unifyType (TreeNodeType* t, Type* type) {
    assert (t != nullptr);
    assert (type != nullptr);


    TUGUARD (t->isNonVoid () != type->isVoid ());

    if (t->isNonVoid ()) {
        const auto tnv = static_cast<TypeNonVoid*>(type);
        TUGUARD (unifyType (t->secType (), tnv->secrecSecType ()));
        TUGUARD (unifyType (t->dataType (), tnv->secrecDataType ()));
        TUGUARD (unifyType (t->dimType (), tnv->secrecDimType ()));
    }

    return true;
}

/*******************************************************************************
  TreeNodeSecTypeF
*******************************************************************************/

// TODO: figure out why we have retained a domain symbols in the current
// scope. This should definitely not happen.
bool TypeUnifier::unifyType (TreeNodeSecTypeF* t, SecurityType* secType) {
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

bool TypeUnifier::unifyType (TreeNodeDataTypeF* t, DataType* dataType) {
    return t->acceptUnifier (*this, dataType);
}

/*******************************************************************************
  TreeNodeDataTypeConstF
*******************************************************************************/

bool TreeNodeDataTypeConstF::acceptUnifier (TypeUnifier& tu, DataType* dataType) {
    return tu.unifyType (this, dataType);
}

bool TypeUnifier::unifyType (TreeNodeDataTypeConstF* t, DataType* dataType) {
    assert (t != nullptr);
    assert (dataType != nullptr);
    return dataType->equals (t->secrecDataType ());
}

/*******************************************************************************
  TreeNodeDataTypeVarF
*******************************************************************************/

bool TreeNodeDataTypeVarF::acceptUnifier (TypeUnifier& tu, DataType* dataType) {
    return tu.unifyType (this, dataType);
}

bool TypeUnifier::unifyType (TreeNodeDataTypeVarF* t, DataType* dataType) {
    assert (t != nullptr);
    assert (dataType != nullptr);
    const StringRef name = t->identifier ()->value ();
//    if (SymbolDataType* sym = m_st->find<SYM_TYPE>(name)) {
//        return dataType->equals (sym->dataType ());
//    }

    return bind (name, dataType);
}

/*******************************************************************************
  TreeNodeDataTypeTemplateF
*******************************************************************************/

bool TreeNodeDataTypeTemplateF::acceptUnifier (TypeUnifier& tu, DataType* dataType) {
    return tu.unifyType (this, dataType);
}

bool TypeUnifier::unifyType (TreeNodeDataTypeTemplateF* t, DataType* dataType) {
    assert (t != nullptr);
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
        TUGUARD (unifyType (&arg, expectedArg));
    }

    return true;
}

/*******************************************************************************
  TreeNodeDimTypeF
*******************************************************************************/

bool TypeUnifier::unifyType (TreeNodeDimTypeF* t, SecrecDimType dimType) {
    return t->acceptUnifier (*this, dimType);
}

/*******************************************************************************
  TreeNodeDimTypeConstF
*******************************************************************************/

bool TreeNodeDimTypeConstF::acceptUnifier (TypeUnifier& tu, SecrecDimType dimType) {
    return tu.unifyType (this, dimType);
}

bool TypeUnifier::unifyType (TreeNodeDimTypeConstF* t, SecrecDimType dimType) {
    return t->cachedType () == dimType;
}

/*******************************************************************************
  TreeNodeDimTypeVarF
*******************************************************************************/

bool TreeNodeDimTypeVarF::acceptUnifier (TypeUnifier& tu, SecrecDimType dimType) {
    return tu.unifyType (this, dimType);
}

bool TypeUnifier::unifyType (TreeNodeDimTypeVarF* t, SecrecDimType dimType) {
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

bool TypeUnifier::unifyType (TreeNodeTypeArg* t, const TypeArgument& arg) {
    assert (t != nullptr);
    return t->acceptUnifier (*this, arg);
}

/*******************************************************************************
  TreeNodeTypeArgVar
*******************************************************************************/

bool TreeNodeTypeArgVar::acceptUnifier (TypeUnifier& tu, const TypeArgument& arg) {
    return tu.unifyType (this, arg);
}

bool TypeUnifier::unifyType (TreeNodeTypeArgVar* t, const TypeArgument& arg) {
    assert (t != nullptr);
    return bind (t->identifier ()->value (), arg);
}

/*******************************************************************************
  TreeNodeTypeArgTemplate
*******************************************************************************/

bool TreeNodeTypeArgTemplate::acceptUnifier (TypeUnifier& tu, const TypeArgument& arg) {
    return tu.unifyType (this, arg);
}

bool TypeUnifier::unifyType (TreeNodeTypeArgTemplate* t, const TypeArgument& arg) {
    assert (t != nullptr);
    TUGUARD (arg.isDataType ());
    return unifyType (t, arg.dataType ());
}

/*******************************************************************************
  TreeNodeTypeArgDataTypeConst
*******************************************************************************/

bool TreeNodeTypeArgDataTypeConst::acceptUnifier (TypeUnifier& tu, const TypeArgument& arg) {
    return tu.unifyType (this, arg);
}

bool TypeUnifier::unifyType (TreeNodeTypeArgDataTypeConst* t, const TypeArgument& arg) {
    assert (t != nullptr);
    return arg.equals (t->secrecDataType ());
}

/*******************************************************************************
  TreeNodeTypeArgDimTypeConst
*******************************************************************************/

bool TreeNodeTypeArgDimTypeConst::acceptUnifier (TypeUnifier& tu, const TypeArgument& arg) {
    return tu.unifyType (this, arg);
}

bool TypeUnifier::unifyType (TreeNodeTypeArgDimTypeConst* t, const TypeArgument& arg) {
    assert (t != nullptr);
    return arg.equals (t->secrecDimType ());
}

/*******************************************************************************
  TreeNodeTypeArgPublic
*******************************************************************************/

bool TreeNodeTypeArgPublic::acceptUnifier (TypeUnifier& tu, const TypeArgument& arg) {
    return tu.unifyType (this, arg);
}

bool TypeUnifier::unifyType (TreeNodeTypeArgPublic*, const TypeArgument& arg) {
    TUGUARD (arg.kind () == TA_SEC);
    return arg.secType ()->isPublic ();
}

} // namespace SecreC
