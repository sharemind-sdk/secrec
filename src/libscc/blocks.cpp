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

} // anonymous namespace


namespace SecreC {

/*******************************************************************************
  Blocks
*******************************************************************************/

void Blocks::init (const ICodeList &code) {
    clear ();

    /// \todo Check for empty code

    code.resetIndexes ();
    std::map<Block*, Block*> nextBlock;
    assignToBlocks (code.begin (), code.end (), nextBlock);
    assert (!empty ());
    propagate (nextBlock);
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

    for (const_iterator it = begin(); it != end(); it++) {
        const Block* block = *it;
        os << "  Block " << block->index;
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

        os << std::endl;
    }
    return os.str();
}

// Assigns all instructions to blocks
void Blocks::assignToBlocks (CCI start, CCI end, std::map<Block*, Block*>& nextBlock) {
    unsigned blockCount = 1;

    // 1. find leaders
    std::set<const Imop*> leaders;
    bool nextIsLeader = true;  // first instruction is leader
    for (CCI i = start ; i != end; ++ i) {
        const Imop* imop = *i;

        if (nextIsLeader) {
            leaders.insert (imop);
            nextIsLeader = false;
        }

        // destination of jump is leader
        if (imop->isJump ()) {
            assert (dynamic_cast<const SymbolLabel*>(imop->dest ()) != 0);
            const SymbolLabel* dest = static_cast<const SymbolLabel*>(imop->dest ());
            leaders.insert (dest->target ());
        }

        // anything following terminator is leader
        if (imop->isTerminator ()) {
            nextIsLeader = true;
        }

        // call destinations are leaders
        if (imop->type () == Imop::CALL) {
            leaders.insert (imop->callDest ());
            nextIsLeader = true; // RETCLEAN is leader too
        }
    }

    // 2. assign all instructions to basic blocks
    // Anything in range [leader, nextLeader) is a basic block.
    Block* cur = 0;
    for (CCI i = start ; i != end; ++ i) {
        Imop* imop = *i;
        if (leaders.find (imop) != leaders.end ()) {
            // Leader starts a new block
            Block* newBlock = new Block (blockCount ++);
            nextBlock[cur] = newBlock;
            cur = newBlock;
            push_back (cur);
        }

        assert (cur != 0 && "First instruction not leader?");
        imop->setBlock (cur);
        cur->push_back (imop);
    }
}

// propagate successor/predecessor info
void Blocks::propagate (const std::map<Block*, Block*>& nextBlock) {
    std::set<Block* > visited;
    std::stack<Block* > todo;

    m_entryBlock = front ();
    todo.push (m_entryBlock);

    while (!todo.empty ()) {
        Block* cur = todo.top ();
        todo.pop ();

        if (visited.find (cur) != visited.end ()) continue;
        cur->reachable = true;
        Imop* lastImop = cur->lastImop ();

        // if block falls through we set its successor to be next block
        if (fallsThru (*cur)) {
            Block* next = nextBlock.find (cur)->second;
            todo.push (next);
            assert (lastImop->type () != Imop::JUMP);
            if (!lastImop->isJump ()) {
                linkBlocks (*cur, *next);
            } else {
                linkBlocksCondFalse (*cur, *next);
            }
        }

        // if last instruction is jump, link current block with its destination
        if (lastImop->isJump ()) {
            assert (dynamic_cast<const SymbolLabel*>(lastImop->dest ()) != 0);
            const SymbolLabel* jumpDest = static_cast<const SymbolLabel*>(lastImop->dest ());
            Block* next = jumpDest->target ()->block ();
            todo.push (next);
            if (lastImop->type () == Imop::JUMP) {
                linkBlocks (*cur, *next);
            }
            else {
                linkBlocksCondTrue (*cur, *next);
            }
        }

        if (lastImop->type () == Imop::END) {
            m_exitBlock = lastImop->block ();
        }

        // link call with its destination
        if (lastImop->type () == Imop::CALL) {
            Block* next = lastImop->callDest ()->block ();
            todo.push (next);
            linkCallBlocks (*cur, *next);

            Block* cleanBlock = nextBlock.find (cur)->second;
            todo.push (cleanBlock);
            cleanBlock->callPassFrom = cur;
            cur->callPassTo = cleanBlock;
        }

        // link returning block with all the possible places it may return to
        if (lastImop->type () == Imop::RETURN ||
                lastImop->type () == Imop::RETURNVOID) {
            assert (dynamic_cast<const SymbolLabel*>(lastImop->arg2()) != 0);
            Imop const* firstImop = static_cast<const SymbolLabel*>(lastImop->arg2())->target();
            const IS& ic = firstImop->incomingCalls();
            for (ISCI it(ic.begin()); it != ic.end(); ++ it) {
                assert((*it)->type() == Imop::CALL);
                assert (dynamic_cast<const SymbolLabel*>((*it)->arg2()) != 0);
                Imop const* clean = static_cast<const SymbolLabel*>((*it)->arg2())->target();
                linkRetBlocks (*cur, *clean->block ());
            }
        }

        visited.insert (cur);
    }
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
    users.clear ();
}

Block::~Block () {
    unlink ();
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Blocks &bs) {
    out << bs.toString();
    return out;
}
