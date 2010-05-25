#include "dataflowanalysis.h"

#include <algorithm>
#include <utility>
#include "intermediate.h"
#include "misc.h"
#include "treenode.h"

namespace SecreC {
namespace {

template <class T>
inline std::set<T> &operator+=(std::set<T> &dest, const std::set<T> &src) {
    dest.insert(src.begin(), src.end());
    return dest;
}

#define FOREACH_BLOCKS(it,bs) for (std::set<Block*>::const_iterator it = bs.begin(); it != bs.end(); it++)
#define FOREACH_FORWARDS(it) for (std::set<ForwardDataFlowAnalysis*>::const_iterator it = m_forwards.begin(); it != m_forwards.end(); it++)
#define FOREACH_BACKWARDS(it) for (std::set<BackwardDataFlowAnalysis*>::const_iterator it = m_backwards.begin(); it != m_backwards.end(); it++)

} // anonymous namespace


void DataFlowAnalysisRunner::run(const Blocks &bs) {
    typedef std::set<Block*>::const_iterator BSCI;
    bool changed = false;

    const Block &entryBlock = bs.entryBlock();
    FOREACH_FORWARDS(it)
        (*it)->start(entryBlock);

    do {
        changed = false;

        // For all reachable blocks:
        for (size_t i = 1; i < bs.size(); i++) {
            const Block *b = bs[i];
            if (!b->reachable) continue;

            FOREACH_FORWARDS(it)
                (*it)->startBlock(*b);

            // Recalculate input sets:
            FOREACH_BLOCKS(it,b->predecessors)
                FOREACH_FORWARDS(a)
                    (*a)->inFrom(**it, *b);

            FOREACH_BLOCKS(it,b->predecessorsCondFalse)
                FOREACH_FORWARDS(a)
                    (*a)->inFromFalse(**it, *b);

            FOREACH_BLOCKS(it,b->predecessorsCondTrue)
                FOREACH_FORWARDS(a)
                    (*a)->inFromTrue(**it, *b);

            FOREACH_BLOCKS(it,b->predecessorsCall)
                FOREACH_FORWARDS(a)
                    (*a)->inFromCall(**it, *b);

            FOREACH_BLOCKS(it,b->predecessorsRet)
                FOREACH_FORWARDS(a)
                    (*a)->inFromRet(**it, *b);

            if (b->callPassFrom != 0)
                FOREACH_FORWARDS(a)
                    (*a)->inFromCallPass(*b->callPassFrom, *b);

            // Recalculate output sets:
            FOREACH_FORWARDS(a)
                if ((*a)->finishBlock(*b))
                    changed = true;
        }
    } while (changed);

    FOREACH_FORWARDS(a)
        (*a)->finish();
}

void ReachingDefinitions::start(const Block &entryBlock) {
    makeOuts(entryBlock, m_ins[&entryBlock], m_outs[&entryBlock]);
}

void ReachingDefinitions::startBlock(const Block &b) {
    m_ins[&b].clear();
}

void ReachingDefinitions::inFrom(const Block &from, const Block &to) {
    for (SDefs::const_iterator jt = m_outs[&from].begin(); jt != m_outs[&from].end(); jt++) {
        m_ins[&to][(*jt).first].first += (*jt).second.first;
    }
}

bool ReachingDefinitions::makeOuts(const Block &b, const SDefs &in, SDefs &out) {
    SDefs old = out;
    out = in;
    for (Blocks::CCI it = b.start; it != b.end; it++) {
        if (((*it)->isExpr() || ((*it)->type() == Imop::POPPARAM))
            && (*it)->dest() != 0)
        {
            // Set this def:
            Defs &d = out[(*it)->dest()].first;
            if ((d.begin() != d.end()) && (*d.begin() == *it))
                continue;

            d.clear();
            d.insert(*it);
        }
    }
    return old != out;
}

std::string ReachingDefinitions::toString(const Blocks &bs) const {
    typedef SDefs::const_iterator SDCI;
    typedef Defs::const_iterator DCI;

    std::ostringstream os;

    os << "Reaching definitions analysis results:" << std::endl;
    for (Blocks::const_iterator bi = bs.begin(); bi != bs.end(); bi++) {
        if (!(*bi)->reachable) continue;
        os << "  Block " << (*bi)->index << ": ";

        BDM::const_iterator si = m_ins.find(*bi);
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

void ReachingJumps::start(const Block &entryBlock) {
    m_inPos.insert(std::make_pair(&entryBlock, Jumps()));
    m_inNeg.insert(std::make_pair(&entryBlock, Jumps()));
}

void ReachingJumps::startBlock(const Block &b) {
    m_inNeg[&b].clear();
    m_inPos[&b].clear();
}

void ReachingJumps::inFrom(const Block &from, const Block &to) {
    m_inNeg[&to] += m_outNeg[&from];
    m_inPos[&to] += m_outPos[&from];
}

void ReachingJumps::inFromFalse(const Block &from, const Block &to) {
    const Imop *cjump = from.lastImop();
    assert(cjump->isCondJump());
    Jumps inPosT = m_outPos[&from];
    inPosT.erase(cjump);
    m_inPos[&to] += inPosT;
    m_inNeg[&to] += m_outNeg[&from];
    m_inNeg[&to].insert(cjump);
}

void ReachingJumps::inFromTrue(const Block &from, const Block &to) {
    const Imop *cjump = from.lastImop();
    assert(cjump->isCondJump());
    Jumps inNegT = m_outNeg[&from];
    inNegT.erase(cjump);
    m_inNeg[&to] += inNegT;
    m_inPos[&to] += m_outPos[&from];
    m_inPos[&to].insert(cjump);
}

bool ReachingJumps::finishBlock(const Block &b) {
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

std::string ReachingJumps::toString(const Blocks &bs) const {
    typedef std::set<const Imop*>::const_iterator ISCI;
    typedef std::map<unsigned long, char>::iterator       LCMI;
    typedef std::map<unsigned long, char>::const_iterator LCMCI;
    typedef BJM::const_iterator BJMCI;

    std::ostringstream os;

    os << "Reaching jumps analysis results:" << std::endl;
    for (Blocks::const_iterator bi = bs.begin(); bi != bs.end(); bi++) {
        if (!(*bi)->reachable) continue;

        os << "  Block " << (*bi)->index << ": ";

        BJMCI posi = m_inPos.find(*bi);
        BJMCI negi = m_inNeg.find(*bi);

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
                    } else {
                        jumps.insert(std::make_pair((*jt)->index(), '-'));
                    }
                }
            }
        } else if (negi != m_inNeg.end()) {
            for (ISCI jt = (*negi).second.begin(); jt != (*negi).second.end(); jt++) {
                jumps.insert(std::make_pair((*jt)->index(), '-'));
            }
        }

        if (jumps.empty()) {
            os << "NONE";
        } else {
            for (LCMCI jt = jumps.begin(); jt != jumps.end(); jt++) {
                if (jt != jumps.begin()) os << ", ";
                os << (*jt).first << (*jt).second;
            }
        }
        os << std::endl;
    }
    return os.str();
}

} // namespace SecreC
