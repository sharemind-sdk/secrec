/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_LIVE_VARIABLES_H
#define SECREC_LIVE_VARIABLES_H

#include "../dataflowanalysis.h"

namespace SecreC {

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

    std::string toString(const Program &pr) const;

    const Symbols& def (const Block& block) const { return m_def.find (&block)->second; }
    const Symbols& use (const Block& block) const { return m_use.find (&block)->second; }
    const Symbols& ins (const Block& block) const { return m_ins.find (&block)->second; }
    const Symbols& outs (const Block& block) const { return m_outs.find (&block)->second; }

protected:

    virtual void start (const Program &bs);
    virtual void startBlock(const Block& b);
    virtual void outTo(const Block &from, Edge::Label label, const Block &to) {
        if (Edge::isGlobal (label)) {
            transferGlobal (from, to);
        }
        else {
            transfer (from, to);
        }
    }

    virtual bool finishBlock(const Block &b);
    virtual void finish();


private:

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

} // namespace SecreC

#endif
