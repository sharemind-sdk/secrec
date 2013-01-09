/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "RegisterAllocator.h"

#include <algorithm>
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/interprocess/containers/flat_set.hpp>

#include <libscc/symbol.h>
#include <libscc/constant.h>
#include <libscc/dataflowanalysis.h>
#include <libscc/analysis/LiveVariables.h>

#include "VMCode.h"
#include "VMSymbolTable.h"
#include "VMValue.h"

using namespace SecreC;

namespace {

using namespace SecreCC;

void getImm (VMSymbolTable& st, const Symbol* sym) {
    assert (sym != 0);
    assert (sym->symbolType () == SYM_CONSTANT);
    VMValue* imm = st.find (sym);
    if (imm == 0) {
        uint64_t value = 0xdeadbeef;
        switch (sym->secrecType ()->secrecDataType ()) {
            case DATATYPE_BOOL:    value = static_cast<const ConstantBool*>(sym)->value ();   break;
            case DATATYPE_INT8:    value = static_cast<const ConstantInt8*>(sym)->value ();   break;
            case DATATYPE_INT16:   value = static_cast<const ConstantInt16*>(sym)->value ();  break;
            case DATATYPE_INT32:   value = static_cast<const ConstantInt32*>(sym)->value ();  break;
            case DATATYPE_INT64:   value = static_cast<const ConstantInt64*>(sym)->value ();  break;
            case DATATYPE_UINT8:   value = static_cast<const ConstantUInt8*>(sym)->value ();  break;
            case DATATYPE_UINT16:  value = static_cast<const ConstantUInt16*>(sym)->value (); break;
            case DATATYPE_UINT32:  value = static_cast<const ConstantUInt32*>(sym)->value (); break;
            case DATATYPE_UINT64:  value = static_cast<const ConstantUInt64*>(sym)->value (); break;
            case DATATYPE_FLOAT32: value = static_cast<const ConstantFloat32*>(sym)->value (); break;
            case DATATYPE_FLOAT64: value = static_cast<const ConstantFloat64*>(sym)->value (); break;
            case DATATYPE_STRING: /* string literals are not managed by the RA */ return;
            default:
                assert (false && "Undefined SecreC data type.");
                break;
        }

        imm = st.getImm (value);
        st.store (sym, imm);
    }

    assert (dynamic_cast<VMImm*>(imm) != 0);
}

template <class T, class U>
inline std::set<T> &operator-=(std::set<T> &dest, const std::set<U> &src) {
    typedef typename std::set<U >::const_iterator Iter;
    for (Iter i = src.begin (), e = src.end (); i != e; ++ i)
        dest.erase (*i);
    return dest;
}

/*******************************************************************************
  IGraphImpl
*******************************************************************************/

class IGraphImpl {
private: /* Types: */

    typedef unsigned Vertex;
    typedef unsigned Color;

    typedef std::map<VMVReg*, Vertex> Index; ///< Maps VRegs to vertices
    typedef std::vector<VMVReg*> Vertices; ///< Maps vertices back to VRegs
    typedef boost::container::flat_set<Vertex> Neighbours; ///< Set of vertices. Represented as flat set to conserve space.
    typedef std::vector<Neighbours> AdjacencyList; ///< Maps vertices to neighbours.
    typedef boost::container::flat_set<Color> ColorSet;
    typedef std::map<Color, ColorSet> ColorGraph;

    struct DegreeCmp {
        DegreeCmp (const IGraphImpl* self) : m_adjacencyList (self->m_adjacencyList) { }
        inline bool operator () (Vertex u, Vertex v) const {
            return m_adjacencyList[u].size () > m_adjacencyList[v].size ();
        }

    private:
        const AdjacencyList& m_adjacencyList;
    };

private: /* Methods: */

    std::vector<Vertex> iota () const {
        std::vector<Vertex> out;
        const Vertices::size_type n = m_vertices.size ();
        out.reserve (n);
        for (Vertex i = 0; i < n; ++ i) {
            out.push_back (i);
        }

        return out;
    }

public: /* Methods: */

    Vertex addNode (VMVReg* node) {
        Index::iterator i = m_index.find (node);
        if (i != m_index.end ()) {
            return i->second;
        }

        Vertex u = m_index.size ();
        m_index.insert (i, std::make_pair (node, u));
        m_vertices.push_back (node);
        m_adjacencyList.push_back (Neighbours ());
        return u;
    }

    void addEdge (VMVReg* a, VMVReg* b) {
        Vertex u = addNode (a);
        Vertex v = addNode (b);
        m_adjacencyList[u].insert (v);
        m_adjacencyList[v].insert (u);
    }

    void reset () {
        m_index.clear ();
        m_vertices.clear ();
        m_adjacencyList.clear ();
    }

    template <typename ColorSetter >
    unsigned colorGreedy (ColorSetter setColor) const {
        std::vector<Vertex> vertices = iota ();
        std::sort (vertices.begin (), vertices.end (), DegreeCmp (this));
        ColorSet usedRegisters;
        ColorGraph neighbours; // colors of neighbours
        unsigned count = 0;
        BOOST_FOREACH (unsigned v, vertices) {
            ColorSet candidates = usedRegisters;
            BOOST_FOREACH (unsigned u, neighbours[v])
                candidates.erase(u);
            const Color color = candidates.empty () ? (count ++) : *candidates.begin ();
            usedRegisters.insert (color);
            setColor (m_vertices[v], color);
            BOOST_FOREACH (Vertex u, m_adjacencyList[v]) {
                neighbours[u].insert (color);
            }
        }

        return count;
    }

private: /* Fields: */

