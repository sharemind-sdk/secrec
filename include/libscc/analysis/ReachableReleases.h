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

#ifndef SECREC_REACHABLE_RELEASES_H
#define SECREC_REACHABLE_RELEASES_H

#include "../DataflowAnalysis.h"

namespace SecreC {

class ReachableReleases : public BackwardDataFlowAnalysis {
public: /* Types: */
    using Domain = std::set<const Imop*>;
    using Values = std::map<const Symbol*, Domain>;
    using BV = std::map<const Block*, Values>;
    using Symbols = std::set<const Symbol*>;

public: /* Methods: */

    std::string toString(const Program &pr) const override;

    Values releasedOnExit (const Block& block) const {
        const auto it = m_outs.find (&block);
        if (it != m_outs.end ())
            return it->second;

        return Values ();
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
