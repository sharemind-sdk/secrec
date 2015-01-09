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

#ifndef SECREC_CFG_H
#define SECREC_CFG_H

#include <map>

namespace SecreC {

/*******************************************************************************
  CFGNode
*******************************************************************************/

template <class NodeT, typename LabelT >
class CFGNode {
public: /* Types: */

    using NodeType = NodeT ;
    using LabelType = LabelT ;
    using NeighbourMap = std::map<NodeType*, LabelType> ;
    using edge_type = typename NeighbourMap::value_type ;
    using neighbour_iterator = typename NeighbourMap::iterator ;
    using neighbour_const_iterator = typename NeighbourMap::const_iterator ;
    using neighbour_range = std::pair<neighbour_iterator, neighbour_iterator> ;
    using neighbour_const_range = std::pair<neighbour_const_iterator, neighbour_const_iterator> ;

public:

    neighbour_iterator pred_begin () { return m_predecessors.begin (); }
    neighbour_iterator pred_end () { return m_predecessors.end (); }
    neighbour_const_iterator pred_begin () const { return m_predecessors.begin (); }
    neighbour_const_iterator pred_end () const { return m_predecessors.end (); }
    neighbour_range pred_range () { return {pred_begin (), pred_end ()}; }
    neighbour_const_range pred_range () const { return {pred_begin (), pred_end ()}; }
    neighbour_iterator succ_begin () { return m_successors.begin (); }
    neighbour_iterator succ_end () { return m_successors.end (); }
    neighbour_const_iterator succ_begin () const { return m_successors.begin (); }
    neighbour_const_iterator succ_end () const { return m_successors.end (); }
    neighbour_range succ_range () { return {succ_begin (), succ_end ()}; }
    neighbour_const_range succ_range () const { return {succ_begin (), succ_end ()}; }

    NeighbourMap& successors () { return m_successors; }
    const NeighbourMap& successors () const { return m_successors; }
    NeighbourMap& predecessors () { return m_predecessors; }
    const NeighbourMap& predecessors () const { return m_predecessors; }

    static void addEdge (NodeType& from, LabelType label, NodeType& to) {
        from.addSucc (to, label);
        to.addPred (from, label);
    }

    void unlink () {
        for (auto & pred : m_predecessors)
            pred.first->removeSucc (self ());

        for (auto & succ : m_successors)
            succ.first->removePred (self ());
    }

protected:


    void addPred (NodeType& node, LabelType label) { addEdgeTo (m_predecessors, node, label); }

    void addSucc (NodeType& node, LabelType label) { addEdgeTo (m_successors,   node, label); }


private:

    NodeType& self () { return *static_cast<NodeType*>(this); }
    const NodeType& self () const { return *static_cast<const NodeType*>(this); }

    void removeSucc (NodeType& succ) { m_successors.erase (&succ); }

    void removePred (NodeType& pred) { m_predecessors.erase (&pred); }

    static void addEdgeTo (NeighbourMap& ns, NodeType& node, LabelType label) {
        LabelType& oldLabel = ns[&node];
        oldLabel = static_cast<LabelType>(oldLabel | label);
    }

private: /* Fields: */
    NeighbourMap m_successors;
    NeighbourMap m_predecessors;
}; // CFGNode

} // namespace SecreC

#endif // SECREC_CFG_H
