#include "analysis/LiveVariables.h"

#include <boost/foreach.hpp>
#include <sstream>

#include "blocks.h"
#include "misc.h"
#include "symbol.h"
#include "treenode.h"

namespace SecreC {

namespace /* anonymous */ {

struct CollectGenKill {
    CollectGenKill (LiveVariables::Symbols& gen, LiveVariables::Symbols& kill)
        : m_gen (gen), m_kill (kill)
    { }

    inline void gen (const Symbol* sym) { m_gen.insert (sym); }
    inline void kill (const Symbol* sym) { m_kill.insert (sym); m_gen.erase (sym); }

private: /* Fields: */
    LiveVariables::Symbols& m_gen;
    LiveVariables::Symbols& m_kill;
};

struct UpdateBackwards {
    explicit UpdateBackwards (LiveVariables::Symbols& live)
        : m_live (live)
    { }

    inline void gen (const Symbol* sym) { m_live.insert (sym); }
    inline void kill (const Symbol* sym) { m_live.erase (sym); }

private: /* Fields: */
    LiveVariables::Symbols& m_live;
};

template <class Visitor>
void visitImop (const Imop& imop, Visitor& visitor) {

    BOOST_FOREACH (const Symbol* sym, imop.defRange ()) {
        if (sym->symbolType () == Symbol::SYMBOL) {
            visitor.kill (sym);
        }
    }

    BOOST_FOREACH (const Symbol* sym, imop.useRange ()) {
        if (sym->symbolType () == Symbol::SYMBOL) {
            visitor.gen (sym);
        }
    }
}

struct SimpleInterferenceGraph {

    SimpleInterferenceGraph ()
        : m_count (0)
    { }

    void clear () {
        m_live.clear ();
        m_edges.clear();
    }

    void drawNodes (std::ostream& os) {
        typedef std::pair<const Symbol*, unsigned> Node;
        BOOST_FOREACH (const Node& n, m_numbers) {
            os << "    node_" << n.second
               << " [label=\"" << n.first->name () << "\"];\n";
        }
    }

    void drawEdges (std::ostream& os) {
        typedef std::pair<unsigned, unsigned> Edge;
        BOOST_FOREACH (const Edge& edge, m_edges) {
            const unsigned n = edge.first;
            const unsigned m = edge.second;
            os << "    node_" << n << " -- node_" << m << ";\n";
        }
    }

    unsigned getNumber (const Symbol* sym) {
        std::map<const Symbol*, unsigned>::iterator i = m_numbers.find (sym);
        if (i == m_numbers.end ()) {
            i = m_numbers.insert (i, std::make_pair (sym, m_count ++));
        }

        return i->second;
    }

    inline void gen (const Symbol* s1) {
        m_live.insert (s1);
    }

    inline void kill (const Symbol* sym) {
        m_live.erase (sym);
        BOOST_FOREACH (const Symbol* other, m_live)
            addEdge (sym, other);
    }

    void updateLiveness (const LiveVariables::Symbols& live) {
        m_live = live;
    }

    void addEdge (const Symbol* s1, const Symbol* s2) {
        if (s1 > s2) {
            std::swap (s1, s2);
        }

        if (s1->isGlobal () == s2->isGlobal ()) {
            m_edges.insert (std::make_pair (num (s1), num (s2)));
        }
    }

private:

    unsigned num (const Symbol* sym) {
        std::map<const Symbol*, unsigned>::iterator it = m_numbers.find (sym);
        if (it == m_numbers.end ()) {
            it = m_numbers.insert (it, std::make_pair (sym, m_count ++));
        }

        return it->second;
    }

    std::set<const Symbol*> m_live;
    std::set<std::pair<unsigned, unsigned> > m_edges;
    std::map<const Symbol*, unsigned > m_numbers;
    unsigned m_count;
};

} // namespace anonymous

/*******************************************************************************
  LiveVariables
*******************************************************************************/

void LiveVariables::updateBackwards (const SecreC::Imop& imop, Symbols& live) {
    UpdateBackwards visitor (live);
    visitImop (imop, visitor);
}

void LiveVariables::start (const Program &pr) {
    FOREACH_BLOCK (bi, pr) {
        CollectGenKill collector (m_gen[&*bi], m_kill[&*bi]);
        BOOST_REVERSE_FOREACH (const Imop& imop, *bi) {
            visitImop (imop, collector);
        }
    }
}

void LiveVariables::startBlock (const Block& b) {
    m_outs[&b].clear ();
}

void LiveVariables::outToLocal (const Block &from, const Block &to) {
    m_outs[&to] += m_ins[&from];
}

void LiveVariables::outToGlobal (const Block& from, const Block& to) {
    BOOST_FOREACH (const Symbol* symbol, m_ins[&from]) {
        if (symbol->isGlobal ()) {
            m_outs[&to].insert (symbol);
        }
    }
}

bool LiveVariables::finishBlock (const Block &b) {
    Symbols& in = m_ins[&b];
    const Symbols  old = in;
    const Symbols& out = m_outs[&b];
    in = out;
    in -= m_kill[&b];
    in += m_gen[&b];
    return old != in;
}

void LiveVariables::finish () { }

std::string LiveVariables::toString (const Program &pr) const {
    unsigned index = 0;
    SimpleInterferenceGraph visitor;

    std::stringstream ss;
    ss << "graph InferenceGraph {\n";
    BOOST_FOREACH (const Procedure& proc, pr) {
        ss << "  " << "subgraph cluster_" << index ++ << " {\n";
        ss << "    " << "label=\"" << (proc.name () ? proc.name ()->name() : "GLOBAL") << "\"\n";
        visitor.clear ();
        BOOST_FOREACH (const Block& block, proc) {
            BSM::const_iterator it = m_outs.find (&block);
            if (it == m_outs.end ())
                continue;

            visitor.updateLiveness (it->second);
            BOOST_REVERSE_FOREACH (const Imop& imop, block) {
                visitImop (imop, visitor);
            }
        }

        visitor.drawNodes (ss);
        visitor.drawEdges (ss);
        ss << "  " << "}\n\n";
    }

    ss << '}';

    return ss.str ();
}

} // namespace SecreC
