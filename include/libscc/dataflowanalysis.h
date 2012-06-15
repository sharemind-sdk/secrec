#ifndef SECREC_DATAFLOW_ANALYSIS_H
#define SECREC_DATAFLOW_ANALYSIS_H

#include <set>
#include <string>

#include "blocks.h"

#define FOREACH_BLOCK(IT,pr) \
    for (Program::const_iterator pit = pr.begin (); pit != pr.end (); ++ pit)\
        for (Procedure::const_iterator IT = pit->begin (); IT != pit->end (); ++ IT)

template <class T, class U>
inline std::set<T> &operator+=(std::set<T> &dest, const std::set<U> &src) {
    dest.insert (src.begin(), src.end());
    return dest;
}

template <class T, class U>
inline std::set<T> &operator-=(std::set<T> &dest, const std::set<U> &src) {
    typedef typename std::set<U >::const_iterator Iter;
    for (Iter i = src.begin (), e = src.end (); i != e; ++ i)
        dest.erase (*i);
    return dest;
}

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
protected: /* Methods: */

    DataFlowAnalysis (bool forward, bool backward)
        : m_forward (forward)
        , m_backward (backward)
    { }

public:

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

class ForwardDataFlowAnalysis: public DataFlowAnalysis {
public: /* Methods: */
    inline ForwardDataFlowAnalysis() : DataFlowAnalysis(true, false) {}

    void outTo (const Block&, Edge::Label, const Block&) { }
};

class BackwardDataFlowAnalysis: public DataFlowAnalysis {
public: /* Methods: */
    inline BackwardDataFlowAnalysis() : DataFlowAnalysis(false, true) {}

    void inFrom (const Block&, Edge::Label, const Block&) { }
};

/*******************************************************************************
  DataFlowAnalysisRunner
*******************************************************************************/

class DataFlowAnalysisRunner {
public: /* Types: */
    typedef std::set<DataFlowAnalysis*>         AnalysisSet;
    typedef std::set<BackwardDataFlowAnalysis*> BackwardAnalysisSet;
    typedef std::set<ForwardDataFlowAnalysis*>  ForwardAnalysisSet;

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
