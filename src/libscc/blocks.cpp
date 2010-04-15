#include "blocks.h"
#include <iostream>
#include <map>
#include "secrec/treenode.h"


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
    } while (last->type() == SecreC::Imop::COMMENT && (it--) != b.start);
    if (last->type() == SecreC::Imop::CALL) return false;
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

inline void linkRetBlocks(SecreC::Block &from, SecreC::Block &to) {
    from.successorsRet.insert(&to);
    to.predecessorsRet.insert(&from);
}

inline void printBlockList(std::ostream &os, const char *prefix,
                           const std::set<SecreC::Block*> bl)
{
    if (!bl.empty()) {
        os << prefix;
        std::set<unsigned long> is;
        std::set<unsigned long> ris;
        for (BSCI jt(bl.begin()); jt != bl.end(); jt++) {
            if ((*jt)->status == SecreC::Block::REMOVED) {
                ris.insert((*jt)->index);
            } else {
                is.insert((*jt)->index);
            }
        }
        for (std::set<unsigned long>::const_iterator jt = is.begin(); jt != is.end(); jt++) {
            if (jt != is.begin()) os << ", ";
            os << (*jt);
        }
        if (!is.empty() && !ris.empty()) os << " ";
        if (!ris.empty()) {
            os << "(";
            for (std::set<unsigned long>::const_iterator jt = ris.begin(); jt != ris.end(); jt++) {
                if (jt != ris.begin()) os << ", ";
                os << (*jt);
            }
            os << ")";
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

    IAB jumpFrom, jumpTo, callFrom, callTo, retFrom, retTo;
    CCI next;

    unsigned long i = 1;

    Block *b = new Block(code.begin(), i++);
    next = endBlock(*b, code.end(), jumpFrom, jumpTo, callFrom, callTo, retFrom, retTo);
    m_blocks.push_back(b);
    m_startBlock = b;

    while (next != code.end()) {
        Block *old = b;

        b = new Block(next, i++);
        next = endBlock(*b, code.end(), jumpFrom, jumpTo, callFrom, callTo, retFrom, retTo);
        m_blocks.push_back(b);

        if (fallsThru(*old)) {
            linkBlocks(*old, *b);
        }
    }

    //assert(!fallsThru(*b));

    assert(jumpFrom.empty());
    // assert(to.empty());
    assert(callFrom.empty());
    // assert(callTo.empty());
    // assert(retFrom.empty());
    assert(retTo.empty());

    bool changed;
    do {
        changed = false;
        for (BVCI it(m_blocks.begin()); it != m_blocks.end(); it++) {
            if ((*it)->status != Block::REMOVED && canEliminate(**it)) {
                (*it)->status = Block::REMOVED;
                changed = true;
            }
        }
    } while (changed);
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
        printBlockList(os, "  ..... From: ", (*it)->predecessors);
        printBlockList(os, "  . FromCall: ", (*it)->predecessorsCall);
        printBlockList(os, "  .. FromRet: ", (*it)->predecessorsRet);
        printBlockList(os, "  ....... To: ", (*it)->successors);
        printBlockList(os, "  ... ToCall: ", (*it)->successorsCall);
        printBlockList(os, "  .... ToRet: ", (*it)->successorsRet);
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
                             Blocks::IAB &jumpFrom, Blocks::IAB &jumpTo,
                             Blocks::IAB &callFrom, Blocks::IAB &callTo,
                             Blocks::IAB &retFrom, Blocks::IAB &retTo)
{
    assert(b.start == b.end);
    assert(b.end != end);

    // Patch incoming jumps for block:
    if (!(*b.end)->incoming().empty()) {
        bool mustLeave = false;

        // Iterate over all incoming jumps:
        const IS &ij = (*b.end)->incoming();
        for (ISCI it(ij.begin()); it != ij.end(); it++) {
            // Check whether the jump source is already assigned to a block:
            IAB::const_iterator itJumpFrom = jumpFrom.find(*it);
            if (itJumpFrom != jumpFrom.end()) {
                // Jump source is already assigned to block, lets link:
                linkBlocks(*(*itJumpFrom).second, b);
                // And remove the jump from the jump sources:
                jumpFrom.erase(*it);
            } else {
                // Jump source is not yet assigned to block, let assign target:
                mustLeave = true;
            }
        }

        /*
          If some of the incoming jumps were not yet assigned to blocks, we must
          keep the target assigned to the block for the future.
        */
        if (mustLeave) {
            jumpTo[*b.end] = &b;
        }
    }

    // Patch incoming calls for block:
    if (!(*b.end)->incomingCalls().empty()) {
        bool mustLeave = false;

        // Iterate over all incoming calls:
        const IS &ic = (*b.end)->incomingCalls();
        for (ISCI it(ic.begin()); it != ic.end(); it++) {
            // Check whether the call source is already assigned to a block:
            IAB::const_iterator itCallFrom = callFrom.find(*it);
            if (itCallFrom != callFrom.end()) {
                // Call source is already assigned to block, lets link:
                linkCallBlocks(*(*itCallFrom).second, b);
                // And remove the call from the call sources:
                callFrom.erase(*it);
            } else {
                mustLeave = true;
            }
        }

        /*
          If some of the incoming calls were not yet assigned to blocks, we must
          keep the target assigned to the block for the future.
        */
        if (mustLeave) {
            callTo[*b.end] = &b;
        }
    }

    // Patch incoming returns for block:
    if ((*b.end)->type() == Imop::RETCLEAN) {
        Imop *call = (Imop*) (*b.end)->arg2();
        assert(call != 0);
        assert(call->type() == Imop::CALL);
        assert(dynamic_cast<const SymbolProcedure*>(call->arg1()) != 0);
        Imop *firstImop = static_cast<const SymbolProcedure*>(call->arg1())->decl()->firstImop();
        assert(firstImop->type() == Imop::COMMENT);

        bool mustLeave = false;

        // Iterate over all incoming returns:
        const IS &rs = firstImop->returns();
        for (ISCI it(rs.begin()); it != rs.end(); it++) {
            // Check whether the return source is already assigned to a block:
            IAB::const_iterator itFromRet = retFrom.find(*it);
            if (itFromRet != retFrom.end()) {
                // Return source is assigned to a block, lets link:
                linkRetBlocks(*(*itFromRet).second, b);
            } else {
                mustLeave = true;
            }
        }

        /*
          If some of the incoming returns were not yet assigned to blocks, we
          must keep the target assigned to the block for the future.
        */
        if (mustLeave) {
            retTo[*b.end] = &b;
        }
    }

    do {
        assert(b.end == b.start
               || ((*b.end)->incoming().empty()
                   && (*b.end)->incomingCalls().empty()));

        // If *b.end is a jump instruction, *(b.end + 1) must be a leader:
        if (((*b.end)->type() & SecreC::Imop::JUMP_MASK) != 0
            || (*b.end)->type() == SecreC::Imop::CALL
            || (*b.end)->type() == SecreC::Imop::END
            || (*b.end)->type() == SecreC::Imop::RETURN
            || (*b.end)->type() == SecreC::Imop::RETURNVOID)
        {
            if (((*b.end)->type() & SecreC::Imop::JUMP_MASK) != 0) {
                // Check whether the jump destination is already assigned to a block:
                IAB::const_iterator itTo = jumpTo.find((*b.end)->jumpDest());
                if (itTo != jumpTo.end()) {
                    // Jump destination is already assigned to block, lets link:
                    linkBlocks(b, *(*itTo).second);
                } else {
                    // Destination not assigned to block, lets assign the source:
                    jumpFrom[*b.end] = &b;
                }
            } else if ((*b.end)->type() == Imop::CALL) {
                assert((*(b.end + 1))->type() == Imop::RETCLEAN);
                /*IAB::const_iterator itTo = jumpTo.find(*b.end);
                if (itTo != jumpTo.end()) {
                    linkBlocks(b, *(*itTo).second);
                    // to.erase((*itTo).first);
                } else {
                    jumpFrom[*b.end] = &b;
                }*/

                // Check whether the call destination is already assigned to a block:
                IAB::const_iterator itToCall = callTo.find((*b.end)->callDest());
                if (itToCall != callTo.end()) {
                    // Call destination is assigned to a block, lets link:
                    linkCallBlocks(b, *(*itToCall).second);
                } else {
                    callFrom[*b.end] = &b;
                }
            } else if ((*b.end)->type() == Imop::RETURN
                       || (*b.end)->type() == Imop::RETURNVOID)
            {
                Imop *firstImop = (Imop*) (*b.end)->arg2();
                typedef std::set<Imop*>::const_iterator ISCI;

                // Iterate over all incoming calls
                const std::set<Imop*> &ic = firstImop->incomingCalls();
                for (ISCI it(ic.begin()); it != ic.end(); it++) {
                    assert((*it)->type() == Imop::CALL);

                    // Get RETCLEAN instruction (the target of the return):
                    Imop *clean = (Imop*) (*it)->arg2();
                    assert(clean != 0);
                    assert(clean->type() == Imop::RETCLEAN);

                    // Check whether the target is already assigned to a block:
                    IAB::const_iterator itToRet = retTo.find(clean);
                    if (itToRet != retTo.end()) {
                        // Target is assigned to a block, lets link:
                        linkRetBlocks(b, *(*itToRet).second);
                        // And remove the target from the list of return targets:
                        retTo.erase(clean);
                    } else {
                        retFrom[*b.end] = &b;
                    }
                }
            }
            return ++(b.end);
        }

        b.end++;
    } while (b.end != end && (*b.end)->incoming().empty() && (*b.end)->incomingCalls().empty());
    return b.end;
}

bool Blocks::canEliminate(const SecreC::Block &b) const {
    typedef std::set<Block*>::const_iterator BSCI;

    if (m_startBlock == &b) return false;

    for (BSCI it(b.predecessors.begin()); it != b.predecessors.end(); it++) {
        if ((*it)->status != Block::REMOVED) return false;
    }
    for (BSCI it(b.predecessorsCall.begin()); it != b.predecessorsCall.end(); it++) {
        if ((*it)->status != Block::REMOVED) return false;
    }
    for (BSCI it(b.predecessorsRet.begin()); it != b.predecessorsRet.end(); it++) {
        if ((*it)->status != Block::REMOVED) return false;
    }
    return true;
}

} // namespace SecreC
