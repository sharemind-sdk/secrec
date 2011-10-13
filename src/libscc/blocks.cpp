#include "blocks.h"

#include <algorithm>
#include <iostream>
#include <map>

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
    from.addUser (&to);
    to.addUser (&from);
}

inline void linkBlocks(SecreC::Block &from, SecreC::Block &to) {
    from.addSucc (&to);
    to.addPred (&from);
    addMutualUse (from, to);
}

inline void linkBlocksCondFalse(SecreC::Block &from, SecreC::Block &to) {
    from.addSuccCondFalse (&to);
    to.addPredCondFalse (&from);
    addMutualUse (from, to);
}

inline void linkBlocksCondTrue(SecreC::Block &from, SecreC::Block &to) {
    from.addSuccCondTrue (&to);
    to.addPredCondTrue (&from);
    addMutualUse (from, to);
}

inline void linkCallBlocks(SecreC::Block &from, SecreC::Block &to) {
    from.addSuccCall (&to);
    to.addPredCall (&from);
    addMutualUse (from, to);
}

inline void linkRetBlocks(SecreC::Block &from, SecreC::Block &to) {
    from.addSuccRet (&to);
    to.addPredRet (&from);
    addMutualUse (from, to);
}

template <typename Iter >
void printBlocks(std::ostream &os, const char *prefix, Iter begin, Iter end)
{
    if (begin != end) {
        os << prefix;
        std::set<unsigned long> reachables;
        std::set<unsigned long> unreachables;
        for (Iter jt = begin; jt != end; ++ jt) {
            if ((*jt)->reachable ()) {
                reachables.insert((*jt)->index ());
            } else {
                unreachables.insert((*jt)->index ());
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

void printBlockList(std::ostream &os, const char *prefix, const std::set<SecreC::Block*>& bl) {
    printBlocks (os, prefix, bl.begin (), bl.end ());
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

void Blocks::addBlockToProc (const SymbolProcedure* proc, Block* b) {
    b->setParent (proc);
    m_procBlocks[proc].push_back (b);
}

Blocks::ProcMap::iterator Blocks::beginProc () {
    return m_procBlocks.begin ();
}

Blocks::ProcMap::const_iterator Blocks::beginProc () const {
    return m_procBlocks.begin ();
}

Blocks::ProcMap::iterator Blocks::endProc () {
    return m_procBlocks.end ();
}

Blocks::ProcMap::const_iterator Blocks::endProc () const {
    return m_procBlocks.end ();
}

std::string Blocks::toString() const {
    typedef DataFlowAnalysis RD;

    std::ostringstream os;

    os << "BLOCKS" << std::endl;

    for (const_iterator it = begin(); it != end(); it++) {
        const Block* block = *it;
        os << "  Block " << block->index ();

        if (!block->reachable ()) {
            os << " [REMOVED]";
        }
        else
        if (block->parent () != 0) {
            os << " [" << block->parent ()->toString () << "]";
        }

        os << std::endl;
        printBlockList(os, "  ..... From: ", (*it)->pred ());
        printBlockList(os, "  ... From -: ", (*it)->predCondFalse ());
        printBlockList(os, "  ... From +: ", (*it)->predCondTrue ());
        printBlockList(os, "  . FromCall: ", (*it)->predCall ());
        printBlockList(os, "  .. FromRet: ", (*it)->predRet ());
        printBlockList(os, "  ....... To: ", (*it)->succ ());
        printBlockList(os, "  ..... To -: ", (*it)->succCondFalse ());
        printBlockList(os, "  ..... To +: ", (*it)->succCondTrue ());
        printBlockList(os, "  ... ToCall: ", (*it)->succCall ());
        printBlockList(os, "  .... ToRet: ", (*it)->succRet ());
        if ((*it)->callPassFrom () != 0) {
            os << "  . PassFrom: " << (*it)->callPassFrom ()->index () << std::endl;
        }
        if ((*it)->callPassTo () != 0) {
            os << "  ... PassTo: " << (*it)->callPassTo ()->index () << std::endl;;
        }

        // Print code:
        os << "    Code:" << std::endl;
        for (Block::iterator jt((*it)->begin ()); jt != (*it)->end (); jt++) {
            os << "      " << (*jt)->index () << "  " << (**jt);
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
    /// \todo following is hack to force nodes to be visited in topological order
    std::set<Block* > todo;

    m_entryBlock = front ();
    todo.insert (m_entryBlock);
    m_entryBlock->setParent (0);

    while (!todo.empty ()) {
        Block* cur = *todo.begin ();
        todo.erase (cur);

        if (visited.find (cur) != visited.end ()) continue;
        cur->setReachable ();
        m_procBlocks[cur->parent ()].push_back (cur);
        Imop* lastImop = cur->lastImop ();

        // link call with its destination
        if (lastImop->type () == Imop::CALL) {
            Block* next = lastImop->callDest ()->block ();
            next->setParent (static_cast<const SymbolProcedure*>(lastImop->dest ()));
            todo.insert (next);
            linkCallBlocks (*cur, *next);

            Block* cleanBlock = nextBlock.find (cur)->second;
            cleanBlock->setParent (cur->parent ());
            todo.insert (cleanBlock);
            cleanBlock->setCallPassFrom (cur);
            cur->setCallPassTo (cleanBlock);
        }

        // if block falls through we set its successor to be next block
        if (fallsThru (*cur)) {
            Block* next = nextBlock.find (cur)->second;
            next->setParent (cur->parent ());
            todo.insert (next);
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
            next->setParent (cur->parent ());
            todo.insert (next);
            if (lastImop->type () == Imop::JUMP) {
                linkBlocks (*cur, *next);
            }
            else {
                linkBlocksCondTrue (*cur, *next);
            }
        }

        if (lastImop->type () == Imop::END) {
            assert (m_exitBlock == 0 && "Can only have one exit block!");
            m_exitBlock = lastImop->block ();
        }

        // link returning block with all the possible places it may return to
        if (lastImop->type () == Imop::RETURN ||
                lastImop->type () == Imop::RETURNVOID) {
            assert (dynamic_cast<const SymbolLabel*>(lastImop->dest()) != 0);
            Imop const* firstImop = static_cast<const SymbolLabel*>(lastImop->dest())->target();
            const IS& ic = firstImop->incomingCalls();
            for (ISCI it(ic.begin()); it != ic.end(); ++ it) {
                const Imop* callImop = *it;
                assert(callImop->type() == Imop::CALL);
                linkRetBlocks (*cur, *nextBlock.find (callImop->block ())->second);
            }
        }

        visited.insert (cur);
    }
}

/*******************************************************************************
  Block
*******************************************************************************/

void Block::unlink () {
    Set::iterator i, e;
    for (i = m_users.begin (), e = m_users.end (); i != e; ++ i) {
        Block* that = *i;
        that->m_users.erase (this);
        that->m_predecessors.erase (this);
        that->m_predecessorsCondFalse.erase (this);
        that->m_predecessorsCondTrue.erase (this);
        that->m_predecessorsCall.erase (this);
        that->m_predecessorsRet.erase (this);
        that->m_successors.erase (this);
        that->m_successorsCondFalse.erase (this);
        that->m_successorsCondTrue.erase (this);
        that->m_successorsCall.erase (this);
        that->m_successorsRet.erase (this);
        if (that->m_callPassFrom == this) that->setCallPassFrom (0);
        if (that->m_callPassTo == this) that->setCallPassTo (0);
    }

    m_users.clear ();
}

Block::~Block () {
    unlink ();
}

void Block::getIncoming (std::set<Block*>& inc) const {
    inc.clear ();
    inc.insert (m_successors.begin (), m_successors.end ());
    inc.insert (m_successorsCondFalse.begin (), m_successorsCondFalse.end ());
    inc.insert (m_successorsCondTrue.begin (), m_successorsCondTrue.end ());
    inc.insert (m_successorsCall.begin (), m_successorsCall.end ());
    inc.insert (m_successorsRet.begin (), m_successorsRet.end ());
    if (m_callPassFrom != 0) {
        inc.insert (m_callPassFrom);
    }
}

void Block::getOutgoing (std::set<Block*>& out) const {
    out.clear ();
    out.insert (m_predecessors.begin (), m_predecessors.end ());
    out.insert (m_predecessorsCondFalse.begin (), m_predecessorsCondFalse.end ());
    out.insert (m_predecessorsCondTrue.begin (), m_predecessorsCondTrue.end ());
    out.insert (m_predecessorsCall.begin (), m_predecessorsCall.end ());
    out.insert (m_predecessorsRet.begin (), m_predecessorsRet.end ());
    if (m_callPassFrom != 0) {
        out.insert (m_callPassTo);
    }
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Blocks &bs) {
    out << bs.toString();
    return out;
}
