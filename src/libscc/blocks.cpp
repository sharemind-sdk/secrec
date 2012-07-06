#include "blocks.h"

#include <algorithm>
#include <boost/foreach.hpp>
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

    const SecreC::Imop* last = 0;
    BOOST_REVERSE_FOREACH (const Imop& imop, b) {
        last = &imop;
        if (imop.type () != SecreC::Imop::COMMENT) {
            break;
        }
    }

    assert (last != 0);
    switch (last->type ()) {
    case SecreC::Imop::CALL:
    case SecreC::Imop::JUMP:
    case SecreC::Imop::END:
    case SecreC::Imop::RETURN:
    case SecreC::Imop::ERROR:
        return false;
    default:
        break;
    }

    return true;
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

template <typename Iter>
std::map<Edge::Label, std::set<Block*> > transpose (Iter begin, Iter end) {
    std::map<Edge::Label, std::set<Block*> > out;
    for (Iter i = begin; i != end; ++ i) {
        for (Edge::Label label = Edge::begin; label != Edge::end; label = Edge::next (label)) {
            if (i->second & label) {
                out[label].insert (i->first);
            }
        }
    }

    return out;
}

void printHeader (std::ostream& os, const Block& block) {
    std::map<Edge::Label, std::set<Block*> > succ = transpose (block.succ_begin (), block.succ_end ());
    std::map<Edge::Label, std::set<Block*> > pred = transpose (block.pred_begin (), block.pred_end ());
    printBlockList(os, "    ..... From: ", pred[Edge::Jump]);
    printBlockList(os, "    ... From -: ", pred[Edge::False]);
    printBlockList(os, "    ... From +: ", pred[Edge::True]);
    printBlockList(os, "    . FromCall: ", pred[Edge::Call]);
    printBlockList(os, "    .. FromRet: ", pred[Edge::Ret]);
    printBlockList(os, "    . PassFrom: ", pred[Edge::CallPass]);
    printBlockList(os, "    ....... To: ", succ[Edge::Jump]);
    printBlockList(os, "    ..... To -: ", succ[Edge::False]);
    printBlockList(os, "    ..... To +: ", succ[Edge::True]);
    printBlockList(os, "    ... ToCall: ", succ[Edge::Call]);
    printBlockList(os, "    .... ToRet: ", succ[Edge::Ret]);
    printBlockList(os, "    ... PassTo: ", succ[Edge::CallPass]);
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
        os << '\n';
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
        if (style) os << style;
        os << ";";
    }
}

