#include "codegen.h"

namespace SecreC {

/*******************************************************************************
  CodeGen
*******************************************************************************/

void CodeGen::codeGenSize (CGResult& result) {
    assert (result.symbol () != 0);
    Symbol* resSym = result.symbol ();
    Symbol* size = resSym->getSizeSym ();
    assert (size != 0);

    Imop* i = new Imop (m_node, Imop::ASSIGN, size, st->constantInt(1));
    pushImopAfter (result, i);

    for (Symbol::dim_iterator it (resSym->dim_begin ()); it != resSym->dim_end (); ++ it) {
        i = new Imop (m_node, Imop::MUL, size, size, *it);
        code.push_imop (i);
    }
}

void CodeGen::copyShapeFrom (CGResult& result, Symbol* sym) {
    Symbol* resSym = result.symbol ();
    assert (sym != 0 && resSym != 0);
    Symbol::dim_iterator dj = resSym->dim_begin();
    Imop* i = 0;

    for (Symbol::dim_iterator di (sym->dim_begin()); di != sym->dim_end(); ++ di, ++ dj) {
        assert (dj != resSym->dim_end ());
        i = new Imop (m_node, Imop::ASSIGN, *dj, *di);
        pushImopAfter (result, i);
    }

    i = new Imop (m_node, Imop::ASSIGN, resSym->getSizeSym (), sym->getSizeSym ());
    pushImopAfter (result, i);
}

void CodeGen::allocResult (CGResult& result)  {
    Symbol* sym = result.symbol ();
    if (sym->secrecType ().isScalar ())
        return;

    Imop* i = new Imop (m_node, Imop::ALLOC, sym,
                        st->defaultConstant (sym->secrecType ().secrecDataType ()),
                        sym->getSizeSym ());
    pushImopAfter (result, i);
}

Symbol* CodeGen::generateResultSymbol (CGResult& result, TreeNodeExpr* node) {
    const TypeNonVoid ty (TypeNonVoid (SECTYPE_PUBLIC, DATATYPE_INT, 0));
    assert (node->haveResultType ());
    if (!node->resultType().isVoid()) {
        assert (dynamic_cast<const TypeNonVoid*>(&node->resultType()) != 0);
        Symbol* sym = st->appendTemporary(static_cast<const TypeNonVoid&>(node->resultType()));
        result.setResult (sym);
        for (unsigned i = 0; i < node->resultType().secrecDimType(); ++ i) {
            sym->setDim (i, st->appendTemporary (ty));
        }

        sym->setSizeSym (st->appendTemporary (ty));
        return sym;
    }

    return 0;
}

/*******************************************************************************
  CodeGenStride
*******************************************************************************/

CGResult CodeGenStride::codeGenStride (Symbol* sym) {
    const TypeNonVoid ty (SECTYPE_PUBLIC, DATATYPE_INT, 0);
    CGResult result;
    const unsigned n = sym->secrecType ().secrecDimType ();

    if (n == 0) { // scalar doesn't have stride
        log.debug () << "Generating stride of scalar!";
        return result;
    }

    m_stride.clear ();
    m_stride.reserve (n);

    for (unsigned it = 0; it < n; ++ it) {
        m_stride.push_back (st->appendTemporary (ty));
    }

    Imop* i = new Imop (m_node, Imop::ASSIGN, m_stride[0], st->constantInt(1));
    pushImopAfter (result, i);

    for (unsigned it = 1; it < n; ++ it) {
        Symbol* symDim = sym->getDim (it - 1);
        i = new Imop (m_node, Imop::MUL, m_stride[it], m_stride[it - 1], symDim);
        code.push_imop(i);
    }

    return result;
}

/*******************************************************************************
  CodeGenLoop
*******************************************************************************/

CGResult CodeGenLoop::enterLoop (Symbol* sym, const IndexList& indices) {
    assert (m_jumpStack.empty ());

    CGResult result;
    IndexList::const_iterator idxIt = indices.begin ();
    for (unsigned count = 0; idxIt != indices.end (); ++ idxIt, ++ count) {
        Symbol* idx  = *idxIt;

        Imop* i = new Imop (m_node, Imop::ASSIGN, idx, st->constantInt (0));
        code.push_imop (i);
        result.patchFirstImop (i);

        i = new Imop (m_node, Imop::JGE, 0, idx, sym->getDim (count));
        code.push_imop (i);
        m_jumpStack.push (i);
    }

    return result;
}

CGResult CodeGenLoop::enterLoop (const SPV& spv, const IndexList& indices) {
    assert (m_jumpStack.empty ());

    CGResult result;
    IndexList::const_iterator idxIt = indices.begin ();
    for (SPV::const_iterator it (spv.begin ()); it != spv.end (); ++ it, ++ idxIt) {
        Symbol* i_lo = it->first;
        Symbol* i_hi = it->second;
        Symbol* idx  = *idxIt;

        Imop* i = new Imop (m_node, Imop::ASSIGN, idx, i_lo);
        code.push_imop (i);
        result.patchFirstImop (i);

        i = new Imop (m_node, Imop::JGE, 0, idx, i_hi);
        code.push_imop (i);
        m_jumpStack.push (i);
    }

    return result;
}

CGResult CodeGenLoop::exitLoop (const IndexList& indices) {
    CGResult result;
    Imop* prevJump = 0;
    for (IndexList::const_reverse_iterator it (indices.rbegin ()); it != indices.rend (); ++ it) {
        Symbol* idx = *it;
        Imop* i = new Imop (m_node, Imop::ADD, idx, idx, st->constantInt (1));
        code.push_imop (i);
        result.patchFirstImop (i);
        if (prevJump != 0) {
            prevJump->setJumpDest (st->label (i));
        }

        i = new Imop (m_node, Imop::JUMP, (Symbol*) 0);
        code.push_imop (i);
        i->setJumpDest (st->label (m_jumpStack.top ()));
        prevJump = m_jumpStack.top ();
        m_jumpStack.pop ();
    }

    if (prevJump != 0) {
        result.addToNextList (prevJump);
    }

    return result;
}

/*******************************************************************************
  CodeGenSubscript
*******************************************************************************/

CGResult CodeGenSubscript::codeGenSubscript (Symbol* x, TreeNode* node) {
    typedef TreeNode::ChildrenListConstIterator CLCI;

    assert (node != 0);
    assert (x != 0);
    assert (node->type () == NODE_SUBSCRIPT);

    m_slices.clear ();
    m_spv.clear ();

    CGResult result;
    CLCI it    = node->children ().begin ();
    CLCI itEnd = node->children ().end ();

    // 1. evaluate the indices and manage the syntactic suggar
    for (unsigned count = 0; it != itEnd; ++ it, ++ count) {
        TreeNode* t = *it;
        Symbol* r_lo = st->constantInt (0);
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
            r_hi = st->appendTemporary (TypeNonVoid (SECTYPE_PUBLIC, DATATYPE_INT, 0));
            Imop* i = new Imop (m_node, Imop::ADD, r_hi, r_lo, st->constantInt (1));
            pushImopAfter (result, i);
        }

        m_spv.push_back (std::make_pair (r_lo, r_hi));
    }

    // 2. check that indices are legal
    {
        std::stringstream ss;
        ss << "Index out of bounds at " << m_node->location () << ".";
        Imop* jmp = new Imop(m_node, Imop::JUMP, (Symbol*) 0);
        Imop* err = newError (m_node, st->constantString (ss.str ()));
        SymbolLabel* errLabel = st->label (err);

        Symbol::dim_iterator dit = x->dim_begin ();
        for (SPV::iterator it  (m_spv.begin ()); it != m_spv.end (); ++ it, ++ dit) {
            Symbol* s_lo = it->first;
            Symbol* s_hi = it->second;
            Symbol* d = *dit;

            Imop* i = new Imop(m_node, Imop::JGT, (Symbol*) 0, st->constantInt(0), s_lo);
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

} // namespace SecreC
