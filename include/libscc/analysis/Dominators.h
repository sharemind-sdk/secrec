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

#ifndef SECREC_ANALYSIS_DOMINATORS_H
#define SECREC_ANALYSIS_DOMINATORS_H

#include <iosfwd>
#include <map>
#include <vector>

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
    using ChildrenList = std::vector<DominanceNode*>;
public: /* Methods: */

    explicit
    DominanceNode (Block* block)
        : m_block (block)
        , m_parent (nullptr)
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
    using NodeMap = std::map<const Block*, DominanceNode*>;
public: /* Methods: */
    Dominators () { }
    ~Dominators ();

    void calculate (Program* prog);
    void calculate (Procedure* proc);
    void calculate (Block* root);

    void dumpToDot (std::ostream& os);

private:

    DominanceNode* findNode (Block* block) {
        DominanceNode*& node = m_nodes[block];
        if (node == nullptr) {
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
