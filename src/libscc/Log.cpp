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

#include "Log.h"

#include "TreeNode.h"


namespace SecreC {

#define DEFINE_LOG_IN_PROC(small,Caps) \
    CompileLogStream CompileLog::small ## InProc(const TreeNode * n) { \
        if (n->containingProcedure()) \
            small() << "In procedure '" << n->containingProcedure()->printableSignature() << "':"; \
        return CompileLogStream(*this, CompileLogMessage::Caps); \
    }

DEFINE_LOG_IN_PROC(fatal,Fatal)
DEFINE_LOG_IN_PROC(error,Error)
DEFINE_LOG_IN_PROC(warning,Warning)
DEFINE_LOG_IN_PROC(info,Info)
DEFINE_LOG_IN_PROC(debug,Debug)

} // namespace SecreC
