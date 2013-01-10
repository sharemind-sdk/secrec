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
#include <boost/interprocess/containers/flat_set.hpp>

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

    typedef boost::container::flat_set<const Symbol* > Symbols;
    typedef std::map<const Block*, Symbols> BSM;

    struct BlockInfo {
        Symbols gen;
        Symbols kill;
        Symbols in;
        Symbols out;
    };

    typedef std::map<const Block*, BlockInfo> BlockInfoMap;

public: /* Methods: */

    std::string toString (const Program &pr) const;

    /**
     * @brief liveOnExit returns abstract values after the given basic block
     * @param block
     * @return the abstract values
     */
    const Symbols& liveOnExit (const Block& block) const {
        return findBlock (block).out;
    }

    const Symbols& ins (const Block& block) const {
        return findBlock (block).in;
    }

    static void updateBackwards (const Imop& imop, Symbols& live);

protected:

    virtual void start (const Program &bs);
    virtual void startBlock(const Block& b);
    virtual void outTo(const Block &from, Edge::Label label, const Block &to) {
        if (Edge::isGlobal (label)) {
            outToGlobal (from, to);
        }
        else {
            outToLocal (from, to);
        }
    }

    virtual bool finishBlock(const Block &b);
    virtual void finish();


private:

    void outToLocal (const Block &from, const Block &to);
    void outToGlobal (const Block &from, const Block &to);

    BlockInfo & findBlock (const Block & block) {
        return m_blocks[&block];
    }

    const BlockInfo & findBlock (const Block & block) const {
        BlockInfoMap::const_iterator it = m_blocks.find (&block);
        assert (it != m_blocks.end ());
        return it->second;
    }

private: /* Fields: */

    BlockInfoMap m_blocks;
};

} // namespace SecreC

#endif
