#include "analysis/LiveVariables.h"

#include "Blocks.h"
#include "Misc.h"
#include "Symbol.h"
#include "TreeNode.h"

#include <boost/foreach.hpp>
#include <sstream>

namespace SecreC {

namespace { /* anonymous */

struct CollectGenKill {
    CollectGenKill(LiveVariables::Symbols & gen, LiveVariables::Symbols & kill)
        : m_gen(gen), m_kill(kill)
    { }

    inline void gen(const Symbol * sym) {
        m_gen.insert(sym);
    }

    inline void kill(const Symbol * sym) {
        m_kill.insert(sym);
        m_gen.erase(sym);
    }

private: /* Fields: */
    LiveVariables::Symbols & m_gen;
    LiveVariables::Symbols & m_kill;
};

struct UpdateBackwards {
    explicit UpdateBackwards(LiveVariables::Symbols & live)
        : m_live(live)
    { }

    inline void gen(const Symbol * sym) {
        m_live.insert(sym);
    }
    inline void kill(const Symbol * sym) {
        m_live.erase(sym);
    }

private: /* Fields: */
    LiveVariables::Symbols & m_live;
};

template <class Visitor>
void visitImop(const Imop & imop, Visitor & visitor) {

    BOOST_FOREACH(const Symbol * sym, imop.defRange()) {
        if (sym->symbolType() == SYM_SYMBOL) {
            visitor.kill(sym);
        }
    }

    BOOST_FOREACH(const Symbol * sym, imop.useRange()) {
        if (sym->symbolType() == SYM_SYMBOL) {
            visitor.gen(sym);
        }
    }
}

struct SimpleInterferenceGraph {

    SimpleInterferenceGraph()
        : m_count(0)
    { }

    void clear() {
        m_live.clear();
        m_edges.clear();
    }

    void drawNodes(std::ostream & os) {
        typedef std::pair<const Symbol *, unsigned> Node;
        BOOST_FOREACH (const Node & n, m_numbers) {
            os << "    node_" << n.second
               << " [label=\"" << n.first->name() << "\"];\n";
        }
    }

    void drawEdges(std::ostream & os) {
        typedef std::pair<unsigned, unsigned> Edge;
        BOOST_FOREACH (const Edge & edge, m_edges) {
            const unsigned n = edge.first;
            const unsigned m = edge.second;
            os << "    node_" << n << " -- node_" << m << ";\n";
        }
    }

    unsigned getNumber(const Symbol * sym) {
        std::map<const Symbol *, unsigned>::iterator i = m_numbers.find(sym);

        if (i == m_numbers.end()) {
            i = m_numbers.insert(i, std::make_pair(sym, m_count ++));
        }

        return i->second;
    }

    inline void gen(const Symbol * s1) {
        m_live.insert(s1);
    }

    inline void kill(const Symbol * sym) {
        m_live.erase(sym);
        BOOST_FOREACH (const Symbol * other, m_live) {
            addEdge(sym, other);
        }
    }

    void updateLiveness(const LiveVariables::Symbols & live) {
        m_live = live;
    }

    void addEdge(const Symbol * s1, const Symbol * s2) {
        if (s1 > s2) {
            std::swap(s1, s2);
        }

        if (s1->isGlobal() == s2->isGlobal()) {
            m_edges.insert(std::make_pair(num(s1), num(s2)));
        }
    }

private:

    unsigned num(const Symbol * sym) {
        std::map<const Symbol *, unsigned>::iterator it = m_numbers.find(sym);

        if (it == m_numbers.end()) {
            it = m_numbers.insert(it, std::make_pair(sym, m_count ++));
        }

        return it->second;
    }

    LiveVariables::Symbols m_live;
    std::set<std::pair<unsigned, unsigned> > m_edges;
    std::map<const Symbol *, unsigned > m_numbers;
    unsigned m_count;
};

inline void operator += (LiveVariables::Symbols& out, LiveVariables::Symbols& arg) {
    out.insert(arg.begin(), arg.end());
}

inline void operator -= (LiveVariables::Symbols& out, const LiveVariables::Symbols& arg) {
    BOOST_FOREACH (const Symbol* sym, arg) {
        out.erase(sym);
    }
}

} // namespace anonymous

/*******************************************************************************
  LiveVariables
*******************************************************************************/

void LiveVariables::updateBackwards(const SecreC::Imop & imop, Symbols & live) {
    UpdateBackwards visitor(live);
    visitImop(imop, visitor);
}

void LiveVariables::start(const Program & pr) {
    // we need to make sure to allocate all ins and outs
    FOREACH_BLOCK (bi, pr) {
        const Block & block = *bi;
        BlockInfo & blockInfo = m_blocks[&block];
        CollectGenKill collector (blockInfo.gen, blockInfo.kill);
        BOOST_REVERSE_FOREACH (const Imop & imop, block) {
            visitImop(imop, collector);
        }
    }
}

void LiveVariables::startBlock(const Block & b) {
    findBlock (b).out.clear ();
}

void LiveVariables::outToLocal(const Block & from, const Block & to) {
    findBlock (to).out += findBlock (from).in;
}

void LiveVariables::outToGlobal(const Block & from, const Block & to) {
    Symbols & out = findBlock (to).out;
    BOOST_FOREACH (const Symbol * symbol, findBlock (from).in) {
        if (symbol->isGlobal()) {
            out.insert(symbol);
        }
    }
}

bool LiveVariables::finishBlock(const Block & b) {
    BlockInfo & blockInfo = findBlock (b);
    Symbols & in = blockInfo.in;
    const Symbols old = in;
    const Symbols & out = blockInfo.out;
    in = out;
    in -= blockInfo.kill;
    in += blockInfo.gen;
    return old != in;
}

void LiveVariables::finish() { }

std::string LiveVariables::toString(const Program & pr) const {
    unsigned index = 0;
    SimpleInterferenceGraph visitor;

    std::stringstream ss;
    ss << "graph InferenceGraph {\n";
    BOOST_FOREACH (const Procedure & proc, pr) {
        ss << "  " << "subgraph cluster_" << index ++ << " {\n";
        ss << "    " << "label=\"" << (proc.name() ? proc.name()->name() : "GLOBAL") << "\"\n";
        visitor.clear();
        BOOST_FOREACH (const Block & block, proc) {
            BlockInfoMap::const_iterator it = m_blocks.find (&block);

            if (it == m_blocks.end()) {
                continue;
            }

            visitor.updateLiveness (it->second.out);
            BOOST_REVERSE_FOREACH (const Imop & imop, block) {
                visitImop(imop, visitor);
            }
        }

        visitor.drawNodes(ss);
        visitor.drawEdges(ss);
        ss << "  " << "}\n\n";
    }

    ss << '}';
    return ss.str();
}

} // namespace SecreC
