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

#ifndef SECREC_REACHABLE_USES_H
#define SECREC_REACHABLE_USES_H

#include "../DataflowAnalysis.h"
#include "AbstractReachable.h"

#include <boost/interprocess/containers/flat_set.hpp>
#include <map>
#include <string>

namespace SecreC {

/*******************************************************************************
  ReachableUses
*******************************************************************************/

struct UsesVisitor {
    void operator()(const Imop& imop, AbstractReachableVisitor& visitor) {
        for (const Symbol* sym : imop.defRange()) {
            if (sym->symbolType() == SYM_SYMBOL) {
                visitor.kill(sym);
            }
        }

        for (const Symbol* sym : imop.useRange()) {
            if (sym->symbolType() == SYM_SYMBOL) {
                visitor.gen(sym, imop);
            }
        }
    }
};

struct ReachableUses : public AbstractReachable<UsesVisitor> {

    std::string toString(const Program& pr) const override;

};

} // namespace SecreC

#endif // SECREC_REACHABLE_USES_H
