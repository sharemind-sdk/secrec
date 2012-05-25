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

    typedef std::map<const Block*, unsigned >      IM;
    typedef std::map<const Block*, const Block* >  BM;

public: /* Methods: */

    std::string toString(const Program &program) const;

    const Block* idom (const Block*) const;
    void dominators (const Block* block, std::list<const Block*>& doms) const;

protected:

    virtual void start (const Program &bs);
    virtual void startBlock(const Block& b);
    virtual void inFrom(const Block& from , const Block& to);
    virtual void inFromTrue(const Block& from, const Block& to) { inFrom (from, to); }
    virtual void inFromFalse(const Block& from, const Block& to) {inFrom (from, to); }
    virtual void inFromCallPass(const Block & from, const Block& to) {inFrom (from, to); }
    virtual bool finishBlock(const Block &b);
    virtual inline void finish();

private:

    const Block* intersect (const Block* b1, const Block* b2);
    unsigned dfs (const Block& entry, unsigned n);
    bool visited (const Block& block) const;

private: /* Fields: */

    const Block*   m_newIdom;
    BM             m_doms;
    IM             m_num;
};

} // namespace SecreC

#endif /* SECREC_ANALYSIS_DOMINATORS_H */
