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

#include "analysis/ReachableReturns.h"

#include "TreeNode.h"

#include <boost/range/adaptor/reversed.hpp>
#include <sstream>

using boost::adaptors::reverse;

namespace SecreC {

void ReachableReturns::update(const Imop& imop, Returns& rets) {
    if (imop.type() == Imop::RETURN)
        rets.insert(&imop);
}

void ReachableReturns::start(const Program& pr) {
    m_ins.clear();
    m_outs.clear();

    FOREACH_BLOCK (bi, pr) {
        const Block& block = *bi;
        Returns& in = m_ins[&block];
        m_outs[&block];
        for (const Imop& imop : reverse(block)) {
            update(imop, in);
        }
    }
}

void ReachableReturns::startBlock(const Block&) { }

void ReachableReturns::outTo(const Block& from, Edge::Label label, const Block& to) {
    if (Edge::isGlobal(label))
        return;

    Returns& rets = m_ins[&from];
    m_outs[&to].insert(rets.begin(), rets.end());
}

bool ReachableReturns::finishBlock(const Block& block) {
    Returns old = m_ins[&block];
    Returns& in = m_ins[&block];
    in.insert(m_outs[&block].begin(), m_outs[&block].end());
    return in != old;
}

void ReachableReturns::finish() { }

std::string ReachableReturns::toString(const Program& pr) const {
    std::ostringstream os;

    os << "Reachable returns analysis results:" << std::endl;
    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable ()) {
            continue;
        }

        os << "  Block " << bi->index () << ": ";

        auto si = m_ins.find (&*bi);

        if (si == m_ins.end () || (*si).second.empty ()) {
            os << " NONE" << std::endl;
        } else {
            os << std::endl;
            for (const Imop* ret : si->second) {
                os << "      " << *ret;
                if (ret->creator () != nullptr) {
                    os << " // Created by " << ret->creator ()->location ();
                }
                os << std::endl;
            }
        }
    }

    return os.str ();
}

} // namespace SecreC
