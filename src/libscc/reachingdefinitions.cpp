#include "reachingdefinitions.h"

#include "intermediate.h"


namespace SecreC {
namespace {

typedef ICodeList::const_iterator CLCI;

inline bool isFirstImop(const Imop *i) {
    return i->block()->firstImop() == i;
}

inline ReachingDefinitions::Defs &operator+=(ReachingDefinitions::Defs &dest,
                                             const ReachingDefinitions::Defs &src)
{
    dest.insert(src.begin(), src.end());
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
    bool changed;

    const std::vector<Block*> &bs = m_code.blocks().blocks();

    BDM outs;

    makeOuts(*bs[0], m_ins[bs[0]], outs[bs[0]]);
    do {
        changed = false;
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
    SDefs old(out);
    out = in;

    for (Blocks::CCI it = b.start; it != b.end; it++) {
        if ((*it)->type() == Imop::PARAMINTRO
            || ((*it)->type() & Imop::EXPR_MASK) != 0)
        {
            // Set this def:
            Defs &d = out[(*it)->dest()];
            d.clear();
            d.insert(*it);
        }
    }
    return old != out;
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::ReachingDefinitions &rd) {
    typedef SecreC::ReachingDefinitions::SDefs::const_iterator SDCI;
    typedef SecreC::ReachingDefinitions::Defs::const_iterator DCI;

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
                    out << (*jt)->index();
                }
                out << std::endl;
            }
        }
    }

    return out;
}
