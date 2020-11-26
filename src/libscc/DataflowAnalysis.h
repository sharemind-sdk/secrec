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

#ifndef SECREC_DATAFLOW_ANALYSIS_H
#define SECREC_DATAFLOW_ANALYSIS_H

#include "Blocks.h"

#include <set>
#include <string>


namespace SecreC {

class Block;
class Blocks;
class Symbol;
class ICode;
class Imop;

class ForwardAnalysisRunner;
class BackwardAnalysisRunner;

/*******************************************************************************
  DataFlowAnalysis
*******************************************************************************/

class DataFlowAnalysis {
    friend class ForwardAnalysisRunner;
    friend class BackwardAnalysisRunner;

protected: /* Types: */

    class ImopSet: public std::set<Imop const*> {

    public: /* Methods: */

        using std::set<Imop const*>::set;
        using std::set<Imop const*>::operator=;

        ImopSet & operator+=(ImopSet const & src) {
            insert(src.begin(), src.end());
            return *this;
        }

        template <class T>
        ImopSet & operator-=(ImopSet const & src) {
            for (auto & e : src)
                erase(e);
            return *this;
        }

    };

protected: /* Methods: */

    DataFlowAnalysis (bool forward, bool backward)
        : m_forward (forward)
        , m_backward (backward)
    { }

public:

    DataFlowAnalysis(DataFlowAnalysis const &) noexcept = default;

    virtual ~DataFlowAnalysis () { }

    inline bool isForward () const { return m_forward; }
    inline bool isBackward () const { return m_backward; }

    virtual std::string toString (const Program& pr) const = 0;

protected:

    virtual void start (const Program&) {}
    virtual void startBlock (const Block &) {}
    virtual void inFrom (const Block& from, Edge::Label label, const Block& to) = 0;
    virtual void outTo (const Block& from, Edge::Label label, const Block& to) = 0;
    virtual bool finishBlock (const Block &) { return false; }
    virtual void finish () { }

private: /* Fields: */
    const bool m_forward;
    const bool m_backward;
};

/*******************************************************************************
  ForwardDataFlowAnalysis
*******************************************************************************/

class ForwardDataFlowAnalysis: public DataFlowAnalysis {
public: /* Methods: */
    inline ForwardDataFlowAnalysis() : DataFlowAnalysis(true, false) {}

    void outTo (const Block&, Edge::Label, const Block&) override { }
};

/*******************************************************************************
  BackwardDataFlowAnalysis
*******************************************************************************/

class BackwardDataFlowAnalysis: public DataFlowAnalysis {
public: /* Methods: */
    inline BackwardDataFlowAnalysis() : DataFlowAnalysis(false, true) {}

    void inFrom (const Block&, Edge::Label, const Block&) override { }
};

/*******************************************************************************
  DataFlowAnalysisRunner
*******************************************************************************/

class DataFlowAnalysisRunner {
public: /* Types: */
    using AnalysisSet = std::set<DataFlowAnalysis*>;
    using BackwardAnalysisSet = std::set<BackwardDataFlowAnalysis*>;
    using ForwardAnalysisSet = std::set<ForwardDataFlowAnalysis*>;

public: /* Methods: */
    inline DataFlowAnalysisRunner& addAnalysis (DataFlowAnalysis& a) {
        m_as.insert (&a);
        return *this;
    }

    DataFlowAnalysisRunner& run (const Program &program);
    std::string toString (const Program& program);

private: /* Fields: */
    AnalysisSet m_as;
};

} // namespace SecreC

#endif // REACHINGDEFINITIONS_H
