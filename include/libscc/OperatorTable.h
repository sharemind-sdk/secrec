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

#ifndef SECREC_OPERATORTABLE_H
#define SECREC_OPERATORTABLE_H

#include "Symbol.h"

#include <vector>

namespace SecreC {

class OperatorTable {

public: /* Methods: */

    OperatorTable () {}

    ~OperatorTable ();

    void appendOperator (SymbolProcedure* op);

    std::vector<SymbolProcedure*> findOperators (StringRef name);

private: /* Fields: */

    std::vector<SymbolProcedure*> m_ops;
};

}

#endif // SECREC_OPERATORTABLE_H
