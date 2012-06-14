#include "dataflowanalysis.h"

#include <algorithm>
#include <iomanip>
#include <utility>
#include <boost/foreach.hpp>

#include "intermediate.h"
#include "misc.h"
#include "treenode.h"

namespace SecreC {

DataFlowAnalysisRunner& DataFlowAnalysisRunner::run (const Program &pr) {
    assert(!m_as.empty());

    AnalysisSet aas;
    BackwardAnalysisSet bas;
    ForwardAnalysisSet fas;

    BOOST_FOREACH (DataFlowAnalysis* a, m_as) {
        a->start (pr);
        aas.insert (a);
        if (a->isBackward()) {
            assert(dynamic_cast<BackwardDataFlowAnalysis*>(a) != 0);
            bas.insert(static_cast<BackwardDataFlowAnalysis*>(a));
        }
        if (a->isForward()) {
            assert(dynamic_cast<ForwardDataFlowAnalysis*>(a) != 0);
            fas.insert(static_cast<ForwardDataFlowAnalysis*>(a));
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
                BOOST_FOREACH (ForwardDataFlowAnalysis* a, fas) {
                    a->startBlock (*i);
                }

                const Block& to = *i;
                BOOST_FOREACH (const Block::edge_type& edge, to.pred_range ()) {
                    Edge::Label label = edge.second;
                    const Block& from = *edge.first;
                    BOOST_FOREACH (ForwardDataFlowAnalysis* a, fas) {
                        a->inFrom (from, label, to);
                    }
                }

                // Recalculate the output sets:
                BOOST_FOREACH (ForwardDataFlowAnalysis* a, fas) {
                    if (a->finishBlock(*i)) {
                        // Analysis changed output set for this block
                        unchanged.erase(a);
                        funchanged.erase(a);
                    }
                }
            }

            // For backward analysis:
            if (!bas.empty() && !i->isProgramExit ()) {
                // Notify of start of analyzing block:
                BOOST_FOREACH (BackwardDataFlowAnalysis* a, bas) {
                    a->startBlock(*i);
                }

                const Block& to = *i;
                BOOST_FOREACH (const Block::edge_type& edge, to.succ_range ()) {
                    Edge::Label label = edge.second;
                    const Block& from = *edge.first;
                    BOOST_FOREACH (BackwardDataFlowAnalysis* a, bas) {
                        a->outTo (from, label, to);
                    }
                }

                // Recalculate the input sets:
                BOOST_FOREACH (BackwardDataFlowAnalysis* a, bas) {
                    if (a->finishBlock(*i)) {
                        // Analysis changed input set for this block
                        unchanged.erase(a);
                        bunchanged.erase(a);
                    }
                }
            }
        }

        // If all active analyses converged this round, finish all analyses that were run:
        if (unchanged.size() == aas.size()) {
            BOOST_FOREACH (DataFlowAnalysis* a, m_as) {
                a->finish();
            }

            return *this;
        }

        // Purge the analyses that have finished from the list of active analyses:
        BOOST_FOREACH (BackwardDataFlowAnalysis* a, bunchanged) {
            aas.erase (a);
            bas.erase (a);
        }

        BOOST_FOREACH (ForwardDataFlowAnalysis* a, funchanged) {
            aas.erase (a);
            fas.erase (a);
        }
    }

    return *this;
}

std::string DataFlowAnalysisRunner::toString (const Program& program) {
    std::ostringstream os;
    BOOST_FOREACH (DataFlowAnalysis* a, m_as) {
        os << a->toString (program);
    }

    return os.str ();
}

} // namespace SecreC
