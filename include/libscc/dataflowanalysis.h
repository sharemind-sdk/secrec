#ifndef REACHINGDEFINITIONS_H
#define REACHINGDEFINITIONS_H

#include <cassert>
#include <map>
#include <set>
#include <string>

namespace SecreC {

class Block;
class Blocks;
class Symbol;
class ICode;
class Imop;


class DataFlowAnalysis {
    public: /* Methods: */
        inline DataFlowAnalysis(bool forward, bool backward) : m_forward(forward), m_backward(backward) {}

        inline bool isForward() const { return m_forward; }

        virtual void start(const Block & /* entry or exit block */) {}
        virtual void startBlock(const Block &) {}
        virtual void inFrom(const Block & /* from */, const Block & /* to */) {}
        virtual void inFromTrue(const Block & /* from */, const Block & /* to */) {}
        virtual void inFromFalse(const Block & /* from */, const Block & /* to */) {}
        virtual void inFromCall(const Block & /* from */, const Block & /* to */) {}
        virtual void inFromCallPass(const Block & /* from */, const Block & /* to */) {}
        virtual void inFromRet(const Block & /* from */, const Block & /* to */) {}
        virtual void outTo(const Block & /* from */, const Block & /* to */) {}
        virtual void outToTrue(const Block & /* from */, const Block & /* to */) {}
        virtual void outToFalse(const Block & /* from */, const Block & /* to */) {}
        virtual void outToCall(const Block & /* from */, const Block & /* to */) {}
        virtual void outToCallPass(const Block & /* from */, const Block & /* to */) {}
        virtual void outToRet(const Block & /* from */, const Block & /* to */) {}
        virtual bool finishBlock(const Block &) { return false; }
        virtual void finish() {}

    private: /* Fields: */
        bool m_forward;
        bool m_backward;
};

class ForwardDataFlowAnalysis: public DataFlowAnalysis {
    public: /* Methods: */
        inline ForwardDataFlowAnalysis() : DataFlowAnalysis(true, false) {}
};

class BackwardDataFlowAnalysis: public DataFlowAnalysis {
    public: /* Methods: */
        inline BackwardDataFlowAnalysis() : DataFlowAnalysis(false, true) {}
};

class DataFlowAnalysisRunner {
    public: /* Methods: */
        inline void addAnalysis(DataFlowAnalysis *a) {
            if (a->isForward()) {
                assert(dynamic_cast<ForwardDataFlowAnalysis*>(a));
                m_forwards.insert(static_cast<ForwardDataFlowAnalysis*>(a));
            } else {
                assert(dynamic_cast<BackwardDataFlowAnalysis*>(a));
                m_backwards.insert(static_cast<BackwardDataFlowAnalysis*>(a));
            }
        }
        void run(const Blocks &blocks);

    private: /* Fields: */
        std::set<ForwardDataFlowAnalysis*> m_forwards;
        std::set<BackwardDataFlowAnalysis*> m_backwards;
};

class ReachingDefinitions: public ForwardDataFlowAnalysis {
    public: /* Types: */
        typedef std::set<const Imop*>           Defs;
        typedef std::set<const Imop*>           Jumps;
        typedef std::pair<Defs, Jumps>          SReach;
        typedef std::map<const Symbol*, SReach> SDefs;
        typedef std::map<const Block*, SDefs>   BDM;

    public: /* Methods: */
        inline const SDefs &getReaching(const Block &b) const {
            assert(m_ins.find(&b) != m_ins.end());
            return (*m_ins.find(&b)).second;
        }

        virtual void start(const Block &entryBlock);
        virtual void startBlock(const Block &b);
        virtual void inFrom(const Block &from, const Block &to);
        virtual inline void inFromTrue(const Block &from, const Block &to) { inFrom(from, to); }
        virtual inline void inFromFalse(const Block &from, const Block &to) { inFrom(from, to); }
        virtual inline bool finishBlock(const Block &b) { return makeOuts(b, m_ins[&b], m_outs[&b]); }
        virtual inline void finish() { m_outs.clear(); }

        std::string toString(const Blocks &bs) const;

    private: /* Methods: */
        bool makeOuts(const Block &b, const SDefs &in, SDefs &out);

    private: /* Fields: */
        BDM           m_ins;
        BDM           m_outs;
};

class ReachingJumps: public ForwardDataFlowAnalysis {
public: /* Types: */
        typedef std::set<const Imop*>         Jumps;
        typedef std::map<const Block*, Jumps> BJM;

    public: /* Methods: */
        inline const BJM &getPosJumps() const { return m_inPos; }
        inline const BJM &getNegJumps() const { return m_inNeg; }

        virtual void start(const Block &entryBlock);
        virtual void startBlock(const Block &b);
        virtual void inFrom(const Block &from, const Block &to);
        virtual void inFromTrue(const Block &from, const Block &to);
        virtual void inFromFalse(const Block &from, const Block &to);
        virtual bool finishBlock(const Block &b);
        virtual inline void finish() { m_outPos.clear(); m_outNeg.clear(); }

        std::string toString(const Blocks &bs) const;

    private: /* Fields: */
        BJM           m_inPos;
        BJM           m_inNeg;
        BJM           m_outPos;
        BJM           m_outNeg;
};


} // namespace SecreC


#endif // REACHINGDEFINITIONS_H
