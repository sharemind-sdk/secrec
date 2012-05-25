#include "dataflowanalysis.h"

#include <algorithm>
#include <iomanip>
#include <utility>
#include <boost/foreach.hpp>

#include "intermediate.h"
#include "misc.h"
#include "treenode.h"

#define FOREACH_BLOCKS(it,bs) for (std::set<Block*>::const_iterator it = bs.begin(); it != bs.end(); it++)
#define FOREACH_ANALYSIS(it,as) for (std::set<DataFlowAnalysis*>::const_iterator it = as.begin(); it != as.end(); it++)
#define FOREACH_BANALYSIS(it,as) for (std::set<BackwardDataFlowAnalysis*>::const_iterator it = as.begin(); it != as.end(); it++)
#define FOREACH_FANALYSIS(it,as) for (std::set<ForwardDataFlowAnalysis*>::const_iterator it = as.begin(); it != as.end(); it++)

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

} // namespace SecreC
