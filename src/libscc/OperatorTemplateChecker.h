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

#ifndef SECREC_OPERATOR_TEMPLATE_CHECKER_H
#define SECREC_OPERATOR_TEMPLATE_CHECKER_H

#include "ParserEnums.h"
#include "TemplateChecker.h"

namespace SecreC {

/**
 * @brief Type checking of operator definition template.
 */
class OperatorTemplateVarChecker: public TemplateVarChecker {

public: /* Methods: */

    explicit OperatorTemplateVarChecker(TypeChecker & typeChecker, SymbolTable* st, CompileLog& log)
        : TemplateVarChecker(typeChecker, st, log)
        , m_seenDomainVar (false)
    { }

    virtual bool visitQuantifier (TreeNodeQuantifier* q) override;

    virtual bool visitType (TreeNodeType* t) override;

    virtual bool visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t) override;

    virtual bool visitDataTypeConstF (TreeNodeDataTypeConstF* t) override;

    virtual bool visitDimTypeConstF (TreeNodeDimTypeConstF* t) override;

    virtual bool visitDimTypeZeroF (TreeNodeDimTypeZeroF* t) override;

    bool checkLUB (TreeNodeTemplate* templ);

private: /* Methods: */

    void badType (TreeNode* t);

private: /* Fields: */

    bool m_seenDomainVar;
};

} /* namespace SecreC */

#endif /* SECREC_OPERATOR_TEMPLATE_CHECKER_H */
