#include "analysis/LiveVariables.h"

#include <boost/foreach.hpp>

#include "symbol.h"
#include "treenode.h"

namespace SecreC {

/*******************************************************************************
  LiveVariables
*******************************************************************************/

void LiveVariables::start (const Program &pr) {
    FOREACH_BLOCK (bi, pr) {
        typedef Imop::OperandConstIterator OCI;
        BOOST_FOREACH (const Imop& imop, *bi) {
            BOOST_FOREACH (const Symbol* sym, imop.useRange ()) {
                useSymbol (*bi, sym);
            }

            BOOST_FOREACH (const Symbol* sym, imop.defRange ()) {
                defSymbol (*bi, sym);
            }
        }
    }
}

void LiveVariables::useSymbol (const Block& block, const Symbol *sym) {
    assert (sym != 0);
    switch (sym->symbolType ()) {
    case Symbol::SYMBOL:
        if (m_def[&block].find (sym) == m_def[&block].end ())
            m_use[&block].insert (sym);
    default:
        break;
    }
}

void LiveVariables::defSymbol (const Block& block, const Symbol *sym) {
    assert (sym != 0);
    switch (sym->symbolType ()) {
    case Symbol::SYMBOL:
        if (m_use[&block].find (sym) == m_use[&block].end ())
            m_def[&block].insert (sym);
    default:
        break;
    }
}

void LiveVariables::startBlock (const Block& b) {
    m_outs[&b].clear ();
}

// transfer all inputs
void LiveVariables::transfer (const Block &from, const Block &to) {
    m_outs[&to] += m_ins[&from];
}

// transfer only global inputs
void LiveVariables::transferGlobal (const Block& from, const Block& to) {
    BOOST_FOREACH (const Symbol* symbol, m_ins[&from]) {
        if (symbol->isGlobal ()) {
            m_outs[&to].insert (symbol);
        }
    }
}

bool LiveVariables::finishBlock (const Block &b) {
    Symbols& in = m_ins [&b];
    const Symbols& out = m_outs [&b];
    const Symbols  old = in;
    in = out;
    in -= m_def[&b];
    in += m_use[&b];
    return old != in;
}

void LiveVariables::finish () { }

std::string LiveVariables::toString (const Program &pr) const {
    std::stringstream ss;
    ss << "Live variables\n";
    FOREACH_BLOCK (bi, pr) {
        const Block* block = &*bi;
        BSM::const_iterator es [4] = { m_use.end (), m_def.end (), m_ins.end (), m_outs.end () };
        BSM::const_iterator is [4] = { m_use.find (block), m_def.find (block), m_ins.find (block), m_outs.find (block) };
        const char* names [4] = {"USE", "DEF", " IN", "OUT"};
        bool headerPrinted = false;
        for (int k = 0; k < 4; ++ k) {
            if (is[k] != es[k]) {
                const Symbols& syms = is[k]->second;
                if (!syms.empty ()) {
                    if (!headerPrinted)
                        ss << "[Block " << block->index () << "]\n";
                    ss << '\t' << names[k] << ": ";
                    for (Symbols::const_iterator it = syms.begin (); it != syms.end (); ++ it) {
                        if (it != syms.begin ()) ss << ", ";
                        ss << (*it)->name ();
                    }

                    ss << '\n';
                    headerPrinted = true;
                }
            }
        }
    }

    return ss.str ();
}

} // namespace SecreC
