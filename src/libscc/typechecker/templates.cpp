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
TreeNodeProcDef* TemplateInstantiator::add (const Instantiation& i, SymbolTable* st) {
    std::map<const Instantiation, InstanceInfo >::iterator it = m_instanceInfo.find (i);
    if (it == m_instanceInfo.end ()) {
        TreeNode* cloned = i.getTemplate ()->decl ()->body ()->clone (0);
        InstanceInfo info;
        SymbolTable* local = st->globalScope ()->newScope ();
        m_workList.push_back (i);
        info.m_generatedBody = static_cast<TreeNodeProcDef*>(cloned);
        info.m_localSymbolTable = local;
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

    return it->second.m_generatedBody;
}

bool TemplateInstantiator::getForInstantiation (TreeNodeProcDef*& proc, SymbolTable*& st) {
    while (! m_workList.empty ()) {
        Instantiation i = m_workList.front ();
        m_workList.pop_front ();
        if (! isInstantiated (i)) {
            m_generated.insert (i);
            proc = m_instanceInfo[i].m_generatedBody;
            st = m_instanceInfo[i].m_localSymbolTable;
            return true;
        }
    }

    return false;
}

} // namespace SecreC
