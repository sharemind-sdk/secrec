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
#include "symboltable.h"
#include "treenode.h"

namespace SecreC {

/*******************************************************************************
  Instantiation
*******************************************************************************/

unsigned Instantiation::unrestrictedTemplateParamCount () const {
    unsigned count = 0;
    const TreeNodeTemplate* templ = getTemplate ()->decl ();
    BOOST_FOREACH (TreeNodeDomainQuantifier& quant, templ->quantifiers ()) {
        if (quant.kind () == 0)
            ++ count;
    }

    return count;
}

unsigned Instantiation::quantifiedDomainOccurrenceCount () const {
    unsigned count = 0;
    std::set<StringRef, StringRef::FastCmp > quantifiedDomains;
    TreeNodeTemplate* templ = getTemplate ()->decl ();
    TreeNodeProcDef* body = templ->body ();
    BOOST_FOREACH (TreeNodeDomainQuantifier& quant, templ->quantifiers ()) {
        quantifiedDomains.insert (quant.domain ()->value ());
    }

    BOOST_FOREACH (TreeNodeStmtDecl& decl, body->params ()) {
        TreeNodeType* t = decl.varType ();
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
    InstanceInfoMap::iterator it = m_instanceInfo.find (i);
    if (it == m_instanceInfo.end ()) {
        TreeNodeProcDef* cloned = static_cast<TreeNodeProcDef*>(i.getTemplate ()->decl ()->body ()->clone (0));
        mod.body()->addGeneratedInstance(cloned);
        InstanceInfo info;
        SymbolTable* local = mod.codeGenState ().st ()->newScope ();
        local->setName ("Template " + i.getTemplate ()->name ());
        m_workList.push_back (i);
        info.m_generatedBody = cloned;
        info.m_moduleInfo = &mod;
        info.m_localScope = local;
        it = m_instanceInfo.insert (it, std::make_pair (i, info));

        Instantiation::const_iterator secIt = i.begin ();
        BOOST_FOREACH (TreeNodeDomainQuantifier& quant, i.getTemplate ()->decl ()->quantifiers ()) {
            StringRef qname = quant.domain ()->value ();
            SecurityType* argSecTy = *secIt;
            local->appendSymbol(new SymbolDomain(qname, argSecTy));
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
