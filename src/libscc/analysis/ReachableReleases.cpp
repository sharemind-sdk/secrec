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

#include "analysis/ReachableReleases.h"

#include "Misc.h"
#include "Symbol.h"
#include "TreeNode.h"

#include <boost/range/adaptor/reversed.hpp>
#include <sstream>

using boost::adaptors::reverse;

namespace SecreC {

namespace { /* anonymous */

using namespace SecreC;

struct CollectGenKill {

    CollectGenKill(ReachableReleases::Values & gen, ReachableReleases::Symbols & kill)
        : m_gen(gen), m_kill(kill)
    { }

    inline void gen(const Symbol * sym, const Imop & imop) {
        m_gen[sym].insert(&imop);
    }

    inline void kill(const Symbol * sym) {
        m_kill.insert(sym);
    }

    ReachableReleases::Values  & m_gen;
    ReachableReleases::Symbols & m_kill;
};

struct UpdateValues {
    UpdateValues(ReachableReleases::Values & vs)
        : m_values(vs)
    { }

    inline void gen(const Symbol * sym, const Imop & imop) {
        m_values[sym].insert(&imop);
    }

    inline void kill(const Symbol * sym) {
        m_values.erase(sym);
    }

    ReachableReleases::Values & m_values;
};

template <class Visitor>
void visitImop(const Imop & imop, Visitor & visitor) {
    for (const Symbol * dest : imop.defRange()) {
        visitor.kill(dest);
    }

    if (imop.type() == Imop::RELEASE) {
        visitor.gen(imop.arg1(), imop);
    }

    if (imop.type() == Imop::RETURN) {
        for (const Symbol * arg : imop.useRange()) {
            if (arg->isArray()) {
                visitor.gen(arg, imop);
            }
        }
    }
}

} // namespace anonymous

/*******************************************************************************
  ReachableReleases
*******************************************************************************/

void ReachableReleases::update(const Imop & imop, Values & vals) {
    UpdateValues visitor(vals);
    visitImop(imop, visitor);
}

void ReachableReleases::start(const Program & pr) {
    m_gen.clear();
    m_kill.clear();
    m_ins.clear();
    m_outs.clear();

    FOREACH_BLOCK (bi, pr) {
        CollectGenKill collector(m_gen[&*bi], m_kill[&*bi]);
        for (const Imop& imop : reverse (*bi)) {
            visitImop(imop, collector);
        }
    }
}

void ReachableReleases::outToLocal(const Block & from, const Block & to) {
    Values & dest = m_outs[&to];
    const Values & src = m_ins[&from];

    for (Values::const_reference sv : src) {
        dest[sv.first] += sv.second;
    }
}

void ReachableReleases::outToGlobal(const Block & from, const Block & to) {
    Values & dest = m_outs[&to];
    const Values & src = m_ins[&from];

    for (Values::const_reference sv : src) {
        if (sv.first->isGlobal()) {
            dest[sv.first] += sv.second;
        }
    }
}

void ReachableReleases::startBlock(const Block & b) {
    m_outs[&b].clear();
}

bool ReachableReleases::finishBlock(const Block & b) {
    Values & in = m_ins[&b];
    const Values old = in;
    const Values & out = m_outs[&b];
    in = out;

    for (Symbols::const_reference sym : m_kill[&b]) {
        in.erase(sym);
    }

    for (Values::const_reference sv : m_gen[&b]) {
        in[sv.first] += sv.second;
    }

    return old != in;
}


std::string ReachableReleases::toString(const Program & pr) const {
    std::stringstream ss;
    ss << "Reachable releases:\n";
    FOREACH_BLOCK (bi, pr) {
        Values after;
        auto i = m_outs.find(&*bi);

        if (i != m_outs.end()) {
            after = i->second;
        }

        for (const Imop & imop : reverse (*bi)) {
            for (const Symbol * dest : imop.defRange()) {
                if (! dest->isArray()) {
                    continue;
                }

                ss << imop.index() << ": " << imop << " // " << imop.creator()->location() << "\n";
                for (const Imop * release : after[dest]) {
                    ss << '\t' << release->index() << ": " << *release << '\n';
                }
            }

            update(imop, after);
        }
    }

    return ss.str();
}

} // namespace SecreC
