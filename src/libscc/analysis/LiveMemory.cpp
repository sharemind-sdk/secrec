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

#include "analysis/LiveMemory.h"

#include "Symbol.h"

#include <boost/range/adaptor/reversed.hpp>
#include <sstream>

using boost::adaptors::reverse;

namespace { /* anonymous */

using namespace SecreC;

LiveMemory::Domain & operator |= (LiveMemory::Domain & dest, LiveMemory::Domain src) {
    return (dest = static_cast<LiveMemory::Domain>(dest | src));
}

struct CollectGenKill {
    CollectGenKill(LiveMemory::Values & gen, LiveMemory::Symbols & kill)
        : m_gen(gen), m_kill(kill)
    { }

    inline void gen(const Symbol * sym, LiveMemory::Domain dom) {
        if (sym->isArray()) {
            m_gen[sym] |= dom;
        }
    }

    inline void kill(const Symbol * sym) {
        m_kill.insert(sym);
    }

    LiveMemory::Values & m_gen;
    LiveMemory::Symbols & m_kill;
};

struct UpdateValues {
    UpdateValues(LiveMemory::Values & vs)
        : m_values(vs)
    { }


    inline void gen(const Symbol * sym, LiveMemory::Domain dom) {
        if (sym->isArray()) {
            m_values[sym] |= dom;
        }
    }

    inline void kill(const Symbol * sym) {
        m_values.erase(sym);
    }

    LiveMemory::Values & m_values;
};

template <class Visitor>
void visitImop(const Imop & imop, Visitor & visitor) {

    if (imop.isVectorized()) {
        for (const Symbol * arg : imop.useRange()) {
            visitor.gen(arg, LiveMemory::Read);
        }

        visitor.gen(imop.dest(), LiveMemory::Write);
        return;
    }


    switch (imop.type()) {
    case Imop::STORE:    visitor.gen(imop.dest(), LiveMemory::Write); break;
    case Imop::PUSHCREF: visitor.gen(imop.arg1(), LiveMemory::Write); break;
    case Imop::PUSH:     visitor.gen(imop.arg1(), LiveMemory::Live);  break;
    case Imop::PUSHREF:  visitor.gen(imop.arg1(), LiveMemory::Live);  break;
    case Imop::LOAD:     visitor.gen(imop.arg1(), LiveMemory::Read);  break;
    case Imop::COPY:     visitor.gen(imop.arg1(), LiveMemory::Read);  /* FALLTHROUGH */
    case Imop::ALLOC:    /* intentionally empty */
    case Imop::PARAM:    /* intentionally empty */
    case Imop::SYSCALL:  visitor.kill(imop.dest());                   break;
    case Imop::CALL:
        for (const Symbol* arg : imop.useRange()) {
            visitor.gen(arg, LiveMemory::Read);
        }

        for (const Symbol* dest : imop.defRange()) {
            visitor.kill(dest);
        }

        break;

    default:
        break;
    }
}

inline bool isRedundantCopy(LiveMemory::Domain dest, LiveMemory::Domain src) {
    if (src == LiveMemory::Dead) return true;
    if ((dest & LiveMemory::Read) == 0x0) return true;
    if (((dest & LiveMemory::Write) == 0x0) && ((src & LiveMemory::Write) == 0x0)) return true;
    return false;
}

} // namespace anonymous

