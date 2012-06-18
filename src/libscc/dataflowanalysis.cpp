/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "dataflowanalysis.h"

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include "intermediate.h"

namespace /* anonymous */ {

using namespace SecreC;

template <class Analysis>
class AnalysisRunner {
public: /* Methods: */
    AnalysisRunner (Analysis& a, const Program& p)
        : m_analysis (a)
        , m_program (p)
    { }

    ~AnalysisRunner () { }

protected: /* Fields: */
    Analysis&       m_analysis;
    const Program&  m_program;
};

} // namespace anonymous

namespace SecreC {

/*******************************************************************************
  ForwardAnalysisRunner
*******************************************************************************/

class ForwardAnalysisRunner : AnalysisRunner<ForwardDataFlowAnalysis> {
private: /* Types: */
    struct BlockCmp {
        inline bool operator () (const Block* b1, const Block* b2) const {
            assert (b1 != 0 && b2 != 0);
            return b1->dfn () < b2->dfn ();
        }
    };

    typedef std::set<const Block*, BlockCmp> WorkList;

public: /* Methods: */

    ForwardAnalysisRunner (ForwardDataFlowAnalysis& a, const Program& p)
        : AnalysisRunner (a, p)
    { }

    inline void operator () () const {
        WorkList current, next;
        FOREACH_BLOCK (blockIt, m_program) {
            current.insert (&*blockIt);
        }

        m_analysis.start (m_program);
        while (! current.empty ()) {
            WorkList::const_iterator i = current.begin ();
            const Block& cur = **i;
            current.erase (i);
            if (! cur.reachable ()) continue;
            if (cur.isProgramEntry ()) continue;
            m_analysis.startBlock (cur);
            BOOST_FOREACH (const Block::edge_type& edge, cur.pred_range ()) {
                m_analysis.inFrom (*edge.first, edge.second, cur);
            }

            if (m_analysis.finishBlock (cur)) {
                BOOST_FOREACH (const Block::edge_type& edge, cur.succ_range ()) {
                    next.insert (edge.first);
                }
            }

            if (current.empty ()) {
                std::swap (current, next);
            }
        }

        m_analysis.finish ();
    }
};

/*******************************************************************************
  BackwardAnalysisRunner
*******************************************************************************/

class BackwardAnalysisRunner : AnalysisRunner<BackwardDataFlowAnalysis> {
private: /* Types: */

    struct BlockCmp {
        inline bool operator () (const Block* b1, const Block* b2) const {
            assert (b1 != 0 && b2 != 0);
            return b1->dfn () > b2->dfn ();
        }
    };

    typedef std::set<const Block*, BlockCmp> WorkList;

public: /* Methods: */

    BackwardAnalysisRunner (BackwardDataFlowAnalysis& a, const Program& p)
        : AnalysisRunner (a, p)
    { }

    inline void operator () () const {
        WorkList current, next;
        FOREACH_BLOCK (blockIt, m_program) {
            current.insert (&*blockIt);
        }

        m_analysis.start (m_program);
        while (! current.empty ()) {
            WorkList::const_iterator i = current.begin ();
            const Block& cur = **i;
            current.erase (i);
            if (! cur.reachable ()) continue;
            if (cur.isProgramExit ()) continue;
            m_analysis.startBlock (cur);
            BOOST_FOREACH (const Block::edge_type& edge, cur.succ_range ()) {
                m_analysis.outTo (*edge.first, edge.second, cur);
            }

            if (m_analysis.finishBlock (cur)) {
                BOOST_FOREACH (const Block::edge_type& edge, cur.pred_range ()) {
                    next.insert (edge.first);
                }
            }

            if (current.empty ()) {
                std::swap (current, next);
            }
        }

        m_analysis.finish ();
    }
};

/*******************************************************************************
  DataFlowAnalysisRunner
*******************************************************************************/

DataFlowAnalysisRunner& DataFlowAnalysisRunner::run (const Program &pr) {
    boost::thread_group threads;
    BOOST_FOREACH (DataFlowAnalysis* a, m_as) {
        if (a->isForward ()) {
            assert (dynamic_cast<ForwardDataFlowAnalysis*>(a) != 0);
            ForwardDataFlowAnalysis& fa = *static_cast<ForwardDataFlowAnalysis*>(a);
            threads.create_thread (ForwardAnalysisRunner (fa, pr));
        }

        if (a->isBackward ()) {
            assert (dynamic_cast<BackwardDataFlowAnalysis*>(a) != 0);
            BackwardDataFlowAnalysis& ba = *static_cast<BackwardDataFlowAnalysis*>(a);
            threads.create_thread (BackwardAnalysisRunner (ba, pr));
        }
    }

    threads.join_all ();
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
