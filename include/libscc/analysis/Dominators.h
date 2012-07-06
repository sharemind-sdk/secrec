/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_ANALYSIS_DOMINATORS_H
#define SECREC_ANALYSIS_DOMINATORS_H

#include <iosfwd>
#include <vector>
#include <map>

namespace SecreC {

class Block;
class Dominators;
class Procedure;
class Program;

/*******************************************************************************
  DominanceNode
*******************************************************************************/

class DominanceNode {
    friend class Dominators;
private: /* Types: */
    typedef std::vector<DominanceNode*> ChildrenList;
public: /* Methods: */

    explicit
    DominanceNode (Block* block)
        : m_block (block)
        , m_parent (0)
    { }

    ~DominanceNode ();

    DominanceNode* parent () const { return m_parent; }
    Block* block () const { return m_block; }
    const ChildrenList& children () const { return m_children; }

    void addChild (DominanceNode* node) { m_children.push_back (node); }
    void setParent (DominanceNode* node) { m_parent = node; }

private: /* Fields: */
    Block* const    m_block;     ///< Basic block
    DominanceNode*  m_parent;    ///< The immediate dominator
    ChildrenList    m_children;  ///< Children
}; /* class DominanceNode { */

/*******************************************************************************
  Dominators
*******************************************************************************/

class Dominators {
private: /* Types: */
    typedef std::map<const Block*, DominanceNode*> NodeMap;
public: /* Methods: */
    Dominators () { }
    ~Dominators ();

    void calculate (Program* prog);
    void calculate (Procedure* proc);
    void calculate (Block *root);

    void dumpToDot (std::ostream& os);

private:

    DominanceNode* findNode (Block* block) {
        DominanceNode*& node = m_nodes[block];
        if (node == 0) {
            node = new DominanceNode (block);
        }

        return node;
    }

private: /* Fields: */
    std::vector<DominanceNode*> m_roots;
    NodeMap m_nodes;
}; /*class Dominators { */


} // namespace SecreC

#endif /* SECREC_ANALYSIS_DOMINATORS_H */
