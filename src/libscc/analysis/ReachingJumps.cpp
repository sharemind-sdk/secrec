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

#include "ReachingJumps.h"

#include <sstream>


namespace SecreC {

/*******************************************************************************
  ReachingJumps
*******************************************************************************/

void ReachingJumps::start(const Program &) {
    m_inPos.clear();
    m_inNeg.clear();
    m_outPos.clear();
    m_outNeg.clear();
}

void ReachingJumps::startBlock(const Block & b) {
    m_inNeg[&b].clear();
    m_inPos[&b].clear();
}

void ReachingJumps::inFrom(const Block & from, Edge::Label label, const Block & to) {
    if ((label & (Edge::Jump | Edge::CallPass)) != 0) {
        m_inNeg[&to] += m_outNeg[&from];
        m_inPos[&to] += m_outPos[&from];
    }

    if ((label & Edge::False) != 0) {
        const Imop & cjump = from.back();
        assert(cjump.isCondJump());
        Jumps inPosT = m_outPos[&from];
        inPosT.erase(&cjump);
        m_inPos[&to] += inPosT;
        m_inNeg[&to] += m_outNeg[&from];
        m_inNeg[&to].insert(&cjump);
    }

    if ((label & Edge::True) != 0) {
        const Imop & cjump = from.back();
        assert(cjump.isCondJump());
        Jumps inNegT = m_outNeg[&from];
        inNegT.erase(&cjump);
        m_inNeg[&to] += inNegT;
        m_inPos[&to] += m_outPos[&from];
        m_inPos[&to].insert(&cjump);
    }
}

bool ReachingJumps::finishBlock(const Block & b) {
    bool changed = false;

    if (m_inNeg[&b] != m_outNeg[&b]) {
        changed = true;
        m_outNeg[&b] = m_inNeg[&b];
    }

    if (m_inPos[&b] != m_outPos[&b]) {
        changed = true;
        m_outPos[&b] = m_inPos[&b];
    }

    return changed;
}

std::string ReachingJumps::toString(const Program & pr) const {
    std::ostringstream os;

    os << "Reaching jumps analysis results:" << std::endl;
    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable()) {
            continue;
        }

        os << "  Block " << bi->index() << ": ";

        auto posi = m_inPos.find(&*bi);
        auto negi = m_inNeg.find(&*bi);

        std::map<unsigned long, char> jumps;

        if (posi != m_inPos.end()) {
            for (const auto & elem : posi->second) {
                jumps.insert(std::make_pair(elem->index(), '+'));
            }

            if (negi != m_inNeg.end()) {
                for (const auto & elem : negi->second) {
                    auto kt = jumps.find(elem->index());

                    if (kt != jumps.end()) {
                        (*kt).second = '*';
                    }
                    else {
                        jumps.insert(std::make_pair(elem->index(), '-'));
                    }
                }
            }
        }
        else if (negi != m_inNeg.end()) {
            for (const auto & elem : negi->second) {
                jumps.insert(std::make_pair(elem->index(), '-'));
            }
        }

        if (jumps.empty()) {
            os << "NONE";
        }
        else {
            for (auto jt = jumps.begin(); jt != jumps.end(); ++ jt) {
                if (jt != jumps.begin()) {
                    os << ", ";
                }

                os << (*jt).first << (*jt).second;
            }
        }

        os << std::endl;
    }
    return os.str();
}

} // namespace SecreC
