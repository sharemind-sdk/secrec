#include "dataflowanalysis.h"

#include <algorithm>
#include <iomanip>
#include <utility>
#include <boost/foreach.hpp>

#include "intermediate.h"
#include "misc.h"
#include "treenode.h"

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
#define FOREACH_BLOCK(IT,pr) \
    for (Program::const_iterator pit = pr.begin (); pit != pr.end (); ++ pit)\
        for (Procedure::const_iterator IT = pit->begin (); IT != pit->end (); ++ IT)


} // anonymous namespace

namespace SecreC {

void DataFlowAnalysisRunner::run(const Program &pr) {
    assert(!m_as.empty());

    AnalysisSet aas;
    BackwardAnalysisSet bas;
    ForwardAnalysisSet fas;

    FOREACH_ANALYSIS(a, m_as) {
        (*a)->start(pr);
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
        FOREACH_BLOCK (i, pr) {
            if (!i->reachable ()) continue;

            // For forward analysis:
            if (!fas.empty() && !i->isProgramEntry ()) {
                // Notify of start of analyzing block:
                FOREACH_FANALYSIS(a, fas)
                    (*a)->startBlock(*i);

                // Recalculate input sets:
                FOREACH_BLOCKS(it,i->pred ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFrom(**it, *i);

                FOREACH_BLOCKS(it,i->predCondFalse ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromFalse(**it, *i);

                FOREACH_BLOCKS(it,i->predCondTrue ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromTrue(**it, *i);

                FOREACH_BLOCKS(it,i->predCall ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromCall(**it, *i);

                FOREACH_BLOCKS(it,i->predRet ())
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromRet(**it, *i);

                if (i->callPassFrom () != 0)
                    FOREACH_FANALYSIS(a, fas)
                        (*a)->inFromCallPass(*i->callPassFrom (), *i);

                // Recalculate the output sets:
                FOREACH_FANALYSIS(a, fas)
                    if ((*a)->finishBlock(*i)) {
                        // Analysis changed output set for this block
                        unchanged.erase(*a);
                        funchanged.erase(static_cast<ForwardDataFlowAnalysis*>(*a));
                    }
            }

            // For backward analysis:
            if (!bas.empty() && !i->isProgramExit ()) {
                // Notify of start of analyzing block:
                FOREACH_BANALYSIS(a, bas)
                    (*a)->startBlock(*i);

                // Recalculate output sets:
                FOREACH_BLOCKS(it,i->succ ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outTo(**it, *i);

                FOREACH_BLOCKS(it,i->succCondFalse ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToFalse(**it, *i);

                FOREACH_BLOCKS(it,i->succCondTrue ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToTrue(**it, *i);

                FOREACH_BLOCKS(it,i->succCall ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToCall(**it, *i);

                FOREACH_BLOCKS(it,i->succRet ())
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToRet(**it, *i);

                if (i->callPassTo () != 0)
                    FOREACH_BANALYSIS(a, bas)
                        (*a)->outToCallPass (*i->callPassTo (), *i);

                // Recalculate the input sets:
                FOREACH_BANALYSIS(a, bas)
                    if ((*a)->finishBlock(*i)) {
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

std::string DataFlowAnalysisRunner::toString (const Program& program) {
    std::ostringstream os;
    FOREACH_ANALYSIS(a, m_as) {
        os << (*a)->toString (program);
    }

    return os.str ();
}

/*******************************************************************************
  ReachingDefinitions
*******************************************************************************/

void ReachingDefinitions::inFrom(const Block &from, const Block &to, bool globalOnly) {
    if (!globalOnly) {
        for (SDefs::const_iterator jt = m_outs[&from].begin(); jt != m_outs[&from].end(); jt++) {
            m_ins[&to][(*jt).first].first += (*jt).second.first;
        }
    } else {
        for (SDefs::const_iterator jt = m_outs[&from].begin(); jt != m_outs[&from].end(); jt++) {
            const Symbol *s = (*jt).first;
            if ((s->symbolType() == Symbol::SYMBOL)
                && static_cast<const SymbolSymbol*>(s)->scopeType() == SymbolSymbol::GLOBAL)
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
        const Imop& imop = *it;
        BOOST_FOREACH (const Symbol* symbol, imop.defRange ()) {
            Defs& d = out[symbol].first;
            d.clear ();
            d.insert (&imop);
        }
    }

    return old != out;
}

std::string ReachingDefinitions::toString(const Program &pr) const {
    typedef SDefs::const_iterator SDCI;
    typedef Defs::const_iterator DCI;

    std::ostringstream os;

    os << "Reaching definitions analysis results:" << std::endl;
    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable ()) continue;
        os << "  Block " << bi->index () << ": ";

        BDM::const_iterator si = m_ins.find(&*bi);
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

/*******************************************************************************
  Dominators
*******************************************************************************/

bool Dominators::visited (const Block& block) const {
    return m_num.find (&block)->second != 0;
}

/// \todo make this non-recursive
unsigned Dominators::dfs (const Block& entry, unsigned n) {
    if (!visited (entry)) {
        m_num[&entry] = ++ n;
        std::set<Block* > succ;
        entry.getOutgoing (succ);
        for (std::set<Block*>::iterator i = succ.begin (), e = succ.end (); i != e; ++ i) {
            n = dfs (**i, n);
        }
    }

    return n;
}

void Dominators::start (const Program &pr) {
    FOREACH_BLOCK (i, pr) {
        m_doms[&*i] = 0;
        m_num[&*i] = 0;
    }

    unsigned n = 0;
    for (Program::const_iterator i = pr.begin (), e = pr.end (); i != e; ++ i) {
        n = dfs (*i->entry (), n);
        m_doms[i->entry ()] = i->entry ();
    }
}

void Dominators::startBlock (const Block&) {
    m_newIdom = 0;
}

const Block* Dominators::intersect (const Block* b1, const Block* b2) {
       while (b1 != b2) {
           while (m_num[b1] > m_num[b2]) b1 = m_doms[b1];
           while (m_num[b2] > m_num[b1]) b2 = m_doms[b2];
       }

       return b1;
}

void Dominators::inFrom (const Block& from, const Block&) {
    if (m_newIdom == 0) {
        m_newIdom = &from;
        return;
    }

    const Block* idom = m_doms[&from];
    if (idom != 0) {
        m_newIdom = intersect (m_newIdom, idom);
    }
}

bool Dominators::finishBlock (const Block& b) {
    if (m_newIdom != m_doms[&b]) {
        m_doms[&b] = m_newIdom;
        return true;
    }

    return false;
}

void Dominators::finish () { }

const Block* Dominators::idom (const Block* block) const {
    return m_doms.find (block)->second;
}

void Dominators::dominators (const Block* block, std::list<const Block*>& doms) const {
    const Block* prev = block;
    doms.clear ();
    do {
        doms.push_back (block);
        prev = block;
        block = idom (block);
    } while (prev != block);
}

std::string Dominators::toString (const Program& pr) const {
    std::ostringstream ss;

    ss << "IDOMS:\n";
    FOREACH_BLOCK(i, pr) {
        const Block* dominator = m_doms.find (&*i)->second;
        ss << "  " << i->index () << " - " << (dominator ? dominator->index () : 0) << std::endl;
    }

    return ss.str ();
}

/*******************************************************************************
  LiveVariables
*******************************************************************************/

void LiveVariables::start (const Program &pr) {
    FOREACH_BLOCK (bi, pr) {
        typedef Imop::OperandConstIterator OCI;
        for (Block::const_iterator it (bi->begin ()); it != bi->end (); ++ it) {
            BOOST_FOREACH (const Symbol* sym, it->useRange ()) {
                useSymbol (*bi, sym);
            }

            BOOST_FOREACH (const Symbol* sym, it->defRange ()) {
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
    const Symbols& ins = m_ins[&from];
    for (Symbols::const_iterator i = ins.begin (), e = ins.end (); i != e; ++ i) {
        const Symbol* symbol = *i;
        switch (symbol->symbolType ()) {
        case Symbol::SYMBOL:
            if (static_cast<const SymbolSymbol*>(symbol)->scopeType () == SymbolSymbol::GLOBAL)
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

/*******************************************************************************
  ReachingJumps
*******************************************************************************/

void ReachingJumps::start(const Program&) { }

void ReachingJumps::startBlock(const Block &b) {
    m_inNeg[&b].clear();
    m_inPos[&b].clear();
}

void ReachingJumps::inFrom(const Block &from, const Block &to) {
    m_inNeg[&to] += m_outNeg[&from];
    m_inPos[&to] += m_outPos[&from];
}

void ReachingJumps::inFromFalse(const Block &from, const Block &to) {
    const Imop& cjump = from.back ();
    assert(cjump.isCondJump());
    Jumps inPosT = m_outPos[&from];
    inPosT.erase(&cjump);
    m_inPos[&to] += inPosT;
    m_inNeg[&to] += m_outNeg[&from];
    m_inNeg[&to].insert(&cjump);
}

void ReachingJumps::inFromTrue(const Block &from, const Block &to) {
    const Imop& cjump = from.back ();
    assert(cjump.isCondJump());
    Jumps inNegT = m_outNeg[&from];
    inNegT.erase(&cjump);
    m_inNeg[&to] += inNegT;
    m_inPos[&to] += m_outPos[&from];
    m_inPos[&to].insert(&cjump);
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

std::string ReachingJumps::toString(const Program &pr) const {
    typedef std::set<const Imop*>::const_iterator ISCI;
    typedef std::map<unsigned long, char>::iterator       LCMI;
    typedef std::map<unsigned long, char>::const_iterator LCMCI;
    typedef BJM::const_iterator BJMCI;

    std::ostringstream os;

    os << "Reaching jumps analysis results:" << std::endl;
    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable ()) continue;

        os << "  Block " << bi->index () << ": ";

        BJMCI posi = m_inPos.find(&*bi);
        BJMCI negi = m_inNeg.find(&*bi);

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

/*******************************************************************************
  ReachingDeclassify
*******************************************************************************/

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
        const Imop& imop = *it;
        if (!imop.isExpr()) {
            if (imop.type() != Imop::PARAM)
                continue;
        } else {
            if (imop.dest() == 0) continue;
            if (imop.type() == Imop::DECLASSIFY) {
                m_ds[&imop] = out[imop.arg1()];
                continue;
            }
        }

        if (imop.dest()->secrecType()->secrecSecType()->isPublic ()) continue;

        Defs &d = out[imop.dest()];

        switch (imop.type()) {
            case Imop::PARAM:
            case Imop::CALL:
                d.nonsensitive.clear();
                d.sensitive.clear();
                d.sensitive.insert(&imop);
                break;
            case Imop::ASSIGN:
            case Imop::UMINUS:
            case Imop::UNEG:
                if (imop.arg1()->symbolType() != Symbol::CONSTANT) {
                    if (imop.dest() != imop.arg1()) {
                        d = out[imop.arg1()];
                    }
                } else {
                    d.nonsensitive.clear();
                    d.nonsensitive.insert(&imop);
                    d.sensitive.clear();
                }
                break;
            default:
                d.nonsensitive.clear();
                d.nonsensitive.insert(&imop);
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

std::string ReachingDeclassify::toString(const Program&) const {
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
