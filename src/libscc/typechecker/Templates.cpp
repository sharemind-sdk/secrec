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

#include "typechecker/Templates.h"

#include "CastTemplateChecker.h"
#include "Log.h"
#include "ModuleInfo.h"
#include "OperatorTemplateChecker.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TemplateChecker.h"
#include "TreeNode.h"
#include "TypeChecker.h"

#include <memory>

namespace SecreC {

/*******************************************************************************
  TreeNodeTemplate
*******************************************************************************/

/**
 * Type check template definitions.
 * We do not perform two phase name lookup like C++ so at the pass over
 * template declarations we perform no type checking of the template body --
 * it only needs to parse.
 */
TypeChecker::Status TypeChecker::visitTemplate(TreeNodeTemplate * templ) {
    TreeNodeProcDef* body = templ->body ();
    TreeNodeIdentifier* id = body->identifier ();
    std::unique_ptr<TemplateVarChecker> varChecker;

    if (body->isOperator ()) {
        SecrecOperator op = static_cast<TreeNodeOpDef*> (body)->getOperator ();
        varChecker.reset (
            new OperatorTemplateVarChecker {m_st, m_log, op});
    }
    else if (body->isCast ()) {
        varChecker.reset (
            new CastTemplateVarChecker {m_st, m_log});
    }
    else {
        varChecker.reset (
            new TemplateVarChecker {m_st, m_log});
    }

    for (auto& quant : templ->quantifiers ()) {
        if (! varChecker->visitQuantifier (&quant))
            return E_TYPE;

        TCGUARD (visitQuantifier (&quant));
    }

    // We need to check return postion first in order for the parameters to
    // override the positional information of type variables.
    varChecker->setArgPosition (ArgReturn);
    if (! varChecker->visitType (body->returnType ()))
        return E_TYPE;

    varChecker->setArgPosition (ArgParameter);
    for (TreeNodeStmtDecl& decl : body->params ()) {
        if (! varChecker->visitType (decl.varType ())) {
            return E_TYPE;
        }
    }

    if (body->isOperator ()) {
        OperatorTemplateVarChecker* checker =
            static_cast<OperatorTemplateVarChecker*> (varChecker.get ());
        if (!checker->checkLUB (templ))
            return E_TYPE;
    }

    bool expectsSecType = false;
    bool expectsDataType = false;
    bool expectsDimType = false;

    std::vector<TreeNodeIdentifier*> unboundTVs;
    for (const auto& v : varChecker->vars ()) {
        auto& tv = v.second;
        if (! tv.bound) {
            unboundTVs.push_back (tv.id);
            continue;
        }

        if (tv.pos == ArgReturn) {
            if (tv.kind == TA_SEC) expectsSecType = true;
            if (tv.kind == TA_DATA) expectsDataType = true;
            if (tv.kind == TA_DIM) expectsDimType = true;
        }
    }

    if (! unboundTVs.empty()) {
        bool first = true;
        std::stringstream ss;
        for (TreeNodeIdentifier* id : unboundTVs) {
            if (! first)
                ss << ",";
            ss << " \'" << id->value () << "\' at " << id->location();
            first = false;
        }

        m_log.fatal() << "Template definition has free type variables:" << ss.str () << '.';
        return E_TYPE;
    }

    SymbolTemplate* s;
    if (body->isOperator () || body->isCast ())
        s = new SymbolOperatorTemplate (templ);
    else
        s = new SymbolProcedureTemplate (templ, expectsSecType, expectsDataType, expectsDimType);

    s->setName (id->value ());
    m_st->appendSymbol (s);
    return OK;
}

/*******************************************************************************
  TemplateInstantiator
*******************************************************************************/

/// \todo figure out how to delay copying of procedures even more (or completeley avoid
/// if possible).
const InstanceInfo& TemplateInstantiator::add (const Instantiation& i, ModuleInfo& mod) {
    auto it = m_instanceInfo.find (i);
    if (it == m_instanceInfo.end ()) {
        TreeNodeProcDef* cloned = static_cast<TreeNodeProcDef*>(i.getTemplate ()->decl ()->body ()->clone (nullptr));
        mod.body()->addGeneratedInstance(cloned);
        InstanceInfo info;
        SymbolTable* local = mod.codeGenState ().st ()->newScope ();
        local->setName ("Template " + i.getTemplate ()->name ());
        m_workList.push_back (i);
        info.m_generatedBody = cloned;
        info.m_moduleInfo = &mod;
        info.m_localScope = local;
        it = m_instanceInfo.insert (it, std::make_pair (i, info));

        auto it = i.getParams ().begin ();
        for (TreeNodeQuantifier& quant : i.getTemplate ()->decl ()->quantifiers ()) {
            StringRef qname = quant.typeVariable ()->value ();
            local->appendSymbol (it->bind (qname));
            ++ it;
        }
    }

    return it->second;
}

bool TemplateInstantiator::getForInstantiation (InstanceInfo& info) {
    while (! m_workList.empty ()) {
        Instantiation i = m_workList.front ();
        m_workList.pop_front ();
        if (! isInstantiated (i)) {
            info = m_instanceInfo[i];
            assert (info.m_generatedBody != nullptr);
            assert (info.m_moduleInfo != nullptr);
            assert (info.m_localScope != nullptr);
            m_generated.insert (i);
            return true;
        }
    }

    return false;
}

} // namespace SecreC
