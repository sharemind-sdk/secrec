#include "analysis/LiveMemory.h"

#include <boost/foreach.hpp>
#include <sstream>

#include "symbol.h"

namespace /* anonymous */ {

using namespace SecreC;

bool isGlobal (const Symbol* symbol) {
    assert (symbol != 0);
    switch (symbol->symbolType ()) {
    case Symbol::SYMBOL:
        if (static_cast<const SymbolSymbol*>(symbol)->scopeType () == SymbolSymbol::GLOBAL)
            return true;
    default:
        break;
    }

    return false;
}

bool isArray (const Symbol* symbol) {
    assert (symbol != 0);
    switch (symbol->symbolType ()) {
    case Symbol::SYMBOL:
        if (static_cast<const SymbolSymbol*>(symbol)->secrecType ()->secrecDimType () > 0)
            return true;
    default:
        break;
    }

    return false;
}

LiveMemory::Domain& operator |= (LiveMemory::Domain& dest, LiveMemory::Domain src) {
    return (dest = static_cast<LiveMemory::Domain>(dest | src));
}

struct CollectGenKill {
    CollectGenKill (LiveMemory::Values& gen, LiveMemory::Symbols& kill)
        : m_gen (gen), m_kill (kill)
    { }

    inline void gen (const Symbol* sym, LiveMemory::Domain dom) {
        if (isArray (sym)) {
            m_gen[sym] |= dom;
        }
    }

    inline void kill (const Symbol* sym) {
        m_kill.insert (sym);
    }

    LiveMemory::Values&  m_gen;
    LiveMemory::Symbols& m_kill;
};

struct UpdateValues {
    UpdateValues (LiveMemory::Values& vs)
        : m_values (vs)
    { }


    inline void gen (const Symbol* sym, LiveMemory::Domain dom) {
        if (isArray (sym)) {
            m_values[sym] |= dom;
        }
    }

    inline void kill (const Symbol* sym) {
        m_values.erase (sym);
    }

    LiveMemory::Values& m_values;
};

template <class Visitor>
void visitImop (const Imop& imop, Visitor& visitor) {

    if (imop.isVectorized ()) {
        BOOST_FOREACH (const Symbol* arg, imop.useRange ()) {
            visitor.gen (arg, LiveMemory::Read);
        }

        visitor.gen (imop.dest (), LiveMemory::Write);
        return;
    }


    switch (imop.type ()) {
    case Imop::STORE:     visitor.gen (imop.dest (), LiveMemory::Write);  break;
    case Imop::PUSHCREF:  visitor.gen (imop.dest (), LiveMemory::Write);  break;
    case Imop::PUSH:      visitor.gen (imop.dest (), LiveMemory::Live);   break;
    case Imop::PUSHREF:   visitor.gen (imop.dest (), LiveMemory::Live);   break;
    case Imop::LOAD:      visitor.gen (imop.arg1 (), LiveMemory::Read);   break;
    case Imop::COPY:      visitor.gen (imop.arg1 (), LiveMemory::Read);   /* FALLTHROUGH */
    case Imop::ALLOC:     /* intentionally empty */
    case Imop::PARAM:     /* intentionally empty */
    case Imop::SYSCALL:   visitor.kill (imop.dest ());                    break;
    case Imop::CALL:
        BOOST_FOREACH (const Symbol* arg, imop.useRange ())
            visitor.gen (arg, LiveMemory::Read);

        BOOST_FOREACH (const Symbol* dest, imop.defRange ())
            visitor.kill (dest);

        break;
    default:
        break;
    }
}

} // namespace anonymous

namespace SecreC {

/*******************************************************************************
  LiveMemory
*******************************************************************************/

void LiveMemory::start (const Program& pr) {
    FOREACH_BLOCK (bi, pr) {
        CollectGenKill collector (m_gen[&*bi], m_kill[&*bi]);
        BOOST_REVERSE_FOREACH (const Imop& imop, *bi) {
            visitImop (imop, collector);
        }
    }
}

void LiveMemory::startBlock (const Block& b) {
    m_outs[&b].clear ();
}

void LiveMemory::outToLocal (const Block& from, const Block& to) {
    Values& dest = m_outs[&to];
    const Values& src = m_ins[&from];
    BOOST_FOREACH (Values::const_reference sv, src) {
        dest[sv.first] |= sv.second;
    }
}

void LiveMemory::outToGlobal (const Block& from, const Block& to) {
    Values& dest = m_outs[&to];
    const Values& src = m_ins[&from];
    BOOST_FOREACH (Values::const_reference sv, src) {
        if (isGlobal (sv.first))
            dest[sv.first] |= sv.second;
    }
}

bool LiveMemory::finishBlock (const Block& b) {
    Values& in = m_ins[&b];
    const Values old = in;
    const Values& out = m_outs[&b];
    in = out;
    BOOST_FOREACH (Symbols::const_reference sym, m_kill[&b]) {
        in.erase (sym);
    }

    BOOST_FOREACH (Values::const_reference sv, m_gen[&b]) {
        in[sv.first] |= sv.second;
    }

    return old != in;
}

std::string LiveMemory::toString (const Program& pr) const {
    std::stringstream ss;
    ss << "Memory liveness:\n";
    FOREACH_BLOCK (bi, pr) {
        const Block* block = &*bi;
        BV::const_iterator valsIt = m_outs.find (block);
        if (valsIt == m_outs.end ()) continue;

        const Values& vals = valsIt->second;
        if (vals.empty ()) continue;

        ss << "[Block " << block->index () << "] (size = " << vals.size () << ")\n";
        BOOST_FOREACH (Values::const_reference val, vals) {
            switch (val.second) {
            case Live:  ss << "LIVE";  break;
            case Read:  ss << "READ";  break;
            case Write: ss << "WRITE"; break;
            case Dead:  ss << "DEAD";  break;
            }

            ss << ' ' << val.first->toString () << '\n';
        }
    }

    return ss.str ();
}


} // namespace SecreC