    Index         m_index;
    Vertices      m_vertices;
    AdjacencyList m_adjacencyList;
};


} // anonymous namespace

namespace SecreCC {


/*******************************************************************************
  RegisterAllocator::InferenceGraph
*******************************************************************************/

class RegisterAllocator::InferenceGraph {
private:

    struct SetReg {
        SetReg (VMSymbolTable & st) : st (st) { }
        inline void operator () (VMVReg* reg, unsigned color) {
            reg->setActualReg (st.getReg (color));
        }

    private:
        VMSymbolTable& st;
    };

    struct SetStack {
        SetStack (VMSymbolTable & st) : st (st) { }
        inline void operator () (VMVReg* reg, unsigned color) {
            reg->setActualReg (st.getStack (color));
        }

    private:
        VMSymbolTable& st;
    };

public: /* Methods: */

    void addNode (VMVReg* node) {
        if (node->isGlobal ())
            m_global.addNode (node);
        else
            m_local.addNode (node);
    }


    void addEdge (VMVReg* a, VMVReg* b) {
        // the inference graph is colored by two different types of colors (global, and local registers)
        // thus there is no reason to consider edges between those two components
        if (a->isGlobal () && b->isGlobal ())
            m_global.addEdge (a, b);
        if (!a->isGlobal () && !b->isGlobal ())
            m_local.addEdge (a, b);
    }

    unsigned colorLocal (VMSymbolTable& st) { return m_local.colorGreedy (SetStack (st)); }

    unsigned colorGlobal (VMSymbolTable& st) { return m_global.colorGreedy (SetReg (st)); }

    void resetLocal () { m_local.reset (); }

private: /* Fields: */

    IGraphImpl m_local, m_global;
};


/*******************************************************************************
  RegisterAllocator
*******************************************************************************/

RegisterAllocator::RegisterAllocator ()
    : m_st (0)
    , m_lv (0)
    , m_inferenceGraph (0)
{ }

RegisterAllocator::~RegisterAllocator () {
    delete m_inferenceGraph;
}

void RegisterAllocator::init (VMSymbolTable& st, LVPtr lv) {
    assert (m_st == 0 && m_lv.get () == 0 && m_inferenceGraph == 0);
    m_st = &st;
    m_lv = lv;
    m_inferenceGraph = new InferenceGraph ();
}

VMVReg* RegisterAllocator::temporaryReg () {
    VMVReg* reg = m_st->getVReg (m_isGlobal);
    m_temporaries.push_back (reg);
    m_inferenceGraph->addNode (reg);
    BOOST_FOREACH (VMVReg* other, m_live) {
        m_inferenceGraph->addEdge (reg, other);
    }

    m_live.insert (reg);
    return reg;
}

void RegisterAllocator::enterFunction (VMFunction& function) {
    m_isGlobal = function.isStart ();
}

void RegisterAllocator::exitFunction (VMFunction& function) {
    if (!m_isGlobal) {
        function.setNumLocals (m_inferenceGraph->colorLocal (*m_st));
        m_inferenceGraph->resetLocal ();
    }
}

unsigned RegisterAllocator::globalCount () {
    return m_inferenceGraph->colorGlobal (*m_st);
}

void RegisterAllocator::enterBlock (VMBlock& block) {
    const LiveVariables::Symbols& in = m_lv->ins (*block.secrecBlock ());
    m_live.clear ();
    BOOST_FOREACH (const Symbol* sym, in) {
        VMValue* reg = m_st->find (sym);
        if (reg == 0) {
            reg = m_st->getVReg (sym->isGlobal ());
            m_st->store (sym, reg);
        }

        assert (dynamic_cast<VMVReg*>(reg) != 0);
        m_live.insert (static_cast<VMVReg*>(reg));
    }
}

void RegisterAllocator::exitBlock (VMBlock &) { }

void RegisterAllocator::getReg (const SecreC::Imop& imop) {
    BOOST_FOREACH (VMVReg* temp, m_temporaries) {
        m_live.erase (temp);
    }

    m_temporaries.clear ();

    BOOST_FOREACH (const Symbol* symbol, imop.useRange ()) {
        switch (symbol->symbolType ()) {
        case SYM_SYMBOL:
            assert (m_st->find (symbol) != 0);
            break;
        case SYM_CONSTANT:
            getImm (*m_st, symbol);
        default:
            break;
        }
    }

    BOOST_FOREACH (const Symbol* symbol, imop.defRange ()) {
        defSymbol (symbol);
    }
}

void RegisterAllocator::defSymbol (const Symbol* symbol) {
    VMValue* reg = m_st->find (symbol);
    if (reg == 0) {
        reg = m_st->getVReg (symbol->isGlobal ());
        m_st->store (symbol, reg);
    }

    assert (dynamic_cast<VMVReg*>(reg) != 0);
    VMVReg* vreg = static_cast<VMVReg*>(reg);
    m_inferenceGraph->addNode (vreg);
    BOOST_FOREACH (VMVReg* other, m_live) {
        m_inferenceGraph->addEdge (vreg, other);
    }

    m_live.insert (vreg);
}


} // namespace SecreCC
