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
 * @brief Type checking of template operator declaration.
 */
class OperatorTemplateVarChecker: public TemplateVarChecker {

public: /* Methods: */

    explicit OperatorTemplateVarChecker (SymbolTable* st, CompileLog& log, SecrecOperator op)
        : TemplateVarChecker (st, log)
        , m_op (op)
        , m_seenDataTypeVar (false)
        , m_seenDomainVar (false)
    { }

    virtual bool visit (TreeNodeIdentifier* id, TypeArgumentKind kind) override;

    virtual bool visitQuantifier (TreeNodeQuantifier* q) override;

    virtual bool visitType (TreeNodeType* t) override;

    virtual bool visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t) override;

    virtual bool visitDataTypeConstF (TreeNodeDataTypeConstF* t) override;

    virtual bool visitSecTypeF (TreeNodeSecTypeF* t, TypeArgumentKind kind) override;

    virtual bool visitDimTypeConstF (TreeNodeDimTypeConstF* t) override;

    bool checkLUB (TreeNodeTemplate* templ);

private: /* Methods: */

    const char* thing ();

    void badType (TreeNode* t);

private: /* Fields: */

    SecrecOperator m_op;
    bool m_seenDataTypeVar;
    bool m_seenDomainVar;
};

} /* namespace SecreC */

#endif /* SECREC_OPERATOR_TEMPLATE_CHECKER_H */
