/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "typechecker/templates.h"

namespace SecreC {

/*******************************************************************************
  TemplateInstantiator
*******************************************************************************/

/// \todo figure out how to delay copying of procedures even more (or completeley avoid
/// if possible).
TreeNodeProcDef* TemplateInstantiator::add (const Instantiation& i, SymbolTable* st) {
    std::map<const Instantiation, InstanceInfo >::iterator it = m_instanceInfo.find (i);
    std::cerr << "add: " << i << std::endl;
    if (it == m_instanceInfo.end ()) {
        TreeNode* cloned = i.getTemplate ()->decl ()->body ()->clone (0);
        InstanceInfo info;
        SymbolTable* local = st;

        m_workList.push_back (i);

        /// \todo not exactly perfect solution...
        do {
            local = local->parent ();
        } while (local->parent () != 0);
        local = local->newScope ();

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
