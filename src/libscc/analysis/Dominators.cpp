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

#include "Dominators.h"

#include "../Blocks.h"
#include "../Symbol.h"
#include "../TreeNode.h"

#include <boost/range/adaptor/reversed.hpp>


using boost::adaptors::reverse;

namespace SecreC {

namespace { /* anonymous */

struct DomInfo {

    explicit DomInfo(Block * block_)
        : block(block_)
        , idom(0)
    { }

    Block*               block;
    size_t               idom;
    std::vector<size_t>  preds;
};

size_t intersect(const std::vector<DomInfo> & info, size_t a, size_t b) {
    while (a != b) {
        while (a < b) a = info[a].idom;
        while (b < a) b = info[b].idom;
    }

    return a;
}

void calculateIDoms(std::vector<DomInfo> & info, size_t startNode) {
    const size_t undef = info.size();
    for (DomInfo & node : info) {
        node.idom = undef;
    }

    const Block * startNodeBlock = info[startNode].block;
    info[startNode].idom = startNode;
    bool changed = true;

    while (changed) {
        changed = false;
        for (DomInfo & node : reverse (info)) {
            if (node.block == startNodeBlock) {
                continue;
            }

            size_t newIDom = undef;
            for (size_t pred : node.preds) {
                if (info[pred].idom != undef) {
                    if (newIDom == undef) {
                        newIDom = pred;
                    }
                    else {
                        newIDom = intersect(info, pred, newIDom);
                    }
                }
            }

            if (node.idom != newIDom) {
                node.idom = newIDom;
                changed = true;
            }
        }
    }
}

void printEdge(std::ostream & os, DominanceNode * parent, DominanceNode * child) {
    os << "    "
       << parent->block()->dfn() << " -> "
       << child->block()->dfn() << ";\n";
}

void printTree(std::ostream & os, DominanceNode * node) {
    std::vector<DominanceNode *> todo;
    todo.push_back(node);

    os << "  subgraph cluster" << node->block()->dfn() << " {\n";
    os << "    " << node->block()->dfn() << " [shape=box];\n";

    while (! todo.empty()) {
        DominanceNode * parent = todo.back();
        todo.pop_back();
        for (auto const & child : parent->children()) {
            printEdge(os, parent, child.get());
            todo.push_back(child.get());
        }
    }

    os << "  }\n\n";
}

} // namespace anonymous

/*******************************************************************************
  DominanceNode
*******************************************************************************/

DominanceNode::~DominanceNode() {
    m_children.clear();
    m_parent = nullptr;
}

/*******************************************************************************
  Dominators
*******************************************************************************/

void Dominators::calculate(Program * prog) {
    for (Procedure & proc : *prog) {
        calculate(&proc);
    }
}

void Dominators::calculate(Procedure * proc) {
    calculate(proc->entry());
}

void Dominators::calculate(Block * root) {
    std::set<Block *> visited;
    std::vector<std::pair<Block *, Block::neighbour_const_iterator> > workList;
    std::vector<DomInfo> info;
    std::map<Block *, size_t> pon; // post-order number

    // Number the nodes in postorder:
    visited.insert(root);
    workList.push_back(std::make_pair(root, root->succ_begin()));

    while (! workList.empty()) {
        Block * block = workList.back().first;
        Block::neighbour_const_iterator & it = workList.back().second;

        if (it == block->succ_end()) {
            pon[block] = info.size();
            info.push_back(DomInfo(block));
            workList.pop_back();
            continue;
        }

        // skip non-local edges
        if (Edge::isGlobal(it->second)) {
            it ++;
            continue;
        }

        Block * next = (it ++)->first;
        assert(next != nullptr);

        if (visited.insert(next).second) {
            workList.push_back(std::make_pair(next, next->succ_begin()));
        }
    }

    // Cache predcessors:
    for (auto v : pon) {
        Block * block = v.first;
        for (Block::edge_type e : block->predecessors ()) {
            if (Edge::isLocal(e.second)) {
                info[v.second].preds.push_back(pon[e.first]);
            }
        }
    }

    // Calculate immediate dominators:
    calculateIDoms(info, pon[root]);

    // Build dominator tree, and add it to forest:
    for (const DomInfo & i : info) {
        DominanceNode * child = findNode(i.block);
        assert(i.idom < info.size());

        if (i.idom < info.size()) {
            DominanceNode * parent = findNode(info[i.idom].block);
            child->setParent(parent);

            if (&i == &info[i.idom]) {
                m_roots.emplace_back(parent);
            }
            else {
                parent->addChild(child);
            }
        }
    }
}

void Dominators::dumpToDot(std::ostream & os) {
    os << "digraph IDOM {\n";
    for (auto const & root : m_roots) {
        printTree(os, root.get());
    }

    os << "}\n";
}

} // namespace SecreC
