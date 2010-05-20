#include "reachingdefinitions.h"

#include <utility>
#include "intermediate.h"
#include "misc.h"
#include "treenode.h"
#include <iostream>

namespace SecreC {
namespace {

typedef ICodeList::const_iterator CLCI;

inline bool isFirstImop(const Imop *i) {
    return i->block()->firstImop() == i;
}

template <class T>
inline std::set<T> &operator+=(std::set<T> &dest, const std::set<T> &src) {
    std::cout << "S" << std::flush;
    dest.insert(src.begin(), src.end());
    return dest;
}

ReachingDefinitions::CJumps &operator*=(ReachingDefinitions::CJumps &dest,
                                               const ReachingDefinitions::CJumps &src)
{
    std::cout << "*" << std::flush;
    typedef ReachingDefinitions::CJumps::const_iterator CJCI;
    typedef ReachingDefinitions::CJumps::iterator       CJI;

    for (CJCI it = src.begin(); it != src.end(); it++) {
        std::cout << "[J " << (*it).first->index() << "]" << std::flush;
        assert(!(*it).second.empty());
        CJI d = dest.find((*it).first);
        if (d == dest.end()) {
            dest.insert(*it);
        } else {
            (*d).second += (*it).second;
        }
    }
    return dest;
}

ReachingDefinitions::CJumps &operator+=(ReachingDefinitions::CJumps &dest,
                                               const ReachingDefinitions::CJumps &src)
{
    std::cout << "+" << std::flush;
    typedef ReachingDefinitions::CJumps::const_iterator CJCI;
    typedef ReachingDefinitions::CJumps::iterator       CJI;

    for (CJCI it = src.begin(); it != src.end(); it++) {
        std::cout << "[J " << (*it).first->index() << "]" << std::flush;
        CJI d = dest.find((*it).first);
        if (d == dest.end()) {
            std::pair<ReachingDefinitions::CJumps::iterator, bool> r = dest.insert(*it);
            assert(r.second == true);
            (*r.first).second.insert(true);
        } else {
            if ((*it).second.empty()) {
                (*d).second.insert(true);
            } else {
                (*d).second += (*it).second;
            }
            assert(!(*d).second.empty());
        }
    }
    return dest;
}

ReachingDefinitions::CJumps &operator-=(ReachingDefinitions::CJumps &dest,
                                               const ReachingDefinitions::CJumps &src)
{
    std::cout << "-" << std::flush;
    typedef ReachingDefinitions::CJumps::const_iterator CJCI;
    typedef ReachingDefinitions::CJumps::iterator       CJI;

    for (CJCI it = src.begin(); it != src.end(); it++) {
        std::cout << "[J " << (*it).first->index() << "]" << std::flush;
        CJI d = dest.find((*it).first);
        if (d == dest.end()) {
            std::cout << "J" << std::flush;
            std::pair<ReachingDefinitions::CJumps::iterator, bool> r = dest.insert(*it);
            assert(r.second == true);
            (*r.first).second.insert(false);
        } else {
            std::cout << "E" << std::flush;
            if ((*it).second.empty()) {
                std::cout << "J" << std::flush;
                (*d).second.insert(false);
            } else {
                std::cout << "E" << std::flush;
                (*d).second += (*it).second;
            }
            assert(!(*d).second.empty());
        }
    }
    return dest;
}

ReachingDefinitions::Defs &operator*=(ReachingDefinitions::Defs &dest,
                                             const ReachingDefinitions::Defs &src)
{
    typedef ReachingDefinitions::Defs::const_iterator DCI;

    for (DCI it = src.begin(); it != src.end(); it++) {
        std::cout << "[I*" << (*it).first->index() << "]" << std::flush;
        dest[(*it).first] *= (*it).second;
    }
    return dest;
}

ReachingDefinitions::Defs &operator+=(ReachingDefinitions::Defs &dest,
                                             const ReachingDefinitions::Defs &src)
{
    typedef ReachingDefinitions::Defs::const_iterator DCI;

    for (DCI it = src.begin(); it != src.end(); it++) {
        std::cout << "[I+" << (*it).first->index() << "]" << std::flush;
        dest[(*it).first] += (*it).second;
    }
    return dest;
}

ReachingDefinitions::Defs &operator-=(ReachingDefinitions::Defs &dest,
                                             const ReachingDefinitions::Defs &src)
{
    typedef ReachingDefinitions::Defs::const_iterator DCI;

    for (DCI it = src.begin(); it != src.end(); it++) {
        std::cout << "[I-" << (*it).first->index() << "]" << std::flush;
        dest[(*it).first] -= (*it).second;
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

            std::cout << std::endl << "(B " << b->index << ")" << std::flush;

            // Recalculate input set:
            m_ins[b].clear();
            std::cout << "*" << std::flush;
            for (BSCI it = b->predecessors.begin(); it != b->predecessors.end(); it++) {
                for (SDefs::const_iterator jt = outs[*it].begin(); jt != outs[*it].end(); jt++) {
                    m_ins[b][(*jt).first] *= (*jt).second;
                }
            }
            std::cout << "-" << std::flush;
            for (BSCI it = b->predecessorsCondFalse.begin(); it != b->predecessorsCondFalse.end(); it++) {
                for (SDefs::const_iterator jt = outs[*it].begin(); jt != outs[*it].end(); jt++) {
                    m_ins[b][(*jt).first] -= (*jt).second;
                }
            }
            std::cout << "+" << std::flush;
            for (BSCI it = b->predecessorsCondTrue.begin(); it != b->predecessorsCondTrue.end(); it++) {
                for (SDefs::const_iterator jt = outs[*it].begin(); jt != outs[*it].end(); jt++) {
                    m_ins[b][(*jt).first] += (*jt).second;
                }
            }

            for (SDefs::const_iterator it = m_ins[b].begin(); it != m_ins[b].end(); it++)
                for (Defs::const_iterator jt = (*it).second.begin(); jt != (*it).second.end(); jt++)
                    for (CJumps::const_iterator kt = (*jt).second.begin(); kt != (*jt).second.end(); kt++)
                        assert(!(*kt).second.empty());

            // Recalculate output set:
            if (makeOuts(*b, m_ins[b], outs[b])) {
                changed = true;
            }
        }
    } while (changed);
}

bool ReachingDefinitions::makeOuts(const Block &b, const SDefs &in, SDefs &out) {
    typedef Defs::const_iterator DCI;
    typedef Defs::iterator       DI;
    typedef SDefs::iterator      SDI;
    typedef CJumps::const_iterator CJCI;

    SDefs old(out);
    out = in;

    for (Blocks::CCI it = b.start; it != b.end; it++) {
        if (((*it)->type() == Imop::POPPARAM
            || ((*it)->type() & Imop::EXPR_MASK) != 0x0)
            && (*it)->dest() != 0)
        {
            // Set this def:
            Defs &d = out[(*it)->dest()];
            std::cout << "{" << std::flush;
            CJumps jumps;
            for (DCI jt = d.begin(); jt != d.end(); jt++) {
                std::cout << "[I " << (*jt).first->index() << "]" << std::flush;
                for (CJCI kt = (*jt).second.begin(); kt != (*jt).second.end(); kt++) {
                    assert((*kt).second.empty() == false);
                }
                jumps *= (*jt).second;
            }
            d.clear();
            d.insert(make_pair(*it, jumps));
            std::cout << "}" << std::flush;
        }
    }

    // If block ends with conditional jump:
    const Imop *lastImop = *(b.end - 1);
    if ((lastImop->type() & Imop::JUMP_MASK) != 0x0
        && lastImop->type() != Imop::JUMP)
    {
        // For all (const Symbol*, Defs) *it in out:
        for (SDI it = out.begin(); it != out.end(); it++) {
            // For all (const Imop*, CJumps) *jt in (*it).second:
            for (DI jt = (*it).second.begin(); jt != (*it).second.end(); jt++) {
                // Lets add another jump with unknown branches taken:
                (*jt).second.insert(make_pair(lastImop, CJConds()));
            }
        }
    }
    return old != out;
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::ReachingDefinitions &rd) {
    typedef SecreC::ReachingDefinitions RD;
    typedef RD::SDefs::const_iterator SDCI;
    typedef RD::Defs::const_iterator DCI;
    typedef RD::CJumps::const_iterator JCI;

    out << "Reaching definitions result: " << std::endl;

    const std::vector<SecreC::Block*> &bs = rd.icode().blocks().blocks();
    for (size_t i = 1; i < bs.size(); i++) {
        const SecreC::Block *b = bs[i];
        if (!b->reachable) continue;
        const RD::SDefs &sd = rd.getReaching(*b);
        if (!sd.empty()) {
            out << "  " << b->index << ":" << std::endl;
            for (SDCI it = sd.begin(); it != sd.end(); it++) {
                out << "    " << *(*it).first << ": ";
                const RD::Defs &ds = (*it).second;
                for (DCI jt = ds.begin(); jt != ds.end(); jt++) {
                    if (jt != ds.begin()) out << ", ";
                    out << (*jt).first->index();

                    // Print conditional jumps only if more than one definition:
                    if (ds.size() > 1) {
                        const RD::CJumps &js = (*jt).second;
                        if (!js.empty()) {
                            out << " [";
                            for (JCI kt = js.begin(); kt != js.end(); kt++) {
                                if (kt != js.begin()) out << ", ";
                                out << (*kt).first->index();

                                const SecreC::ReachingDefinitions::CJConds &cs = (*kt).second;
                                if (cs.size() == 2) {
                                    out << '*';
                                } else if (cs.size() == 0) {
                                    out << '?';
                                } else {
                                    assert(cs.size() == 1);
                                    out << (cs.find(true) != cs.end() ? '+' : '-');
                                }

                                // out << " " << (*kt).first->creator()->location();
                            }
                            out << "]";
                        }
                    }
                }
                out << std::endl;
            }
        }
    }

    return out;
}
