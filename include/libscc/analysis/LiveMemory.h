/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_LIVE_MEMORY_H
#define SECREC_LIVE_MEMORY_H

#include "../dataflowanalysis.h"

namespace SecreC {

/*******************************************************************************
  LiveMemory
*******************************************************************************/

class LiveMemory : public BackwardDataFlowAnalysis {
public: /* Types: */

    enum Domain {
        Dead  = 0x0,
        Read  = 0x1,
        Write = 0x2,
        Live  = 0x3
    };

    typedef std::set<const Symbol*> Symbols;
    typedef std::map<const Symbol*, Domain> Values;
    typedef std::map<const Block*, Values> BV;
    typedef std::map<const Block*, Symbols> BS;

public: /* Methods: */

    std::string toString(const Program &pr) const;

    std::set<const Imop*> deadCopies (const Program& pr) const;

    /**
     * @brief liveOnExit returns abstract values after the given basic block
     * @param block
     * @return the abstract values
     */
    const Values& liveOnExit (const Block& block) {
        return m_outs[&block];
    }

    /**
     * @brief update the value mapping from after the instruction to before
     * @param imop the instruction to update the values at
     * @param vals abstract values for every symbol
     */
    static void update (const Imop& imop, Values& vals);

protected:

    virtual void start (const Program& pr);
    virtual void startBlock(const Block& b);
    virtual void outTo(const Block &from, Edge::Label label, const Block &to) {
        if (Edge::isGlobal (label))
            outToGlobal (from, to);
        else
            outToLocal (from, to);
    }

    virtual bool finishBlock(const Block &b);
    virtual void finish () { }

private:

    void outToLocal (const Block &from, const Block &to);
    void outToGlobal (const Block &from, const Block &to);

    std::string printDeadCopies (const Program& pr) const;

private: /* Fields: */
    BV m_gen;
    BS m_kill;
    BV m_ins, m_outs;
}; // class LiveMemory

} // namespace SecreC

#endif // SECREC_LIVE_MEMORY_H