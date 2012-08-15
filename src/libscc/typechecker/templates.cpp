/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "typechecker/templates.h"

#include <boost/foreach.hpp>

#include "ModuleInfo.h"
#include "treenode.h"

namespace SecreC {

/*******************************************************************************
  Instantiation
*******************************************************************************/

unsigned Instantiation::unrestrictedTemplateParamCount () const {
    unsigned count = 0;
    const TreeNodeTemplate* templ = getTemplate ()->decl ();
    BOOST_FOREACH (TreeNode* _quant, templ->quantifiers ()) {
        TreeNodeQuantifier* quant = static_cast<TreeNodeQuantifier*>(_quant);
        if (quant->kind () == 0)
            ++ count;
    }

    return count;
}

unsigned Instantiation::quantifiedDomainOccurrenceCount () const {
    unsigned count = 0;
    std::set<std::string > quantifiedDomains;
    const TreeNodeTemplate* templ = getTemplate ()->decl ();
    const TreeNodeProcDef* body = templ->body ();
    BOOST_FOREACH (TreeNode* _quant, templ->quantifiers ()) {
        TreeNodeQuantifier* quant = static_cast<TreeNodeQuantifier*>(_quant);
        quantifiedDomains.insert (quant->domain ()->value ());
    }

    BOOST_FOREACH (TreeNode* _d, body->paramRange ()) {
        assert (dynamic_cast<TreeNodeStmtDecl*>(_d) != 0);
        TreeNodeStmtDecl* d = static_cast<TreeNodeStmtDecl*>(_d);
        TreeNodeType* t = d->varType ();
        if (! t->secType ()->isPublic ()) {
            TreeNodeIdentifier* id = t->secType ()->identifier ();
            if (quantifiedDomains.find (id->value ()) != quantifiedDomains.end ()) {
                ++ count;
            }
        }
    }

    return count;
}


/*******************************************************************************
  TemplateInstantiator
*******************************************************************************/

/// \todo figure out how to delay copying of procedures even more (or completeley avoid
/// if possible).
const InstanceInfo& TemplateInstantiator::add (const Instantiation& i, ModuleInfo& mod) {
    std::map<const Instantiation, InstanceInfo >::iterator it = m_instanceInfo.find (i);
    if (it == m_instanceInfo.end ()) {
        TreeNode* cloned = i.getTemplate ()->decl ()->body ()->clone (0);
        InstanceInfo info;
        SymbolTable* local = mod.codeGenState ().st ()->newScope ();
        local->setName ("Template " + i.getTemplate ()->name ());
        m_workList.push_back (i);
        info.m_generatedBody = static_cast<TreeNodeProcDef*>(cloned);
        info.m_moduleInfo = &mod;
        info.m_localScope = local;
        it = m_instanceInfo.insert (it, std::make_pair (i, info));

        Instantiation::const_iterator secIt = i.begin ();
        BOOST_FOREACH (TreeNode* _quant, i.getTemplate ()->decl ()->quantifiers ()) {
            TreeNodeQuantifier* quant = static_cast<TreeNodeQuantifier*>(_quant);
            const std::string& qname = quant->domain ()->value ();
            SecurityType* argSecTy = *secIt;
            SymbolDomain* dom = new SymbolDomain (argSecTy);
            dom->setName (qname);
            local->appendSymbol (dom);
            ++ secIt;
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
            assert (info.m_generatedBody != 0);
            assert (info.m_moduleInfo != 0);
            assert (info.m_localScope != 0);
            m_generated.insert (i);
            return true;
        }
    }

    return false;
}

} // namespace SecreC
