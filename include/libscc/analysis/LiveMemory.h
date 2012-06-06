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
    typedef std::vector<Values> LivenessVector;

public: /* Methods: */

    std::string toString(const Program &pr) const;

    std::string deadCopies (const Program& pr) const;

protected:

    virtual void start (const Program& pr);
    virtual void startBlock(const Block& b);
    virtual void outTo(const Block &from, const Block &to) { outToLocal (from, to); }
    virtual void outToTrue(const Block &from, const Block &to) { outToLocal (from, to); }
    virtual void outToFalse(const Block &from, const Block &to) { outToLocal (from, to); }
    virtual void outToCallPass(const Block &from, const Block &to) { outToLocal (from, to); }
    virtual void outToCall(const Block &from, const Block &to)  { outToGlobal (from, to); }
    virtual void outToRet(const Block &from, const Block &to) { outToGlobal (from, to); }
    virtual bool finishBlock(const Block &b);
    virtual void finish () { }

private:

    void outToLocal (const Block &from, const Block &to);
    void outToGlobal (const Block &from, const Block &to);

private: /* Fields: */
    BV m_gen;
    BS m_kill;
    BV m_ins, m_outs;
}; // class LiveMemory

} // namespace SecreC

#endif // SECREC_LIVE_MEMORY_H
