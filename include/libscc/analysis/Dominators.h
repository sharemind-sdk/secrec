/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_ANALYSIS_DOMINATORS_H
#define SECREC_ANALYSIS_DOMINATORS_H

#include "../dataflowanalysis.h"

namespace SecreC {

/*******************************************************************************
  Dominators
*******************************************************************************/

class Dominators : public ForwardDataFlowAnalysis {
public: /* Types: */

    typedef std::map<const Block*, const Block* >  BM;

public: /* Methods: */

    std::string toString(const Program &program) const;

    const Block* idom (const Block*) const;
    void dominators (const Block* block, std::vector<const Block*>& doms) const;

protected:

    virtual void start (const Program &bs);
    virtual void startBlock(const Block& b);
    virtual void inFrom(const Block& from, Edge::Label label, const Block& to);
    virtual bool finishBlock(const Block &b);
    virtual inline void finish();

private:

    const Block* intersect (const Block* b1, const Block* b2);

private: /* Fields: */

    const Block*   m_newIdom;
    BM             m_doms;
};

} // namespace SecreC

#endif /* SECREC_ANALYSIS_DOMINATORS_H */
