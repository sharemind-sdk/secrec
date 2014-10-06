/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_TEMPLATE_CHECKER_H
#define SECREC_TEMPLATE_CHECKER_H

#include "StringRef.h"
#include "TypeArgument.h"
#include "TreeNodeFwd.h"

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

    bool visit (TreeNodeIdentifier* id, TypeArgumentKind kind);

    bool visitQuantifier (TreeNodeQuantifier* q);

    bool visitType (TreeNodeType* t);

    bool visitSecTypeF (TreeNodeSecTypeF* t, TypeArgumentKind kind);

    bool visitDataTypeF (TreeNodeDataTypeF* t, TypeArgumentKind kind);
    bool visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t);
    bool visitDataTypeVarF (TreeNodeDataTypeVarF* t);
    bool visitDataTypeConstF (TreeNodeDataTypeConstF* t);

    bool visitDimTypeF (TreeNodeDimTypeF* t, TypeArgumentKind kind);
    bool visitDimTypeVarF (TreeNodeDimTypeVarF* t);
    bool visitDimTypeConstF (TreeNodeDimTypeConstF* t);

    bool visitTypeArg (TreeNodeTypeArg* t, TypeArgumentKind kind);
    bool visitTypeArgVar (TreeNodeTypeArgVar* t, TypeArgumentKind kind);
    bool visitTypeArgTemplate (TreeNodeTypeArgTemplate* t, TypeArgumentKind kind);
    bool visitTypeArgDataTypeConst (TreeNodeTypeArgDataTypeConst* t, TypeArgumentKind kind);
    bool visitTypeArgDimTypeConst (TreeNodeTypeArgDimTypeConst* t, TypeArgumentKind kind);
    bool visitTypeArgPublic (TreeNodeTypeArgPublic* t, TypeArgumentKind kind);

    const TemplateVarMap& vars () const { return m_vars; }
    TemplateVarMap& vars () { return m_vars; }

    void setArgPosition (TemplateArgPos pos) { m_pos = pos; }

private:

    bool verifyKind (TypeArgumentKind expected,
                     TypeArgumentKind given,
                     const Location& loc);

private: /* Fields: */
    SymbolTable*   m_st;
    CompileLog&    m_log;
    TemplateVarMap m_vars;
    TemplateArgPos m_pos;
};

} /* namespace SecreC */

#endif /* SECREC_TEMPLATE_CHECKER_H */
