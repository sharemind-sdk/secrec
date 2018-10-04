/*
 * Copyright (C) 2016 Cybernetica
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

#include "ReachableDefinitions.h"
#include "BoostInsertWorkaround.h"

#include "../Symbol.h"
#include "../TreeNode.h"

#include <boost/interprocess/containers/flat_map.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <sstream>


using boost::adaptors::reverse;

namespace SecreC {

void ReachableDefinitions::update(const Imop& imop, Definitions& defs) {
    for (const Symbol* sym : imop.defRange ()) {
        // Erase previous definition
        for (auto it = defs.begin (); it != defs.end (); ++it) {
            const Imop* old = *it;
            bool found = false;
            for (const Symbol* oldSym : old->defRange ()) {
                if (oldSym == sym) {
                    found = true;
                    break;
                }
            }
            if (found) {
                defs.erase (it);
                break;
            }
        }
    }

    defs.insert (&imop);
}

void ReachableDefinitions::start(const Program& pr) {
    m_ins.clear();
    m_outs.clear();

    FOREACH_BLOCK (bi, pr) {
        const Block& block = *bi;
        Definitions& in = m_ins[&block];
        m_outs[&block];

        boost::container::flat_map<const Symbol*, const Imop*> defs;

        for (const Imop& imop : reverse(block)) {
            for (const Symbol* s : imop.defRange ()) {
                defs[s] = &imop;
            }
        }

        for (auto& it : defs) {
            in.insert (it.second);
        }
    }
}

void ReachableDefinitions::startBlock(const Block&) { }

void ReachableDefinitions::outTo(const Block& from, Edge::Label label, const Block& to) {
    if (Edge::isGlobal(label)) {
        for (const Imop* def : m_ins[&from]) {
            bool global = false;
            for (const Symbol* s : def->defRange()) {
                if (s->isGlobal()) {
                    global = true;
                    break;
                }
            }
            if (global)
                m_outs[&to].insert(def);
        }
    } else {
        const Definitions& defs = m_ins[&from];
        insertWorkaround(m_outs[&to], defs.begin(), defs.end());
    }
}

bool ReachableDefinitions::finishBlock(const Block& block) {
    const Definitions old = m_ins[&block];
    Definitions& in = m_ins[&block];
    const Definitions& out = m_outs[&block];
    insertWorkaround(in, out.begin(), out.end());
    return in != old;
}

void ReachableDefinitions::finish() { }

std::string ReachableDefinitions::toString(const Program& pr) const {
    std::stringstream ss;
    ss << "Reachable definitions:\n";

    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable ()) {
            continue;
        }

        ss << "  Block " << bi->index () << ": ";

        auto si = m_ins.find (&*bi);

        if (si == m_ins.end () || (*si).second.empty ()) {
            ss << " NONE" << std::endl;
        } else {
            ss << std::endl;
            for (const Imop* def : si->second) {
                ss << "      " << *def;
                if (def->creator () != nullptr) {
                    ss << " // Created by " << def->creator ()->location ();
                }
                ss << std::endl;
            }
        }
    }

    return ss.str();
}

} // namespace SecreC
