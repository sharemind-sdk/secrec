/*
 * Copyright (C) 2016 Cybernetica
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

#ifndef SECREC_REACHABLE_RETURNS_H
#define SECREC_REACHABLE_RETURNS_H

#include "../DataflowAnalysis.h"

#include <boost/interprocess/containers/flat_set.hpp>
#include <map>

namespace SecreC {

class ReachableReturns : public BackwardDataFlowAnalysis {

public: /* Types: */

    using Returns = boost::container::flat_set<const Imop*>;
    using BlockMap = std::map<const Block*, Returns>;

public: /* Methods: */

    std::string toString(const Program& pr) const override;

    Returns returnsOnExit(const Block& block) const {
        const auto it = m_outs.find(&block);
        if (it != m_outs.end())
            return it->second;
        return Returns();
    }

    static void update(const Imop& imop, Returns& vals);

protected:

    virtual void start(const Program& bs) override;
    virtual void startBlock(const Block& b) override;
    virtual void outTo(const Block& from, Edge::Label label, const Block& to) override;
    virtual bool finishBlock(const Block& b) override;
    virtual void finish() override;

private: /* Fields: */

    BlockMap m_outs;
    BlockMap m_ins;

}; // class Returns

} // namespace SecreC

#endif // SECREC_REACHABLE_RETURNS_H
