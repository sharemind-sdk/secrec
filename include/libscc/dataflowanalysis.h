#ifndef REACHINGDEFINITIONS_H
#define REACHINGDEFINITIONS_H

#include <cassert>
#include <map>
#include <set>
#include <string>
#include "blocks.h"

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
        inline bool isBackward() const { return m_backward; }

        virtual void start(const Blocks &) {}
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

        virtual std::string toString (const Blocks& bs) const = 0;

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
    public: /* Types: */
        typedef std::set<DataFlowAnalysis*>         AnalysisSet;
        typedef std::set<BackwardDataFlowAnalysis*> BackwardAnalysisSet;
        typedef std::set<ForwardDataFlowAnalysis*>  ForwardAnalysisSet;

    public: /* Methods: */
        inline void addAnalysis(DataFlowAnalysis *a) { m_as.insert(a); }
        void run(const Blocks &blocks);

    private: /* Fields: */
        AnalysisSet m_as;
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

        virtual void start(const Blocks &bs) {
            // Initialize the OUT set of the entry block:
            makeOuts(bs.entryBlock(), m_ins[&bs.entryBlock()], m_outs[&bs.entryBlock()]);
        }
        virtual void startBlock(const Block &b) { m_ins[&b].clear(); }
        virtual inline void inFrom(const Block &from, const Block &to) { inFrom(from, to, false); }
        virtual void inFrom(const Block &from, const Block &to, bool globalOnly);
        virtual inline void inFromTrue(const Block &from, const Block &to) { inFrom(from, to, false); }
        virtual inline void inFromFalse(const Block &from, const Block &to) { inFrom(from, to, false); }
        virtual inline void inFromCall(const Block &from, const Block &to) { inFrom(from, to, true); }
        virtual inline void inFromCallPass(const Block &from, const Block &to) { inFrom(from, to, false); }
        virtual inline void inFromRet(const Block &from, const Block &to) { inFrom(from, to, true); }
        virtual inline bool finishBlock(const Block &b) { return makeOuts(b, m_ins[&b], m_outs[&b]); }
        virtual inline void finish() { m_outs.clear(); }

        std::string toString(const Blocks &bs) const;

    private: /* Methods: */
        bool makeOuts(const Block &b, const SDefs &in, SDefs &out);

    private: /* Fields: */
        BDM           m_ins;
        BDM           m_outs;
};

/**
 * @brief This analysis computes: used variables, defined variables
 * and set of live-on-exit variables for every basic block.
 */
class LiveVariables : public BackwardDataFlowAnalysis {
    public:
        typedef std::set<Symbol const* > Symbols;
    public:
        virtual void start (const Blocks &bs);
        virtual void startBlock(const Block &);
        virtual void outTo(const Block &from, const Block &to);
        virtual void outToTrue(const Block &from, const Block &to) { outTo (from, to); }
        virtual void outToFalse(const Block &from, const Block &to) { outTo (from, to); }
        virtual void outToCall(const Block &from, const Block &to) { outTo (from, to); }
        virtual void outToCallPass(const Block &from, const Block &to) { outTo (from, to); }
        virtual void outToRet(const Block &from, const Block &to) { outTo (from, to); }
        virtual bool finishBlock(const Block &b);
        virtual void finish();

        std::string toString(const Blocks &bs) const;

        const Symbols& use (const Block* block) { return m_use[block]; }
        const Symbols& defs (const Block* block) { return m_defs[block]; }
        const Symbols& liveAtExit (const Block* block) { return m_outs[block]; }

    private:
        std::map<const Block*, Symbols > m_use;
        std::map<const Block*, Symbols > m_defs;
        std::map<const Block*, Symbols > m_outs;
        std::map<const Block*, Symbols > m_ins;

        void useSymbol (const Block* block, const Symbol* sym);
        void defSymbol (const Block* block, const Symbol* sym);
};

class ReachingJumps: public ForwardDataFlowAnalysis {
    public: /* Types: */
        typedef std::set<const Imop*>         Jumps;
        typedef std::map<const Block*, Jumps> BJM;

    public: /* Methods: */
        inline const BJM &getPosJumps() const { return m_inPos; }
        inline const BJM &getNegJumps() const { return m_inNeg; }

        virtual void start(const Blocks &bs);
        virtual void startBlock(const Block &b);
        virtual void inFrom(const Block &from, const Block &to);
        virtual void inFromTrue(const Block &from, const Block &to);
        virtual void inFromFalse(const Block &from, const Block &to);
        virtual inline void inFromCallPass(const Block &from, const Block &to) { inFrom(from, to); }
        virtual bool finishBlock(const Block &b);
        virtual inline void finish() { m_outPos.clear(); m_outNeg.clear(); }

        std::string toString(const Blocks &bs) const;

    private: /* Fields: */
        BJM           m_inPos;
        BJM           m_inNeg;
        BJM           m_outPos;
        BJM           m_outNeg;
};

class ReachingDeclassify: public ForwardDataFlowAnalysis {
    public: /* Types: */
        typedef std::set<const Imop*> ImopSet;
        struct Defs {
            ImopSet sensitive;
            ImopSet nonsensitive;
            inline bool operator==(const Defs &o) const {
                return (sensitive == o.sensitive)
                        && (nonsensitive == o.nonsensitive);
            }
        };
        typedef std::map<const Symbol*, Defs> PDefs;
        typedef std::map<const Block*, PDefs> RD;
        typedef std::map<const Imop*, Defs> DD;

    public: /* Methods: */
        virtual void start(const Blocks &bs) {
            // Initialize the OUT set of the entry block:
            makeOuts(bs.entryBlock(), m_ins[&bs.entryBlock()], m_outs[&bs.entryBlock()]);
        }
        virtual void startBlock(const Block &b) { m_ins[&b].clear(); }
        virtual void inFrom(const Block &from, const Block &to);
        virtual inline void inFromTrue(const Block &from, const Block &to) { inFrom(from, to); }
        virtual inline void inFromFalse(const Block &from, const Block &to) { inFrom(from, to); }
        virtual inline void inFromCallPass(const Block &from, const Block &to) { inFrom(from, to); }
        virtual inline bool finishBlock(const Block &b) { return makeOuts(b, m_ins[&b], m_outs[&b]); }
        virtual void finish();

        std::string toString(const Blocks& bs) const;

    private: /* Methods: */
        bool makeOuts(const Block &b, const PDefs &in, PDefs &out);

    private: /* Fields: */
        RD m_ins;
        RD m_outs;
        DD m_ds;
};


} // namespace SecreC


#endif // REACHINGDEFINITIONS_H
