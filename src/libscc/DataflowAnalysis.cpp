/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "DataflowAnalysis.h"

#include "Intermediate.h"

#include <functional>
#include <thread>
#include <vector>

namespace SecreC {

namespace /* anonymous */ {

template <class Analysis>
class AnalysisRunner {
public: /* Methods: */
    AnalysisRunner (Analysis& a, const Program& p)
        : m_analysis (a)
        , m_program (p)
    { }

    template <typename WorkList>
    void populateWorkList (WorkList& list) const {
        FOREACH_BLOCK (blockIt, m_program) {
            const Block& block = *blockIt;
            if (block.reachable ()) {
                list.insert (std::cref (block));
            }
        }
    }

protected: /* Fields: */
    Analysis&       m_analysis;
    const Program&  m_program;
};

} // namespace anonymous

/*******************************************************************************
  ForwardAnalysisRunner
*******************************************************************************/

class ForwardAnalysisRunner : AnalysisRunner<ForwardDataFlowAnalysis> {
private: /* Types: */
    struct BlockCmp {
        inline bool operator () (const Block& b1, const Block& b2) const {
            return b1.dfn () > b2.dfn ();
        }
    };

    using WorkList = std::set<std::reference_wrapper<const Block>, BlockCmp>;
    using Base = AnalysisRunner<ForwardDataFlowAnalysis>;

public: /* Methods: */

    ForwardAnalysisRunner (ForwardDataFlowAnalysis& a, const Program& p)
        : AnalysisRunner<ForwardDataFlowAnalysis> (a, p)
    { }

    inline void operator () () const {
        WorkList current, next;
        Base::populateWorkList (current);
        m_analysis.start (m_program);
        while (! current.empty ()) {
            auto i = current.begin ();
            const Block& cur = *i;
            current.erase (i);
            if (! cur.reachable ()) continue;
            m_analysis.startBlock (cur);
            for (const auto& edge : cur.predecessors ()) {
                m_analysis.inFrom (*edge.first, edge.second, cur);
            }

            if (m_analysis.finishBlock (cur)) {
                for (const auto& edge : cur.successors ()) {
                    next.insert (std::cref (*edge.first));
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
        inline bool operator () (const Block& b1, const Block& b2) const {
            return b1.dfn () < b2.dfn ();
        }
    };

    using WorkList = std::set<std::reference_wrapper<const Block>, BlockCmp>;
    using Base = AnalysisRunner<BackwardDataFlowAnalysis>;

public: /* Methods: */

    BackwardAnalysisRunner (BackwardDataFlowAnalysis& a, const Program& p)
        : AnalysisRunner<BackwardDataFlowAnalysis> (a, p)
    { }

    inline void operator () () const {
        WorkList current, next;
        Base::populateWorkList (current);
        m_analysis.start (m_program);
        while (! current.empty ()) {
            auto i = current.begin ();
            const Block& cur = *i;
            current.erase (i);
            if (! cur.reachable ()) continue;
            m_analysis.startBlock (cur);
            for (const auto& edge : cur.successors ()) {
                m_analysis.outTo (*edge.first, edge.second, cur);
            }

            if (m_analysis.finishBlock (cur)) {
                for (const auto& edge : cur.predecessors ()) {
                    next.insert (std::cref (*edge.first));
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
    std::vector<std::thread> threads;
    threads.reserve (m_as.size ());
    for (DataFlowAnalysis* a : m_as) {
        if (a->isForward ()) {
            assert (dynamic_cast<ForwardDataFlowAnalysis*>(a) != nullptr);
            ForwardDataFlowAnalysis& fa = *static_cast<ForwardDataFlowAnalysis*>(a);
            threads.emplace_back (ForwardAnalysisRunner (fa, pr));
        }

        if (a->isBackward ()) {
            assert (dynamic_cast<BackwardDataFlowAnalysis*>(a) != nullptr);
            BackwardDataFlowAnalysis& ba = *static_cast<BackwardDataFlowAnalysis*>(a);
            threads.emplace_back (BackwardAnalysisRunner (ba, pr));
        }
    }

    for (auto& thread : threads) {
        thread.join ();
    }

    return *this;
}

std::string DataFlowAnalysisRunner::toString (const Program& program) {
    std::ostringstream os;
    for (DataFlowAnalysis* a : m_as) {
        os << a->toString (program);
    }

    return os.str ();
}

} // namespace SecreC
