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
                                 SymbolTemplate* sym,
                                 Context& cxt)
        : m_st {st}
        , m_sym {sym}
        , m_securityType {nullptr}
        , m_domainVar {nullptr}
        , m_cxt {cxt}
        { }

    AbstractOperatorTypeUnifier (const AbstractOperatorTypeUnifier&) = delete;
    AbstractOperatorTypeUnifier& operator = (const AbstractOperatorTypeUnifier&) = delete;
    AbstractOperatorTypeUnifier (AbstractOperatorTypeUnifier&&) = default;
    AbstractOperatorTypeUnifier& operator = (AbstractOperatorTypeUnifier&&) = default;

    bool visitType (TreeNodeType* t, Type* type);

    virtual bool visitSecTypeF (TreeNodeSecTypeF* t, SecurityType* secType) = 0;

    bool visitDataTypeF (TreeNodeType* t,
                         TypeNonVoid* type);
    bool visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* ttemplate,
                                 TreeNodeType* t,
                                 TypeNonVoid* type);
    bool visitDataTypeVarF (TreeNodeDataTypeVarF* tvar,
                            TreeNodeType* t,
                            TypeNonVoid* type);
    bool visitDataTypeConstF (TreeNodeDataTypeConstF* tconst,
                              TreeNodeType* t,
                              TypeNonVoid* dataType);

    bool visitDimTypeF (TreeNodeDimTypeF* t, SecrecDimType dimType);
    bool visitDimTypeVarF (TreeNodeDimTypeVarF* t, SecrecDimType dimType);
    virtual bool visitDimTypeConstF (TreeNodeDimTypeConstF* t, SecrecDimType dimType) = 0;

    bool checkKind ();

    void getTypeArguments (std::vector<TypeArgument>& params);

protected:

    virtual bool bind (StringRef name, const TypeArgument& arg, SecurityType* sec = nullptr);

protected: /* Fields: */

    SymbolTable* m_st;
    SymbolTemplate* m_sym;
    SecurityType* m_securityType;
    TreeNodeQuantifierDomain* m_domainVar;
    Context& m_cxt;
    TypeVarMap m_names;
};

} /* namespace SecreC */

#endif /* SECREC_ABSTRACT_OPERATOR_TYPE_UNIFIER_H */
