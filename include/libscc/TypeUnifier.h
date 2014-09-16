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

class Type;
class Location;

/*******************************************************************************
  TypeUnifier
*******************************************************************************/

class TypeUnifier {
private: /* Types: */

    using TypeVarMap = std::map<StringRef, TypeArgument, StringRef::FastCmp>;

public:

    using result_type = bool;

public: /* Methods: */

    TypeUnifier () = default;
    TypeUnifier (const TypeUnifier&) = delete;
    TypeUnifier& operator = (const TypeUnifier&) = delete;
    TypeUnifier (TypeUnifier&&) = default;
    TypeUnifier& operator = (TypeUnifier&&) = default;

    bool visitType (TreeNodeType* t, Type* type);

    bool visitSecTypeF (TreeNodeSecTypeF* t, SecurityType* secType);

    bool visitDataTypeF (TreeNodeDataTypeF* t, DataType* dataType);
    bool visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t, DataType* dataType);
    bool visitDataTypeVarF (TreeNodeDataTypeVarF* t, DataType* dataType);
    bool visitDataTypeConstF (TreeNodeDataTypeConstF* t, DataType* dataType);

    bool visitDimTypeF (TreeNodeDimTypeF* t, SecrecDimType dimType);
    bool visitDimTypeVarF (TreeNodeDimTypeVarF* t, SecrecDimType dimType);
    bool visitDimTypeConstF(TreeNodeDimTypeConstF* t, SecrecDimType dimType);

    bool visitTypeArg (TreeNodeTypeArg* t, const TypeArgument& arg);
    bool visitTypeArgVar (TreeNodeTypeArgVar* t, const TypeArgument& arg);
    bool visitTypeArgTemplate (TreeNodeTypeArgTemplate* t, const TypeArgument& arg);
    bool visitTypeArgDataTypeConst (TreeNodeTypeArgDataTypeConst* t, const TypeArgument& arg);
    bool visitTypeArgDimTypeConst (TreeNodeTypeArgDimTypeConst* t, const TypeArgument& arg);
    bool visitTypeArgPublic (TreeNodeTypeArgPublic* t, const TypeArgument& arg);

    bool findName (StringRef name, TypeArgument& arg) const;
    const TypeVarMap& typeVars () const { return m_names; }

private:

    bool bind (StringRef name, const TypeArgument& arg);

private: /* Fields: */
    TypeVarMap m_names;
};


} /* namespace SecreC */

#endif /* SECREC_TYPE_UNIFIER_H */

