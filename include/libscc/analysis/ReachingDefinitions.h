/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_REACHING_DEFINITIONS_H
#define SECREC_REACHING_DEFINITIONS_H

#include "../DataflowAnalysis.h"

namespace SecreC {

/*******************************************************************************
  ReachingDefinitions
*******************************************************************************/

class ReachingDefinitions: public ForwardDataFlowAnalysis {
public: /* Types: */

    using Defs = std::set<const Imop*>;
    using SDefs = std::map<const Symbol*, Defs>;
    using BDM = std::map<const Block*, SDefs>;

public: /* Methods: */

    static void updateSDefs (const Imop& imop, SDefs& defs);

    inline const SDefs &getReaching(const Block &b) {
        return m_ins[&b];
    }

    std::string toString(const Program &program) const override;

protected:

    virtual void start(const Program &pr) override {
        makeOuts(*pr.entryBlock(), m_ins[pr.entryBlock()], m_outs[pr.entryBlock()]);
    }

    virtual void startBlock(const Block &b) override { m_ins[&b].clear(); }
    virtual void inFrom (const Block &from, Edge::Label label, const Block &to) override {
        return inFrom (from, to, Edge::isGlobal (label));
    }

    virtual inline bool finishBlock(const Block &b) override { return makeOuts(b, m_ins[&b], m_outs[&b]); }
    virtual inline void finish() override { m_outs.clear(); }

private:

    void inFrom (const Block &from, const Block &to, bool globalOnly);
    bool makeOuts (const Block &b, const SDefs &in, SDefs &out);

private: /* Fields: */
    BDM           m_ins;
    BDM           m_outs;
}; // class ReachingDefinitions

} // namespace SecreC

#endif // SECREC_REACHING_DEFINITIONS_H