void printEdges (std::ostream& os, const Block& from, Edge::Label label, const char* style = 0) {
    bool foundAny = false;
    BOOST_FOREACH (const Block::edge_type& edge, from.succ_range ()) {
        if (edge.second & label) {
            if (! foundAny) {
                os << "    ";
            }

            printEdge (os, from, *edge.first, style);
            foundAny = true;
        }
    }

    if (foundAny)
        os << "\n";
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

const Block* Program::entryBlock () const {
    assert (! empty ());
    return &front ().front ();
}

const Block* Program::exitBlock () const {
    assert (! empty ());
    return &front ().back ();
}

void Program::init (ICodeList &code) {
    code.resetIndexes ();
    assignToBlocks (code);
    propagate ();
    numberBlocks ();
}

void Program::assignToBlocks (ICodeList& imops) {
    assert (!imops.empty ());

    // 1. find leaders
    std::set<const Imop*> leaders;
    std::map<const Imop*, std::set<SymbolLabel*> > jumps;
    std::map<const Imop*, const SymbolProcedure*> functions;
    functions[&imops.front ()] = 0;

    bool nextIsLeader = true;  // first instruction is leader
    BOOST_FOREACH (Imop& imop, imops) {
        if (nextIsLeader) {
            leaders.insert (&imop);
            nextIsLeader = false;
        }

        // destination of jump is leader
        if (imop.isJump ()) {
            assert (dynamic_cast<const SymbolLabel*>(imop.dest ()) != 0);
            SymbolLabel* dest = static_cast<SymbolLabel*>(imop.dest ());
            leaders.insert (dest->target ());
            jumps[dest->target ()].insert (dest);
        }

        // anything following terminator is leader
        if (imop.isTerminator ()) {
            nextIsLeader = true;
        }

        // call destinations are leaders (we might have interprocedural analysis)
        if (imop.type () == Imop::CALL) {
            leaders.insert (imop.callDest ());
            functions[imop.callDest ()] = static_cast<const SymbolProcedure*>(imop.dest ());
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
            curBlock = new Block ();
            curProc->push_back (*curBlock);
            BOOST_FOREACH (SymbolLabel* incoming, jumps[&imop]) {
                incoming->setBlock (curBlock);
            }
        }

        i = imops.erase (i);
        curBlock->push_back (imop);


        switch (imop.type ()) {
        case Imop::END:
        case Imop::RETURN:
        case Imop::ERROR:
            curProc->addExit (*curBlock);
        default:
            break;
        }
    }
}

struct BlockCmp {
    bool operator () (const Procedure::iterator& i, const Procedure::iterator& j) const {
        return &*i < &*j;
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
            Block::addEdge (*cur, Edge::Call, *next);

            Procedure::iterator cleanBlock = cur;
            ++ cleanBlock;
            assert (cleanBlock != curProc->end () && "Expecting RETCLEAN!");
            todo.insert (cleanBlock);
            Block::addEdge (*cur, Edge::CallPass, *cleanBlock);

            BOOST_FOREACH (Block* exitBlock, callTarget->exitBlocks ()) {
                switch (exitBlock->back ().type ()) {
                case Imop::RETURN:
                    Block::addEdge (*exitBlock, Edge::Ret, *cleanBlock);
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
            Edge::Label label = Edge::None;
            switch (lastImop.type ()) {
            case Imop::JUMP: assert (false);      break;
            case Imop::JT:   label = Edge::False; break;
            case Imop::JF:   label = Edge::True;  break;
            default:         label = Edge::Jump;  break;
            }

            Block::addEdge (*cur, label, *next);
        }

        // if last instruction is jump, link current block with its destination
        if (lastImop.isJump ()) {
            assert (dynamic_cast<const SymbolLabel*>(lastImop.dest ()) != 0);
            const SymbolLabel* jumpDest = static_cast<const SymbolLabel*>(lastImop.dest ());
            Procedure::iterator next = procIterator (*jumpDest->block ());
            todo.insert (next);
            Edge::Label label = Edge::None;
            switch (lastImop.type ()) {
            case Imop::JUMP: label = Edge::Jump;  break;
            case Imop::JT:   label = Edge::True;  break;
            case Imop::JF:   label = Edge::False; break;
            default:         assert (false);      break;
            }

            Block::addEdge (*cur, label, *next);
        }

        visited.insert (cur);
    }
}

void Program::numberBlocks () {
    std::set<Block*> visited;
    std::vector<Block::neighbour_const_range> stack;
    size_t number = 0;

    BOOST_FOREACH (Procedure& proc, *this) {
        Block* entry = proc.entry ();
        if (visited.insert (entry).second) {
            entry->setDfn (++ number);
            stack.push_back (entry->succ_range ());
        }

        while (!stack.empty ()) {
            Block::neighbour_const_range& range = stack.back ();
            if (range.first == range.second) {
                stack.pop_back ();
                continue;
            }

            Block* block = range.first->first;
            Edge::Label label = range.first->second;
            ++ range.first;

            if (Edge::isGlobal (label)) {
                continue;
            }

            if (visited.insert (block).second) {
                block.setDfn (++ number);
                stack.push_back (block->succ_range ());
            }
        }
    }
}

std::string Program::toString() const {
    typedef DataFlowAnalysis RD;

    std::ostringstream os;

    os << "PROCEDURES:" << std::endl;
    BOOST_FOREACH (const Procedure& proc, *this) {
        if (proc.name ())
            os << "  " << proc.name ()->toString () << std::endl;
        printBlockList(os, "  .. From: ", proc.callFrom ());
        printBlockList(os, "  .... To: ", proc.returnTo ());
        os << "  BLOCKS:" << std::endl;
        BOOST_FOREACH (const Block& block, proc) {
            os << "    Block " << block.index ();
            if (!block.reachable ()) os << " [REMOVED]";
            if (block.isExit ()) os << " [EXIT]";

            os << std::endl;
            printHeader (os, block);

            // Print code:
            os << "    CODE:" << std::endl;
            BOOST_FOREACH (const Imop& imop, block) {
                os << "      " << imop.index () << "  " << imop;
                if (imop.creator() != 0) {
                    os << " // Created by "
                       << TreeNode::typeName(imop.creator()->type())
                       << " at "
                       << imop.creator()->location();
                    if (imop.creator()->type() == NODE_EXPR_CLASSIFY) {
                        assert(imop.creator()->parent() != 0);
                        os << " for "
                           << TreeNode::typeName(imop.creator()->parent()->type())
                           << " at "
                           << imop.creator()->parent()->location();
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
    uint64_t uniq = 0;

    os << "digraph CFG {\n";
    BOOST_FOREACH (const Procedure& proc, *this) {
        os << "  subgraph cluster" << uniq ++ << " {\n";
        BOOST_FOREACH (const Block& block, proc) {
            printNode (os, block);
        }

        BOOST_FOREACH (const Block& block, proc) {
            printEdges (os, block, Edge::Jump);
            printEdges (os, block, Edge::False, "[label=\"-\"]");
            printEdges (os, block, Edge::True,  "[label=\"+\"]");
            printEdges (os, block, Edge::CallPass);
        }

        printProcName (os, proc);
        os << "  }\n\n";
    }

    BOOST_FOREACH (const Procedure& proc, *this) {
        BOOST_FOREACH (const Block& block, proc) {
            printEdges (os, block, Edge::Call, "[style = \"dotted\"]");
            printEdges (os, block, Edge::Ret,  "[style = \"dotted\"]");
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

bool Block::hasIncomingJumps () const {
    BOOST_FOREACH (edge_type edge, pred_range ()) {
        const Block& from = *edge.first;
        if (from.empty ()) {
            if (from.hasIncomingJumps ())
                return true;
            continue;
        }

        const Edge::Label label = edge.second;
        const Imop::Type type = from.back ().type ();
        if (label & Edge::Jump)
            if (type == Imop::JUMP)
                return true;

        if (label & Edge::True)
            if (type == Imop::JT)
                return true;

        if (label & Edge::False)
            if (type == Imop::JF)
                return true;
    }

    return false;
}

Block::~Block () {
    unlink ();
    clear_and_dispose (disposer<Imop> ());
}

bool Block::isProgramEntry () const {
    return isEntry () && (proc ()->name () == 0);
}

bool Block::isProgramExit () const {
    return isExit () && (proc ()->name () == 0);
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
