/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TemplateChecker.h"

#include "TreeNode.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "Log.h"
#include "Visitor.h"

namespace SecreC {

namespace /* anonymous */ {

const char* kindAsString (TypeArgumentKind kind) {
    switch (kind) {
    case TA_SEC: return "domain";
    case TA_DATA: return "data";
    case TA_DIM: return "dimensionality";
    }
}

SymbolCategory symbolCategory (TypeArgumentKind kind) {
    switch (kind) {
    case TA_SEC: return SYM_DOMAIN;
    case TA_DATA: return SYM_TYPE;
    case TA_DIM: return SYM_DIM;
    }
}

} // namespace anonymous

/*******************************************************************************
  TemplateChecker
*******************************************************************************/

bool TemplateVarChecker::visit (TreeNodeIdentifier* id, TypeArgumentKind kind) {
    const StringRef name = id->value ();
    const auto it = m_vars.find (name);
    if (it != m_vars.end ()) {
        TemplateTypeVar& tv = it->second;
        if (tv.kind != kind) {
            m_log.fatal () << "Unexpected " << kindAsString (kind)
                           << " type variable \'" << name
                           << "\' at " << id->location () << ". "
                           << "Expecting " <<  kindAsString (tv.kind)
                           << " type variable.";
            return false;
        }

        tv.bound = true;
        tv.pos = m_pos;
    }
    else {
        Symbol* sym = m_st->find (symbolCategory (kind), name);
        if (sym == nullptr) {
            m_log.fatal () << "Unable to find " << kindAsString (kind)
                           << " type variable \'" << name
                           << "\' at " << id->location () << ". ";
            return false;
        }
    }

    return true;
}

bool TemplateVarChecker::verifyKind (TypeArgumentKind expected,
                                     TypeArgumentKind given,
                                     const Location& loc)
{
    if (expected != given) {
        m_log.fatal () << "Unexpected " << kindAsString (given)
                       << " type at " << loc << ". "
                       << "Expecting " << kindAsString (expected) << " type.";
        return false;
    }

    return true;
}

bool TemplateVarChecker::visitQuantifier (TreeNodeQuantifier* q) {
    const StringRef name = q->typeVariable ()->value ();
    auto it = m_vars.find (name);
    if (it != m_vars.end ()) {
        m_log.fatal () << "Redeclaration of a type variable \'" << name << '\''
                       << " at " << q->location () << '.';
        return false;
    }

    m_vars.insert (it, std::make_pair (name,
        TemplateTypeVar (q->typeVariable (), quantifierKind (*q))));
    return true;
}

bool TemplateVarChecker::visitType (TreeNodeType* t) {
    assert (t != nullptr);

    if (m_pos == ArgParameter && ! t->isNonVoid ()) {
        m_log.fatal () << "Void type a function argument at " << t->location () << ".";
        return false;
    }

    if (t->isNonVoid ()) {
        return visitSecTypeF (t->secType (), TA_SEC) &&
               visitDataTypeF (t->dataType (), TA_DATA) &&
               visitDimTypeF (t->dimType (), TA_DIM);
    }

    return true;
}

bool TemplateVarChecker::visitSecTypeF (TreeNodeSecTypeF* t, TypeArgumentKind kind) {
    assert (t != nullptr);

    if (! verifyKind (TA_SEC, kind, t->location ()))
        return false;

    if (t->isPublic ())
        return true;

    return visit (t->identifier (), TA_SEC);
}

bool TemplateVarChecker::visitDataTypeF (TreeNodeDataTypeF* t, TypeArgumentKind kind) {
    assert (t != nullptr);

    if (! verifyKind (TA_DATA, kind, t->location ()))
        return false;

    return dispatchDataTypeF (*this, t);
}

// TODO: duplicate code much...
// We should unify regular type type checking to this class...
bool TemplateVarChecker::visitDataTypeTemplateF (TreeNodeDataTypeTemplateF* t) {
    assert (t != nullptr);

    const StringRef name = t->identifier ()->value ();
    SymbolStruct* sym = m_st->find<SYM_STRUCT>(name);
    if (sym == nullptr) {
        m_log.fatal () << "Invalid structure name at " << t->identifier ()->location () << ".";
        return false;
    }

    TreeNodeStructDecl* structDecl = sym->decl ();
    auto quants = structDecl->quantifiers ();
    auto args = t->arguments ();

    if (quants.size () != args.size ()) {
        m_log.fatal () << "Invalid number of type arguments at " << t->location () << ".";
        return false;
    }

    for (size_t i = 0; i < args.size (); ++ i) {
        TreeNodeTypeArg& arg = args[i];
        TypeArgumentKind kind = quantifierKind (quants[i]);
        if (! visitTypeArg (&arg, kind))
            return false;

        // TODO: mismatching domain quantifier?
    }

    return true;
}

bool TemplateVarChecker::visitDataTypeVarF (TreeNodeDataTypeVarF* t) {
    return visit (t->identifier (), TA_DATA);
}

bool TemplateVarChecker::visitDataTypeConstF (TreeNodeDataTypeConstF*) {
    return true;
}

bool TemplateVarChecker::visitDimTypeF (TreeNodeDimTypeF* t, TypeArgumentKind kind) {
    if (! verifyKind (TA_DIM, kind, t->location ()))
        return false;

    return dispatchDimTypeF (*this, t);
}

bool TemplateVarChecker::visitDimTypeVarF (TreeNodeDimTypeVarF* t) {
    return visit (t->identifier (), TA_DIM);
}

bool TemplateVarChecker::visitDimTypeConstF (TreeNodeDimTypeConstF*) {
    return true;
}

bool TemplateVarChecker::visitTypeArg (TreeNodeTypeArg* t, TypeArgumentKind kind) {
    return dispatchTypeArg (*this, t, kind);
}

bool TemplateVarChecker::visitTypeArgVar (TreeNodeTypeArgVar* t, TypeArgumentKind kind) {
    return visit (t->identifier (), kind);
}

bool TemplateVarChecker::visitTypeArgTemplate (TreeNodeTypeArgTemplate* t, TypeArgumentKind kind) {
    assert (t != nullptr);

    if (! verifyKind (TA_DATA, kind, t->location ()))
        return false;

    const StringRef name = t->identifier ()->value ();
    SymbolStruct* sym = m_st->find<SYM_STRUCT>(name);
    if (sym == nullptr) {
        m_log.fatal () << "Invalid structure name at " << t->identifier ()->location () << ".";
        return false;
    }

    TreeNodeStructDecl* structDecl = sym->decl ();
    auto quants = structDecl->quantifiers ();
    auto args = t->arguments ();

    if (quants.size () != args.size ()) {
        m_log.fatal () << "Invalid number of type arguments at " << t->location () << ".";
        return false;
    }

    for (size_t i = 0; i < args.size (); ++ i) {
        TreeNodeTypeArg& arg = args[i];
        TypeArgumentKind kind = quantifierKind (quants[i]);
        if (! visitTypeArg (&arg, kind))
            return false;

        // TODO: mismatching domain quantifier?
    }

    return true;
}

bool TemplateVarChecker::visitTypeArgDataTypeConst (TreeNodeTypeArgDataTypeConst* t, TypeArgumentKind kind) {
    assert (t != nullptr);
    return verifyKind (TA_DATA, kind, t->location ());
}

bool TemplateVarChecker::visitTypeArgDimTypeConst (TreeNodeTypeArgDimTypeConst* t, TypeArgumentKind kind) {
    assert (t != nullptr);
    return verifyKind (TA_DIM, kind, t->location ());
}

bool TemplateVarChecker::visitTypeArgPublic (TreeNodeTypeArgPublic* t, TypeArgumentKind kind) {
    assert (t != nullptr);
    return verifyKind (TA_SEC, kind, t->location ());
}

} // namespace SecreC
