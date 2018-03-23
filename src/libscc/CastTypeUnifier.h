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

#ifndef SECREC_CAST_TYPE_UNIFIER_H
#define SECREC_CAST_TYPE_UNIFIER_H

#include "AbstractOperatorTypeUnifier.h"
#include "Context.h"
#include "StringRef.h"
#include "SymbolTable.h"
#include "TreeNodeFwd.h"
#include "TypeArgument.h"
#include "Types.h"
#include "TreeNode.h"

#include <map>

namespace SecreC {

/*******************************************************************************
  CastTypeUnifier
*******************************************************************************/

class CastTypeUnifier: public AbstractOperatorTypeUnifier {

public: /* Methods: */

    CastTypeUnifier (const TypeBasic* argType,
                     SymbolTable* st,
                     SymbolTemplate* sym);

    CastTypeUnifier (const CastTypeUnifier&) = delete;
    CastTypeUnifier& operator = (const CastTypeUnifier&) = delete;
    CastTypeUnifier (CastTypeUnifier&&) = default;
    CastTypeUnifier& operator = (CastTypeUnifier&&) = default;

    bool visitDimTypeConstF (TreeNodeDimTypeConstF* t, SecrecDimType dimType) override;

    bool visitDimTypeZeroF (TreeNodeDimTypeZeroF* t, SecrecDimType dimType) override;

    bool visitSecTypeF (TreeNodeSecTypeF* t, const SecurityType* secType) override;
};

}; /* namespace SecreC */

#endif /* SECREC_CAST_TYPE_UNIFIER_H */
