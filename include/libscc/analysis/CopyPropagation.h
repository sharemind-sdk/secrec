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

#ifndef SECREC_COPY_PROPAGATION_H
#define SECREC_COPY_PROPAGATION_H

#include "../DataflowAnalysis.h"

#include <boost/interprocess/containers/flat_set.hpp>
#include <map>

namespace SecreC {

/*******************************************************************************
  CopyPropagation
*******************************************************************************/

class CopyPropagation : public ForwardDataFlowAnalysis {

public: /* Types: */

    using Symbols = boost::container::flat_set<const Symbol*>;
    using Copies = boost::container::flat_set<const Imop*>;
    using BlockMap = std::map<const Block*, Copies>;

public: /* Methods: */

    static void update(const Imop& imop, Copies& copies);

    inline Copies getCopies(const Block& b) const {
        const auto it = m_ins.find(&b);
        if (it != m_ins.end())
            return it->second;
        return Copies();
    }

    std::string toString(const Program& program) const override;

protected:

    virtual void start(const Program& pr) override;
    virtual void startBlock(const Block& b) override;
    virtual void inFrom(const Block& from, Edge::Label label, const Block& to) override;
    virtual bool finishBlock(const Block& b) override;
    virtual void finish() override;

private: /* Fields: */

    BlockMap m_ins;
    BlockMap m_outs;
}; // class CopyPropagation

} // namespace SecreC

#endif // SECREC_COPY_PROPAGATION_H
