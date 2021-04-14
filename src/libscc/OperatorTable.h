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

    void appendOperator (Symbol* op);

    std::vector<SymbolProcedure *> findOperators(sharemind::StringView name);

    std::vector<SymbolOperatorTemplate *>
    findOperatorTemplates(sharemind::StringView name);

private: /* Fields: */

    std::vector<Symbol*> m_ops;
};

} /* namespace SecreC { */

#endif // SECREC_OPERATORTABLE_H
