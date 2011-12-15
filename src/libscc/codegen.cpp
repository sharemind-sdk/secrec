#include "codegen.h"

#include <boost/foreach.hpp>

#include "constant.h"

namespace SecreC {

/*******************************************************************************
  CodeGen
*******************************************************************************/

Imop* CodeGen::pushComment (const std::string& comment) {
    ConstantString* str = ConstantString::get (getContext (), comment);
    Imop* c = new Imop (0, Imop::COMMENT, 0, str);
    code.push_imop (c);
    return c;
}

void CodeGen::codeGenSize (CGResult& result) {
    assert (result.symbol () != 0);
    SymbolSymbol* resSym = 0;
    if ((resSym = dynamic_cast<SymbolSymbol*>(result.symbol ())) != 0) {
        Symbol* size = resSym->getSizeSym ();
        if (size == 0) return;
        ConstantInt* one = ConstantInt::get (getContext (), 1);
        Imop* i = new Imop (m_node, Imop::ASSIGN, size, one);
        pushImopAfter (result, i);

        for (dim_iterator it = dim_begin (resSym), e = dim_end (resSym); it != e; ++ it) {
            i = new Imop (m_node, Imop::MUL, size, size, *it);
            code.push_imop (i);
        }
    }
}

void CodeGen::copyShapeFrom (CGResult& result, Symbol* tmp) {
    assert (dynamic_cast<SymbolSymbol*>(result.symbol ()) != 0);
    assert (dynamic_cast<SymbolSymbol*>(tmp) != 0);
    SymbolSymbol* resSym = static_cast<SymbolSymbol*>(result.symbol ());
    SymbolSymbol* sym = static_cast<SymbolSymbol*>(tmp);
    dim_iterator dj = dim_begin (resSym);
    Imop* i = 0;

    for (dim_iterator di (dim_begin (sym)); di != dim_end (sym); ++ di, ++ dj) {
        assert (dj != dim_end (resSym));
        i = new Imop (m_node, Imop::ASSIGN, *dj, *di);
        pushImopAfter (result, i);
    }

    if (sym->getSizeSym () != 0) {
        i = new Imop (m_node, Imop::ASSIGN, resSym->getSizeSym (), sym->getSizeSym ());
        pushImopAfter (result, i);
    }
}

void CodeGen::allocResult (CGResult& result, Symbol* val, bool isVariable)  {
    if (result.symbol ()->secrecType ()->isScalar ()) {
        log.warning () << "Allocating result for scala! Ignoring.";
        return;
    }

    SymbolSymbol* sym = dynamic_cast<SymbolSymbol*>(result.symbol ());
    if (val == 0) {
        val = defaultConstant (getContext (), sym->secrecType ()->secrecDataType ());
    }

    Imop* i = new Imop (m_node, Imop::ALLOC, sym, val, sym->getSizeSym ());
    if (! isVariable) {
        result.addTempAlloc (sym);
    }

    pushImopAfter (result, i);
}

SymbolSymbol* CodeGen::generateResultSymbol (CGResult& result, TreeNodeExpr* node) {
    TypeNonVoid* ty = TypeNonVoid::get (getContext (), DATATYPE_INT);
    assert (node->haveResultType ());
    if (!node->resultType ()->isVoid()) {
        assert (dynamic_cast<TypeNonVoid*>(node->resultType()) != 0);
        SymbolSymbol* sym = st->appendTemporary(static_cast<TypeNonVoid*>(node->resultType()));
        result.setResult (sym);
        for (unsigned i = 0; i < node->resultType ()->secrecDimType(); ++ i) {
            sym->setDim (i, st->appendTemporary (ty));
        }

        if (node->resultType ()->secrecDimType () != 0) {
            sym->setSizeSym (st->appendTemporary (ty));
        }

        return sym;
    }

    return 0;
}

CGResult CodeGen::codeGenStride (ArrayStrideInfo& strideInfo) {
    TypeNonVoid* ty = TypeNonVoid::get (getContext (), DATATYPE_INT);
    CGResult result;
    Symbol* tmp = strideInfo.symbol ();
    const unsigned n = tmp->secrecType ()->secrecDimType ();
    if (n == 0) { // scalar doesn't have stride
        log.debug () << "Generating stride of scalar!";
        return result;
    }

    SymbolSymbol* sym = dynamic_cast<SymbolSymbol*>(tmp);
    strideInfo.clear ();
    strideInfo.reserve (n);

    for (unsigned it = 0; it < n; ++ it) {
        strideInfo.push_back (st->appendTemporary (ty));
    }

    Imop* i = new Imop (m_node, Imop::ASSIGN, strideInfo.at (0), ConstantInt::get (getContext (), 1));
    pushImopAfter (result, i);

    for (unsigned it = 1; it < n; ++ it) {
        Symbol* symDim = sym->getDim (it - 1);
        i = new Imop (m_node, Imop::MUL,
            strideInfo.at (it), strideInfo.at (it - 1), symDim);
        code.push_imop(i);
    }

    return result;
}

CGResult CodeGen::enterLoop (LoopInfo& loopInfo, Symbol* tmp) {
    assert (loopInfo.empty ());
    CGResult result;
    assert (dynamic_cast<SymbolSymbol*>(tmp) != 0);
    SymbolSymbol* sym = static_cast<SymbolSymbol*>(tmp);
    unsigned count = 0;
    BOOST_FOREACH (Symbol* idx, loopInfo) {
        Imop* i = new Imop (m_node, Imop::ASSIGN, idx, ConstantInt::get (getContext (),0));
        code.push_imop (i);
        result.patchFirstImop (i);

        i = new Imop (m_node, Imop::JGE, 0, idx, sym->getDim (count ++));
        code.push_imop (i);
        loopInfo.pushJump (i);
    }

    return result;
}

CGResult CodeGen::enterLoop (LoopInfo& loopInfo, const SubscriptInfo::SPV& spv) {
    typedef SubscriptInfo::SPV SPV;
    assert (loopInfo.empty ());
    CGResult result;
    LoopInfo::const_iterator idxIt = loopInfo.begin ();
    BOOST_FOREACH (const SPV::value_type& v, spv) {
        Symbol* idx  = *idxIt;

        Imop* i = new Imop (m_node, Imop::ASSIGN, idx, v.first);
        code.push_imop (i);
        result.patchFirstImop (i);

        i = new Imop (m_node, Imop::JGE, 0, idx, v.second);
        code.push_imop (i);
        loopInfo.pushJump (i);
        ++ idxIt;
    }

    return result;
}

CGResult CodeGen::exitLoop (LoopInfo& loopInfo) {
    CGResult result;
    Imop* prevJump = 0;
    BOOST_REVERSE_FOREACH (Symbol* idx, loopInfo) {
        Imop* i = new Imop (m_node, Imop::ADD, idx, idx, ConstantInt::get (getContext (),1));
        code.push_imop (i);
        result.patchFirstImop (i);
        if (prevJump != 0) {
            prevJump->setJumpDest (st->label (i));
        }

        i = new Imop (m_node, Imop::JUMP, (Symbol*) 0);
        code.push_imop (i);
        i->setJumpDest (st->label (loopInfo.top ()));
        prevJump = loopInfo.top ();
        loopInfo.pop ();
    }

    if (prevJump != 0) {
        result.addToNextList (prevJump);
    }

    return result;
}

void CodeGen::releaseTempAllocs (CGResult& result, Symbol* ex) {
    BOOST_FOREACH (Symbol* s, result.tempAllocs ()) {
        if (s != ex) {
            Imop* i = new Imop (m_node, Imop::RELEASE, 0, s);
            pushImopAfter (result, i);
        }
    }

    result.clearTempAllocs ();
}

void CodeGen::releaseLocalAllocs (CGResult& result, Symbol* ex) {
    BOOST_FOREACH (SymbolSymbol* sym, m_allocs) {
        if (sym == ex) {
            continue;
        }

        if (sym->scopeType () == SymbolSymbol::LOCAL) {
            Imop* imop = new Imop (m_node, Imop::RELEASE, 0, sym);
            pushImopAfter (result, imop);
        }
    }
}

void CodeGen::releaseGlobalAllocs (CGResult& result) {
    BOOST_FOREACH (SymbolSymbol* sym, m_allocs) {
        Imop* i = new Imop (m_node, Imop::RELEASE, 0, sym);
        pushImopAfter (result, i);
    }
}

CGResult CodeGen::codeGenSubscript (SubscriptInfo& subInfo, Symbol* tmp, TreeNode* node) {
    typedef SubscriptInfo::SPV SPV;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    assert (node != 0);
    assert (tmp != 0);
    assert (node->type () == NODE_SUBSCRIPT);
    assert (dynamic_cast<SymbolSymbol*>(tmp) != 0);

    SubscriptInfo::SliceIndices& m_slices = subInfo.m_slices;
    SubscriptInfo::SPV& m_spv = subInfo.m_spv;
    m_slices.clear ();
    m_spv.clear ();

    CGResult result;
    unsigned count = 0;
    SymbolSymbol* x = static_cast<SymbolSymbol*>(tmp);

    // 1. evaluate the indices and manage the syntactic suggar
    BOOST_FOREACH (TreeNode* t, node->children ()) {
        Symbol* r_lo = ConstantInt::get (getContext (), 0);
        Symbol* r_hi = x->getDim (count);

        // lower bound
        TreeNode* t_lo = t->children().at(0);
        if (t_lo->type() != NODE_EXPR_NONE) {
            TreeNodeExpr* e_lo = static_cast<TreeNodeExpr*>(t_lo);
            const CGResult& eResult = codeGen (e_lo);
            append (result, eResult);
            if (result.isNotOk ()) {
                return result;
            }

            r_lo = eResult.symbol ();
        }

        // upper bound
        if (t->type() == NODE_INDEX_SLICE) {
            TreeNode* t_hi = t->children().at(1);
            if (t_hi->type() != NODE_EXPR_NONE) {
                TreeNodeExpr* e_hi = static_cast<TreeNodeExpr*>(t_hi);
                const CGResult& eResult = codeGen (e_hi);
                if (result.isNotOk ()) {
                    return result;
                }

                r_hi = eResult.symbol ();
            }

            m_slices.push_back (count);
        }
        else {
            // if there is no upper bound then make one up
            r_hi = st->appendTemporary (TypeNonVoid::get (getContext (), DATATYPE_INT));
            ConstantInt* one = ConstantInt::get (getContext (), 1);
            Imop* i = new Imop (m_node, Imop::ADD, r_hi, r_lo,one);
            pushImopAfter (result, i);
        }

        m_spv.push_back (std::make_pair (r_lo, r_hi));
        ++ count;
    }

    // 2. check that indices are legal
    {
        std::stringstream ss;
        ss << "Index out of bounds at " << m_node->location () << ".";
        Imop* jmp = new Imop(m_node, Imop::JUMP, (Symbol*) 0);
        Imop* err = newError (m_node, ConstantString::get (getContext (), ss.str ()));
        SymbolLabel* errLabel = st->label (err);

        dim_iterator dit = dim_begin (x);
        for (SPV::iterator it  (m_spv.begin ()); it != m_spv.end (); ++ it, ++ dit) {
            Symbol* s_lo = it->first;
            Symbol* s_hi = it->second;
            Symbol* d = *dit;
            ConstantInt* zero = ConstantInt::get (getContext (), 0);
            Imop* i = new Imop(m_node, Imop::JGT, (Symbol*) 0, zero, s_lo);
            pushImopAfter (result, i);
            i->setJumpDest (errLabel);

            i = new Imop(m_node, Imop::JGT, (Symbol*) 0, s_lo, s_hi);
            code.push_imop(i);
            i->setJumpDest(errLabel);

            i = new Imop(m_node, Imop::JGT, (Symbol*) 0, s_hi, d);
            code.push_imop(i);
            i->setJumpDest(errLabel);
        }

        pushImopAfter (result, jmp);
        result.addToNextList (jmp);
        code.push_imop(err);
        return result;
    }
}

/*******************************************************************************
  ScopedAllocations
*******************************************************************************/

void ScopedAllocations::allocTemporary (Symbol* dest, Symbol* def, Symbol* size) {
    Imop* i = new Imop (m_codeGen.currentNode (), Imop::ALLOC, dest, def, size);
    m_codeGen.pushImopAfter (m_result, i);
    m_allocs.push_back (dest);
}

void ScopedAllocations::freeAllocs () {
    BOOST_FOREACH (Symbol* sym, m_allocs) {
        Imop* i = new Imop (m_codeGen.currentNode (), Imop::RELEASE, 0, sym);
        m_codeGen.pushImopAfter (m_result, i);
    }
}

} // namespace SecreC
