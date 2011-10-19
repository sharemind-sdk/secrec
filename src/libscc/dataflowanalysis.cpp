#include "dataflowanalysis.h"

#include <algorithm>
#include <iomanip>
#include <utility>

#include "intermediate.h"
#include "misc.h"
#include "treenode.h"

namespace SecreC {
namespace {

template <class T, class U>
inline std::set<T> &operator+=(std::set<T> &dest, const std::set<U> &src) {
    dest.insert(src.begin(), src.end());
    return dest;
}

template <class T, class U>
inline std::set<T> &operator-=(std::set<T> &dest, const std::set<U> &src) {
    typedef typename std::set<U >::const_iterator Iter;
    for (Iter i = src.begin (), e = src.end (); i != e; ++ i)
        dest.erase (*i);
    return dest;
}

#define FOREACH_BLOCKS(it,bs) for (std::set<Block*>::const_iterator it = bs.begin(); it != bs.end(); it++)
#define FOREACH_ANALYSIS(it,as) for (std::set<DataFlowAnalysis*>::const_iterator it = as.begin(); it != as.end(); it++)
#define FOREACH_BANALYSIS(it,as) for (std::set<BackwardDataFlowAnalysis*>::const_iterator it = as.begin(); it != as.end(); it++)
#define FOREACH_FANALYSIS(it,as) for (std::set<ForwardDataFlowAnalysis*>::const_iterator it = as.begin(); it != as.end(); it++)

} // anonymous namespace


void DataFlowAnalysisRunner::run(const Blocks &bs) {
    assert(!m_as.empty());

    AnalysisSet aas;
    BackwardAnalysisSet bas;
    ForwardAnalysisSet fas;

    FOREACH_ANALYSIS(a, m_as) {
        (*a)->start(bs);
        aas.insert(*a);
        if ((*a)->isBackward()) {
            assert(dynamic_cast<BackwardDataFlowAnalysis*>(*a) != 0);
            bas.insert(static_cast<BackwardDataFlowAnalysis*>(*a));
        }
        if ((*a)->isForward()) {
            assert(dynamic_cast<ForwardDataFlowAnalysis*>(*a) != 0);
            fas.insert(static_cast<ForwardDataFlowAnalysis*>(*a));
        }
    }

    for (;;) {
        AnalysisSet unchanged = aas;
        BackwardAnalysisSet bunchanged = bas;
        ForwardAnalysisSet funchanged = fas;

        // For all reachable blocks:
        for (size_t i = 0; i < bs.size(); i++) {
            const Block *b = bs[i];
            if (!b->reachable ()) continue;

            // For forward analysis:
            if (!fas.empty() && b != &bs.entryBlock()) {
                // Notify of start of analyzing block:
                FOREACH_FANALYSIS(a, fas)
                    (*a)->startBlock(*b);

                // Recalculate input sets:
                FOREACH_BLOCKS(it,b->pred ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFrom(**it, *b);

                FOREACH_BLOCKS(it,b->predCondFalse ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromFalse(**it, *b);

                FOREACH_BLOCKS(it,b->predCondTrue ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromTrue(**it, *b);

                FOREACH_BLOCKS(it,b->predCall ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromCall(**it, *b);

                FOREACH_BLOCKS(it,b->predRet ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromRet(**it, *b);

                if (b->callPassFrom () != 0)
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromCallPass(*b->callPassFrom (), *b);

                // Recalculate the output sets:
                FOREACH_FANALYSIS(a, fas)
                    if ((*a)->finishBlock(*b)) {
                        // Analysis changed output set for this block
                        unchanged.erase(*a);
                        funchanged.erase(static_cast<ForwardDataFlowAnalysis*>(*a));
                    }
            }

            // For backward analysis:
            if (!bas.empty() && b != &bs.exitBlock()) {
                // Notify of start of analyzing block:
                FOREACH_BANALYSIS(a, bas)
                    (*a)->startBlock(*b);

                // Recalculate output sets:
                FOREACH_BLOCKS(it,b->succ ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outTo(**it, *b);

                FOREACH_BLOCKS(it,b->succCondFalse ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToFalse(**it, *b);

                FOREACH_BLOCKS(it,b->succCondTrue ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToTrue(**it, *b);

                FOREACH_BLOCKS(it,b->succCall ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToCall(**it, *b);

                FOREACH_BLOCKS(it,b->succRet ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToRet(**it, *b);

                if (b->callPassTo () != 0)
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToCallPass (*b->callPassTo (), *b);

                // Recalculate the input sets:
                FOREACH_BANALYSIS(a, bas)
                    if ((*a)->finishBlock(*b)) {
                        // Analysis changed input set for this block
                        unchanged.erase(*a);
                        bunchanged.erase(static_cast<BackwardDataFlowAnalysis*>(*a));
                    }
            }
        }

        // If all active analyses converged this round, finish all analyses that were run:
        if (unchanged.size() == aas.size()) {
            FOREACH_ANALYSIS(a, m_as)
                (*a)->finish();
            return;
        }

        // Purge the analyses that have finished from the list of active analyses:
        FOREACH_BANALYSIS(a, bunchanged) {
            aas.erase(*a);
            bas.erase(*a);
        }
        FOREACH_FANALYSIS(a, funchanged) {
            aas.erase(*a);
            fas.erase(*a);
        }
    }
}

void ReachingDefinitions::inFrom(const Block &from, const Block &to, bool globalOnly) {
    if (!globalOnly) {
        for (SDefs::const_iterator jt = m_outs[&from].begin(); jt != m_outs[&from].end(); jt++) {
            m_ins[&to][(*jt).first].first += (*jt).second.first;
        }
    } else {
        for (SDefs::const_iterator jt = m_outs[&from].begin(); jt != m_outs[&from].end(); jt++) {
            const Symbol *s = (*jt).first;
            if (((s->symbolType() == Symbol::SYMBOL)
                 && static_cast<const SymbolSymbol*>(s)->scopeType() == SymbolSymbol::GLOBAL)
                || ((s->symbolType() == Symbol::TEMPORARY)
                    && static_cast<const SymbolTemporary*>(s)->scopeType() == SymbolTemporary::GLOBAL))
            {
                m_ins[&to][s].first += (*jt).second.first;
            }
        }
    }
}

bool ReachingDefinitions::makeOuts(const Block &b, const SDefs &in, SDefs &out) {
    SDefs old = out;
    out = in;
    for (Block::const_iterator it = b.begin (); it != b.end (); it++) {
//        if (((*it)->isExpr() || ((*it)->type() == Imop::POP))
        if (((*it)->isExpr())
            && (*it)->dest() != 0)
        {
            // Set this def:
            Defs &d = out[(*it)->dest()].first;

            d.clear();
            d.insert(*it);
        }
    }
    return old != out;
}

std::string ReachingDefinitions::toString(const Blocks &bs) const {
    typedef SDefs::const_iterator SDCI;
    typedef Defs::const_iterator DCI;

    std::ostringstream os;

    os << "Reaching definitions analysis results:" << std::endl;
    for (Blocks::const_iterator bi = bs.begin(); bi != bs.end(); bi++) {
        if (!(*bi)->reachable ()) continue;
        os << "  Block " << (*bi)->index () << ": ";

        BDM::const_iterator si = m_ins.find(*bi);
        if (si == m_ins.end() || (*si).second.empty()) {
            os << " NONE" << std::endl;
        } else {
            os << std::endl;
            for (SDCI it = (*si).second.begin(); it != (*si).second.end(); it++) {
                os << "      " << *(*it).first << ": ";
                const Defs &ds = (*it).second.first;
                for (DCI jt = ds.begin(); jt != ds.end(); jt++) {
                    if (jt != ds.begin()) os << ", ";
                    os << (*jt)->index();
                }
                os << std::endl;
            }
        }
    }
    return os.str();
}

void LiveVariables::start (const Blocks &bs) {
    std::vector<const Symbol*> use, def;
    for (Blocks::const_iterator bi (bs.begin ()); bi != bs.end (); ++ bi) {
        typedef Imop::OperandConstIterator OCI;
        const Block* block = *bi;
        for (Block::const_iterator it (block->begin ()); it != block->end (); ++ it) {
            const Imop* imop = *it;
            imop->getDef (def);
            imop->getUse (use);

            for (std::vector<const Symbol*>::const_iterator i = use.begin (), e = use.end (); i != e; ++ i) {
                useSymbol (block, *i);
            }

            for (std::vector<const Symbol*>::const_iterator i = def.begin (), e = def.end (); i != e; ++ i) {
                defSymbol (block, *i);
            }
        }
    }
}

void LiveVariables::useSymbol (const Block* block, const Symbol *sym) {
    assert (sym != 0);
    switch (sym->symbolType ()) {
    case Symbol::SYMBOL:
    case Symbol::TEMPORARY:
        if (m_def[block].find (sym) == m_def[block].end ())
            m_use[block].insert (sym);
    default:
        break;
    }
}

void LiveVariables::defSymbol (const Block* block, const Symbol *sym) {
    assert (sym != 0);
    switch (sym->symbolType ()) {
    case Symbol::SYMBOL:
    case Symbol::TEMPORARY:
        if (m_use[block].find (sym) == m_use[block].end ())
            m_def[block].insert (sym);
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
    const Symbols& ins = m_ins[&from];
    for (Symbols::const_iterator i = ins.begin (), e = ins.end (); i != e; ++ i) {
        const Symbol* symbol = *i;
        switch (symbol->symbolType ()) {
        case Symbol::TEMPORARY:
            if (static_cast<const SymbolSymbol*>(symbol)->scopeType () == SymbolSymbol::GLOBAL)
                m_outs[&to].insert (symbol);
            break;
        case Symbol::SYMBOL:
            if (static_cast<const SymbolTemporary*>(symbol)->scopeType () == SymbolTemporary::GLOBAL)
                m_outs[&to].insert (symbol);
            break;
        default:
            break;
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

std::string LiveVariables::toString (const Blocks &bs) const {
    std::stringstream ss;
    ss << "Live variables\n";
    for (Blocks::const_iterator bi (bs.begin ()); bi != bs.end (); ++ bi) {
        const Block* block = *bi;
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
                    for (Symbols::const_iterator it (syms.begin ()); it != syms.end (); ++ it) {
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

/**
  \todo ReachingJumps fails on "while (e1) if (e2) break;"
*/

void ReachingJumps::start(const Blocks &bs) {
    m_inPos.insert(std::make_pair(&bs.entryBlock(), Jumps()));
    m_inNeg.insert(std::make_pair(&bs.entryBlock(), Jumps()));
}

void ReachingJumps::startBlock(const Block &b) {
    m_inNeg[&b].clear();
    m_inPos[&b].clear();
}

void ReachingJumps::inFrom(const Block &from, const Block &to) {
    m_inNeg[&to] += m_outNeg[&from];
    m_inPos[&to] += m_outPos[&from];
}

void ReachingJumps::inFromFalse(const Block &from, const Block &to) {
    const Imop *cjump = from.lastImop();
    assert(cjump->isCondJump());
    Jumps inPosT = m_outPos[&from];
    inPosT.erase(cjump);
    m_inPos[&to] += inPosT;
    m_inNeg[&to] += m_outNeg[&from];
    m_inNeg[&to].insert(cjump);
}

void ReachingJumps::inFromTrue(const Block &from, const Block &to) {
    const Imop *cjump = from.lastImop();
    assert(cjump->isCondJump());
    Jumps inNegT = m_outNeg[&from];
    inNegT.erase(cjump);
    m_inNeg[&to] += inNegT;
    m_inPos[&to] += m_outPos[&from];
    m_inPos[&to].insert(cjump);
}

bool ReachingJumps::finishBlock(const Block &b) {
    bool changed = false;
    if (m_inNeg[&b] != m_outNeg[&b]) {
        changed = true;
        m_outNeg[&b] = m_inNeg[&b];
    }
    if (m_inPos[&b] != m_outPos[&b]) {
        changed = true;
        m_outPos[&b] = m_inPos[&b];
    }
    return changed;
}

std::string ReachingJumps::toString(const Blocks &bs) const {
    typedef std::set<const Imop*>::const_iterator ISCI;
    typedef std::map<unsigned long, char>::iterator       LCMI;
    typedef std::map<unsigned long, char>::const_iterator LCMCI;
    typedef BJM::const_iterator BJMCI;

    std::ostringstream os;

    os << "Reaching jumps analysis results:" << std::endl;
    for (Blocks::const_iterator bi = bs.begin(); bi != bs.end(); bi++) {
        if (!(*bi)->reachable ()) continue;

        os << "  Block " << (*bi)->index () << ": ";

        BJMCI posi = m_inPos.find(*bi);
        BJMCI negi = m_inNeg.find(*bi);

        std::map<unsigned long, char> jumps;

        if (posi != m_inPos.end()) {
            for (ISCI jt = (*posi).second.begin(); jt != (*posi).second.end(); jt++) {
                jumps.insert(std::make_pair((*jt)->index (), '+'));
            }
            if (negi != m_inNeg.end()) {
                for (ISCI jt = (*negi).second.begin(); jt != (*negi).second.end(); jt++) {
                    LCMI kt = jumps.find((*jt)->index());
                    if (kt != jumps.end()) {
                        (*kt).second = '*';
                    } else {
                        jumps.insert(std::make_pair((*jt)->index(), '-'));
                    }
                }
            }
        } else if (negi != m_inNeg.end()) {
            for (ISCI jt = (*negi).second.begin(); jt != (*negi).second.end(); jt++) {
                jumps.insert(std::make_pair((*jt)->index(), '-'));
            }
        }

        if (jumps.empty()) {
            os << "NONE";
        } else {
            for (LCMCI jt = jumps.begin(); jt != jumps.end(); jt++) {
                if (jt != jumps.begin()) os << ", ";
                os << (*jt).first << (*jt).second;
            }
        }
        os << std::endl;
    }
    return os.str();
}

void ReachingDeclassify::inFrom(const Block &from, const Block &to) {
    for (PDefs::const_iterator jt = m_outs[&from].begin(); jt != m_outs[&from].end(); jt++) {
        m_ins[&to][(*jt).first].nonsensitive += (*jt).second.nonsensitive;
        m_ins[&to][(*jt).first].sensitive += (*jt).second.sensitive;
    }
}

bool ReachingDeclassify::makeOuts(const Block &b, const PDefs &in, PDefs &out) {
    PDefs old = out;
    out = in;
    for (Block::const_iterator it = b.begin (); it != b.end (); ++ it) {
        if (!(*it)->isExpr()) {
            if ((*it)->type() != Imop::PARAM)
                continue;
        } else {
            if ((*it)->dest() == 0) continue;
            if ((*it)->type() == Imop::DECLASSIFY) {
                m_ds[*it] = out[(*it)->arg1()];
                continue;
            }
        }
        if ((*it)->dest()->secrecType().secrecSecType() == SECTYPE_PUBLIC) continue;

        Defs &d = out[(*it)->dest()];

        switch ((*it)->type()) {
            case Imop::PARAM:
            case Imop::CALL:
                d.nonsensitive.clear();
                d.sensitive.clear();
                d.sensitive.insert(*it);
                break;
            case Imop::ASSIGN:
            case Imop::UMINUS:
            case Imop::UNEG:
                if ((*it)->arg1()->symbolType() != Symbol::CONSTANT) {
                    if ((*it)->dest() != (*it)->arg1()) {
                        d = out[(*it)->arg1()];
                    }
                } else {
                    d.nonsensitive.clear();
                    d.nonsensitive.insert(*it);
                    d.sensitive.clear();
                }
                break;
            default:
                d.nonsensitive.clear();
                d.nonsensitive.insert(*it);
                d.sensitive.clear();
                break;
        }
    }
    return old != out;
}

void ReachingDeclassify::finish() {
    m_outs.clear();
    m_ins.clear();

    /// \todo optimize this:
    DD oldDs(m_ds);
    m_ds.clear();
    for (DD::const_iterator it = oldDs.begin(); it != oldDs.end(); it++) {
        if (!(*it).second.sensitive.empty()) {
            m_ds.insert(*it);
        }
    }
}

std::string ReachingDeclassify::toString(const Blocks&) const {
    assert(m_ins.empty());
    std::ostringstream os;
    os << "Trivial declassify analysis results:" << std::endl;
    if (m_ds.empty()) {
        os << "    No trivial leaks found! :)" << std::endl;
        return os.str();
    }
    for (DD::const_iterator it = m_ds.begin(); it != m_ds.end(); it++) {
        os << "    declassify at "
           << (*it).first->creator()->location()
           << ((*it).second.nonsensitive.empty() ? " leaks the value from:" : " might fully leak the value from:")
           << std::endl;
        for (std::set<const Imop*>::const_iterator jt = (*it).second.sensitive.begin(); jt != (*it).second.sensitive.end(); jt++) {
            os << "        ";
            switch ((*jt)->type()) {
                case Imop::PARAM:
                case Imop::CALL:
                    if (dynamic_cast<TreeNodeStmtDecl*>((*jt)->creator()) != 0) {
                        os << "parameter "
                           << static_cast<TreeNodeStmtDecl*>((*jt)->creator())->variableName()
                           << " declared at " << (*jt)->creator()->location();
                    } else {
                        assert(dynamic_cast<TreeNodeExprProcCall*>((*jt)->creator()) != 0);
                        TreeNodeExprProcCall *c = static_cast<TreeNodeExprProcCall*>((*jt)->creator());
                        assert(c->symbolProcedure() != 0);
                        assert(c->symbolProcedure()->decl() != 0);
                        os << "call to " << c->symbolProcedure()->decl()->procedureName()
                           << " at " << c->location();
                    }
                    break;
                default:
                    assert(false);
            }
            os << std::endl;
        }
    }
    return os.str();
}

} // namespace SecreC
