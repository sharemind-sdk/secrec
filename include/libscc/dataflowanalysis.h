#ifndef SECREC_DATAFLOW_ANALYSIS_H
#define SECREC_DATAFLOW_ANALYSIS_H

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

/*******************************************************************************
  DataFlowAnalysis
*******************************************************************************/

class DataFlowAnalysis {
    public: /* Methods: */
        inline DataFlowAnalysis(bool forward, bool backward) : m_forward(forward), m_backward(backward) {}

        inline bool isForward() const { return m_forward; }
        inline bool isBackward() const { return m_backward; }

        virtual void start(const Program&) {}
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

        virtual std::string toString (const Program& pr) const = 0;

    private: /* Fields: */
        const bool m_forward;
        const bool m_backward;
};

class ForwardDataFlowAnalysis: public DataFlowAnalysis {
    public: /* Methods: */
        inline ForwardDataFlowAnalysis() : DataFlowAnalysis(true, false) {}
};

class BackwardDataFlowAnalysis: public DataFlowAnalysis {
    public: /* Methods: */
        inline BackwardDataFlowAnalysis() : DataFlowAnalysis(false, true) {}
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
        inline void addAnalysis(DataFlowAnalysis *a) { m_as.insert(a); }
        void run(const Program &program);
        std::string toString (const Program& program);

    private: /* Fields: */
        AnalysisSet m_as;
};

/*******************************************************************************
  ReachingDefinitions
*******************************************************************************/

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

        virtual void start(const Program &pr) {
            // Initialize the OUT set of the entry block:
            makeOuts(*pr.entryBlock(), m_ins[pr.entryBlock()], m_outs[pr.entryBlock()]);
        }
        virtual void startBlock(const Block &b) { m_ins[&b].clear(); }
        virtual inline void inFrom(const Block &from, const Block &to) { inFrom(from, to, false); }
        virtual void inFrom(const Block &from, const Block &to, bool globalOnly);
        virtual inline void inFromTrue(const Block &from, const Block &to) { inFrom(from, to, false); }
        virtual inline void inFromFalse(const Block &from, const Block &to) { inFrom(from, to, false); }
        virtual inline void inFromCallPass(const Block &from, const Block &to) { inFrom(from, to, false); }
        virtual inline void inFromCall(const Block &from, const Block &to) { inFrom(from, to, true); }
        virtual inline void inFromRet(const Block &from, const Block &to) { inFrom(from, to, true); }
        virtual inline bool finishBlock(const Block &b) { return makeOuts(b, m_ins[&b], m_outs[&b]); }
        virtual inline void finish() { m_outs.clear(); }

        std::string toString(const Program &program) const;

    private: /* Methods: */
        bool makeOuts(const Block &b, const SDefs &in, SDefs &out);

    private: /* Fields: */
        BDM           m_ins;
        BDM           m_outs;
};

/*******************************************************************************
  Dominators
*******************************************************************************/

class Dominators : public ForwardDataFlowAnalysis {
public: /* Types: */

    typedef std::map<const Block*, unsigned >      IM;
    typedef std::map<const Block*, const Block* >  BM;

public: /* Methods: */

    virtual void start (const Program &bs);
    virtual void startBlock(const Block& b);
    virtual void inFrom(const Block& from , const Block& to);
    virtual void inFromTrue(const Block& from, const Block& to) { inFrom (from, to); }
    virtual void inFromFalse(const Block& from, const Block& to) {inFrom (from, to); }
    virtual void inFromCallPass(const Block & from, const Block& to) {inFrom (from, to); }
    virtual bool finishBlock(const Block &b);
    virtual inline void finish();

    std::string toString(const Program &program) const;

    const Block* idom (const Block*) const;
    void dominators (const Block* block, std::list<const Block*>& doms) const;

protected:

    const Block* intersect (const Block* b1, const Block* b2);
    unsigned dfs (const Block& entry, unsigned n);
    bool visited (const Block& block) const;

private: /* Fields: */

    const Block*   m_newIdom;
    BM             m_doms;
    IM             m_num;
};


/*******************************************************************************
  LiveVariables
*******************************************************************************/

