/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_CFG_H
#define SECREC_CFG_H

#include <boost/foreach.hpp>
#include <map>

namespace SecreC {

/*******************************************************************************
  CFGNode
*******************************************************************************/

template <class NodeT, typename LabelT >
class CFGNode {
public: /* Types: */

    typedef NodeT NodeType;
    typedef LabelT LabelType;
    typedef std::map<NodeType*, LabelType> NeighbourMap;
    typedef typename NeighbourMap::value_type edge_type;
    typedef typename NeighbourMap::iterator neighbour_iterator;
    typedef typename NeighbourMap::const_iterator neighbour_const_iterator;
    typedef std::pair<neighbour_iterator, neighbour_iterator> neighbour_range;
    typedef std::pair<neighbour_const_iterator, neighbour_const_iterator> neighbour_const_range;

public:

    CFGNode () { }

    ~CFGNode () { }

    neighbour_iterator pred_begin () { return m_predecessors.begin (); }
    neighbour_iterator pred_end () { return m_predecessors.end (); }
    neighbour_const_iterator pred_begin () const { return m_predecessors.begin (); }
    neighbour_const_iterator pred_end () const { return m_predecessors.end (); }
    neighbour_range pred_range () { return std::make_pair (pred_begin (), pred_end ()); }
    neighbour_const_range pred_range () const { return std::make_pair (pred_begin (), pred_end ()); }

    neighbour_iterator succ_begin () { return m_successors.begin (); }
    neighbour_iterator succ_end () { return m_successors.end (); }
    neighbour_const_iterator succ_begin () const { return m_successors.begin (); }
    neighbour_const_iterator succ_end () const { return m_successors.end (); }
    neighbour_range succ_range () { return std::make_pair (succ_begin (), succ_end ()); }
    neighbour_const_range succ_range () const { return std::make_pair (succ_begin (), succ_end ()); }

    static void addEdge (NodeType& from, LabelType label, NodeType& to) {
        from.addSucc (to, label);
        to.addPred (from, label);
    }

    void unlink () {
        BOOST_FOREACH (typename NeighbourMap::reference pred, m_predecessors)
            pred.first->removeSucc (self ());

        BOOST_FOREACH (typename NeighbourMap::reference succ, m_successors)
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