namespace SecreC {

/*******************************************************************************
  LiveMemory
*******************************************************************************/

void LiveMemory::update(const Imop & imop, Values & vals) {
    UpdateValues visitor(vals);
    visitImop(imop, visitor);
}

void LiveMemory::start(const Program & pr) {
    FOREACH_BLOCK (bi, pr) {
        CollectGenKill collector(m_gen[&*bi], m_kill[&*bi]);
        for (const Imop & imop : reverse (*bi)) {
            visitImop(imop, collector);
        }
    }
}

void LiveMemory::startBlock(const Block & b) {
    m_outs[&b].clear();
}

void LiveMemory::outToLocal(const Block & from, const Block & to) {
    Values & dest = m_outs[&to];
    const Values & src = m_ins[&from];
    for (Values::const_reference sv : src) {
        dest[sv.first] |= sv.second;
    }
}

void LiveMemory::outToGlobal(const Block & from, const Block & to) {
    Values & dest = m_outs[&to];
    const Values & src = m_ins[&from];
    for (Values::const_reference sv : src) {
        if (sv.first->isGlobal()) {
            dest[sv.first] |= sv.second;
        }
    }
}

bool LiveMemory::finishBlock(const Block & b) {
    Values & in = m_ins[&b];
    const Values old = in;
    const Values & out = m_outs[&b];
    in = out;
    for (Symbols::const_reference sym : m_kill[&b]) {
        in.erase(sym);
    }

    for (Values::const_reference sv : m_gen[&b]) {
        in[sv.first] |= sv.second;
    }

    return old != in;
}

std::set<const Imop *> LiveMemory::deadCopies(const Program & pr) const {
    std::set<const Imop *> out;
    Values after; // analysis info after current code point.
    UpdateValues visitor(after);
    FOREACH_BLOCK (bi, pr) {
        if (! bi->reachable()) {
            continue;
        }

        after.clear();
        auto i = m_outs.find(&*bi);

        if (i != m_outs.end()) {
            after = i->second;
        }

        for (const Imop & imop : reverse (*bi)) {
            if (imop.type() == Imop::COPY) {
                if (isRedundantCopy(after[imop.dest()], after[imop.arg1()])) {
                    out.insert(&imop);
                }
            }

            visitImop(imop, visitor);
        }
    }

    return out;
}

std::string LiveMemory::printDeadCopies(const Program & pr) const {
    std::ostringstream ss;
    ss << "The following copies are redundant:\n";

    size_t num_copies = 0;
    size_t num_eliminated = 0;
    FOREACH_BLOCK (bi, pr) {
        Values after;
        UpdateValues visitor(after);

        auto i = m_outs.find(&*bi);

        if (i != m_outs.end()) {
            after = i->second;
        }

        for (const Imop & imop : reverse (*bi)) {
            for (const Symbol * dest : imop.defRange()) {
                if (dest->isArray() && after[dest] == Dead) {
                    ss << imop.index() << ": " << imop << " redundant value " << *dest << "\n";
                }
            }

            if (imop.type() == Imop::COPY) {
                ++ num_copies;

                if ((after[imop.dest()] & Read) == 0x0) {
                    ss << imop.index() << ": " << imop << " (dest is never read)\n";
                    ++ num_eliminated;
                }
                else if (after[imop.arg1()] == Dead) {
                    ss << imop.index() << ": " << imop << '\n';
                    ++ num_eliminated;
                }
                else if (((after[imop.dest()] & Write) == 0x0) &&
                         ((after[imop.arg1()] & Write) == 0x0)) {
                    ss << imop.index() << ": " << imop << " (src/dest are read-only)\n";
                    ++ num_eliminated;
                }
            }

            visitImop(imop, visitor);
        }
    }

    if (num_copies > 0) {
        ss << "Can eliminate " << num_eliminated << " out of " << num_copies << " copies." << std::endl;
    }

    return ss.str();
}

std::string LiveMemory::toString(const Program & pr) const {
    std::stringstream ss;
    ss << "Memory liveness:\n";
    FOREACH_BLOCK (bi, pr) {
        const Block * block = &*bi;
        auto valsIt = m_outs.find(block);

        if (valsIt == m_outs.end()) {
            continue;
        }

        const Values & vals = valsIt->second;

        if (vals.empty()) {
            continue;
        }

        ss << "[Block " << block->index() << "] (size = " << vals.size() << ")\n";
        for (Values::const_reference val : vals) {
            switch (val.second) {
            case Live:  ss << "LIVE";  break;
            case Read:  ss << "READ";  break;
            case Write: ss << "WRITE"; break;
            case Dead:  ss << "DEAD";  break;
            }

            ss << ' ' << *val.first << '\n';
        }
    }

    ss << '\n' << printDeadCopies(pr) << '\n';


    return ss.str();
}

} // namespace SecreC
