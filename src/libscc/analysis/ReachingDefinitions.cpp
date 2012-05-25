/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "analysis/ReachingDefinitions.h"

#include <boost/foreach.hpp>

#include "symbol.h"
#include "treenode.h"

namespace SecreC {

/*******************************************************************************
  ReachingDefinitions
*******************************************************************************/

void ReachingDefinitions::inFrom(const Block &from, const Block &to, bool globalOnly) {
    if (!globalOnly) {
        for (SDefs::const_iterator jt = m_outs[&from].begin(); jt != m_outs[&from].end(); jt++) {
            m_ins[&to][(*jt).first].first += (*jt).second.first;
        }
    } else {
        for (SDefs::const_iterator jt = m_outs[&from].begin(); jt != m_outs[&from].end(); jt++) {
            const Symbol *s = (*jt).first;
            if ((s->symbolType() == Symbol::SYMBOL)
                && static_cast<const SymbolSymbol*>(s)->scopeType() == SymbolSymbol::GLOBAL)
            {
                m_ins[&to][s].first += (*jt).second.first;
            }
        }
    }
}

bool ReachingDefinitions::makeOuts(const Block &b, const SDefs &in, SDefs &out) {
    SDefs old = out;
    out = in;
    for (Block::const_iterator it = b.begin (); it != b.end (); it++) {
        const Imop& imop = *it;
        BOOST_FOREACH (const Symbol* symbol, imop.defRange ()) {
            Defs& d = out[symbol].first;
            d.clear ();
            d.insert (&imop);
        }
    }

    return old != out;
}

std::string ReachingDefinitions::toString(const Program &pr) const {
    typedef SDefs::const_iterator SDCI;
    typedef Defs::const_iterator DCI;

    std::ostringstream os;

    os << "Reaching definitions analysis results:" << std::endl;
    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable ()) continue;
        os << "  Block " << bi->index () << ": ";

        BDM::const_iterator si = m_ins.find(&*bi);
        if (si == m_ins.end() || (*si).second.empty()) {
            os << " NONE" << std::endl;
        } else {
            os << std::endl;
            for (SDCI it = (*si).second.begin(); it != (*si).second.end(); it++) {
                os << "      " << *(*it).first << ": ";
                const Defs &ds = (*it).second.first;
                for (DCI jt = ds.begin(); jt != ds.end(); jt++) {
                    if (jt != ds.begin()) os << ", ";
                    os << (*jt)->index();
                }
                os << std::endl;
            }
        }
    }
    return os.str();
}

} // namespace SecreC
