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

#ifndef SECREC_CAST_TEMPLATE_CHECKER_H
#define SECREC_CAST_TEMPLATE_CHECKER_H

#include "TemplateChecker.h"

namespace SecreC {

/**
 * @brief Type checking of cast definition template.
 */
class CastTemplateVarChecker: public TemplateVarChecker {

public: /* Methods: */

    explicit CastTemplateVarChecker (SymbolTable * st, CompileLog & log)
        : TemplateVarChecker (st, log)
        , m_seenDomainVar (false)
    { }

    virtual bool visitQuantifier (TreeNodeQuantifier* q) override;

    virtual bool visitType (TreeNodeType* t) override;

    virtual bool visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t) override;

    virtual bool visitDataTypeConstF (TreeNodeDataTypeConstF* t) override;

    virtual bool visitDimTypeConstF (TreeNodeDimTypeConstF* t) override;

private: /* Methods: */

    void badType (TreeNode* t);

private: /* Fields: */

    bool m_seenDomainVar;
};

} /* namespace SecreC */

#endif /* SECREC_CAST_TEMPLATE_CHECKER_H */
