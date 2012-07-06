/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "analysis/Dominators.h"

#include <boost/foreach.hpp>

#include "symbol.h"
#include "blocks.h"
#include "treenode.h"

namespace SecreC {

namespace /* anonymous */ {

struct DomInfo {

    explicit DomInfo (Block* block)
        : block (block)
        , idom (0)
    { }

    Block*               block;
    size_t               idom;
    std::vector<size_t>  preds;
};

size_t intersect (const std::vector<DomInfo>& info, size_t a, size_t b) {
    while (a != b) {
        while (a < b) a = info[a].idom;
        while (b < a) b = info[b].idom;
    }

    return a;
}

void findIDoms (std::vector<DomInfo>& info, size_t startNode) {
    const size_t undef = info.size ();
    BOOST_FOREACH (DomInfo& node, info) {
        node.idom = undef;
    }

    const Block* startNodeBlock = info[startNode].block;
    info[startNode].idom = startNode;
    bool changed = true;
    while (changed) {
        changed = false;
        BOOST_REVERSE_FOREACH (DomInfo& node, info) {
            if (node.block == startNodeBlock)
                continue;

            size_t newIDom = undef;
            BOOST_FOREACH (size_t pred, node.preds) {
                if (info[pred].idom != undef) {
                    if (newIDom == undef)
                        newIDom = pred;
                    else
                        newIDom = intersect (info, pred, newIDom);
                }
            }

            if (node.idom != newIDom) {
                node.idom = newIDom;
                changed = true;
            }
        }
    }
}

void printEdge (std::ostream& os, DominanceNode* parent, DominanceNode* child) {
    os << "    "
       << parent->block ()->dfn () << " -> "
       << child->block ()->dfn () << ";\n";
}

void printTree (std::ostream& os, DominanceNode* node) {
    std::vector<DominanceNode*> todo;
    std::set<DominanceNode*> visited;
    todo.push_back (node);

    os << "  subgraph cluster" << node << " {\n";
    os << "    " << node->block ()->dfn () << " [shape=box];\n";
    while (! todo.empty ()) {
        DominanceNode* parent = todo.back ();
        visited.insert (parent);
        todo.pop_back ();
        BOOST_FOREACH (DominanceNode* child, parent->children ()) {
            printEdge (os, parent, child);
            if (visited.find (child) == visited.end ()) {
                todo.push_back (child);
            }
        }
    }

    os << "  }\n\n";
}

} // namespace anonymous

/*******************************************************************************
  DominanceNode
*******************************************************************************/

DominanceNode::~DominanceNode () {
    BOOST_FOREACH (DominanceNode* child, m_children) {
        delete child;
    }

    m_children.clear ();
    m_parent = 0;
}

/*******************************************************************************
  Dominators
*******************************************************************************/

Dominators::~Dominators () {
    BOOST_FOREACH (DominanceNode* root, m_roots) {
        delete root;
    }
}

void Dominators::calculate (Program* prog) {
    BOOST_FOREACH (Procedure& proc, *prog) {
        calculate (&proc);
    }
}

void Dominators::calculate (Procedure* proc) {
    calculate (proc->entry ());
}

void Dominators::calculate (Block* root) {
    std::set<Block* > visited;
    std::vector<std::pair<Block*, Block::neighbour_const_range> > workList;
    std::vector<DomInfo> info;
    std::map<Block*, size_t> pon; // post-order number

    // Number the nodes in postorder:
    visited.insert (root);
    workList.push_back (std::make_pair (root, root->succ_range ()));

    while (! workList.empty ()) {
        Block::neighbour_const_range& range = workList.back ().second;
        if (range.first == range.second) {
            Block* block = workList.back ().first;
            pon[block] = info.size ();
            info.push_back (DomInfo (block));
            workList.pop_back ();
            continue;
        }

        // skip non-local edges
        if (Edge::isGlobal (range.first->second)) {
            range.first ++;
            continue;
        }

        Block* next = (range.first ++)->first;
        assert (next != 0);
        if (visited.insert (next).second) {
            workList.push_back (std::make_pair (next, next->succ_range ()));
        }
    }

    // Find predcessors:
    typedef std::map<Block*, size_t> MapType;
    BOOST_FOREACH (MapType::value_type v, pon) {
        Block* block = v.first;
        BOOST_FOREACH (Block::edge_type e, block->pred_range ()) {
            if (Edge::isLocal (e.second)) {
                info[v.second].preds.push_back (pon[e.first]);
            }
        }
    }

    // Calculate immediate dominators:
    findIDoms (info, pon[root]);

    // Build dominator tree, and add it to forest:
    BOOST_FOREACH (const DomInfo& i, info) {
        DominanceNode* child = findNode (i.block);
        assert (i.idom < info.size ());
        if (i.idom < info.size ()) {
            DominanceNode* parent = findNode (info[i.idom].block);
            child->setParent (parent);

            if (&i == &info[i.idom]) {
                m_roots.push_back (parent);
            } else {
                parent->addChild (child);
            }
        }
    }
}

void Dominators::dumpToDot (std::ostream& os) {
    os << "digraph IDOM {\n";
    BOOST_FOREACH (DominanceNode* root, m_roots) {
        printTree (os, root);
    }

    os << "}\n";
}

} // namespace SecreC
