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

#ifndef SECREC_ANALYSIS_REACHING_JUMPS_H
#define SECREC_ANALYSIS_REACHING_JUMPS_H

#include "../DataflowAnalysis.h"

namespace SecreC {

/*******************************************************************************
  ReachingJumps
*******************************************************************************/

class ReachingJumps: public ForwardDataFlowAnalysis {
public: /* Types: */
    using Jumps = std::set<const Imop*>;
    using BJM = std::map<const Block*, Jumps>;

public: /* Methods: */

    std::string toString(const Program &pr) const override;

    inline const BJM &getPosJumps() const { return m_inPos; }
    inline const BJM &getNegJumps() const { return m_inNeg; }

protected:

    virtual void start(const Program &bs) override;
    virtual void startBlock(const Block &b) override;
    virtual void inFrom(const Block &from, Edge::Label label, const Block &to) override;
    virtual bool finishBlock(const Block &b) override;
    virtual inline void finish() override { m_outPos.clear(); m_outNeg.clear(); }

private: /* Fields: */
    BJM m_inPos;
    BJM m_inNeg;
    BJM m_outPos;
    BJM m_outNeg;
};

} // namespace SecreC

#endif /* SECREC_ANALYSIS_REACHING_JUMPS_H */
