/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#ifndef SECREC_LIVE_VARIABLES_H
#define SECREC_LIVE_VARIABLES_H

#include "../DataflowAnalysis.h"

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

    using Symbols = boost::container::flat_set<const Symbol* >;
    using BSM = std::map<const Block*, Symbols>;

    struct BlockInfo {
        Symbols gen;
        Symbols kill;
        Symbols in;
        Symbols out;
    };

    using BlockInfoMap = std::map<const Block*, BlockInfo>;

public: /* Methods: */

    std::string toString (const Program &pr) const override;

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

    virtual void start (const Program &bs) override;
    virtual void startBlock(const Block& b) override;
    virtual void outTo(const Block &from, Edge::Label label, const Block &to) override {
        if (Edge::isGlobal (label)) {
            outToGlobal (from, to);
        }
        else {
            outToLocal (from, to);
        }
    }

    virtual bool finishBlock(const Block &b) override;
    virtual void finish() override;


private:

    void outToLocal (const Block &from, const Block &to);
    void outToGlobal (const Block &from, const Block &to);

    BlockInfo & findBlock (const Block & block) {
        return m_blocks[&block];
    }

    const BlockInfo & findBlock (const Block & block) const {
        auto it = m_blocks.find (&block);
        assert (it != m_blocks.end ());
        return it->second;
    }

private: /* Fields: */

    BlockInfoMap m_blocks;
};

} // namespace SecreC

#endif
