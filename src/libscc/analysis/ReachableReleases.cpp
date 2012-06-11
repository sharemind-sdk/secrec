#include "analysis/ReachableReleases.h"

#include <boost/foreach.hpp>
#include <sstream>

#include "symbol.h"
#include "treenode.h"

namespace /* anonymous */ {

using namespace SecreC;

struct UpdateValues {
    UpdateValues (ReachableReleases::Values& vs)
        : m_values (vs)
    { }


    inline void gen (const Symbol* sym, const Imop& imop) {
        m_values[sym].insert (&imop);
    }

    inline void kill (const Symbol* sym) {
        m_values.erase (sym);
    }

    ReachableReleases::Values& m_values;
};

template <class Visitor>
void visitImop (const Imop& imop, Visitor& visitor) {
    BOOST_FOREACH (const Symbol* dest, imop.defRange ()) {
        visitor.kill (dest);
    }

    if (imop.type () == Imop::RELEASE) {
        visitor.gen (imop.arg1 (), imop);
    }
}

} // namespace anonymous

namespace SecreC {

/*******************************************************************************
  ReachableReleases
*******************************************************************************/

void ReachableReleases::update (const Imop& imop, Values& vals) {
    UpdateValues visitor (vals);
    visitImop (imop, visitor);
}

void ReachableReleases::start (const Program& /* pr */) { }

void ReachableReleases::outToLocal (const Block& from, const Block& to) {
    Values& dest = m_outs[&to];
    const Values& src = m_ins[&from];
    BOOST_FOREACH (Values::const_reference sv, src) {
        dest[sv.first] += sv.second;
    }
}

void ReachableReleases::outToGlobal (const Block& from, const Block& to) {
    Values& dest = m_outs[&to];
    const Values& src = m_ins[&from];
    BOOST_FOREACH (Values::const_reference sv, src) {
        if (sv.first->isGlobal ())
            dest[sv.first] += sv.second;
    }
}

void ReachableReleases::startBlock (const Block& b) {
    m_outs[&b].clear ();
}

bool ReachableReleases::finishBlock (const Block& b) {
    Values& in = m_ins[&b];
    const Values old = in;
    const Values& out = m_outs[&b];
    in = out;

    BOOST_REVERSE_FOREACH (const Imop& imop, b) {
        update (imop, in);
    }

    return old != in;
}


std::string ReachableReleases::toString (const Program& pr) const {
    std::stringstream ss;
    ss << "Reachable releases:\n";
    FOREACH_BLOCK (bi, pr) {
        Values after;
        BV::const_iterator i = m_outs.find (&*bi);
        if (i != m_outs.end ()) {
            after = i->second;
        }

        BOOST_REVERSE_FOREACH (const Imop& imop, *bi) {
            BOOST_FOREACH (const Symbol* dest, imop.defRange ()) {
                if (! dest->isArray ())
                    continue;

                ss << imop.index () << ": " << imop.toString () << " // " << imop.creator ()->location () << "\n";
                BOOST_FOREACH (const Imop* release, after[dest]) {
                    ss << '\t' << release->index () << ": " << release->toString () << '\n';
                }
            }

            update (imop, after);
        }
    }

    return ss.str ();
}

} // namespace SecreC
