/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "analysis/ReachingDefinitions.h"

#include "misc.h"
#include "symbol.h"
#include "treenode.h"

#include <boost/foreach.hpp>
#include <sstream>

namespace SecreC {

/*******************************************************************************
  ReachingDefinitions
*******************************************************************************/

void ReachingDefinitions::updateSDefs(const Imop & imop, ReachingDefinitions::SDefs & defs) {
    BOOST_FOREACH (const Symbol * symbol, imop.defRange()) {
        ReachingDefinitions::Defs & d = defs[symbol];
        d.clear();
        d.insert(&imop);
    }
}

void ReachingDefinitions::inFrom(const Block & from, const Block & to, bool globalOnly) {
    if (globalOnly) {
        BOOST_FOREACH (SDefs::const_reference r, m_outs[&from]) {
            const Symbol * s = r.first;

            if ((s->symbolType() == SYM_SYMBOL)
                    && static_cast<const SymbolSymbol *>(s)->scopeType() == SymbolSymbol::GLOBAL)
            {
                m_ins[&to][s] += r.second;
            }
        }
    }
    else {
        BOOST_FOREACH (SDefs::const_reference r, m_outs[&from]) {
            const Symbol * s = r.first;
            m_ins[&to][s] += r.second;
        }
    }
}

bool ReachingDefinitions::makeOuts(const Block & b, const SDefs & in, SDefs & out) {
    SDefs old = out;
    out = in;
    BOOST_FOREACH (const Imop& imop, b) {
        updateSDefs(imop, out);
    }

    return old != out;
}

std::string ReachingDefinitions::toString(const Program & pr) const {
    typedef Defs::const_iterator DCI;

    std::ostringstream os;

    os << "Reaching definitions analysis results:" << std::endl;
    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable()) {
            continue;
        }

        os << "  Block " << bi->index() << ": ";

        BDM::const_iterator si = m_ins.find(&*bi);

        if (si == m_ins.end() || (*si).second.empty()) {
            os << " NONE" << std::endl;
        }
        else {
            os << std::endl;
            BOOST_FOREACH (SDefs::const_reference sdef, si->second) {
                os << "      " << *sdef.first << ": ";
                const Defs & ds = sdef.second;

                for (DCI jt = ds.begin(); jt != ds.end(); ++ jt) {
                    if (jt != ds.begin()) {
                        os << ", ";
                    }

                    os << (*jt)->index();
                }

                os << std::endl;
            }
        }
    }

    return os.str();
}

} // namespace SecreC
