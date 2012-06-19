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

bool isGlobalSymbol (const Symbol* symbol) {
    assert (symbol != 0);
    switch (symbol->symbolType ()) {
    case Symbol::SYMBOL:
        assert (dynamic_cast<const SymbolSymbol*>(symbol) != 0);
        return static_cast<const SymbolSymbol*>(symbol)->scopeType () == SymbolSymbol::GLOBAL;
    default:
        assert (false && "Allocating non-variable!");
        return true;
    }
}

void getImm (VMSymbolTable& st, const Symbol* sym) {
    assert (sym != 0);
    assert (sym->symbolType () == Symbol::CONSTANT);
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

// \todo consider register-register assignments as different kinds of edges
class IGraphImpl {

public: /* Types: */

    typedef std::map<VMVReg*, std::set<VMVReg*> > Type;
    typedef Type::value_type value_type;
    typedef Type::iterator Iterator;
    typedef Type::const_iterator ConstIterator;
    typedef std::set<VMVReg*> Neighbours;
    typedef Neighbours::iterator NIterator;
    typedef Neighbours::const_iterator NConstIterator;
    typedef std::pair<NIterator, NIterator> NRange;
    typedef std::pair<NConstIterator, NConstIterator> NConstRange;

private:

    struct DegreeCmp {
        DegreeCmp (const Type& nodes) : m_nodes (nodes) { }
        bool operator () (VMVReg* a, VMVReg* b) const {
            return  m_nodes.find (a)->second.size () >
                    m_nodes.find (b)->second.size ();
        }

    private:
        const Type& m_nodes;
    };

public: /* Methods: */

    IGraphImpl () { }

    ~IGraphImpl () { }

    void addEdge (VMVReg* a, VMVReg* b) {
        assert (a != 0 && b != 0);
        m_nodes[a].insert (b);
        m_nodes[b].insert (a);
    }

    void addNode (VMVReg* node) {
        assert (node != 0);
        Iterator i = m_nodes.find (node);
        if (i == m_nodes.end ()) {
            m_nodes.insert (i, std::make_pair (node, std::set<VMVReg*>()));
        }
    }

    void reset () {
        m_nodes.clear ();
    }

    void order (std::vector<VMVReg*>& vertices) const {
        vertices.clear ();
        vertices.reserve (m_nodes.size ());
        for (ConstIterator i = begin (), e = end (); i != e; ++ i) {
            vertices.push_back (i->first);
        }

        std::sort (vertices.begin (), vertices.end (), DegreeCmp (m_nodes));
    }

    // also acts as trivial coloring
    unsigned label (std::map<VMVReg*, unsigned>& labels) const {
        unsigned count = 0;
        labels.clear ();
        for (ConstIterator i = begin (), e = end (); i != e; ++ i)
            labels[i->first] = count ++;
        return count;
    }

    unsigned colorGreedy (std::map<VMVReg*, unsigned>& colors) const {
        std::vector<VMVReg*> vertices;
        std::set<unsigned> usedRegisters;
        std::map<VMVReg*, std::set<unsigned > > neighbours; // colors of neighbours
        unsigned count = 0;
        order (vertices);
        colors.clear ();
        BOOST_FOREACH (VMVReg* v, vertices) {
            std::set<unsigned> candidates = usedRegisters;
            candidates -= neighbours[v];
            const unsigned color = candidates.empty () ? (count ++) : *candidates.begin ();
            usedRegisters.insert (color);
            colors[v] = color;
            BOOST_FOREACH (VMVReg* u, neighbourRange (v)) {
                neighbours[u].insert (color);
            }
        }

        return count;
    }

    void dump () const {
        std::cerr << "IGraphImpl:\n";
        std::map<VMVReg*, unsigned > labels;
        label (labels);
        for (ConstIterator i = begin (), e = end (); i != e; ++ i) {
            const unsigned from = labels[i->first];
            BOOST_FOREACH (VMVReg* u, i->second) {
                const unsigned to = labels[u];
                if (from <= to)
                    std::cerr << from << " - " << to << std::endl;
            }
        }
    }

protected:

    Iterator begin () { return m_nodes.begin (); }
    ConstIterator begin () const { return m_nodes.begin (); }
    Iterator end () { return m_nodes.end (); }
    ConstIterator end () const { return m_nodes.end (); }

    NConstIterator beginNeighbours (VMVReg* v) const {
        return m_nodes.find (v)->second.begin ();
    }

    NConstIterator endNeighbours (VMVReg* v) const {
        return m_nodes.find (v)->second.end ();
    }

    NConstRange neighbourRange (VMVReg* v) const {
        return std::make_pair (beginNeighbours (v), endNeighbours (v));
    }

private: /* Fields: */

    Type     m_nodes;
};

} // anonymous namespace

namespace SecreCC {


/*******************************************************************************
  RegisterAllocator::InferenceGraph
*******************************************************************************/

class RegisterAllocator::InferenceGraph {

public: /* Methods: */

    InferenceGraph () { }

    ~InferenceGraph () { }

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

    unsigned colorLocal (VMSymbolTable& st) { return color (st, false); }

    unsigned colorGlobal (VMSymbolTable& st) { return color (st, true); }

    void resetLocal () { m_local.reset (); }

    void dump () const {
        m_local.dump ();
        m_global.dump ();
    }

protected:

    unsigned color (VMSymbolTable& st, bool isGlobal) {
        std::map<VMVReg*, unsigned> colors;
        unsigned count = 0;
        if (isGlobal)
            count = m_global.label (colors);
        else
            count = m_local.label (colors);
        assignColors (st, colors, isGlobal);
        return count;
    }

    void assignColors (VMSymbolTable& st, const std::map<VMVReg*, unsigned>& colors, bool isGlobal) {
        typedef std::map<VMVReg*, unsigned>::value_type RC_pair;
        BOOST_FOREACH (const RC_pair& rc, colors) {
            VMVReg* reg = rc.first;
            const unsigned color = rc.second;
            reg->setActualReg (isGlobal ? (VMValue*) st.getReg (color) : (VMValue*) st.getStack (color));
        }
    }

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

void RegisterAllocator::exitBlock (VMBlock &block) {
    (void) block;
}

void RegisterAllocator::getReg (const SecreC::Imop& imop) {
    BOOST_FOREACH (VMVReg* temp, m_temporaries) {
        m_live.erase (temp);
    }

    m_temporaries.clear ();

    BOOST_FOREACH (const Symbol* symbol, imop.useRange ()) {
        switch (symbol->symbolType ()) {
        case Symbol::SYMBOL:
            assert (m_st->find (symbol) != 0);
            break;
        case Symbol::CONSTANT:
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
