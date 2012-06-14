/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "analysis/Dominators.h"

#include <boost/foreach.hpp>

#include "symbol.h"
#include "treenode.h"

namespace SecreC {

/*******************************************************************************
  Dominators
*******************************************************************************/

bool Dominators::visited (const Block& block) const {
    return m_num.find (&block)->second != 0;
}

/// \todo make this non-recursive
unsigned Dominators::dfs (const Block& entry, unsigned n) {
    if (!visited (entry)) {
        m_num[&entry] = ++ n;
        BOOST_FOREACH (Block::edge_type edge, entry.succ_range ()) {
            n = dfs (*edge.first, n);
        }
    }

    return n;
}

void Dominators::start (const Program &pr) {
    FOREACH_BLOCK (i, pr) {
        m_doms[&*i] = 0;
        m_num[&*i] = 0;
    }

    unsigned n = 0;
    BOOST_FOREACH (const Procedure& block, pr) {
        const Block* entry = block.entry ();
        n = dfs (*entry, n);
        m_doms[entry] = entry;
    }
}

void Dominators::startBlock (const Block&) {
    m_newIdom = 0;
}

const Block* Dominators::intersect (const Block* b1, const Block* b2) {
       while (b1 != b2) {
           while (m_num[b1] > m_num[b2]) b1 = m_doms[b1];
           while (m_num[b2] > m_num[b1]) b2 = m_doms[b2];
       }

       return b1;
}

void Dominators::inFrom (const Block& from, Edge::Label label, const Block&) {
    if (Edge::isGlobal (label))
        return;

    if (m_newIdom == 0) {
        m_newIdom = &from;
        return;
    }

    const Block* idom = m_doms[&from];
    if (idom != 0) {
        m_newIdom = intersect (m_newIdom, idom);
    }
}

bool Dominators::finishBlock (const Block& b) {
    if (m_newIdom != m_doms[&b]) {
        m_doms[&b] = m_newIdom;
        return true;
    }

    return false;
}

void Dominators::finish () { }

const Block* Dominators::idom (const Block* block) const {
    return m_doms.find (block)->second;
}

void Dominators::dominators (const Block* block, std::vector<const Block*>& doms) const {
    const Block* prev = block;
    doms.clear ();
    do {
        doms.push_back (block);
        prev = block;
        block = idom (block);
    } while (prev != block);
}

std::string Dominators::toString (const Program& pr) const {
    std::ostringstream ss;

    ss << "IDOMS:\n";
    FOREACH_BLOCK(i, pr) {
        const Block* dominator = m_doms.find (&*i)->second;
        ss << "  " << i->index () << " - " << (dominator ? dominator->index () : 0) << std::endl;
    }

    return ss.str ();
}

} // namespace SecreC
