/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_ANALYSIS_REACHING_JUMPS_H
#define SECREC_ANALYSIS_REACHING_JUMPS_H

#include "../dataflowanalysis.h"

namespace SecreC {

/*******************************************************************************
  ReachingJumps
*******************************************************************************/

class ReachingJumps: public ForwardDataFlowAnalysis {
public: /* Types: */
    typedef std::set<const Imop*>         Jumps;
    typedef std::map<const Block*, Jumps> BJM;

public: /* Methods: */

    std::string toString(const Program &pr) const;

    inline const BJM &getPosJumps() const { return m_inPos; }
    inline const BJM &getNegJumps() const { return m_inNeg; }

protected:

    virtual void start(const Program &bs);
    virtual void startBlock(const Block &b);
    virtual void inFrom(const Block &from, Edge::Label label, const Block &to);
    virtual bool finishBlock(const Block &b);
    virtual inline void finish() { m_outPos.clear(); m_outNeg.clear(); }

private: /* Fields: */
    BJM           m_inPos;
    BJM           m_inNeg;
    BJM           m_outPos;
    BJM           m_outNeg;
};

} // namespace SecreC

#endif /* SECREC_ANALYSIS_REACHING_JUMPS_H */
