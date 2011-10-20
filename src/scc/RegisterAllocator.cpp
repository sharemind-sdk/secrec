/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "RegisterAllocator.h"
#include "VMCode.h"
#include "VMSymbolTable.h"
#include "VMValue.h"
#include "args.h"

#include <libscc/symbol.h>
#include <libscc/constant.h>
#include <libscc/dataflowanalysis.h>

#include <algorithm>
#include <iostream>

using namespace SecreC;

namespace {

using namespace SecreCC;

bool isGlobalSymbol (const Symbol* symbol) {
    switch (symbol->symbolType ()) {
    case Symbol::SYMBOL:
        return static_cast<const SymbolSymbol*>(symbol)->scopeType () == SymbolSymbol::GLOBAL;
    case Symbol::TEMPORARY:
        return static_cast<const SymbolTemporary*>(symbol)->scopeType () == SymbolTemporary::GLOBAL;
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
        switch (sym->secrecType ().secrecDataType ()) {
            case DATATYPE_BOOL:   value = static_cast<const ConstantBool*>(sym)->value ();   break;
            case DATATYPE_INT8:   value = static_cast<const ConstantInt8*>(sym)->value ();   break;
            case DATATYPE_INT16:  value = static_cast<const ConstantInt16*>(sym)->value ();  break;
            case DATATYPE_INT32:  value = static_cast<const ConstantInt32*>(sym)->value ();  break;
            case DATATYPE_INT64:  value = static_cast<const ConstantInt64*>(sym)->value ();  break;
            case DATATYPE_INT:    value = static_cast<const ConstantInt*>(sym)->value ();    break;
            case DATATYPE_UINT8:  value = static_cast<const ConstantUInt8*>(sym)->value ();  break;
            case DATATYPE_UINT16: value = static_cast<const ConstantUInt16*>(sym)->value (); break;
            case DATATYPE_UINT32: value = static_cast<const ConstantUInt32*>(sym)->value (); break;
            case DATATYPE_UINT64: value = static_cast<const ConstantUInt64*>(sym)->value (); break;
            case DATATYPE_UINT:   value = static_cast<const ConstantUInt*>(sym)->value ();   break;
            case DATATYPE_STRING:
                assert (false && "No support for strings yet!");
            default:
                assert (false);
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
    typedef Type::iterator Iterator;
    typedef Type::const_iterator ConstIterator;
    typedef std::set<VMVReg*> Neighbours;
    typedef Neighbours::iterator NIterator;
    typedef Neighbours::const_iterator NConstIterator;

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
        for (unsigned k = 0; k < vertices.size (); ++ k) {
            VMVReg* v = vertices[k];
            std::set<unsigned> candidates = usedRegisters;
            candidates -= neighbours[v];
            const unsigned color = candidates.empty () ? (count ++) : *candidates.begin ();
            usedRegisters.insert (color);
            colors[v] = color;
            for (NConstIterator i = beginNeighbours (v), e = endNeighbours (v); i != e; ++ i) {
                neighbours[*i].insert (color);
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
            for (NConstIterator j = i->second.begin (), f = i->second.end (); j != f; ++ j) {
                const unsigned to = labels[*j];
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
    NConstIterator beginNeighbours (VMVReg* v) const { return m_nodes.find (v)->second.begin (); }
    NConstIterator endNeighbours (VMVReg* v) const { return m_nodes.find (v)->second.end (); }

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
            count = m_global.colorGreedy (colors);
        else
            count = m_local.colorGreedy (colors);
        assignColors (st, colors, isGlobal);
        return count;
    }

    void assignColors (VMSymbolTable& st, const std::map<VMVReg*, unsigned>& colors, bool isGlobal) {
        typedef std::map<VMVReg*, unsigned>::const_iterator VRUMCI;
        for (VRUMCI i = colors.begin (), e = colors.end (); i != e; ++ i) {
            VMVReg* reg = i->first;
            const unsigned color = i->second;
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
    , m_inferenceGraph (new InferenceGraph ())
{ }

RegisterAllocator::~RegisterAllocator () {
    delete m_inferenceGraph;
}

VMVReg* RegisterAllocator::temporaryReg () {
    VMVReg* reg = m_st->getVReg (m_isGlobal);
    m_temporaries.push (reg);
    m_inferenceGraph->addNode (reg);
    for (std::set<VMVReg*>::const_iterator i = m_live.begin (), e = m_live.end (); i != e; ++ i) {
        m_inferenceGraph->addEdge (reg, *i);
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
    const LiveVariables::Symbols& in (m_lv->ins (block.secrecBlock ()));
    m_live.clear ();
    for (Symbols::iterator i = in.begin (), e = in.end (); i != e; ++ i) {
        VMValue* reg = m_st->find (*i);
        if (reg == 0) {
            reg = m_st->getVReg (isGlobalSymbol (*i));
            m_st->store (*i, reg);
        }

        assert (dynamic_cast<VMVReg*>(reg) != 0);
        m_live.insert (static_cast<VMVReg*>(reg));
    }
}

void RegisterAllocator::exitBlock (VMBlock &block) {
    (void) block;
}

void RegisterAllocator::getReg (const SecreC::Imop& imop) {
    typedef std::vector<const Symbol*> SV;

    SV use, def;
    imop.getUse (use);
    imop.getDef (def);

    while (! m_temporaries.empty ()) {
        m_live.erase (m_temporaries.top ());
        m_temporaries.pop ();
    }

    for (SV::const_iterator i = use.begin (), e = use.end (); i != e; ++ i) {
        const Symbol* symbol = *i;
        switch (symbol->symbolType ()) {
        case Symbol::TEMPORARY:
        case Symbol::SYMBOL:
            assert (m_st->find (symbol) != 0);
            break;
        case Symbol::CONSTANT:
            getImm (*m_st, symbol);
        default:
            break;
        }
    }

    for (SV::const_iterator i = def.begin (), e = def.end (); i != e; ++ i) {
        defSymbol (*i);
    }
}

void RegisterAllocator::defSymbol (const Symbol* symbol) {
    VMValue* reg = m_st->find (symbol);
    if (reg == 0) {
        const bool isGlobal = isGlobalSymbol (symbol);
        reg = m_st->getVReg (isGlobal);
        m_st->store (symbol, reg);
    }

    assert (dynamic_cast<VMVReg*>(reg) != 0);
    VMVReg* vreg = static_cast<VMVReg*>(reg);
    m_inferenceGraph->addNode (vreg);
    for (std::set<VMVReg*>::const_iterator i = m_live.begin (), e = m_live.end (); i != e; ++ i) {
        m_inferenceGraph->addEdge (vreg, *i);
    }

    m_live.insert (vreg);
}


} // namespace SecreCC
