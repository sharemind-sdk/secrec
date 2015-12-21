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

#ifndef SECREC_OPERATOR_TYPE_UNIFIER_H
#define SECREC_OPERATOR_TYPE_UNIFIER_H

#include "Log.h"
#include "StringRef.h"
#include "Symbol.h"
#include "TreeNodeFwd.h"
#include "TypeArgument.h"
#include "Types.h"

#include <map>

namespace SecreC {

class Type;

/*******************************************************************************
  OperatorTypeUnifier
*******************************************************************************/

class OperatorTypeUnifier {

private: /* Types: */

    using TypeVarMap = std::map<StringRef, TypeArgument, StringRef::FastCmp>;

public:

    using result_type = bool;

public: /* Methods: */

    OperatorTypeUnifier (const std::vector<TypeBasic*>& argTypes,
                         SymbolTemplate* sym);
    OperatorTypeUnifier (const OperatorTypeUnifier&) = delete;
    OperatorTypeUnifier& operator = (const OperatorTypeUnifier&) = delete;
    OperatorTypeUnifier (OperatorTypeUnifier&&) = default;
    OperatorTypeUnifier& operator = (OperatorTypeUnifier&&) = default;

    bool visitType (TreeNodeType* t, Type* type);
    bool visitDimTypeF (TreeNodeDimTypeF* t, SecrecDimType dimType);
    bool visitDimTypeVarF (TreeNodeDimTypeVarF* t, SecrecDimType dimType);
    bool visitDimTypeConstF (TreeNodeDimTypeConstF* t, SecrecDimType dimType);

    bool checkKind ();
    bool checkSecLUB ();

    void getTypeArguments (std::vector<TypeArgument>& params);

private:

    bool bind (StringRef name, const TypeArgument& arg);

private: /* Fields: */

    SymbolTemplate* m_sym;
    // LUB of expression operands binds domain
    SecurityType* m_securityType;
    TreeNodeQuantifierDomain* m_domainVar;
    TypeVarMap m_names;
};

} /* namespace SecreC */

#endif /* SECREC_OPERATOR_TYPE_UNIFIER_H */

