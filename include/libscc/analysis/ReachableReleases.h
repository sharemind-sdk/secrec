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

#include "../dataflowanalysis.h"

namespace SecreC {

class ReachableReleases : public BackwardDataFlowAnalysis {
public: /* Methods: */
    typedef std::set<const Imop*> Domain;
    typedef std::map<const Symbol*, Domain> Values;
    typedef std::map<const Block*, Values> BV;
    typedef std::set<const Symbol*> Symbols;

public: /* Types: */

    std::string toString(const Program &pr) const;

    const Values& releasedOnExit (const Block& block) {
        return m_outs[&block];
    }

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

private: /* Fields: */
    BV m_gen;
    std::map<const Block*, Symbols> m_kill;

    BV m_ins, m_outs;
}; /* class ReachableReleases { */

} /* namespace SecreC { */

#endif /* SECREC_REACHABLE_RELEASES_H */
