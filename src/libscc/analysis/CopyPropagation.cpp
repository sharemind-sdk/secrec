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

#include "CopyPropagation.h"
#include "BoostInsertWorkaround.h"

#include "../Symbol.h"
#include "../TreeNode.h"

#include <sstream>

namespace SecreC {

namespace {

void killCopies(Symbol* s, CopyPropagation::Copies& copies, CopyPropagation::Copies& kill) {
    for (const Imop* copy : copies) {
        if (copy->dest() == s || copy->arg1() == s)
            kill.insert(copy);
    }
}

}

/*******************************************************************************
  CopyPropagation
*******************************************************************************/

void CopyPropagation::update(const Imop& imop, Copies& copies) {
    Copies kill;

    if (imop.isSyscall ()) {
        for (auto op : imop.syscallOperands ()) {
            const auto sym = op.operand();
            switch (op.passingConvention()) {
                case Push:
                    if (!op.isReadOnly())
                        killCopies(sym, copies, kill);
                    break;
                case PushRef:
                case PushCRef:
                case Return:
                    killCopies(sym, copies, kill);
                    break;

            }
        }
    } else {
        for (Symbol* defSym : imop.defRange()) {
            killCopies(defSym, copies, kill);
        }

        if (imop.writesDest()) {
            killCopies(imop.dest(), copies, kill);
        }

        if (imop.type() == Imop::CALL) {
            for (Symbol* useSym : imop.useRange()) {
                killCopies(useSym, copies, kill);
            }
        }
    }

    for (const Imop* copy : kill) {
        copies.erase(copy);
    }

    if (imop.type() == Imop::COPY) {
        copies.insert(&imop);
    }
}

void CopyPropagation::start(const Program&) {
    m_ins.clear();
    m_outs.clear();
}

void CopyPropagation::startBlock(const Block& b) {
    m_ins.erase(&b);
}

void CopyPropagation::inFrom(const Block& from, Edge::Label label, const Block& to) {
    if (Edge::isGlobal(label)) {
        for (const Imop* copy : m_outs[&from]) {
            if (copy->dest()->isGlobal() && copy->arg1()->isGlobal())
                m_ins[&to].insert(copy);
        }
    } else {
        if (m_ins.count(&to) == 0) {
            // First set
            insertWorkaround(m_ins[&to], m_outs[&from].begin(), m_outs[&from].end());
        } else {
            // Union
            Copies bad;
            for (const Imop* copy : m_ins[&to]) {
                if (m_outs[&from].count(copy) == 0)
                    bad.insert(copy);
            }
            for (const Imop* copy : bad) {
                m_ins[&to].erase(copy);
            }
        }
    }
}

bool CopyPropagation::finishBlock(const Block& b) {
    Copies old = m_outs[&b];
    Copies& out = m_outs[&b];
    out = m_ins[&b];

    for (const Imop& imop : b) {
        update(imop, out);
    }

    return old != out;
}

void CopyPropagation::finish() { }

std::string CopyPropagation::toString(const Program& pr) const {
    std::ostringstream os;

    os << "Copy propagation analysis results:" << std::endl;
    FOREACH_BLOCK (bi, pr) {
        if (!bi->reachable()) {
            continue;
        }

        os << "  Block " << bi->index() << ": ";

        auto si = m_ins.find(&*bi);

        if (si == m_ins.end() || (*si).second.empty()) {
            os << " NONE" << std::endl;
        } else {
            os << std::endl;
            for (const Imop* copy : si->second) {
                os << "      " << *copy;
                if (copy->creator() != nullptr) {
                    os << " // Created by " << copy->creator()->location();
                }
                os << std::endl;
            }
        }
    }

    return os.str();
}

} // namespace SecreC
