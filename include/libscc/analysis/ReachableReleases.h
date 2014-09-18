/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_REACHABLE_RELEASES_H
#define SECREC_REACHABLE_RELEASES_H

#include "../DataflowAnalysis.h"

namespace SecreC {

class ReachableReleases : public BackwardDataFlowAnalysis {
public: /* Methods: */
    using Domain = std::set<const Imop*>;
    using Values = std::map<const Symbol*, Domain>;
    using BV = std::map<const Block*, Values>;
    using Symbols = std::set<const Symbol*>;

public: /* Types: */

    std::string toString(const Program &pr) const override;

    const Values& releasedOnExit (const Block& block) {
        return m_outs[&block];
    }

    static void update (const Imop& imop, Values& vals);

protected:

    virtual void start (const Program& pr) override;
    virtual void startBlock(const Block& b) override;
    virtual void outTo(const Block &from, Edge::Label label, const Block &to) override {
        if (Edge::isGlobal (label))
            outToGlobal (from, to);
        else
            outToLocal (from, to);
    }

    virtual bool finishBlock(const Block &b) override;
    virtual void finish () override { }

private:

    void outToLocal (const Block &from, const Block &to);
    void outToGlobal (const Block &from, const Block &to);

private: /* Fields: */
    BV m_gen;
    std::map<const Block*, Symbols> m_kill;

    BV m_ins, m_outs;
}; /* class ReachableReleases { */

} /* namespace SecreC { */

#endif /* SECREC_REACHABLE_RELEASES_H */
