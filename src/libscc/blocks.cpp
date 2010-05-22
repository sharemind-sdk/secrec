#include "blocks.h"
#include <iostream>
#include <map>
#include <stack>
#include "reachingdefinitions.h"
#include "treenode.h"


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

inline void linkBlocksCondFalse(SecreC::Block &from, SecreC::Block &to) {
    from.successorsCondFalse.insert(&to);
    to.predecessorsCondFalse.insert(&from);
}

inline void linkBlocksCondTrue(SecreC::Block &from, SecreC::Block &to) {
    from.successorsCondTrue.insert(&to);
    to.predecessorsCondTrue.insert(&from);
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
        std::set<unsigned long> reachables;
        std::set<unsigned long> unreachables;
        for (BSCI jt(bl.begin()); jt != bl.end(); jt++) {
            if ((*jt)->reachable) {
                reachables.insert((*jt)->index);
            } else {
                unreachables.insert((*jt)->index);
            }
        }
        for (std::set<unsigned long>::const_iterator jt = reachables.begin(); jt != reachables.end(); jt++) {
            if (jt != reachables.begin()) os << ", ";
            os << (*jt);
        }
        if (!reachables.empty() && !unreachables.empty()) os << " ";
        if (!unreachables.empty()) {
            os << "(";
            for (std::set<unsigned long>::const_iterator jt = unreachables.begin(); jt != unreachables.end(); jt++) {
                if (jt != unreachables.begin()) os << ", ";
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

Blocks::Status Blocks::init(const ICodeList &code) {
    /// \todo Check for empty code

    code.resetIndexes();

    IAB jumpFrom, jumpTo, callFrom, callTo, retFrom, retTo;
    CCI next;

    unsigned long i = 1;

    Block *b = new Block(code.begin(), i++);
    next = endBlock(*b, code.end(), jumpFrom, jumpTo, callFrom, callTo, retFrom, retTo);
    m_blocks.push_back(b);
    m_entryBlock = b;

    while (next != code.end()) {
        Block *old = b;

        b = new Block(next, i++);
        next = endBlock(*b, code.end(), jumpFrom, jumpTo, callFrom, callTo, retFrom, retTo);
        m_blocks.push_back(b);

        if (fallsThru(*old)) {
            if (((*(old->end - 1))->type() & Imop::JUMP_MASK) == 0x0
                || (*(old->end - 1))->type() == Imop::JUMP) {
                linkBlocks(*old, *b);
            } else {
                linkBlocksCondFalse(*old, *b);
            }
        }
    }

    //assert(!fallsThru(*b));

    assert(jumpFrom.empty());
    // assert(to.empty());
    assert(callFrom.empty());
    // assert(callTo.empty());
    // assert(retFrom.empty());
    assert(retTo.empty());

    std::stack<Block*> bs;
    bs.push(m_entryBlock);
    m_entryBlock->reachable = true;
    do {
        typedef std::set<Block*>::const_iterator BSCI;

        Block *b = bs.top();
        bs.pop();

        for (BSCI it(b->successors.begin()); it != b->successors.end(); it++) {
            if (!(*it)->reachable) {
                (*it)->reachable = true;
                bs.push(*it);
            }
        }
        for (BSCI it(b->successorsCondFalse.begin()); it != b->successorsCondFalse.end(); it++) {
            if (!(*it)->reachable) {
                (*it)->reachable = true;
                bs.push(*it);
            }
        }
        for (BSCI it(b->successorsCondTrue.begin()); it != b->successorsCondTrue.end(); it++) {
            if (!(*it)->reachable) {
                (*it)->reachable = true;
                bs.push(*it);
            }
        }
        for (BSCI it(b->successorsCall.begin()); it != b->successorsCall.end(); it++) {
            if (!(*it)->reachable) {
                (*it)->reachable = true;
                bs.push(*it);
            }
        }
        for (BSCI it(b->successorsRet.begin()); it != b->successorsRet.end(); it++) {
            if (!(*it)->reachable) {
                (*it)->reachable = true;
                bs.push(*it);
            }
        }
    } while (!bs.empty());


    if (m_status == NOT_READY) m_status = OK;
    return m_status;
}

Blocks::~Blocks() {
    for (BVCI it(m_blocks.begin()); it != m_blocks.end(); it++) {
        delete *it;
    }
}

std::string Blocks::toString(const ReachingDefinitions *rd) const {
    std::ostringstream os;

    os << "BLOCKS" << std::endl;
    unsigned long i = 1;
    for (BVCI it(m_blocks.begin()); it != m_blocks.end(); it++) {
        os << "  Block " << i;
        if (!(*it)->reachable) {
            os << " [REMOVED]";
        }
        os << std::endl;
        printBlockList(os, "  ..... From: ", (*it)->predecessors);
        printBlockList(os, "  ... From -: ", (*it)->predecessorsCondFalse);
        printBlockList(os, "  ... From +: ", (*it)->predecessorsCondTrue);
        printBlockList(os, "  . FromCall: ", (*it)->predecessorsCall);
        printBlockList(os, "  .. FromRet: ", (*it)->predecessorsRet);
        printBlockList(os, "  ....... To: ", (*it)->successors);
        printBlockList(os, "  ..... To -: ", (*it)->successorsCondFalse);
        printBlockList(os, "  ..... To +: ", (*it)->successorsCondTrue);
        printBlockList(os, "  ... ToCall: ", (*it)->successorsCall);
        printBlockList(os, "  .... ToRet: ", (*it)->successorsRet);
        if (rd != 0) {
            os << "    Reaching definitions:";
            if (!(*it)->reachable) {
                os << " NOT CALCULATED (unreachable block)" << std::endl;
            } else {
                const SecreC::ReachingDefinitions::SDefs &sd = rd->getReaching(**it);
                if (sd.empty()) {
                    os << " NONE" << std::endl;
                } else {
                    typedef SecreC::ReachingDefinitions::SDefs::const_iterator SDCI;
                    typedef SecreC::ReachingDefinitions::Defs::const_iterator DCI;
                    typedef SecreC::ReachingDefinitions::CJumps::const_iterator JCI;

                    os << std::endl;
                    for (SDCI it = sd.begin(); it != sd.end(); it++) {
                        os << "      " << *(*it).first << ": ";
                        const SecreC::ReachingDefinitions::Defs &ds = (*it).second;
                        for (DCI jt = ds.begin(); jt != ds.end(); jt++) {
                            if (jt != ds.begin()) os << ", ";
                            os << (*jt).first->index();
                            const SecreC::ReachingDefinitions::CJumps &js = (*jt).second;
                            if (!js.empty()) {
                                os << " [";
                                for (JCI kt = js.begin(); kt != js.end(); kt++) {
                                    if (kt != js.begin()) os << ", ";
                                    os << (*kt)->index();
                                }
                                os << "]";
                            }
                        }
                        os << std::endl;
                    }
                }
            }
        }
        os << "    Code:" << std::endl;
        for (CCI jt((*it)->start); jt != (*it)->end; jt++) {
            os << "      " << (*jt)->index() << "  " << (**jt);
            if ((*jt)->creator() != 0) {
                os << " // Created by "
                   << TreeNode::typeName((*jt)->creator()->type())
                   << " at "
                   << (*jt)->creator()->location() << std::endl;
            }
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
                if ((*it)->type() == Imop::JUMP) {
                    linkBlocks(*(*itJumpFrom).second, b);
                } else {
                    linkBlocksCondTrue(*(*itJumpFrom).second, b);
                }
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

        (*b.start)->setBlock(&b);

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
                    if ((*b.end)->type() == Imop::JUMP) {
                        linkBlocks(b, *(*itTo).second);
                    } else {
                        linkBlocksCondTrue(b, *(*itTo).second);
                    }
                } else {
                    // Destination not assigned to block, lets assign the source:
                    jumpFrom[*b.end] = &b;
                }
            } else if ((*b.end)->type() == Imop::CALL) {
                assert((*(b.end + 1))->type() == Imop::RETCLEAN);

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

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Blocks &bs) {
    out << bs.toString();
    return out;
}
