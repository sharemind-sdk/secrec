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

#include "OperatorTable.h"

#include "TreeNode.h"

namespace SecreC {

OperatorTable::~OperatorTable () {
    for (Symbol* op : m_ops)
        delete op;
}

void OperatorTable::appendOperator (Symbol* op) {
    m_ops.push_back (op);
}

std::vector<SymbolProcedure *>
OperatorTable::findOperators(sharemind::StringView name) {
    std::vector<SymbolProcedure*> res;
    for (Symbol* s : m_ops) {
        if (s->symbolType () == SYM_PROCEDURE) {
            SymbolProcedure* op = static_cast<SymbolProcedure*> (s);
            if (op->procedureName () == name)
                res.push_back (op);

        }
    }
    return res;
}

std::vector<SymbolOperatorTemplate *>
OperatorTable::findOperatorTemplates(sharemind::StringView name) {
    std::vector<SymbolOperatorTemplate*> res;
    for (Symbol* s : m_ops) {
        if (s->symbolType () == SYM_OPERATOR_TEMPLATE) {
            SymbolOperatorTemplate* op = static_cast<SymbolOperatorTemplate*> (s);
            if (op->decl ()->body ()->procedureName () == name)
                res.push_back (op);
        }
    }
    return res;
}

} // namespace SecreC {
