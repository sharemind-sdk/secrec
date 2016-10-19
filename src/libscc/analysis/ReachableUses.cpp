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

#include "analysis/ReachableUses.h"

#include "Blocks.h"
#include "Symbol.h"
#include "TreeNode.h"

#include <boost/range/adaptor/reversed.hpp>
#include <sstream>

using boost::adaptors::reverse;

namespace SecreC {

namespace { /* anonymous */

struct CollectGenKill {
    CollectGenKill(ReachableUses::SymbolUses& gen, ReachableUses::Symbols& kill)
        : m_gen(gen), m_kill(kill)
    { }

    inline void gen(const Symbol* sym, const Imop& imop) {
        m_gen[sym].insert(const_cast<Imop*>(&imop));
    }

    inline void kill(const Symbol* sym) {
        m_kill.insert(sym);
    }

private: /* Fields: */
    ReachableUses::SymbolUses& m_gen;
    ReachableUses::Symbols& m_kill;
};

struct UpdateValues {
    UpdateValues(ReachableUses::SymbolUses& values)
        : m_values(values)
    { }

    inline void gen(const Symbol* sym, const Imop& imop) {
        m_values[sym].insert(const_cast<Imop*>(&imop));
    }

    inline void kill(const Symbol* sym) {
        m_values.erase(sym);
    }

    ReachableUses::SymbolUses& m_values;
};

template<class Visitor>
void visitImop(const Imop& imop, Visitor& visitor) {
    for (const Symbol* sym : imop.defRange()) {
        if (sym->symbolType() == SYM_SYMBOL) {
            visitor.kill(sym);
        }
    }

    for (const Symbol* sym : imop.useRange()) {
        if (sym->symbolType() == SYM_SYMBOL) {
            visitor.gen(sym, imop);
        }
    }
}

inline void operator += (ReachableUses::SymbolUses& out, ReachableUses::SymbolUses& in) {
    for (const auto it : in) {
        out[it.first].insert(it.second.begin(), it.second.end());
    }
}

} // namespace anonymous

void ReachableUses::update(const Imop& imop, SymbolUses& vals) {
    UpdateValues visitor(vals);
    visitImop(imop, visitor);
}

void ReachableUses::start(const Program& pr) {
    FOREACH_BLOCK (bi, pr) {
        const Block& block = *bi;
        BlockInfo& blockInfo = m_blocks[&block];
        CollectGenKill collector(blockInfo.gen, blockInfo.kill);
        for (const Imop& imop : reverse(block)) {
            visitImop(imop, collector);
        }
    }
}

void ReachableUses::startBlock(const Block& b) {
    findBlock(b).out.clear();
}

void ReachableUses::outToLocal(const Block& from, const Block& to) {
    SymbolUses& in = findBlock(from).in;
    SymbolUses& out = findBlock(to).out;
    out += in;
}

void ReachableUses::outToGlobal(const Block& from, const Block& to) {
    SymbolUses& in = findBlock(from).in;
    SymbolUses& out = findBlock(to).out;

    for (const auto& it : in) {
        if (it.first->isGlobal()) {
            out[it.first].insert(it.second.begin(), it.second.end());
        }
    }
}

bool ReachableUses::finishBlock(const Block& b) {
    BlockInfo& blockInfo = findBlock(b);
    SymbolUses& in = blockInfo.in;
    const SymbolUses old = in;
    const SymbolUses& out = blockInfo.out;
    in = out;

    in += blockInfo.gen;

    for (const Symbol* s : blockInfo.kill) {
        in.erase(s);
    }

    return old != in;
}

void ReachableUses::finish() {}

std::string ReachableUses::toString(const Program& pr) const {
    std::stringstream ss;
    ss << "Reachable uses:\n";
    FOREACH_BLOCK (bi, pr) {
        SymbolUses after;
        auto i = m_blocks.find(&*bi);

        if (i != m_blocks.end()) {
            after = i->second.out;
        }

        for (const Imop& imop : reverse(*bi)) {
            for (const Symbol* dest : imop.defRange()) {
                ss << imop.index() << ": " << imop << " // " << imop.creator()->location() << "\n";
                for (const Imop* use : after[dest]) {
                    ss << '\t' << use->index() << ": " << *use << '\n';
                }
            }

            update(imop, after);
        }
    }

    return ss.str();
}

} // namespace SecreC
