/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_TYPE_UNIFIER_H
#define SECREC_TYPE_UNIFIER_H

#include "StringRef.h"
#include "TypeArgument.h"
#include "TreeNodeFwd.h"

#include <map>

namespace SecreC {

class Context;
class CompileLog;
class SymbolTable;
class Type;

/*******************************************************************************
  TypeUnifier
*******************************************************************************/

class TypeUnifier {
private: /* Types: */

    using TypeVarMap = std::map<StringRef, TypeArgument, StringRef::FastCmp>;

public: /* Methods: */

    explicit TypeUnifier (SymbolTable* st)
        : m_st (st)
    { }

    TypeUnifier (const TypeUnifier&) = delete;
    TypeUnifier& operator = (const TypeUnifier&) = delete;
    TypeUnifier (TypeUnifier&&) = default;
    TypeUnifier& operator = (TypeUnifier&&) = default;

    bool unifyType (TreeNodeType* t, Type* type);

    bool unifyType (TreeNodeSecTypeF* t, SecurityType* secType);

    bool unifyType (TreeNodeDataTypeF* t, DataType* dataType);
    bool unifyType (TreeNodeDataTypeTemplateF* t, DataType* dataType);
    bool unifyType (TreeNodeDataTypeVarF* t, DataType* dataType);
    bool unifyType (TreeNodeDataTypeConstF* t, DataType* dataType);

    bool unifyType (TreeNodeDimTypeF* t, SecrecDimType dimType);
    bool unifyType (TreeNodeDimTypeVarF* t, SecrecDimType dimType);
    bool unifyType (TreeNodeDimTypeConstF* t, SecrecDimType dimType);

    bool unifyType (TreeNodeTypeArg* t, const TypeArgument& arg);
    bool unifyType (TreeNodeTypeArgVar* t, const TypeArgument& arg);
    bool unifyType (TreeNodeTypeArgTemplate* t, const TypeArgument& arg);
    bool unifyType (TreeNodeTypeArgDataTypeConst* t, const TypeArgument& arg);
    bool unifyType (TreeNodeTypeArgDimTypeConst* t, const TypeArgument& arg);
    bool unifyType (TreeNodeTypeArgPublic* t, const TypeArgument& arg);

    bool findName (StringRef name, TypeArgument& arg) const;
    const TypeVarMap& typeVars () const { return m_names; }

private:

    bool bind (StringRef name, const TypeArgument& arg);

private: /* Fields: */
    SymbolTable* m_st;
    TypeVarMap   m_names;
};


} /* namespace SecreC */

#endif /* SECREC_TYPE_UNIFIER_H */

