#include "treenode.h"

#include <iostream>

namespace SecreC {

/// Compute size of given result symbol
ICode::Status TreeNodeExpr::computeSize (ICodeList& code, SymbolTable& st) {
    assert (haveResultType() &&
            "ICE: TreeNodeExpr::computeSize called on expression without type.");
    assert (result() != 0 &&
            "ICE: TreeNodeExpr::computeSize called on expression with non-void type but no result.");

    if (resultType().isVoid()) return ICode::OK;
    if (resultType().isScalar()) return ICode::OK;

    assert ((result()->getSizeSym() != 0) &&
            "ICE: TreeNodeExpr::computeSize called on expression without size symbol.");

    Symbol* size = result()->getSizeSym();
    Imop* i = new Imop (this, Imop::ASSIGN, size, st.constantInt(1));
    code.push_imop(i);
    patchFirstImop(i);
    patchNextList(i, st);
    prevPatchNextList(i, st);

    for (Symbol::dim_iterator it (result()->dim_begin()); it != result()->dim_end(); ++ it) {
        assert (*it != 0 &&
                "ICE: TreeNodeExpr::computeSize called on expression with corrupt shape.");
        Imop* i = new Imop (this, Imop::MUL, size, size, *it);
        code.push_imop(i);
    }

    return ICode::OK;
}

/// copy shape and size of result symbol from \a sym
void TreeNodeExpr::copyShapeFrom(Symbol* sym, ICodeList &code, SymbolTable& st) {
    assert (haveResultType());
    assert (sym != 0);

    if (resultType().isScalar()) {
        return;
    }

    Symbol::dim_iterator dj = result()->dim_begin();
    for (Symbol::dim_iterator di (sym->dim_begin()); di != sym->dim_end(); ++ di, ++ dj) {
        Imop* i = new Imop(this, Imop::ASSIGN, *dj, *di);
        code.push_imop(i);
        patchFirstImop(i);
        patchNextList(i, st);
        prevPatchNextList(i, st);
    }

    Symbol* sl = result()->getSizeSym();
    Symbol* sr = sym->getSizeSym();
    Imop* i = new Imop(this, Imop::ASSIGN, sl, sr);
    code.push_imop(i);
    patchFirstImop(i);
    patchNextList(i, st);
    prevPatchNextList(i, st);
}

/// generate appropriately typed result symbol and also initialize dimensionality symbols
void TreeNodeExpr::generateResultSymbol(SymbolTable& st) {
    const TypeNonVoid ty (TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
    assert (haveResultType());
    if (!resultType().isVoid()) {
        assert(dynamic_cast<const TypeNonVoid*>(&resultType()) != 0);
        Symbol* sym = st.appendTemporary(static_cast<const TypeNonVoid&>(resultType()));
        setResult(sym);
        for (unsigned i = 0; i < resultType().secrecDimType(); ++ i) {
            sym->setDim(i, st.appendTemporary(ty));
        }

        sym->setSizeSym(st.appendTemporary(ty));
    }
}

/// Generate code for given subexpression, update first imop, next list, and
/// previously evaluated subexpression which will be set to \a e
ICode::Status TreeNodeBase::generateSubexprCode (TreeNodeExpr* e, ICodeList& code, SymbolTable& st, CompileLog& log, Symbol* r) {
    ICode::Status s = e->generateCode(code, st, log, r);
    if (s != ICode::OK) return s;
    if (e->firstImop() != 0) {
        Imop* i = e->firstImop();
        patchFirstImop(i);
        patchNextList(i, st);
        if (prevSubexpr() != 0)
            prevSubexpr()->patchNextList(i, st);
        setPrevSubexpr(e);
    }

    return ICode::OK;
}

/**
 * Given that \a sym has dimensionalities d[1] to d[n] compute stride:
 * s[0] = 1;
 * s[1] = d[1]
 * ...
 * s[n-1] = s[n-2] * d[n-1]
 */
std::vector<Symbol*> TreeNodeExpr::codegenStride (Symbol* sym,
                                                  ICodeList &code,
                                                  SymbolTable &st) {
    assert (sym != 0);
    std::vector<Symbol*> stride;

    const TypeNonVoid ty (SECTYPE_PUBLIC, DATATYPE_INT, 0);
    const unsigned n = sym->secrecType ().secrecDimType ();

    if (n == 0) { // scalar doesn't have stride
        return stride;
    }

    stride.reserve(n);

    for (unsigned it = 0; it < n; ++ it)
        stride.push_back(st.appendTemporary(ty));

    Imop* i = new Imop(this, Imop::ASSIGN, stride[0], st.constantInt(1));
    code.push_imop(i);
    patchFirstImop (i);
    patchNextList(i, st);
    prevPatchNextList(i, st);

    for (unsigned it = 1; it < n; ++ it) {
        Symbol* symDim = sym->getDim (it - 1);
        i = new Imop(this, Imop::MUL, stride[it], stride[it - 1], symDim);
        code.push_imop(i);
    }

    return stride;
}

/**
 * Generate code for a subscript in context of expression that evaluates to \a x.
 * Context is needed because we have to set the upper bounds of partial slices
 * (such as :) to size of indexed dimension. We also check that all the indices are
 * within bounds.
 */
TreeNodeExpr::SubscriptInfo TreeNodeExpr::codegenSubscript (Symbol* x,
                                                            TreeNode* node,
                                                            ICodeList& code,
                                                            SymbolTable& st,
                                                            CompileLog& log) {
    typedef ChildrenListConstIterator CLCI;
    typedef std::vector<std::pair<Symbol*, Symbol*> > SPV;

    assert (node != 0);
    assert (node->type () == NODE_SUBSCRIPT);

    CLCI it = node->children().begin();
    CLCI it_end = node->children().end();
    std::vector<unsigned> slices;
    SPV spv;

    // 1. evaluate the indices and manage the syntactic suggar
    for (unsigned count = 0; it != it_end; ++ it, ++ count) {
        TreeNode* t = *it;
        Symbol* r_lo = 0;
        Symbol* r_hi = 0;

        // lower bound
        TreeNode* t_lo = t->children().at(0);
        if (t_lo->type() == NODE_EXPR_NONE) {
            r_lo = st.constantInt(0);
        }
        else {
            TreeNodeExpr* e_lo = static_cast<TreeNodeExpr*>(t_lo);
            ICode::Status s = generateSubexprCode(e_lo, code, st, log);
            if (s != ICode::OK) {
                return SubscriptInfo (s);
            }

            r_lo = e_lo->result();
        }

        // upper bound
        if (t->type() == NODE_INDEX_SLICE) {
            TreeNode* t_hi = t->children().at(1);
            if (t_hi->type() == NODE_EXPR_NONE) {
                r_hi = x->getDim(count);
            }
            else {
                TreeNodeExpr* e_hi = static_cast<TreeNodeExpr*>(t_hi);
                ICode::Status s = generateSubexprCode(e_hi, code, st, log);
                if (s != ICode::OK) {
                    return SubscriptInfo (s);
                }

                r_hi = e_hi->result();
            }

            slices.push_back(count);
        }
        else {
            // if there is no upper bound then make one up
            r_hi = st.appendTemporary(TypeNonVoid(SECTYPE_PUBLIC, DATATYPE_INT, 0));
            Imop* i = new Imop(this, Imop::ADD, r_hi, r_lo, st.constantInt(1));
            code.push_imop(i);
            patchFirstImop(i);
            prevPatchNextList(i, st);
        }

        spv.push_back(std::make_pair(r_lo, r_hi));
    }

    // 2. check that indices are legal
    {
        code.push_comment("Validating indices:");

        std::stringstream ss;
        ss << "Index out of bounds at " << location() << ".";
        Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
        Imop* err = newError (this, st.constantString (ss.str ()));
        SymbolLabel* errLabel = st.label(err);

        Symbol::dim_iterator dit = x->dim_begin();
        for (SPV::iterator it (spv.begin()); it != spv.end(); ++ it, ++ dit) {
            Symbol* s_lo = it->first;
            Symbol* s_hi = it->second;
            Symbol* d = *dit;

            Imop* i = new Imop(this, Imop::JGT, (Symbol*) 0, st.constantInt(0), s_lo);
            code.push_imop(i);
            patchFirstImop(i);
            prevPatchNextList(i, st);
            i->setJumpDest(errLabel);

            i = new Imop(this, Imop::JGT, (Symbol*) 0, s_lo, s_hi);
            code.push_imop(i);
            i->setJumpDest(errLabel);

            i = new Imop(this, Imop::JGT, (Symbol*) 0, s_hi, d);
            code.push_imop(i);
            i->setJumpDest(errLabel);
        }

        code.push_imop(jmp);
        patchFirstImop(jmp);
        prevPatchNextList(jmp, st);
        addToNextList(jmp);

        code.push_imop(err);
    }

    SubscriptInfo result (ICode::OK);
    result.slices = slices;
    result.spv = spv;
    return result;
}

}
