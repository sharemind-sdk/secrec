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

using namespace SecreC;

inline bool fallsThru(const SecreC::Block &b) {
    assert (!b.empty () &&
            "Empty basic block.");

    SecreC::Block::const_reverse_iterator i, e;
    const SecreC::Imop* last = 0;
    for (i = b.rbegin (), e = b.rend (); i != e; ++ i) {
        last = &*i;
        if (i->type () != SecreC::Imop::COMMENT) {
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

void printLabel (std::ostream& os, const Block& block) {
    os << "      ";
    os << "label = <<TABLE BORDER=\"0\">";
    os << "<TR><TD COLSPAN=\"2\" BORDER=\"1\" ALIGN=\"CENTER\">Block " << block.index () << "</TD></TR>";
    for (Block::const_iterator i = block.begin (), e = block.end (); i != e; ++ i) {
        os << "<TR>";
        os << "<TD ALIGN=\"LEFT\">" << i->index () << "</TD>";
        os << "<TD ALIGN=\"LEFT\">" << xmlEncode (i->toString ()) << "</TD>";
        os << "</TR>";
    }

    os << "</TABLE>>";
}

void printNode (std::ostream& os, const Block& block) {
    if (block.reachable ()) {
        os << "    \"node" << block.index () << "\" [\n";
        printLabel (os, block);
        os << "      shape = \"rectangle\"\n";
        if (block.isEntry ())
            os << "      style = \"bold\"\n";
        if (block.isExit ())
            os << "      style= \"rounded\"\n";
        os << "    ];\n";
    }
}

void printEdge (std::ostream& os, const Block& from, const Block& to, const char* style = 0) {
    if (from.reachable () && to.reachable ()) {
        os << "node" << from.index () << " -> " << "node" << to.index ();
        if (style) os << "[style=\"" << style << "\"]";
        os << ";";
    }
}

void printEdges (std::ostream& os, const Block& from, const std::set<Block*>& to, const char* style = 0) {
    typedef std::set<Block*>::const_iterator Iter;
    if (!to.empty ()) os << "    ";
    for (Iter i = to.begin (), e = to.end (); i != e; ++ i) {
        printEdge (os, from, **i, style);
        os << ' ';
    }
    if (!to.empty ()) os << "\n";
}

void printProcName (std::ostream& os, const Procedure& pr) {
    if (pr.name ())
        os << "    label = \"" << pr.name ()->toString () << "\";\n";
    else
        os << "    label = \"START\";\n";
}

template<class T >
struct disposer {
    void operator () (T* obj) {
        delete obj;
    }
};

} // anonymous namespace


namespace SecreC {

/*******************************************************************************
  Program
*******************************************************************************/

Program::Program () { }

Program::~Program () {
    clear_and_dispose (disposer<Procedure> ());
}

const Block* Program::entryBlock () const { return &front ().front (); }

const Block* Program::exitBlock () const { return &front ().back (); }

void Program::init (ICodeList &code) {
    code.resetIndexes ();
    assignToBlocks (code);
    propagate ();
}

void Program::assignToBlocks (ICodeList& imops) {
    unsigned blockCount = 1;
    assert (!imops.empty ());

    // 1. find leaders
    std::set<const Imop*> leaders;
    std::map<const Imop*, const SymbolProcedure*> functions;
    functions[&imops.front ()] = 0;

    bool nextIsLeader = true;  // first instruction is leader
    for (ImopList::const_iterator i = imops.begin ();i != imops.end (); ++ i) {
        if (nextIsLeader) {
            leaders.insert (&*i);
            nextIsLeader = false;
        }

        // destination of jump is leader
        if (i->isJump ()) {
            assert (dynamic_cast<const SymbolLabel*>(i->dest ()) != 0);
            const SymbolLabel* dest = static_cast<const SymbolLabel*>(i->dest ());
            leaders.insert (dest->target ());
        }

        // anything following terminator is leader
        if (i->isTerminator ()) {
            nextIsLeader = true;
        }

        // call destinations are leaders
        if (i->type () == Imop::CALL) {
            leaders.insert (i->callDest ());
            functions[i->callDest ()] = static_cast<const SymbolProcedure*>(i->dest ());
            nextIsLeader = true; // RETCLEAN is leader too
        }
    }

    // 2. assign all instructions to basic blocks in procedures
    // Anything in range [leader, nextLeader) is a basic block.
    Procedure* curProc = 0;
    Block* curBlock = 0;
    ImopList::iterator i = imops.begin ();
    while (! imops.empty ()) {
        Imop& imop = *i;
        if (functions.find (&imop) != functions.end ()) {
            curProc = new Procedure (functions[&imop]);
            push_back (*curProc);
        }

        if (leaders.find (&imop) != leaders.end ()) {
            curBlock = new Block (blockCount ++, curProc);
            curProc->push_back (*curBlock);
        }

        imop.setBlock (curBlock);
        i = imops.erase (i);
        curBlock->push_back (imop);


        switch (imop.type ()) {
        case Imop::END:
        case Imop::RETURN:
        case Imop::RETURNVOID:
        case Imop::ERROR:
            curProc->addExit (*curBlock);
        default:
            break;
        }
    }
}

struct BlockCmp {
    bool operator () (const Procedure::iterator& i, const Procedure::iterator& j) const {
        return i->index () < j->index ();
    }
};

void Program::propagate () {
    std::set<Procedure::iterator, BlockCmp > visited, todo;
    todo.insert (front ().begin ());

    while (!todo.empty ()) {
        Procedure::iterator cur = *todo.begin ();
        Procedure* curProc = cur->proc ();
        todo.erase (cur);

        if (visited.find (cur) != visited.end ()) continue;
        cur->setReachable ();
        Imop& lastImop = cur->back ();

        // link call with its destination
        if (lastImop.type () == Imop::CALL) {
            Procedure* callTarget = lastImop.callDest ()->block ()->proc ();
            Procedure::iterator next = procIterator (*lastImop.callDest ()->block ());
            todo.insert (next);
            linkCallBlocks (*cur, *next);
            callTarget->addCallFrom (*cur);

            Procedure::iterator cleanBlock = cur;
            ++ cleanBlock;
            assert (cleanBlock != curProc->end () && "Expecting RETCLEAN!");
            todo.insert (cleanBlock);
            cleanBlock->setCallPassFrom (&*cur);
            cur->setCallPassTo (&*cleanBlock);

            for (std::set<Block*>::const_iterator it = callTarget->exitBlocks ().begin ();
                 it != callTarget->exitBlocks ().end (); ++ it)
            {
                Block* exitBlock = *it;
                switch (exitBlock->back ().type ()) {
                case Imop::RETURN:
                case Imop::RETURNVOID:
                    linkRetBlocks (*exitBlock, *cleanBlock);
                    exitBlock->proc ()->addReturnTo (*cleanBlock);
                default:
                    break;
                }
            }
        }

        // if block falls through we set its successor to be next block
        if (fallsThru (*cur)) {
            Procedure::iterator next = cur;
            ++ next;
            assert (next != curProc->end () && "Must not fall out of procedure!");
            todo.insert (next);
            assert (lastImop.type () != Imop::JUMP);
            if (!lastImop.isJump ()) {
                linkBlocks (*cur, *next);
            } else {
                linkBlocksCondFalse (*cur, *next);
            }
        }

        // if last instruction is jump, link current block with its destination
        if (lastImop.isJump ()) {
            assert (dynamic_cast<const SymbolLabel*>(lastImop.dest ()) != 0);
            const SymbolLabel* jumpDest = static_cast<const SymbolLabel*>(lastImop.dest ());
            Procedure::iterator next = procIterator (*jumpDest->target ()->block ());
            todo.insert (next);
            if (lastImop.type () == Imop::JUMP) {
                linkBlocks (*cur, *next);
            }
            else {
                linkBlocksCondTrue (*cur, *next);
            }
        }

        visited.insert (cur);
    }
}

std::string Program::toString() const {
    typedef DataFlowAnalysis RD;

    std::ostringstream os;

    os << "PROCEDURES:" << std::endl;
    for (const_iterator pi = begin(); pi != end(); ++ pi) {
        if (pi->name ())
            os << "  " << pi->name ()->toString () << std::endl;
        printBlockList(os, "  .. From: ", pi->callFrom ());
        printBlockList(os, "  .... To: ", pi->returnTo ());
        os << "  BLOCKS:" << std::endl;
        for (Procedure::const_iterator it = pi->begin (); it != pi->end (); ++ it) {
            os << "    Block " << it->index ();
            if (!it->reachable ()) os << " [REMOVED]";
            if (it->isExit ()) os << " [EXIT]";

            os << std::endl;
            printBlockList(os, "    ..... From: ", it->pred ());
            printBlockList(os, "    ... From -: ", it->predCondFalse ());
            printBlockList(os, "    ... From +: ", it->predCondTrue ());
            printBlockList(os, "    . FromCall: ", it->predCall ());
            printBlockList(os, "    .. FromRet: ", it->predRet ());
            printBlockList(os, "    ....... To: ", it->succ ());
            printBlockList(os, "    ..... To -: ", it->succCondFalse ());
            printBlockList(os, "    ..... To +: ", it->succCondTrue ());
            printBlockList(os, "    ... ToCall: ", it->succCall ());
            printBlockList(os, "    .... ToRet: ", it->succRet ());
            if (it->callPassFrom () != 0) {
                os << "    . PassFrom: " << it->callPassFrom ()->index () << std::endl;
            }
            if (it->callPassTo () != 0) {
                os << "    ... PassTo: " << it->callPassTo ()->index () << std::endl;;
            }

            // Print code:
            os << "    CODE:" << std::endl;
            for (Block::const_iterator jt(it->begin ()); jt != it->end (); jt++) {
                os << "      " << jt->index () << "  " << (*jt);
                if (jt->creator() != 0) {
                    os << " // Created by "
                       << TreeNode::typeName(jt->creator()->type())
                       << " at "
                       << jt->creator()->location();
                    if (jt->creator()->type() == NODE_EXPR_CLASSIFY) {
                        assert(jt->creator()->parent() != 0);
                        os << " for "
                           << TreeNode::typeName(jt->creator()->parent()->type())
                           << " at "
                           << jt->creator()->parent()->location();
                    }
                }
                os << std::endl;
            }

            os << std::endl;
        }
    }

    return os.str();
}

void Program::toDotty (std::ostream& os) const {
    unsigned uniq = 0;

    os << "digraph CFG {\n";
    for (const_iterator pi = begin(); pi != end(); ++ pi) {
        os << "  subgraph cluster" << uniq ++ << " {\n";
        for (Procedure::const_iterator i = pi->begin (); i != pi->end (); ++ i)
            printNode (os, *i);
        for (Procedure::const_iterator i = pi->begin (); i != pi->end (); ++ i) {
            printEdges (os, *i, i->succ ());
            printEdges (os, *i, i->succCondFalse ());
            printEdges (os, *i, i->succCondTrue ());
            if (i->callPassTo () != 0) {
                os << "    ";
                printEdge (os, *i, *i->callPassTo ());
                os << '\n';
            }
        }

        printProcName (os, *pi);
        os << "  }\n\n";
    }

    for (const_iterator pi = begin(); pi != end(); ++ pi) {
        for (Procedure::const_iterator i = pi->begin (); i != pi->end (); ++ i) {
            printEdges (os, *i, i->succCall (), "dotted");
            printEdges (os, *i, i->succRet (), "dotted");
        }
    }

    os << "}\n";
}


/*******************************************************************************
  Procedure
*******************************************************************************/

Procedure::~Procedure () {
    clear_and_dispose (disposer<Block> ());
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
    clear_and_dispose (disposer<Imop> ());
}

void Block::getOutgoing (std::set<Block*>& inc) const {
    inc.clear ();
    inc.insert (m_successors.begin (), m_successors.end ());
    inc.insert (m_successorsCondFalse.begin (), m_successorsCondFalse.end ());
    inc.insert (m_successorsCondTrue.begin (), m_successorsCondTrue.end ());
    inc.insert (m_successorsCall.begin (), m_successorsCall.end ());
    inc.insert (m_successorsRet.begin (), m_successorsRet.end ());
    if (m_callPassTo != 0) {
        inc.insert (m_callPassTo);
    }
}

void Block::getIncoming (std::set<Block*>& out) const {
    out.clear ();
    out.insert (m_predecessors.begin (), m_predecessors.end ());
    out.insert (m_predecessorsCondFalse.begin (), m_predecessorsCondFalse.end ());
    out.insert (m_predecessorsCondTrue.begin (), m_predecessorsCondTrue.end ());
    out.insert (m_predecessorsCall.begin (), m_predecessorsCall.end ());
    out.insert (m_predecessorsRet.begin (), m_predecessorsRet.end ());
    if (m_callPassFrom != 0) {
        out.insert (m_callPassFrom);
    }
}

bool Block::isEntry () const {
    return this == proc ()->entry ();
}

bool Block::isExit () const {
    const std::set<Block*>& exits = m_proc->exitBlocks ();
    return exits.find (const_cast<Block*>(this)) != exits.end ();
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Program &proc) {
    out << proc.toString();
    return out;
}
