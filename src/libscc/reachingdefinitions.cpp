#include "reachingdefinitions.h"

#include <algorithm>
#include <utility>
#include "intermediate.h"
#include "misc.h"
#include "treenode.h"

namespace SecreC {
namespace {

typedef ICodeList::const_iterator CLCI;

inline bool isFirstImop(const Imop *i) {
    return i->block()->firstImop() == i;
}

template <class T>
inline std::set<T> &operator+=(std::set<T> &dest, const std::set<T> &src) {
    dest.insert(src.begin(), src.end());
    return dest;
}

template <class T>
class CopyToSetExcept {
    public:
        inline CopyToSetExcept(std::set<T> &target, const T &except)
            : m_target(target), m_except(except) {}
        inline void operator()(const T &v) {
            if (v != m_except) m_target.insert(v);
        }
    private:
        std::set<T> &m_target;
        const T      m_except;
};

template <class T>
void setToSetExcept(std::set<T> &dest, const std::set<T> &src, const T &except) {
    std::for_each(src.begin(), src.end(), CopyToSetExcept<T>(dest, except));
}

} // anonymous namespace


ReachingDefinitions::ReachingDefinitions(const ICode &code)
    : m_code(code)
{
    assert(code.status() == ICode::OK);
    assert(code.code().size() > 1);
    assert(isFirstImop(code.code()[0]));
    assert(code.code()[0]->block()->predecessors.empty());
    assert(code.code()[0]->block()->predecessorsCondFalse.empty());
    assert(code.code()[0]->block()->predecessorsCondTrue.empty());
    assert(code.code()[0]->block()->predecessorsCall.empty());
    assert(code.code()[0]->block()->predecessorsRet.empty());
}

void ReachingDefinitions::run() {
    typedef std::set<Block*>::const_iterator BSCI;
    bool changed = false;

    const std::vector<Block*> &bs = m_code.blocks().blocks();

    BDM outs;

    const Block &entryBlock = *bs.at(0);
    makeOuts(entryBlock, m_ins[&entryBlock], outs[&entryBlock]);
    m_inPos.insert(std::make_pair(&entryBlock, Jumps()));
    m_inNeg.insert(std::make_pair(&entryBlock, Jumps()));

    do {
        changed = false;

        // For all reachable blocks:
        for (size_t i = 1; i < bs.size(); i++) {
            const Block *b = bs[i];
            if (!b->reachable) continue;

            // Recalculate input set:
            for (BSCI it = b->predecessors.begin(); it != b->predecessors.end(); it++) {
                for (SDefs::const_iterator jt = outs[*it].begin(); jt != outs[*it].end(); jt++) {
                    m_ins[b][(*jt).first].first += (*jt).second.first;
                }
                m_inNeg[b] += m_inNeg[*it];
                m_inPos[b] += m_inPos[*it];
            }
            for (BSCI it = b->predecessorsCondFalse.begin(); it != b->predecessorsCondFalse.end(); it++) {
                for (SDefs::const_iterator jt = outs[*it].begin(); jt != outs[*it].end(); jt++) {
                    m_ins[b][(*jt).first].first += (*jt).second.first;
                }

                const Imop *cjump = (*it)->lastImop();
                assert(cjump->isCondJump());
                setToSetExcept(m_inPos[b], m_inPos[*it], cjump);
                setToSetExcept(m_inNeg[b], m_inNeg[*it], cjump);
                if (m_inNeg[b].insert(cjump).second) {
                    changed = true;
                }
            }
            for (BSCI it = b->predecessorsCondTrue.begin(); it != b->predecessorsCondTrue.end(); it++) {
                for (SDefs::const_iterator jt = outs[*it].begin(); jt != outs[*it].end(); jt++) {
                    m_ins[b][(*jt).first].first += (*jt).second.first;
                }

                const Imop *cjump = (*it)->lastImop();
                assert(cjump->isCondJump());
                setToSetExcept(m_inPos[b], m_inPos[*it], cjump);
                setToSetExcept(m_inNeg[b], m_inNeg[*it], cjump);
                if (m_inPos[b].insert(cjump).second) {
                    changed = true;
                }
            }

            // Recalculate output set:
            if (makeOuts(*b, m_ins[b], outs[b])) {
                changed = true;
            }
        }
    } while (changed);

    // For all reachable blocks:
    for (size_t i = 1; i < bs.size(); i++) {
        const Block *b = bs[i];
        if (!b->reachable) continue;

        // For all symbols whose definitions are reachable in block:
        for (SDefs::iterator sit = m_ins[b].begin(); sit != m_ins[b].end(); sit++) {
            const Symbol *s = (*sit).first;
            Defs &defs = (*sit).second.first;
            Jumps &validConds = (*sit).second.second;
            assert(validConds.empty());

            /*
                Condition 1:
                    Discard cond. jumps in hotspot which don't have a definition
                    of symbol at their position.
                Condition 2:
                    Discard cond. jump if reachable in hotspot by [+-] but in
                    definitions only with the opposite [-+].
            */
            for (Jumps::const_iterator jit = m_inPos[b].begin(); jit != m_inPos[b].end(); jit++) {
                // Condition 1:
                const Block *jb = (*jit)->block();
                if (outs[jb].find(s) == outs[jb].end())
                    continue;

                // Condition 2:
                bool reachable = false;
                for (Defs::const_iterator dit = defs.begin(); dit != defs.end(); dit++) {
                    const Block *db = (*dit)->block();
                    if (m_inNeg[db].find(*jit) != m_inNeg[db].end()) {
                        reachable = true;
                        break;
                    }
                }

                if (reachable) {
                    validConds.insert(*jit);
                }
            }
            for (Jumps::const_iterator jit = m_inNeg[b].begin(); jit != m_inNeg[b].end(); jit++) {
                if (validConds.find(*jit) != validConds.end()) continue;

                // Condition 1:
                const Block *jb = (*jit)->block();
                if (outs[jb].find(s) == outs[jb].end())
                    continue;

                // Condition 2:
                bool reachable = false;
                for (Defs::const_iterator dit = defs.begin(); dit != defs.end(); dit++) {
                    const Block *db = (*dit)->block();
                    if (m_inPos[db].find(*jit) != m_inPos[db].end()) {
                        reachable = true;
                        break;
                    }
                }

                if (reachable) {
                    validConds.insert(*jit);
                }
            }
        }
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

} // namespace SecreC
