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

#include "ReachingDefinitions.h"

#include "../Misc.h"
#include "../Symbol.h"
#include "../TreeNode.h"

#include <sstream>


namespace SecreC {

/*******************************************************************************
  ReachingDefinitions
*******************************************************************************/

void ReachingDefinitions::start(const Program &pr) {
    m_ins.clear();
    m_outs.clear();
    makeOuts(*pr.entryBlock(), m_ins[pr.entryBlock()], m_outs[pr.entryBlock()]);
}

void ReachingDefinitions::updateSDefs(const Imop & imop, ReachingDefinitions::SDefs & defs) {
    for (const Symbol * symbol : imop.defRange()) {
        ReachingDefinitions::Defs & d = defs[symbol];
        d.clear();
        d.insert(&imop);
    }
}

void ReachingDefinitions::inFrom(const Block & from, const Block & to, bool globalOnly) {
    if (globalOnly) {
        for (SDefs::const_reference r : m_outs[&from]) {
            const Symbol * s = r.first;

            if ((s->symbolType() == SYM_SYMBOL)
                    && static_cast<const SymbolSymbol *>(s)->scopeType() == SymbolSymbol::GLOBAL)
            {
                m_ins[&to][s] += r.second;
            }
        }
    }
    else {
        for (SDefs::const_reference r : m_outs[&from]) {
            const Symbol * s = r.first;
            m_ins[&to][s] += r.second;
        }
    }
}

bool ReachingDefinitions::makeOuts(const Block & b, const SDefs & in, SDefs & out) {
    SDefs old = out;
    out = in;
    for (const Imop& imop : b) {
        updateSDefs(imop, out);
    }

    return old != out;
}

std::string ReachingDefinitions::toString(const Program & pr) const {
    std::ostringstream os;

    os << "Reaching definitions analysis results:" << std::endl;
    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable()) {
            continue;
        }

        os << "  Block " << bi->index() << ": ";

        auto si = m_ins.find(&*bi);

        if (si == m_ins.end() || (*si).second.empty()) {
            os << " NONE" << std::endl;
        }
        else {
            os << std::endl;
            for (SDefs::const_reference sdef : si->second) {
                os << "      " << *sdef.first << ": ";
                const Defs & ds = sdef.second;

                for (auto jt = ds.begin(); jt != ds.end(); ++ jt) {
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