/**
 * @brief This analysis computes: used variables, defined variables
 * and set of live-on-exit variables for every basic block.
 */
class LiveVariables : public BackwardDataFlowAnalysis {
    public: /* Types: */

        typedef std::set<Symbol const* > Symbols;
        typedef std::map<const Block*, Symbols> BSM;

    public: /* Methods: */

        virtual void start (const Program &bs);
        virtual void startBlock(const Block& b);
        virtual void outTo(const Block &from, const Block &to) { transfer (from, to); }
        virtual void outToTrue(const Block &from, const Block &to) { transfer (from, to);}
        virtual void outToFalse(const Block &from, const Block &to) { transfer (from, to); }
        virtual void outToCallPass(const Block &from, const Block &to) { transfer (from, to); }
        virtual void outToCall(const Block &from, const Block &to) { transferGlobal (from, to); }
        virtual void outToRet(const Block &from, const Block &to) { transferGlobal (from, to);}
        virtual bool finishBlock(const Block &b);
        virtual void finish();

        const Symbols& def (const Block& block) const { return m_def.find (&block)->second; }
        const Symbols& use (const Block& block) const { return m_use.find (&block)->second; }
        const Symbols& ins (const Block& block) const { return m_ins.find (&block)->second; }
        const Symbols& outs (const Block& block) const { return m_outs.find (&block)->second; }

        std::string toString(const Program &pr) const;

    protected:

        void transfer (const Block &from, const Block &to);
        void transferGlobal (const Block &from, const Block &to);

        void useSymbol (const Block& block, const Symbol* sym);
        void defSymbol (const Block& block, const Symbol* sym);

    private: /* Fields: */

        BSM m_use;
        BSM m_def;
        BSM m_outs;
        BSM m_ins;
};

/*******************************************************************************
  ReachingJumps
*******************************************************************************/

class ReachingJumps: public ForwardDataFlowAnalysis {
    public: /* Types: */
        typedef std::set<const Imop*>         Jumps;
        typedef std::map<const Block*, Jumps> BJM;

    public: /* Methods: */
        inline const BJM &getPosJumps() const { return m_inPos; }
        inline const BJM &getNegJumps() const { return m_inNeg; }

        virtual void start(const Program &bs);
        virtual void startBlock(const Block &b);
        virtual void inFrom(const Block &from, const Block &to);
        virtual void inFromTrue(const Block &from, const Block &to);
        virtual void inFromFalse(const Block &from, const Block &to);
        virtual inline void inFromCallPass(const Block &from, const Block &to) { inFrom(from, to); }
        virtual bool finishBlock(const Block &b);
        virtual inline void finish() { m_outPos.clear(); m_outNeg.clear(); }

        std::string toString(const Program &pr) const;

    private: /* Fields: */
        BJM           m_inPos;
        BJM           m_inNeg;
        BJM           m_outPos;
        BJM           m_outNeg;
};

/*******************************************************************************
  ReachingDeclassify
*******************************************************************************/

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
        virtual void start(const Program &pr) {
            // Initialize the OUT set of the entry block:
            makeOuts(*pr.entryBlock (), m_ins[pr.entryBlock()], m_outs[pr.entryBlock()]);
        }

        virtual void startBlock(const Block &b) { m_ins[&b].clear(); }
        virtual void inFrom(const Block &from, const Block &to);
        virtual inline void inFromTrue(const Block &from, const Block &to) { inFrom(from, to); }
        virtual inline void inFromFalse(const Block &from, const Block &to) { inFrom(from, to); }
        virtual inline void inFromCallPass(const Block &from, const Block &to) { inFrom(from, to); }
        virtual inline bool finishBlock(const Block &b) { return makeOuts(b, m_ins[&b], m_outs[&b]); }
        virtual void finish();

        std::string toString(const Program& bs) const;

    private: /* Methods: */
        bool makeOuts(const Block &b, const PDefs &in, PDefs &out);

    private: /* Fields: */
        RD m_ins;
        RD m_outs;
        DD m_ds;
};


} // namespace SecreC


#endif // REACHINGDEFINITIONS_H
