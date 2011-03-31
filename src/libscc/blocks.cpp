#include "blocks.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <stack>

#include "dataflowanalysis.h"
#include "treenode.h"


typedef std::set<SecreC::Block*>::const_iterator BSCI;
typedef std::set<SecreC::Imop*> IS;
typedef IS::const_iterator ISCI;


namespace {

inline bool fallsThru(const SecreC::Block &b) {
    assert (!b.empty () &&
            "Empty basic block.");

    SecreC::Block::const_reverse_iterator i, e;
    SecreC::Imop* last = 0;
    for (i = b.rbegin (), e = b.rend (); i != e; ++ i) {
        last = *i;
        if ((*i)->type () != SecreC::Imop::COMMENT) {
            break;
        }
    }

    if (last->type() == SecreC::Imop::CALL) return false;
    if (last->type() == SecreC::Imop::JUMP) return false;
    if (last->type() == SecreC::Imop::END) return false;
    if (last->type() == SecreC::Imop::RETURN) return false;
    if (last->type() == SecreC::Imop::RETURNVOID) return false;
    if (last->type() == SecreC::Imop::ERROR) return false;
    return true;
}

inline void addMutualUse (SecreC::Block &from, SecreC::Block &to) {
    from.users.insert (&to);
    to.users.insert (&from);
}

inline void linkBlocks(SecreC::Block &from, SecreC::Block &to) {
    from.successors.insert(&to);
    to.predecessors.insert(&from);
    addMutualUse (from, to);
}

inline void linkBlocksCondFalse(SecreC::Block &from, SecreC::Block &to) {
    from.successorsCondFalse.insert(&to);
    to.predecessorsCondFalse.insert(&from);
    addMutualUse (from, to);
}

inline void linkBlocksCondTrue(SecreC::Block &from, SecreC::Block &to) {
    from.successorsCondTrue.insert(&to);
    to.predecessorsCondTrue.insert(&from);
    addMutualUse (from, to);
}

inline void linkCallBlocks(SecreC::Block &from, SecreC::Block &to) {
    from.successorsCall.insert(&to);
    to.predecessorsCall.insert(&from);
    addMutualUse (from, to);
}

inline void linkRetBlocks(SecreC::Block &from, SecreC::Block &to) {
    from.successorsRet.insert(&to);
    to.predecessorsRet.insert(&from);
    addMutualUse (from, to);
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

/**
 * @brief Visits all blocks in @a blockSet and marks unvisited
 * ones visited and add them to @a reachableBlocks stack
 */
static void markAllReachable (std::stack<SecreC::Block*>& reachableBlocks,
                              std::set<SecreC::Block*>& blockSet) {
    using SecreC::Block;
    std::set<Block*>::const_iterator i, e;
    for (i = blockSet.begin (), e = blockSet.end (); i != e; ++ i) {
        Block* block = *i;
        if (!block->reachable) {
            block->reachable = true;
            reachableBlocks.push (block);
        }
    }
}

} // anonymous namespace


namespace SecreC {

/*******************************************************************************
  Blocks
*******************************************************************************/

void Blocks::init (const ICodeList &code) {
    clear ();

    /// \todo Check for empty code

    code.resetIndexes ();

    IAB jumpFrom, jumpTo, callFrom, callTo, retFrom, retTo;
    CCI next;

    unsigned long i = 1;

    Block *b = new Block (i ++);
    next = endBlock(*b, code.begin (), code.end(), jumpFrom, jumpTo, callFrom, callTo, retFrom, retTo);
    push_back(b);
    m_entryBlock = b;
    m_exitBlock = 0;

    while (next != code.end()) {
        Block *old = b;

        b = new Block (i ++);
        next = endBlock(*b, next, code.end(), jumpFrom, jumpTo, callFrom, callTo, retFrom, retTo);
        push_back(b);

        if (b->lastImop()->type() == Imop::END) {
            assert(m_exitBlock == 0);
            m_exitBlock = b;
        }

        if (fallsThru(*old)) {
            if ((old->lastImop()->type() & Imop::JUMP_MASK) == 0x0
                || old->lastImop()->type() == Imop::JUMP) {
                linkBlocks(*old, *b);
            } else {
                linkBlocksCondFalse(*old, *b);
            }
        }
    }

    assert (jumpFrom.empty());
    assert (callFrom.empty());
    assert (retTo.empty());

    std::stack<Block*> bs;
    bs.push(m_entryBlock);
    m_entryBlock->reachable = true;
    while (!bs.empty ()) {
        Block* b = bs.top();
        bs.pop();
        markAllReachable(bs, b->successors);
        markAllReachable(bs, b->successorsCondFalse);
        markAllReachable(bs, b->successorsCondTrue);
        markAllReachable(bs, b->successorsCall);
        markAllReachable(bs, b->successorsRet);
    }
}

Blocks::~Blocks() {
    for (const_iterator it(begin()); it != end(); it++) {
        delete *it;
    }
}

std::string Blocks::toString() const {
    typedef DataFlowAnalysis RD;

    std::ostringstream os;

    os << "BLOCKS" << std::endl;
    unsigned long i = 1;

    for (const_iterator it = begin(); it != end(); it++) {
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
        if ((*it)->callPassFrom != 0) {
            os << "  . PassFrom: " << (*it)->callPassFrom->index << std::endl;
        }
        if ((*it)->callPassTo != 0) {
            os << "  ... PassTo: " << (*it)->callPassTo->index << std::endl;;
        }

        // Print code:
        os << "    Code:" << std::endl;
        for (Block::iterator jt((*it)->begin ()); jt != (*it)->end (); jt++) {
            os << "      " << (*jt)->index() << "  " << (**jt);
            if ((*jt)->creator() != 0) {
                os << " // Created by "
                   << TreeNode::typeName((*jt)->creator()->type())
                   << " at "
                   << (*jt)->creator()->location();
                if ((*jt)->creator()->type() == NODE_EXPR_CLASSIFY) {
                    assert((*jt)->creator()->parent() != 0);
                    os << " for "
                       << TreeNode::typeName((*jt)->creator()->parent()->type())
                       << " at "
                       << (*jt)->creator()->parent()->location();
                }
            }
            os << std::endl;
        }
        i++;
        os << std::endl;
    }
    return os.str();
}


Blocks::CCI Blocks::endBlock(SecreC::Block &b,
                             Blocks::CCI start, Blocks::CCI end,
                             Blocks::IAB &jumpFrom, Blocks::IAB &jumpTo,
                             Blocks::IAB &callFrom, Blocks::IAB &callTo,
                             Blocks::IAB &retFrom, Blocks::IAB &retTo)
{
    assert (b.empty () &&
            "Initializing non-empty block.");
    assert (start != end &&
            "Initializing block from empty code sequence.");

    // Patch incoming jumps for block:
    if (!(*start)->incoming().empty()) {
        bool mustLeave = false;

        // Iterate over all incoming jumps:
        const IS &ij = (*start)->incoming();
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
            jumpTo[*start] = &b;
        }
    }

