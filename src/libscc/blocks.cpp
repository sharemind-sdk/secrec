#include "blocks.h"

#include <map>


typedef std::vector<SecreC::Block*>::const_iterator BVCI;
typedef std::set<SecreC::Block*>::const_iterator BSCI;
typedef std::set<SecreC::Imop*> IS;
typedef IS::const_iterator ISCI;


namespace {

inline bool fallsThru(const SecreC::Block &b) {
    assert(b.start != b.end);

    SecreC::Blocks::CCI it = b.end - 1;

    SecreC::Imop *last;
    do {
        last = *it;
    } while (last->type() == SecreC::Imop::COMMENT && (--it) != b.start);
    if (last->type() == SecreC::Imop::JUMP) return false;
    if (last->type() == SecreC::Imop::END) return false;
    if (last->type() == SecreC::Imop::RETURN) return false;
    if (last->type() == SecreC::Imop::RETURNVOID) return false;
    return true;
}

inline void linkBlocks(SecreC::Block &from, SecreC::Block &to) {
    from.successors.insert(&to);
    to.predecessors.insert(&from);
}

inline void linkCallBlocks(SecreC::Block &from, SecreC::Block &to) {
    from.successorsCall.insert(&to);
    to.predecessorsCall.insert(&from);
}

inline void printBlockList(std::ostream &os, const char *prefix,
                           const std::set<SecreC::Block*> bl)
{
    if (!bl.empty()) {
        os << prefix;
        std::set<unsigned long> is;
        for (BSCI jt(bl.begin()); jt != bl.end(); jt++) {
            is.insert((*jt)->index);
        }
        for (std::set<unsigned long>::const_iterator jt = is.begin(); jt != is.end(); jt++) {
            if (jt != is.begin()) os << ", ";
             os << (*jt);
        }
        os << std::endl;
    }
}

} // anonymous namespace


namespace SecreC {

/*******************************************************************************
  Blocks
*******************************************************************************/

Blocks::Blocks(const ICode::CodeList &code)
    : m_status(OK)
{
    /// \todo Check for empty code

    code.resetIndexes();

    IAB from, to, callFrom, callTo;
    CCI next;

    unsigned long i = 1;

    Block *b = new Block(code.begin(), i++);
    next = endBlock(*b, code.end(), from, to, callFrom, callTo);
    m_blocks.push_back(b);
    m_startBlock = b;

    while (next != code.end()) {
        Block *old = b;

        b = new Block(next, i++);
        next = endBlock(*b, code.end(), from, to, callFrom, callTo);
        m_blocks.push_back(b);

        if (fallsThru(*old)) {
            linkBlocks(*old, *b);
        }
    }

    assert(!fallsThru(*b));
    assert(from.empty());
    assert(to.empty());
    // assert(callFrom.empty()); We leave this for return code
    assert(callTo.empty());

    for (BVCI it(m_blocks.begin()); it != m_blocks.end(); it++) {
        if (canEliminate(**it)) {
            (*it)->status = Block::REMOVED;
        }
    }
}

std::string Blocks::toString() const {

    std::ostringstream os;

    os << "BLOCKS" << std::endl;
    unsigned long i = 1;
    for (BVCI it(m_blocks.begin()); it != m_blocks.end(); it++) {
        os << "  Block " << i;
        if ((*it)->status == Block::REMOVED) {
            os << " [REMOVED]";
        } else if ((*it)->status == Block::GENERATED) {
            os << " [GENERATED]";
        }
        os << std::endl;
        printBlockList(os, "  .... From: ", (*it)->predecessors);
        printBlockList(os, "  .FromCall: ", (*it)->predecessorsCall);
        printBlockList(os, "  ...... To: ", (*it)->successors);
        printBlockList(os, "  .. ToCall: ", (*it)->successorsCall);
        // printBlockList(os, "  ReturnsTo: ", (*it)->successorsRet);
        // os << "    Code:" << std::endl;
        for (CCI jt((*it)->start); jt != (*it)->end; jt++) {
            os << "    " << (*jt)->index() << "  " << (**jt) << std::endl;
        }
        i++;
        os << std::endl;
    }
    return os.str();
}


Blocks::CCI Blocks::endBlock(SecreC::Block &b, Blocks::CCI end,
                             Blocks::IAB &from, Blocks::IAB &to,
                             Blocks::IAB &callFrom, Blocks::IAB &callTo)
{
    assert(b.start == b.end);
    assert(b.end != end);

    CCI next = b.end;

    if (!(*b.end)->incoming().empty()) {
        const IS &is = (*b.end)->incoming();
        for (ISCI it(is.begin()); it != is.end(); it++) {
            IAB::const_iterator itFrom = from.find(*it);
            if (itFrom != from.end()) {
                linkBlocks(*(*itFrom).second, b);
                from.erase((*itFrom).first);
            } else {
                to[*it] = &b;
            }
        }
    }
    if (!(*b.end)->incomingCalls().empty()) {
        const IS &is = (*b.end)->incomingCalls();
        for (ISCI it(is.begin()); it != is.end(); it++) {
            IAB::const_iterator itFrom = callFrom.find(*it);
            if (itFrom != callFrom.end()) {
                linkCallBlocks(*(*itFrom).second, b);
                callFrom.erase((*itFrom).first);
            } else {
                callTo[*it] = &b;
            }
        }
    }

    do {
        next++;
        assert(b.end == b.start || (*b.end)->incoming().empty());

        // If *b.end is a jump instruction, *next must be a leader:
        if (((*b.end)->type() & SecreC::Imop::JUMP_MASK) != 0
            || (*b.end)->type() == SecreC::Imop::PROCCALL
            || (*b.end)->type() == SecreC::Imop::END
            || (*b.end)->type() == SecreC::Imop::RETURN
            || (*b.end)->type() == SecreC::Imop::RETURNVOID)
        {
            if (((*b.end)->type() & SecreC::Imop::JUMP_MASK) != 0) {
                IAB::const_iterator itTo = to.find(*b.end);
                if (itTo != to.end()) {
                    linkBlocks(b, *(*itTo).second);
                    to.erase((*itTo).first);
                } else {
                    from[*b.end] = &b;
                }
            } else if ((*b.end)->type() == Imop::PROCCALL) {
                IAB::const_iterator itToCall = callTo.find(*b.end);
                if (itToCall != callTo.end()) {
                    linkCallBlocks(b, *(*itToCall).second);
                    callTo.erase((*itToCall).first);
                } else {
                    callFrom[*b.end] = &b;
                }
            } else if ((*b.end)->type() == Imop::RETURN
                       || (*b.end)->type() == Imop::RETURN)
            {
                /// \todo
            }
            b.end++;
            return next;
        }

        // If *next is the target of some jump, *next must be a leader:
        if (next != end && !(*next)->incoming().empty()) {
            return next;
        }

        b.end++;
    } while (b.end != end);
    return next;
}

bool Blocks::canEliminate(const SecreC::Block &b) const {
    if (!b.predecessors.empty()) return false;
    if (!b.predecessorsCall.empty()) return false;
    if (m_startBlock == &b) return false;
    return true;
}

} // namespace SecreC
