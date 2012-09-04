/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "analysis/ReachingJumps.h"

#include <sstream>

namespace SecreC {

/*******************************************************************************
  ReachingJumps
*******************************************************************************/

void ReachingJumps::start(const Program &) { }

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
    typedef std::set<const Imop *>::const_iterator ISCI;
    typedef std::map<unsigned long, char>::iterator       LCMI;
    typedef std::map<unsigned long, char>::const_iterator LCMCI;
    typedef BJM::const_iterator BJMCI;

    std::ostringstream os;

    os << "Reaching jumps analysis results:" << std::endl;
    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable()) {
            continue;
        }

        os << "  Block " << bi->index() << ": ";

        BJMCI posi = m_inPos.find(&*bi);
        BJMCI negi = m_inNeg.find(&*bi);

        std::map<unsigned long, char> jumps;

        if (posi != m_inPos.end()) {
            for (ISCI jt = (*posi).second.begin(); jt != (*posi).second.end(); jt++) {
                jumps.insert(std::make_pair((*jt)->index(), '+'));
            }

            if (negi != m_inNeg.end()) {
                for (ISCI jt = (*negi).second.begin(); jt != (*negi).second.end(); jt++) {
                    LCMI kt = jumps.find((*jt)->index());

                    if (kt != jumps.end()) {
                        (*kt).second = '*';
                    }
                    else {
                        jumps.insert(std::make_pair((*jt)->index(), '-'));
                    }
                }
            }
        }
        else if (negi != m_inNeg.end()) {
            for (ISCI jt = (*negi).second.begin(); jt != (*negi).second.end(); jt++) {
                jumps.insert(std::make_pair((*jt)->index(), '-'));
            }
        }

        if (jumps.empty()) {
            os << "NONE";
        }
        else {
            for (LCMCI jt = jumps.begin(); jt != jumps.end(); jt++) {
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
