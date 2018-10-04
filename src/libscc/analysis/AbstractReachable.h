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

#ifndef SECREC_ABSTRACT_REACHABLE_H
#define SECREC_ABSTRACT_REACHABLE_H

#include "../DataflowAnalysis.h"
#include "../Symbol.h"
#include "BoostInsertWorkaround.h"

#include <boost/interprocess/containers/flat_map.hpp>
#include <boost/interprocess/containers/flat_set.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <map>
#include <string>

using boost::adaptors::reverse;

namespace SecreC {

using Reachable = boost::container::flat_set<Imop*>;
using SymbolReachable = boost::container::flat_map<const Symbol*, Reachable>;

/*******************************************************************************
  AbstractReachable
*******************************************************************************/

template <typename VisitImop>
class AbstractReachable : public BackwardDataFlowAnalysis {

public: /* Types: */

    using Symbols = boost::container::flat_set<const Symbol*>;

    struct BlockInfo {
        SymbolReachable gen;
        Symbols kill;
        SymbolReachable in;
        SymbolReachable out;
    };

    using BlockInfoMap = std::map<const Block*, BlockInfo>;

private: /* Types: */

    struct CollectGenKill {
        CollectGenKill(SymbolReachable& gen, Symbols& kill)
            : m_gen(gen), m_kill(kill)
            { }

        inline void gen(const Symbol* sym, const Imop& imop) {
            m_gen[sym].insert(const_cast<Imop*>(&imop));
        }

        inline void kill(const Symbol* sym) {
            m_kill.insert(sym);
            m_gen.erase(sym);
        }

    private: /* Fields: */
        SymbolReachable& m_gen;
        Symbols& m_kill;
    };

    struct UpdateValues {
        UpdateValues(SymbolReachable& values)
            : m_values(values)
            { }

        inline void gen(const Symbol* sym, const Imop& imop) {
            m_values[sym].insert(const_cast<Imop*>(&imop));
        }

        inline void kill(const Symbol* sym) {
            m_values.erase(sym);
        }

        SymbolReachable& m_values;
    };

public: /* Methods: */

    SymbolReachable reachableOnExit(const Block& block) const {
        const auto it = m_blocks.find(&block);
        if (it != m_blocks.end())
            return it->second.out;
        return SymbolReachable();
    }

    static void update(const Imop& imop, SymbolReachable& vals) {
        UpdateValues visitor(vals);
        VisitImop v;
        v(imop, visitor);
    }

protected:

    virtual void start(const Program& pr) override {
        m_blocks.clear();
        VisitImop v;

        FOREACH_BLOCK (bi, pr) {
            const Block& block = *bi;
            BlockInfo& blockInfo = m_blocks[&block];
            CollectGenKill collector(blockInfo.gen, blockInfo.kill);
            for (const Imop& imop : reverse(block)) {
                v(imop, collector);
            }
        }
    }

    virtual void startBlock(const Block& b) override {
        findBlock(b).out.clear();
    }

    virtual void outTo(const Block& from, Edge::Label label, const Block& to) override {
        if (Edge::isGlobal(label)) {
            outToGlobal(from, to);
        }
        else {
            outToLocal(from, to);
        }
    }

    virtual bool finishBlock(const Block& b) override {
        BlockInfo& blockInfo = findBlock(b);
        SymbolReachable& in = blockInfo.in;
        const SymbolReachable old = in;
        const SymbolReachable& out = blockInfo.out;
        in = out;

        for (const Symbol* s : blockInfo.kill) {
            in.erase(s);
        }

        add(in, blockInfo.gen);

        return old != in;
    }

    virtual void finish() override {}

private:

    void outToLocal(const Block& from, const Block& to) {
        const SymbolReachable& in = findBlock(from).in;
        SymbolReachable& out = findBlock(to).out;
        add(out, in);
    }

    void outToGlobal(const Block& from, const Block& to) {
        const SymbolReachable& in = findBlock(from).in;
        SymbolReachable& out = findBlock(to).out;

        for (const auto& it : in) {
            if (it.first->isGlobal()) {
                insertWorkaround(out[it.first], it.second.begin(), it.second.end());
            }
        }
    }

    BlockInfo& findBlock(const Block& block) {
        return m_blocks[&block];
    }

    const BlockInfo& findBlock(const Block& block) const {
        auto it = m_blocks.find(&block);
        assert(it != m_blocks.end());
        return it->second;
    }

    void add(SymbolReachable& out, const SymbolReachable& in) {
        for (const auto& it : in) {
            insertWorkaround(out[it.first], it.second.begin(), it.second.end());
        }
    }

protected: /* Fields: */

    BlockInfoMap m_blocks;
}; // class AbstractReachable

} // namespace SecreC

#endif // SECREC_ABSTRACT_REACHABLE_H
