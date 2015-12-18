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

#ifndef SECREC_TEMPLATE_CHECKER_H
#define SECREC_TEMPLATE_CHECKER_H

#include "StringRef.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNodeFwd.h"
#include "TypeArgument.h"

#include <map>

namespace SecreC {

class SymbolTable;
class CompileLog;
class Location;

/*******************************************************************************
  TemplateChecker
*******************************************************************************/

enum TemplateArgPos {
    ArgParameter,
    ArgReturn
};

struct TemplateTypeVar {
    TreeNodeIdentifier* id;
    TypeArgumentKind    kind;
    bool                bound;
    TemplateArgPos      pos;

    TemplateTypeVar (TreeNodeIdentifier* id, TypeArgumentKind kind)
        : id (id)
        , kind (kind)
        , bound (false)
        , pos (ArgParameter)
    { }
};

/**
 * @brief Type checking of template function declaration.
 */
class TemplateVarChecker {
public: /* Types: */
    using TemplateVarMap = std::map<StringRef, TemplateTypeVar, StringRef::FastCmp>;

public:

    using result_type = bool;

public: /* Methods: */

    explicit TemplateVarChecker (SymbolTable* st, CompileLog& log)
        : m_st (st)
        , m_log (log)
        , m_pos (ArgParameter)
    { }

    TemplateVarChecker (const TemplateVarChecker&) = delete;
    TemplateVarChecker& operator = (const TemplateVarChecker&) = delete;
    TemplateVarChecker (TemplateVarChecker&&) = default;
    TemplateVarChecker& operator = (TemplateVarChecker&&) = default;

    virtual bool visit (TreeNodeIdentifier* id, TypeArgumentKind kind);

    virtual bool visitQuantifier (TreeNodeQuantifier* q);

    virtual bool visitType (TreeNodeType* t);

    virtual bool visitSecTypeF (TreeNodeSecTypeF* t, TypeArgumentKind kind);

    bool visitDataTypeF (TreeNodeDataTypeF* t, TypeArgumentKind kind);
    virtual bool visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t);
    bool visitDataTypeVarF (TreeNodeDataTypeVarF* t);
    virtual bool visitDataTypeConstF (TreeNodeDataTypeConstF* t);

    bool visitDimTypeF (TreeNodeDimTypeF* t, TypeArgumentKind kind);
    bool visitDimTypeVarF (TreeNodeDimTypeVarF* t);
    virtual bool visitDimTypeConstF (TreeNodeDimTypeConstF* t);

    bool visitTypeArg (TreeNodeTypeArg* t, TypeArgumentKind kind);
    bool visitTypeArgVar (TreeNodeTypeArgVar* t, TypeArgumentKind kind);
    bool visitTypeArgTemplate (TreeNodeTypeArgTemplate* t, TypeArgumentKind kind);
    bool visitTypeArgDataTypeConst (TreeNodeTypeArgDataTypeConst* t, TypeArgumentKind kind);
    bool visitTypeArgDimTypeConst (TreeNodeTypeArgDimTypeConst* t, TypeArgumentKind kind);
    bool visitTypeArgPublic (TreeNodeTypeArgPublic* t, TypeArgumentKind kind);

    const TemplateVarMap& vars () const { return m_vars; }

    void setArgPosition (TemplateArgPos pos) { m_pos = pos; }

protected:

    bool verifyKind (TypeArgumentKind expected,
                     TypeArgumentKind given,
                     const Location& loc);

protected: /* Fields: */
    SymbolTable*   m_st;
    CompileLog&    m_log;
    TemplateVarMap m_vars;
    TemplateArgPos m_pos;
};

const char* kindAsString (TypeArgumentKind kind);

SymbolCategory symbolCategory (TypeArgumentKind kind);

} /* namespace SecreC */

#endif /* SECREC_TEMPLATE_CHECKER_H */