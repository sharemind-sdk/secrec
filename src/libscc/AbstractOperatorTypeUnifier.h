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

#ifndef SECREC_ABSTRACT_OPERATOR_TYPE_UNIFIER_H
#define SECREC_ABSTRACT_OPERATOR_TYPE_UNIFIER_H

#include "Context.h"
#include "StringRef.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TypeArgument.h"
#include "Types.h"
#include "SecurityType.h"
#include "TreeNodeFwd.h"

#include <map>


namespace SecreC {

/*******************************************************************************
  AbstractOperatorTypeUnifier
*******************************************************************************/

class AbstractOperatorTypeUnifier {

private: /* Types: */

    using TypeVarMap = std::map<StringRef, TypeArgument, StringRef::FastCmp>;

public:

    using result_type = bool;

public: /* Methods: */

    AbstractOperatorTypeUnifier (SymbolTable* st,
                                 SymbolTemplate* sym)
        : m_st {st}
        , m_sym {sym}
        , m_securityType {nullptr}
        , m_domainVar {nullptr}
        { }

    AbstractOperatorTypeUnifier (const AbstractOperatorTypeUnifier&) = delete;
    AbstractOperatorTypeUnifier& operator = (const AbstractOperatorTypeUnifier&) = delete;
    AbstractOperatorTypeUnifier (AbstractOperatorTypeUnifier&&) = default;
    AbstractOperatorTypeUnifier& operator = (AbstractOperatorTypeUnifier&&) = default;

    virtual ~AbstractOperatorTypeUnifier() noexcept;

    bool visitType (TreeNodeType* t, const Type* type);

    virtual bool visitSecTypeF (TreeNodeSecTypeF* t, const SecurityType* secType) = 0;

    bool visitDataTypeF (TreeNodeType* t,
                         const TypeNonVoid* type);
    bool visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* ttemplate,
                                 TreeNodeType* t,
                                 const TypeNonVoid* type);
    bool visitDataTypeVarF (TreeNodeDataTypeVarF* tvar,
                            TreeNodeType* t,
                            const TypeNonVoid* type);
    bool visitDataTypeConstF (TreeNodeDataTypeConstF* tconst,
                              TreeNodeType* t,
                              const TypeNonVoid* dataType);

    bool visitDimTypeF (TreeNodeDimTypeF* t, SecrecDimType dimType);
    bool visitDimTypeVarF (TreeNodeDimTypeVarF* t, SecrecDimType dimType);
    virtual bool visitDimTypeZeroF(TreeNodeDimTypeZeroF* t, SecrecDimType dimType) = 0;
    virtual bool visitDimTypeConstF (TreeNodeDimTypeConstF* t, SecrecDimType dimType) = 0;

    bool checkKind ();

    void getTypeArguments (std::vector<TypeArgument>& params);

protected:

    virtual bool bind (StringRef name, const TypeArgument& arg, const SecurityType* sec = nullptr);

protected: /* Fields: */

    SymbolTable* m_st;
    SymbolTemplate* m_sym;
    const SecurityType* m_securityType;
    TreeNodeQuantifierDomain* m_domainVar;
    TypeVarMap m_names;
};

} /* namespace SecreC */

#endif /* SECREC_ABSTRACT_OPERATOR_TYPE_UNIFIER_H */
