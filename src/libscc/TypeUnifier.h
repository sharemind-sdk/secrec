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

#ifndef SECREC_TYPE_UNIFIER_H
#define SECREC_TYPE_UNIFIER_H

#include "StringRef.h"
#include "TypeArgument.h"
#include "TreeNodeFwd.h"

#include <map>

namespace SecreC {

class Location;
class SymbolTable;
class SymbolTemplate;
class Type;

/*******************************************************************************
  TypeUnifier
*******************************************************************************/

class TypeUnifier {
private: /* Types: */

    using TypeVarMap = std::map<StringRef, TypeArgument, StringRef::FastCmp>;

public:

    using result_type = bool;

public: /* Methods: */

    explicit TypeUnifier (SymbolTable* st,
                          SymbolTemplate* sym)
        : m_st {st}
        , m_sym {sym}
    { }

    TypeUnifier (const TypeUnifier&) = delete;
    TypeUnifier& operator = (const TypeUnifier&) = delete;
    TypeUnifier (TypeUnifier&&) = default;
    TypeUnifier& operator = (TypeUnifier&&) = default;

    bool visitType (TreeNodeType* t, const Type* type);

    bool visitSecTypeF (TreeNodeSecTypeF* t, const SecurityType* secType);

    bool visitDataTypeF (TreeNodeDataTypeF* t, const DataType* dataType);
    bool visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t, const DataType* dataType);
    bool visitDataTypeVarF (TreeNodeDataTypeVarF* t, const DataType* dataType);
    bool visitDataTypeConstF (TreeNodeDataTypeConstF* t, const DataType* dataType);

    bool visitDimTypeF (TreeNodeDimTypeF* t, SecrecDimType dimType);
    bool visitDimTypeVarF (TreeNodeDimTypeVarF* t, SecrecDimType dimType);
    bool visitDimTypeZeroF(TreeNodeDimTypeZeroF* t, SecrecDimType dimType);
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
    SymbolTable* m_st;
    SymbolTemplate* m_sym;
    TypeVarMap m_names;
};


} /* namespace SecreC */

#endif /* SECREC_TYPE_UNIFIER_H */

