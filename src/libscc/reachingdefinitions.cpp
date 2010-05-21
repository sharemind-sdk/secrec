#include "reachingdefinitions.h"

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

inline ReachingDefinitions::Defs &operator+=(ReachingDefinitions::Defs &dest,
                                             const ReachingDefinitions::Defs &src)
{
    typedef ReachingDefinitions::Defs::const_iterator DCI;

    for (DCI it = src.begin(); it != src.end(); it++) {
        dest[(*it).first] += (*it).second;
    }
    return dest;
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

    makeOuts(*bs[0], m_ins[bs[0]], outs[bs[0]]);
    do {
        changed = false;

        // For all reachable blocks:
        for (size_t i = 1; i < bs.size(); i++) {
            const Block *b = bs[i];
            if (!b->reachable) continue;

            // Recalculate input set:
            m_ins[b].clear();
            for (BSCI it = b->predecessors.begin(); it != b->predecessors.end(); it++) {
                for (SDefs::const_iterator jt = outs[*it].begin(); jt != outs[*it].end(); jt++) {
                    m_ins[b][(*jt).first] += (*jt).second;
                }
            }
            for (BSCI it = b->predecessorsCondFalse.begin(); it != b->predecessorsCondFalse.end(); it++) {
                for (SDefs::const_iterator jt = outs[*it].begin(); jt != outs[*it].end(); jt++) {
                    m_ins[b][(*jt).first] += (*jt).second;
                }
            }
            for (BSCI it = b->predecessorsCondTrue.begin(); it != b->predecessorsCondTrue.end(); it++) {
                for (SDefs::const_iterator jt = outs[*it].begin(); jt != outs[*it].end(); jt++) {
                    m_ins[b][(*jt).first] += (*jt).second;
                }
            }

            // Recalculate output set:
            if (makeOuts(*b, m_ins[b], outs[b])) {
                changed = true;
            }
        }
    } while (changed);
}

bool ReachingDefinitions::makeOuts(const Block &b, const SDefs &in, SDefs &out) {
    typedef SDefs::const_iterator SDCI;
    typedef Defs::const_iterator DCI;
    typedef SDefs::iterator SDI;
    typedef Defs::iterator DI;

    SDefs old(out);
    out = in;

    for (Blocks::CCI it = b.start; it != b.end; it++) {
        if (((*it)->type() == Imop::POPPARAM
            || ((*it)->type() & Imop::EXPR_MASK) != 0)
            && (*it)->dest() != 0)
        {
            // Set this def:
            Defs &d = out[(*it)->dest()];
            CJumps jumps;
            for (DCI jt = d.begin(); jt != d.end(); jt++) {
                jumps += (*jt).second;
            }
            d.clear();
            d.insert(make_pair(*it, jumps));
        }
    }

    const Imop *lastImop = *(b.end - 1);
    if ((lastImop->type() & Imop::JUMP_MASK)
        && lastImop->type() != Imop::JUMP)
    {
        for (SDI it = out.begin(); it != out.end(); it++) {
            for (DI jt = (*it).second.begin(); jt != (*it).second.end(); jt++) {
                (*jt).second.insert(lastImop);
            }
        }
    }
    return old != out;
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::ReachingDefinitions &rd) {
    typedef SecreC::ReachingDefinitions::SDefs::const_iterator SDCI;
    typedef SecreC::ReachingDefinitions::Defs::const_iterator DCI;
    typedef SecreC::ReachingDefinitions::CJumps::const_iterator JCI;

    out << "Reaching definitions result: " << std::endl;

    const std::vector<SecreC::Block*> &bs = rd.icode().blocks().blocks();
    for (size_t i = 1; i < bs.size(); i++) {
        const SecreC::Block *b = bs[i];
        if (!b->reachable) continue;
        const SecreC::ReachingDefinitions::SDefs &sd = rd.getReaching(*b);
        if (!sd.empty()) {
            out << "  " << b->index << ":" << std::endl;
            for (SDCI it = sd.begin(); it != sd.end(); it++) {
                out << "    " << *(*it).first << ": ";
                const SecreC::ReachingDefinitions::Defs &ds = (*it).second;
                for (DCI jt = ds.begin(); jt != ds.end(); jt++) {
                    if (jt != ds.begin()) out << ", ";
                    out << (*jt).first->index();
                    const SecreC::ReachingDefinitions::CJumps &js = (*jt).second;
                    if (!js.empty()) {
                        out << " [CJUMPS: ";
                        for (JCI kt = js.begin(); kt != js.end(); kt++) {
                            if (kt != js.begin()) out << ", ";
                            out << (*kt)->index() << " " << (*kt)->creator()->location();
                        }
                        out << "]";
                    }
                }
                out << std::endl;
            }
        }
    }

    return out;
}
