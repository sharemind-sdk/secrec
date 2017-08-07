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

#include "ReachableUses.h"

#include "../Blocks.h"
#include "../TreeNode.h"

#include <boost/range/adaptor/reversed.hpp>
#include <sstream>


using boost::adaptors::reverse;

namespace SecreC {

std::string ReachableUses::toString(const Program& pr) const {
    std::stringstream ss;
    ss << "Reachable uses:\n";

    FOREACH_BLOCK (bi, pr) {
        SymbolReachable after;
        auto i = m_blocks.find(&*bi);

        if (i != m_blocks.end()) {
            after = i->second.out;
        }

        for (const Imop& imop : reverse(*bi)) {
            for (const Symbol* dest : imop.defRange()) {
                TreeNode* creator = imop.creator();

                ss << imop.index() << ": " << imop;

                if (creator != nullptr)
                    ss << " // creator " << imop.creator()->location();

                ss << '\n';

                for (const Imop* use : after[dest]) {
                    ss << '\t' << use->index() << ": " << *use << '\n';
                }
            }

            update(imop, after);
        }
    }

    return ss.str();
}

} // namespace SecreC
