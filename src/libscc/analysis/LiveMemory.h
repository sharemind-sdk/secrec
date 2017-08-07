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

#ifndef SECREC_LIVE_MEMORY_H
#define SECREC_LIVE_MEMORY_H

#include "../DataflowAnalysis.h"

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

    using Symbols = std::set<const Symbol*>;
    using Values = std::map<const Symbol*, Domain>;
    using BV = std::map<const Block*, Values>;
    using BS = std::map<const Block*, Symbols>;

public: /* Methods: */

    std::string toString(const Program &pr) const override;

    std::set<const Imop*> deadCopies (const Program& pr) const;

    /**
     * @brief liveOnExit returns abstract values after the given basic block
     * @param block
     * @return the abstract values
     */
    const Values& liveOnExit (const Block& block) const {
        return m_outs.at (&block);
    }

    /**
     * @brief update the value mapping from after the instruction to before
     * @param imop the instruction to update the values at
     * @param vals abstract values for every symbol
     */
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

    std::string printDeadCopies (const Program& pr) const;

private: /* Fields: */
    BV m_gen;
    BS m_kill;
    BV m_ins, m_outs;
}; // class LiveMemory

} // namespace SecreC

#endif // SECREC_LIVE_MEMORY_H
