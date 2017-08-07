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

#include "../analysis/ReachableUses.h"
#include "../Constant.h"
#include "../DataflowAnalysis.h"
#include "../Intermediate.h"
#include "../Optimizer.h"

#include <boost/interprocess/containers/flat_set.hpp>


namespace SecreC {

bool eliminateDeadAllocs (const ReachableUses& ru, ICode& code) {
    std::set<Imop*> replace;
    Program& program = code.program ();
    ConstantString* relComment =
        ConstantString::get (code.context (), "release removed by dead allocation elimination");
    ConstantString* allocComment =
        ConstantString::get (code.context (), "alloc removed by dead alloc elimination");

    FOREACH_BLOCK (bi, program) {
        const Block& block = *bi;
        SymbolReachable uses = ru.reachableOnExit (block);
        for (const Imop& imop : reverse (block)) {
            boost::container::flat_set<const Imop*> releases;

            if (imop.type () == Imop::ALLOC) {
                bool dead = true;
                for (const Imop* use : uses[imop.dest ()]) {
                    if (use->type () == Imop::RELEASE) {
                        releases.insert (use);
                    } else {
                        dead = false;
                        break;
                    }
                }

                if (dead) {
                    for (const Imop* rel : releases) {
                        replace.insert (const_cast<Imop*> (rel));
                    }
                    replace.insert (const_cast<Imop*> (&imop));
                }
            } else {
                ReachableUses::update (imop, uses);
            }
        }
    }

    for (auto i : replace) {
        ConstantString* comment = i->type () == Imop::ALLOC ? allocComment : relComment;
        Imop* newImop = new Imop (i->creator (), Imop::COMMENT, nullptr, comment);
        i->replaceWith (*newImop);
        delete i;
    }

    return replace.size ();
}

} // namespace SecreC
