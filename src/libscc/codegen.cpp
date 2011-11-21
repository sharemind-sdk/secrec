#include "codegen.h"

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
    SymbolSymbol* resSym = dynamic_cast<SymbolSymbol*>(result.symbol ());
    SymbolSymbol* sym = dynamic_cast<SymbolSymbol*>(tmp);
    assert (sym != 0 && resSym != 0);
    dim_iterator dj = dim_begin (resSym);
    Imop* i = 0;

    for (dim_iterator di (dim_begin (sym)); di != dim_end (sym); ++ di, ++ dj) {
        assert (dj != dim_end (resSym));
        i = new Imop (m_node, Imop::ASSIGN, *dj, *di);
        pushImopAfter (result, i);
    }

    i = new Imop (m_node, Imop::ASSIGN, resSym->getSizeSym (), sym->getSizeSym ());
    pushImopAfter (result, i);
}

void CodeGen::allocResult (CGResult& result)  {
    if (result.symbol ()->secrecType ()->isScalar ()) {
        log.warning () << "Allocating result for scala! Ignoring that.";
        return;
    }

    SymbolSymbol* sym = dynamic_cast<SymbolSymbol*>(result.symbol ());
    Imop* i = new Imop (m_node, Imop::ALLOC, sym,
                        defaultConstant (getContext (), sym->secrecType ()->secrecDataType ()),
                        sym->getSizeSym ());
    pushImopAfter (result, i);
}

SymbolSymbol* CodeGen::generateResultSymbol (CGResult& result, TreeNodeExpr* node) {
    TypeNonVoid* ty = TypeNonVoid::create (m_tyChecker.getContext (), DATATYPE_INT);
    assert (node->haveResultType ());
    if (!node->resultType ()->isVoid()) {
        assert (dynamic_cast<TypeNonVoid*>(node->resultType()) != 0);
        SymbolSymbol* sym = st->appendTemporary(static_cast<TypeNonVoid*>(node->resultType()));
        result.setResult (sym);
        for (unsigned i = 0; i < node->resultType ()->secrecDimType(); ++ i) {
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

CGResult CodeGenStride::codeGenStride (Symbol* tmp) {
    TypeNonVoid* ty = TypeNonVoid::create (m_tyChecker.getContext (), DATATYPE_INT);
    CGResult result;
    const unsigned n = tmp->secrecType ()->secrecDimType ();
    if (n == 0) { // scalar doesn't have stride
        log.debug () << "Generating stride of scalar!";
        return result;
    }

    SymbolSymbol* sym = dynamic_cast<SymbolSymbol*>(tmp);
    m_stride.clear ();
    m_stride.reserve (n);

    for (unsigned it = 0; it < n; ++ it) {
        m_stride.push_back (st->appendTemporary (ty));
    }

    Imop* i = new Imop (m_node, Imop::ASSIGN, m_stride[0], ConstantInt::get (getContext (), 1));
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

CGResult CodeGenLoop::enterLoop (Symbol* tmp, const IndexList& indices) {
    assert (m_jumpStack.empty ());

    CGResult result;
    SymbolSymbol* sym = dynamic_cast<SymbolSymbol*>(tmp);
    IndexList::const_iterator idxIt = indices.begin ();
    assert (sym != 0);
    for (unsigned count = 0; idxIt != indices.end (); ++ idxIt, ++ count) {
        Symbol* idx  = *idxIt;

        Imop* i = new Imop (m_node, Imop::ASSIGN, idx, ConstantInt::get (getContext (),0));
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
        Imop* i = new Imop (m_node, Imop::ADD, idx, idx, ConstantInt::get (getContext (),1));
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

CGResult CodeGenSubscript::codeGenSubscript (Symbol* tmp, TreeNode* node) {
    typedef TreeNode::ChildrenListConstIterator CLCI;

    assert (node != 0);
    assert (tmp != 0);
    assert (node->type () == NODE_SUBSCRIPT);

    m_slices.clear ();
    m_spv.clear ();

    CGResult result;
    CLCI it    = node->children ().begin ();
    CLCI itEnd = node->children ().end ();
    SymbolSymbol* x = dynamic_cast<SymbolSymbol*>(tmp);
    assert (x != 0);

    // 1. evaluate the indices and manage the syntactic suggar
    for (unsigned count = 0; it != itEnd; ++ it, ++ count) {
        TreeNode* t = *it;
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
            r_hi = st->appendTemporary (TypeNonVoid::create (m_tyChecker.getContext (), DATATYPE_INT));
            ConstantInt* one = ConstantInt::get (m_tyChecker.getContext (), 1);
            Imop* i = new Imop (m_node, Imop::ADD, r_hi, r_lo,one);
            pushImopAfter (result, i);
        }

        m_spv.push_back (std::make_pair (r_lo, r_hi));
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
            ConstantInt* zero = ConstantInt::get (m_tyChecker.getContext (), 0);
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

} // namespace SecreC
