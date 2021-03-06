/*
 * Copyright (C) 2015 Cybernetica
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

#include "Blocks.h"

#include "DataflowAnalysis.h"
#include "Misc.h"
#include "Symbol.h"
#include "TreeNode.h"

#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>

namespace SecreC {

namespace /* anonymous */ {

inline bool fallsThru(const Block &b) {
    assert (!b.empty () &&
            "Empty basic block.");

    const Imop* last = nullptr;
    for (const Imop& imop : boost::adaptors::reverse (b)) {
        last = &imop;
        if (imop.type () != Imop::COMMENT) {
            break;
        }
    }

    assert (last != nullptr);
    switch (last->type ()) {
    case Imop::CALL:
    case Imop::JUMP:
    case Imop::END:
    case Imop::RETURN:
    case Imop::ERROR:
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
        for (auto jt = reachables.begin(); jt != reachables.end(); ++ jt) {
            if (jt != reachables.begin()) os << ", ";
            os << (*jt);
        }
        if (!reachables.empty() && !unreachables.empty()) os << " ";
        if (!unreachables.empty()) {
            os << "(";
            for (auto jt = unreachables.begin(); jt != unreachables.end(); ++ jt) {
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
    for (const auto & elem : block) {
        os << "<TR>";
        os << "<TD ALIGN=\"LEFT\">" << elem.index () << "</TD>";
        std::stringstream ss;
        ss << elem;
        os << "<TD ALIGN=\"LEFT\">" << xmlEncode (ss.str ()) << "</TD>";
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

void printEdge (std::ostream& os, const Block& from, const Block& to, const char* style = nullptr) {
    if (from.reachable () && to.reachable ()) {
        os << "node" << from.index () << " -> " << "node" << to.index ();
        if (style) os << style;
        os << ";";
    }
}

void printEdges (std::ostream& os, const Block& from, Edge::Label label, const char* style = nullptr) {
    bool foundAny = false;
    for (const auto& edge : from.successors ()) {
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
        os << "    label = \"" << *pr.name () << "\";\n";
    else
        os << "    label = \"START\";\n";
}

template<class T >
struct disposer {
    void operator () (T* obj) const {
        delete obj;
    }
};

struct LeaderInfo {
    std::set<SymbolLabel*>  jumps;
    const SymbolProcedure*  procedure;

    LeaderInfo ()
        : procedure (nullptr)
    { }
};

} // anonymous namespace

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
    propagate (front (), true);
    numberBlocks ();
}

void Program::assignToBlocks (ICodeList& imops) {
    assert (!imops.empty ());

    // 1. find leaders
    std::map<const Imop*, LeaderInfo> leaders;

    bool nextIsLeader = true;  // first instruction is leader
    for (Imop& imop : imops) {
        if (nextIsLeader) {
            leaders[&imop];
            nextIsLeader = false;
        }

        // destination of jump is leader
        if (imop.isJump ()) {
            assert (dynamic_cast<const SymbolLabel*>(imop.dest ()) != nullptr);
            SymbolLabel* dest = static_cast<SymbolLabel*>(imop.dest ());
            leaders[dest->target()].jumps.insert (dest);
        }

        // anything following terminator is leader
        if (imop.isTerminator ()) {
            nextIsLeader = true;
        }

        // call destinations are leaders (we might have interprocedural analysis)
        if (imop.type () == Imop::CALL) {
            leaders[imop.callDest()].procedure = static_cast<const SymbolProcedure*>(imop.dest ());
            nextIsLeader = true; // RETCLEAN is leader too
        }
    }

    // 2. assign all instructions to basic blocks in procedures
    // Anything in range [leader, nextLeader) is a basic block.
    ImopList::iterator i = imops.begin ();
    std::map<const Imop*, LeaderInfo>::iterator it;

    // Initialize the entry procedure.
    auto curProc = new Procedure (nullptr);
    Block* curBlock = nullptr;
    push_back(*curProc);

    while (! imops.empty ()) {
        Imop& imop = *i;
        it = leaders.find (&imop);

        if (it != leaders.end ()) {
            const std::set<SymbolLabel*>& jumps = it->second.jumps;
            const SymbolProcedure* proc = it->second.procedure;

            if (proc != nullptr) {
                curProc = new Procedure (proc);
                push_back (*curProc);
            }

            curBlock = new Block ();
            curProc->push_back (*curBlock);
            for (SymbolLabel* incoming : jumps) {
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

void Program::propagate (Procedure& proc, bool visitCalls) {
    std::set<Procedure::iterator, BlockCmp > visited, todo;
    todo.insert (proc.begin ());

    while (!todo.empty ()) {
        const Procedure::iterator cur = *todo.begin ();
        todo.erase (cur);

        if (visited.find (cur) != visited.end ()) continue;
        cur->setReachable ();
        const Imop& lastImop = cur->back ();

        // link call with its destination
        if (lastImop.type () == Imop::CALL) {
            Procedure* const callTarget = lastImop.callDest ()->block ()->proc ();
            const Procedure::iterator next = procIterator (*lastImop.callDest ()->block ());
            if (visitCalls)
                todo.insert (next);
            Block::addEdge (*cur, Edge::Call, *next);
            callTarget->addCallFrom (*cur);

            const Procedure::iterator cleanBlock = std::next(cur);
            todo.insert (cleanBlock);
            Block::addEdge (*cur, Edge::CallPass, *cleanBlock);

            for (Block* exitBlock : callTarget->exitBlocks ()) {
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
            const Procedure::iterator next = std::next(cur);
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
            assert (dynamic_cast<const SymbolLabel*>(lastImop.dest ()) != nullptr);
            const SymbolLabel* const jumpDest = static_cast<const SymbolLabel*>(lastImop.dest ());
            const Procedure::iterator next = procIterator (*jumpDest->block ());
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

void Program::numberInstructions () {
    unsigned long index = 0;

    for (auto& proc : *this) {
        for (auto& block : proc) {
            for (auto& imop : block) {
                imop.setIndex (index++);
            }
        }
    }
}

void Program::numberBlocks () {
    std::set<Block*> visited;
    std::vector<Block::neighbour_const_range> stack;
    size_t number = 0;

    for (Procedure& proc : *this) {
        Block* const entry = proc.entry ();
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
                block->setDfn (++ number);
                stack.push_back (block->succ_range ());
            }
        }
    }
}

void Program::buildProcedureCFG (Procedure& proc) {
    propagate (proc, true);
    numberBlocks ();
}

std::ostream & Program::print(std::ostream & os) const {
    os << "PROCEDURES:" << std::endl;
    for (const Procedure& proc : *this) {
        if (proc.name ())
            os << "  " << *proc.name () << std::endl;
        printBlockList(os, "  .. From: ", proc.callFrom ());
        printBlockList(os, "  .... To: ", proc.returnTo ());
        os << "  BLOCKS:" << std::endl;
        for (const Block& block : proc) {
            os << "    Block " << block.index ();
            if (!block.reachable ()) os << " [REMOVED]";
            if (block.isExit ()) os << " [EXIT]";

            os << std::endl;
            printHeader (os, block);

            // Print code:
            os << "    CODE:" << std::endl;
            for (const Imop& imop : block) {
                os << std::setw (10) << imop.index () << "  " << imop;
                if (imop.creator() != nullptr) {
                    os << " // Created by "
                       << TreeNode::typeName(imop.creator()->type())
                       << " at "
                       << imop.creator()->location();
                    if (imop.creator()->type() == NODE_EXPR_CLASSIFY) {
                        assert(imop.creator()->parent() != nullptr);
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

    return os;
}

void Program::toDotty (std::ostream& os) const {
    uint64_t uniq = 0;

    os << "digraph CFG {\n";
    for (const Procedure& proc : *this) {
        os << "  subgraph cluster" << uniq ++ << " {\n";
        for (const Block& block : proc) {
            printNode (os, block);
        }

        for (const Block& block : proc) {
            printEdges (os, block, Edge::Jump);
            printEdges (os, block, Edge::False, "[label=\"-\"]");
            printEdges (os, block, Edge::True,  "[label=\"+\"]");
            printEdges (os, block, Edge::CallPass);
        }

        printProcName (os, proc);
        os << "  }\n\n";
    }

    for (const Procedure& proc : *this) {
        for (const Block& block : proc) {
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
    for (edge_type edge : predecessors ()) {
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

bool Block::isEntry () const {
    return this == proc ()->entry ();
}

bool Block::isExit () const {
    const std::set<Block*>& exits = m_proc->exitBlocks ();
    return exits.find (const_cast<Block*>(this)) != exits.end ();
}

} // namespace SecreC