    // Patch incoming calls for block:
    if (!(*start)->incomingCalls().empty()) {
        bool mustLeave = false;

        // Iterate over all incoming calls:
        const IS &ic = (*start)->incomingCalls();
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
            callTo[*start] = &b;
        }
    }

    // Patch incoming returns for block:
    if ((*start)->type() == Imop::RETCLEAN) {
        assert (dynamic_cast<const SymbolLabel*>((*start)->arg2()) != 0);
        Imop const* call = static_cast<const SymbolLabel*>((*start)->arg2())->target ();
        assert(call != 0);
        assert(call->type() == Imop::CALL);
        assert(dynamic_cast<const SymbolProcedure*>(call->arg1()) != 0);
        Imop *firstImop = static_cast<const SymbolProcedure*>(call->arg1())->decl()->firstImop();
        assert(firstImop->type() == Imop::COMMENT);

        // Init call pass edge:
        b.callPassFrom = call->block();
        call->block()->callPassTo = &b;
        b.users.insert (call->block ());
        call->block ()->users.insert (&b);

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
            retTo[*start] = &b;
        }
    }

    Blocks::CCI blockEnd = start;
    do {
        assert(b.empty ()
               || ((*blockEnd)->incoming().empty()
                   && (*blockEnd)->incomingCalls().empty()));

        Imop* imop = *blockEnd;
        b.push_back (imop);
        imop->setBlock(&b);

        // If *b.end is a jump instruction, *(b.end + 1) must be a leader:
        if (imop->isTerminator ()) {
            if (imop->isJump()) {
                // Check whether the jump destination is already assigned to a block:
                IAB::const_iterator itTo = jumpTo.find(imop->jumpDest()->target());
                if (itTo != jumpTo.end()) {
                    // Jump destination is already assigned to block, lets link:
                    if (imop->type() == Imop::JUMP) {
                        linkBlocks(b, *(*itTo).second);
                    } else {
                        linkBlocksCondTrue(b, *(*itTo).second);
                    }
                } else {
                    // Destination not assigned to block, lets assign the source:
                    jumpFrom[imop] = &b;
                }
            } else if (imop->type() == Imop::CALL) {
                assert((*(blockEnd + 1))->type() == Imop::RETCLEAN);

                // Check whether the call destination is already assigned to a block:
                IAB::const_iterator itToCall = callTo.find(imop->callDest());
                if (itToCall != callTo.end()) {
                    // Call destination is assigned to a block, lets link:
                    linkCallBlocks(b, *(*itToCall).second);
                } else {
                    callFrom[imop] = &b;
                }
            } else if (imop->type() == Imop::RETURN
                       || imop->type() == Imop::RETURNVOID)
            {
                assert (dynamic_cast<const SymbolLabel*>(imop->arg2()) != 0);
                Imop const* firstImop = static_cast<const SymbolLabel*>(imop->arg2())->target();
                typedef std::set<Imop*>::const_iterator ISCI;

                // Iterate over all incoming calls
                const std::set<Imop*> &ic = firstImop->incomingCalls();
                for (ISCI it(ic.begin()); it != ic.end(); it++) {
                    assert((*it)->type() == Imop::CALL);
                    assert (dynamic_cast<const SymbolLabel*>((*it)->arg2()) != 0);
                    Imop const* clean = static_cast<const SymbolLabel*>((*it)->arg2())->target();

                    // Get RETCLEAN instruction (the target of the return):
                    //Imop *clean = (Imop*) (*it)->arg2();
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
                        retFrom[imop] = &b;
                    }
                }
            }
            return ++ blockEnd;
        }

        blockEnd ++;
    } while (blockEnd != end && (*blockEnd)->incoming().empty() && (*blockEnd)->incomingCalls().empty());
    return blockEnd;
}

/*******************************************************************************
  Block
*******************************************************************************/

class UnlinkFrom {
public:
    UnlinkFrom (Block* block) : m_block (block) { }
    void operator () (Block* that) const {
        that->users.erase (m_block);
        that->predecessors.erase (m_block);
        that->predecessorsCondFalse.erase (m_block);
        that->predecessorsCondTrue.erase (m_block);
        that->predecessorsCall.erase (m_block);
        that->predecessorsRet.erase (m_block);
        that->successors.erase (m_block);
        that->successorsCondFalse.erase (m_block);
        that->successorsCondTrue.erase (m_block);
        that->successorsCall.erase (m_block);
        that->successorsRet.erase (m_block);
        if (that->callPassFrom == m_block) that->callPassFrom = 0;
        if (that->callPassTo == m_block) that->callPassTo = 0;
    }

private:
    Block* m_block;
};

void Block::unlink () {
    std::for_each (users.begin (), users.end (), UnlinkFrom (this));
}

Block::~Block () {
    unlink ();
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Blocks &bs) {
    out << bs.toString();
    return out;
}
