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

#include "../Intermediate.h"
#include "../Optimizer.h"

#include <memory>
#include <vector>


namespace SecreC {

bool removeEmptyProcedures (ICode& code) {
    std::vector<std::unique_ptr<Procedure>> emptyProcedures;
    for (auto& proc : code.program ()) {
        // TODO: how to identify the START procedure? Currently we use
        // "name != NULL". Not sure if it's ideal.
        if (proc.name () != nullptr && proc.callFrom ().empty ())
            emptyProcedures.emplace_back (std::unique_ptr<Procedure> (&proc));
    }

    for (auto& proc : emptyProcedures)
        proc->unlink ();

    return emptyProcedures.size () > 0;
}

} // namespace SecreC
