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

#include "TemplateChecker.h"

#include <sharemind/abort.h>
#include "Log.h"
#include "TreeNode.h"
#include "Visitor.h"


namespace SecreC {

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

    if (SymbolDataType* sym = m_st->find<SYM_TYPE>(name)) {
        m_log.fatal() << "Template variable \'" << name
                      << "\' overshadows previously definition of "
                      <<  *sym << " at " << q->location () << '.';
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

const char* kindAsString (TypeArgumentKind kind) {
    switch (kind) {
    case TA_SEC: return "domain";
    case TA_DATA: return "data";
    case TA_DIM: return "dimensionality";
    #ifdef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcovered-switch-default"
    #endif
    default: SHAREMIND_ABORT("kAS %d", static_cast<int>(kind));
    #ifdef __clang__
    #pragma GCC diagnostic pop
    #endif
    }
}

SymbolCategory symbolCategory (TypeArgumentKind kind) {
    switch (kind) {
    case TA_SEC: return SYM_DOMAIN;
    case TA_DATA: return SYM_TYPE;
    case TA_DIM: return SYM_DIM;
    #ifdef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcovered-switch-default"
    #endif
    default: SHAREMIND_ABORT("sC %d", static_cast<int>(kind));
    #ifdef __clang__
    #pragma GCC diagnostic pop
    #endif
    }
}

} // namespace SecreC
