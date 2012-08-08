#include "codegen.h"

#include <boost/foreach.hpp>

#include "constant.h"

namespace /* anonymous */ {

using namespace SecreC;

bool isNontrivialResource (TypeNonVoid* tnv) {
    return tnv->secrecDimType () != 0
        || tnv->secrecSecType ()->isPrivate ()
        || tnv->secrecDataType () == DATATYPE_STRING;
}

} // namespace anonymous

namespace SecreC {

/*******************************************************************************
  CodeGen
*******************************************************************************/

CGResult CodeGen::codeGen (TreeNodeExpr* e) {
    TreeNode* const oldNode = m_node;
    m_node = e;
    const CGResult& r (e->codeGenWith (*this));
    m_node = oldNode;
    return r;
}

CGBranchResult CodeGen::codeGenBranch (TreeNodeExpr* e) {
    TreeNode* oldNode = m_node;
    m_node = e;
    const CGBranchResult& r (e->codeGenBoolWith (*this));
    m_node = oldNode;
    return r;
}

CGStmtResult CodeGen::codeGenStmt (TreeNodeStmt* s) {
    TreeNode* oldNode = m_node;
    m_node = s;
    const CGStmtResult& r (s->codeGenWith (*this));
    m_node = oldNode;
    return r;
}

Imop* CodeGen::newComment (const std::string& comment) const {
    ConstantString* str = ConstantString::get (getContext (), comment);
    Imop* c = new Imop (0, Imop::COMMENT, 0, str);
    return c;
}

Imop* CodeGen::pushComment (const std::string& comment) {
    Imop* c = newComment (comment);
    push_imop (c);
    return c;
}

Symbol* CodeGen::getSizeOr (Symbol* sym, int64_t val) {
    assert (sym != 0);
    Symbol* sizeSym = ConstantInt::get (getContext (), val);
    if (sym->symbolType () == Symbol::SYMBOL) {
        assert (dynamic_cast<SymbolSymbol*>(sym) != 0);
        SymbolSymbol* symsym = static_cast<SymbolSymbol*>(sym);
        if (symsym->getSizeSym () != 0) {
            sizeSym = symsym->getSizeSym ();
        }
    }

    return sizeSym;
}

void CodeGen::allocTemporaryResult (CGResult& result, Symbol* val) {
    if (result.symbol ()->secrecType ()->isScalar ()) {
        return;
    }

    assert (dynamic_cast<SymbolSymbol*>(result.symbol ()) != 0);
    SymbolSymbol* sym = static_cast<SymbolSymbol*>(result.symbol ());
    if (val == 0) {
        val = defaultConstant (getContext (), sym->secrecType ()->secrecDataType ());
    }

    Imop* i = new Imop (m_node, Imop::ALLOC, sym, val, sym->getSizeSym ());
    pushImopAfter (result, i);
}

void CodeGen::releaseScopeVariables (CGResult& result) {
    BOOST_FOREACH (Symbol* var, m_st->variables ()) {
        releaseResource (result, var);
    }
}

void CodeGen::releaseProcVariables (CGResult& result, Symbol* ex) {
    BOOST_FOREACH (Symbol* var, m_st->variablesUpTo (m_st->globalScope ())) {
        if (var != ex) {
            releaseResource (result, var);
        }
    }
}

void CodeGen::releaseAllVariables (CGResult& result) {
    BOOST_FOREACH (Symbol* var, m_st->variablesUpTo (0)) {
        releaseResource (result, var);
    }
}

void CodeGen::releaseResource (CGResult& result, Symbol* sym) {
    if (isNontrivialResource (sym->secrecType ())) {
        pushImopAfter (result, new Imop (m_node, Imop::RELEASE, 0, sym));
    }
}

void CodeGen::releaseTemporary (CGResult& result, Symbol* sym) {
    assert (sym != 0);
    if (sym->symbolType () == Symbol::SYMBOL) {
        assert (dynamic_cast<SymbolSymbol*>(sym) != 0);
        SymbolSymbol* ssym = static_cast<SymbolSymbol*>(sym);
        if (ssym->isTemporary ()) {
            releaseResource (result, ssym);
        }
    }
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
            push_imop (i);
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

SymbolSymbol* CodeGen::generateResultSymbol (CGResult& result, SecreC::Type* _ty) {
    if (! _ty->isVoid ()) {
        TypeNonVoid* ty = static_cast<TypeNonVoid*>(_ty);
        TypeNonVoid* intTy = TypeNonVoid::getIndexType (getContext ());
        SymbolSymbol* sym = m_st->appendTemporary (ty);
        result.setResult (sym);

        for (SecrecDimType i = 0; i < ty->secrecDimType(); ++ i) {
            sym->setDim (i, m_st->appendTemporary (intTy));
        }

        if (ty->secrecDimType () != 0) {
            sym->setSizeSym (m_st->appendTemporary (intTy));
        }

        return sym;
    }

    return 0;
}

SymbolSymbol* CodeGen::generateResultSymbol (CGResult& result, TreeNodeExpr* node) {
    assert (node->haveResultType ());
    return generateResultSymbol (result, node->resultType ());
}

CGResult CodeGen::codeGenStride (ArrayStrideInfo& strideInfo) {
    TypeNonVoid* ty = TypeNonVoid::getIndexType (getContext ());
    CGResult result;
    Symbol* tmp = strideInfo.symbol ();
    const unsigned n = tmp->secrecType ()->secrecDimType ();
    if (n == 0) { // scalar doesn't have stride
        return result;
    }

    assert (dynamic_cast<SymbolSymbol*>(tmp) != 0);
    SymbolSymbol* sym = static_cast<SymbolSymbol*>(tmp);
    strideInfo.clear ();
    strideInfo.reserve (n);

    for (unsigned it = 0; it < n; ++ it) {
        strideInfo.push_back (m_st->appendTemporary (ty));
    }

    Imop* i = new Imop (m_node, Imop::ASSIGN, strideInfo.at (n - 1), ConstantInt::get (getContext (), 1));
    pushImopAfter (result, i);

    for (unsigned it = n - 1; it != 0; -- it) {
        Symbol* symDim = sym->getDim (it);
        i = new Imop (m_node, Imop::MUL,
                      strideInfo.at (it - 1), strideInfo.at (it), symDim);
        push_imop (i);
    }

    return result;
}

CGResult CodeGen::enterLoop (LoopInfo& loopInfo, Symbol* tmp) {
    assert (loopInfo.empty ());
    CGResult result;
    assert (dynamic_cast<SymbolSymbol*>(tmp) != 0);
    SymbolSymbol* sym = static_cast<SymbolSymbol*>(tmp);
    ConstantInt* zero = ConstantInt::get (getContext (), 0);
    TypeNonVoid* boolTy = TypeNonVoid::getPublicBoolType (getContext ());
    unsigned count = 0;
    BOOST_FOREACH (Symbol* idx, loopInfo) {
        Imop* i = new Imop (m_node, Imop::ASSIGN, idx, zero);
        push_imop (i);
        result.patchFirstImop (i);

        SymbolTemporary* temp_bool = m_st->appendTemporary (boolTy);
        Imop* test = new Imop (m_node, Imop::GE, temp_bool, idx, sym->getDim (count ++));
        push_imop (test);

        Imop* jump = new Imop (m_node, Imop::JT, 0, temp_bool);
        push_imop (jump);

        loopInfo.pushJump (test, jump);
    }

    return result;
}

CGResult CodeGen::enterLoop (LoopInfo& loopInfo, const SubscriptInfo::SPV& spv) {
    typedef SubscriptInfo::SPV SPV;
    assert (loopInfo.empty ());
    TypeNonVoid* boolTy = TypeNonVoid::getPublicBoolType (getContext ());
    CGResult result;
    LoopInfo::const_iterator idxIt = loopInfo.begin ();
    BOOST_FOREACH (const SPV::value_type& v, spv) {
        Symbol* idx  = *idxIt;
        SymbolTemporary* temp_bool = m_st->appendTemporary (boolTy);

        Imop* i = new Imop (m_node, Imop::ASSIGN, idx, v.first);
        push_imop (i);
        result.patchFirstImop (i);

        Imop* test = new Imop (m_node, Imop::GE, temp_bool, idx, v.second);
        push_imop (test);

        Imop* jump = new Imop (m_node, Imop::JT, 0, temp_bool);
        push_imop (jump);

        loopInfo.pushJump (test, jump);
        ++ idxIt;
    }

    return result;
}

CGResult CodeGen::exitLoop (LoopInfo& loopInfo) {
    CGResult result;
    Imop* prevJump = 0;
    ConstantInt* one = ConstantInt::get (getContext (), 1);
    BOOST_REVERSE_FOREACH (Symbol* idx, loopInfo) {
        Imop* i = new Imop (m_node, Imop::ADD, idx, idx, one);
        push_imop (i);
        result.patchFirstImop (i);
        if (prevJump != 0) {
            prevJump->setJumpDest (m_st->label (i));
        }

        i = new Imop (m_node, Imop::JUMP, (Symbol*) 0);
        push_imop (i);

        i->setJumpDest (m_st->label (loopInfo.top ().test));
        prevJump = loopInfo.top ().jump;
        loopInfo.pop ();
    }

    if (prevJump != 0) {
        result.addToNextList (prevJump);
    }

    return result;
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
            r_hi = m_st->appendTemporary (TypeNonVoid::getIndexType (getContext ()));
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
        SymbolLabel* errLabel = m_st->label (err);

        TypeNonVoid* boolTy = TypeNonVoid::getPublicBoolType (getContext ());
        SymbolTemporary* temp_bool = m_st->appendTemporary (boolTy);

        dim_iterator dit = dim_begin (x);
        for (SPV::iterator it  (m_spv.begin ()); it != m_spv.end (); ++ it, ++ dit) {
            Symbol* s_lo = it->first;
            Symbol* s_hi = it->second;
            Symbol* d = *dit;
            ConstantInt* zero = ConstantInt::get (getContext (), 0);

            Imop* i = 0;

            i = new Imop (m_node, Imop::GT, temp_bool, zero, s_lo);
            pushImopAfter (result, i);

            i = new Imop (m_node, Imop::JT, 0, temp_bool);
            push_imop (i);
            i->setJumpDest (errLabel);

            i = new Imop (m_node, Imop::GT, temp_bool, s_lo, s_hi);
            push_imop (i);

            i = new Imop (m_node, Imop::JT, 0, temp_bool);
            push_imop (i);
            i->setJumpDest (errLabel);

            i = new Imop (m_node, Imop::GT, temp_bool, s_hi, d);
            push_imop (i);

            i = new Imop (m_node, Imop::JT, 0, temp_bool);
            push_imop (i);
            i->setJumpDest (errLabel);
        }

        pushImopAfter (result, jmp);
        result.addToNextList (jmp);
        push_imop(err);
        return result;
    }
}

void CodeGen::startLoop () {
    m_loops.push_back (m_st);
}

void CodeGen::endLoop () {
    m_loops.pop_back ();
}

SymbolTable* CodeGen::loopST () const {
    return m_loops.empty () ? 0 : m_loops.back ();
}


} // namespace SecreC
