#include "codegen.h"

#include "constant.h"
#include "context_impl.h"
#include "intermediate.h"
#include "misc.h"
#include "symboltable.h"
#include "treenode.h"
#include "typechecker.h"

#include <boost/foreach.hpp>

namespace SecreC {

namespace /* anonymous */ {

/**
 * @brief isNontrivialResource Check if the given type is an array, a private variable, or a string.
 * @param tnv The type of the resource.
 * @return Wether a resource of the given type requires memory allocation to store.
 */
bool isNontrivialResource (TypeNonVoid* tnv) {
    return tnv->secrecDimType () != 0
        || tnv->secrecSecType ()->isPrivate ()
        || tnv->secrecDataType ()->isString ();
}

} // namespace anonymous

/*******************************************************************************
  CodeGen
*******************************************************************************/

CodeGen::CodeGen(ICodeList& code, ICode& icode)
    : CodeGenState(code.end(), &icode.symbols())
    , m_code(code)
    , m_log(icode.compileLog())
    , m_modules(icode.modules())
    , m_context(icode.context())
    , m_tyChecker(NULL)
{
    m_tyChecker = new TypeChecker(icode.symbols(), m_log, m_context);
}

CodeGen::~CodeGen() {
    delete m_tyChecker;
}

StringTable& CodeGen::getStringTable () const {
    return m_context.pImpl ()->m_stringTable;
}

void CodeGen::updateTypeChecker() {
    m_tyChecker->setScope(*m_st);
}

void CodeGen::newScope() {
    m_st = m_st->newScope();
    updateTypeChecker();
}

void CodeGen::popScope() {
    m_st = m_st->parent();
    updateTypeChecker();
}

void CodeGen::pushImopAfter(CGResult & result, Imop * imop) {
    assert(imop != NULL);
    result.patchFirstImop(imop);
    if (!result.nextList().empty())
        result.patchNextList(m_st->label(imop));
    push_imop(imop);
}

void CodeGen::append(CGResult & result, const CGResult & other) {
    result.patchFirstImop(other.firstImop());
    // we check for empty next list to avoid creating label
    if (other.firstImop() && !result.nextList().empty()) {
        result.patchNextList(m_st->label(other.firstImop()));
    }

    result.addToNextList(other.nextList());
    result |= other.status();
}

CGResult CodeGen::codeGen(TreeNodeExpr * e) {
    TreeNode * const oldNode = m_node;
    m_node = e;
    const CGResult & r = e->codeGenWith(*this);
    m_node = oldNode;
    return r;
}

CGBranchResult CodeGen::codeGenBranch(TreeNodeExpr * e) {
    TreeNode * oldNode = m_node;
    m_node = e;
    const CGBranchResult & r = e->codeGenBoolWith(*this);
    m_node = oldNode;
    return r;
}

CGStmtResult CodeGen::codeGenStmt(TreeNodeStmt * s) {
    TreeNode * oldNode = m_node;
    m_node = s;
    const CGStmtResult & r = s->codeGenWith(*this);
    m_node = oldNode;
    return r;
}

Imop * CodeGen::newComment(StringRef comment) const {
    ConstantString * str = ConstantString::get(getContext(), comment.str ());
    Imop * c = new Imop(m_node, Imop::COMMENT, NULL, str);
    return c;
}

Imop * CodeGen::pushComment(StringRef comment) {
    Imop * c = newComment(comment);
    push_imop(c);
    return c;
}

Symbol * CodeGen::getSizeOr(Symbol * sym, uint64_t val) {
    assert(sym != NULL);
    Symbol * sizeSym = indexConstant(val);
    if (sym->symbolType() == SYM_SYMBOL) {
        assert(dynamic_cast<SymbolSymbol *>(sym) != NULL);
        SymbolSymbol * symsym = static_cast<SymbolSymbol *>(sym);
        if (symsym->getSizeSym() != NULL) {
            sizeSym = symsym->getSizeSym();
        }
    }

    return sizeSym;
}

SymbolConstant * CodeGen::indexConstant(uint64_t value) {
    return ConstantInt::get(getContext(), DATATYPE_UINT64, value);
}

Symbol* CodeGen::findIdentifier (SymbolCategory type, const TreeNodeIdentifier* id) const {
    return m_st->find (type, id->value ());
}

void CodeGen::allocTemporaryResult(CGResult & result, Symbol * val) {
    if (result.symbol()->secrecType()->isScalar()) {
        return;
    }

    assert(dynamic_cast<SymbolSymbol *>(result.symbol()) != NULL);
    SymbolSymbol * sym = static_cast<SymbolSymbol *>(result.symbol());
    if (val == NULL) {
        val = defaultConstant(getContext(), sym->secrecType()->secrecDataType());
    }

    Imop * i = new Imop(m_node, Imop::ALLOC, sym, val, sym->getSizeSym());
    pushImopAfter(result, i);
}

void CodeGen::initSymbol(CGResult & result, Symbol * sym, Symbol * def) {
    TypeNonVoid * tnv = sym->secrecType();
    if (def == NULL) {
        def = defaultConstant(getContext(), tnv->secrecDataType());
    }

    Imop * i = NULL;
    if (tnv->secrecDimType() > 0)
        i = new Imop(m_node, Imop::ALLOC, sym, def, indexConstant(0));
    else if (tnv->secrecSecType()->isPrivate()) {
        i = new Imop(m_node, Imop::CLASSIFY, sym, def);
    }
    else
        i = new Imop(m_node, Imop::ASSIGN, sym, def);

    pushImopAfter(result, i);
}

void CodeGen::releaseScopeVariables (CGResult& result) {
    BOOST_FOREACH (Symbol * var, m_st->variables()) {
        releaseResource(result, var);
    }
}

void CodeGen::releaseProcVariables(CGResult & result, Symbol * ex) {
    BOOST_FOREACH (Symbol * var, m_st->variablesUpTo(m_st->globalScope())) {
        if (var != ex) {
            releaseResource(result, var);
        }
    }
}

void CodeGen::releaseAllVariables(CGResult & result) {
    BOOST_FOREACH (Symbol * var, m_st->variablesUpTo(NULL)) {
        releaseResource(result, var);
    }
}

void CodeGen::releaseResource(CGResult & result, Symbol * sym) {
    if (isNontrivialResource(sym->secrecType())) {
        pushImopAfter(result, new Imop(m_node, Imop::RELEASE, NULL, sym));
    }
}

void CodeGen::releaseTemporary(CGResult & result, Symbol * sym) {
    assert(sym != NULL);
    if (sym->symbolType() == SYM_SYMBOL) {
        assert(dynamic_cast<SymbolSymbol *>(sym) != NULL);
        SymbolSymbol * ssym = static_cast<SymbolSymbol *>(sym);
        if (ssym->secrecType ()->secrecDataType ()->isComposite ()) {
            BOOST_FOREACH (SymbolSymbol* field, ssym->fields ()) {
                releaseTemporary (result, field);
            }
        }
        else {
            if (ssym->isTemporary()) {
                releaseResource(result, ssym);
            }
        }
    }
}

void CodeGen::codeGenSize(CGResult & result) {
    assert(result.symbol() != NULL);
    SymbolSymbol * resSym = NULL;
    if ((resSym = dynamic_cast<SymbolSymbol *>(result.symbol())) != NULL) {
        Symbol * size = resSym->getSizeSym();
        if (size == 0) return;
        Symbol * one = indexConstant(1);
        Imop * i = new Imop(m_node, Imop::ASSIGN, size, one);
        pushImopAfter(result, i);

        for (dim_iterator it = dim_begin(resSym), e = dim_end(resSym); it != e; ++ it) {
            i = new Imop(m_node, Imop::MUL, size, size, *it);
            push_imop(i);
        }
    }
}

void CodeGen::copyShapeFrom(CGResult & result, Symbol * tmp) {
    assert(dynamic_cast<SymbolSymbol *>(result.symbol()) != NULL);
    assert(dynamic_cast<SymbolSymbol *>(tmp) != NULL);
    SymbolSymbol * resSym = static_cast<SymbolSymbol *>(result.symbol());
    SymbolSymbol * sym = static_cast<SymbolSymbol *>(tmp);
    dim_iterator dj = dim_begin(resSym);
    Imop * i = NULL;

    for (dim_iterator di(dim_begin(sym)); di != dim_end(sym); ++ di, ++ dj) {
        assert(dj != dim_end(resSym));
        i = new Imop(m_node, Imop::ASSIGN, *dj, *di);
        pushImopAfter(result, i);
    }

    if (sym->getSizeSym() != NULL) {
        i = new Imop(m_node, Imop::ASSIGN, resSym->getSizeSym(), sym->getSizeSym());
        pushImopAfter(result, i);
    }
}

SymbolSymbol * CodeGen::generateResultSymbol(CGResult & result, SecreC::Type * _ty) {
    if (! _ty->isVoid()) {
        TypeNonVoid * ty = static_cast<TypeNonVoid *>(_ty);
        TypeBasic * intTy = TypeBasic::getIndexType(getContext());
        SymbolSymbol * sym = m_st->appendTemporary(ty);
        result.setResult(sym);

        for (SecrecDimType i = 0; i < ty->secrecDimType(); ++ i) {
            sym->setDim(i, m_st->appendTemporary(intTy));
        }

        if (ty->secrecDimType() != 0) {
            sym->setSizeSym(m_st->appendTemporary(intTy));
        }

        return sym;
    }

    return 0;
}

SymbolSymbol* CodeGen::generateResultSymbol (CGResult& result, TreeNodeExpr* node) {
    assert (node->haveResultType ());
    return generateResultSymbol (result, node->resultType ());
}

CGResult CodeGen::codeGenStride(ArrayStrideInfo & strideInfo) {
    TypeBasic * ty = TypeBasic::getIndexType(getContext());
    CGResult result;
    Symbol * tmp = strideInfo.symbol();
    const unsigned n = tmp->secrecType()->secrecDimType();
    if (n == 0) { // scalar doesn't have stride
        return result;
    }

    assert(dynamic_cast<SymbolSymbol *>(tmp) != NULL);
    SymbolSymbol * sym = static_cast<SymbolSymbol *>(tmp);
    strideInfo.clear();
    strideInfo.reserve(n);

    for (unsigned it = 0; it < n; ++ it) {
        strideInfo.push_back(m_st->appendTemporary(ty));
    }

    Imop * i = new Imop(m_node, Imop::ASSIGN, strideInfo.at(n - 1), indexConstant(1));
    pushImopAfter(result, i);

    for (unsigned it = n - 1; it != 0; -- it) {
        Symbol * symDim = sym->getDim(it);
        i = new Imop(m_node, Imop::MUL,
                strideInfo.at(it - 1), strideInfo.at(it), symDim);
        push_imop(i);
    }

    return result;
}

CGResult CodeGen::enterLoop(LoopInfo & loopInfo, Symbol * tmp) {
    assert(loopInfo.empty());
    CGResult result;
    assert(dynamic_cast<SymbolSymbol *>(tmp) != NULL);
    SymbolSymbol * sym = static_cast<SymbolSymbol *>(tmp);
    Symbol * zero = indexConstant(0);
    TypeBasic * boolTy = TypeBasic::getPublicBoolType(getContext());
    unsigned count = 0;
    BOOST_FOREACH (Symbol * idx, loopInfo) {
        Imop * i = new Imop(m_node, Imop::ASSIGN, idx, zero);
        push_imop(i);
        result.patchFirstImop(i);

        SymbolTemporary * temp_bool = m_st->appendTemporary(boolTy);
        Imop * test = new Imop(m_node, Imop::GE, temp_bool, idx, sym->getDim(count ++));
        push_imop(test);

        Imop * jump = new Imop(m_node, Imop::JT, 0, temp_bool);
        push_imop(jump);

        loopInfo.pushJump(idx, test, jump);
    }

    return result;
}

CGResult CodeGen::enterLoop(LoopInfo & loopInfo, const SubscriptInfo::SPV & spv) {
    typedef SubscriptInfo::SPV SPV;
    assert(loopInfo.empty());
    TypeBasic * boolTy = TypeBasic::getPublicBoolType(getContext());
    CGResult result;
    LoopInfo::const_iterator idxIt;

    idxIt = loopInfo.begin();
    BOOST_FOREACH (const SPV::value_type & v, spv) {
        if (! v.second) {
            Symbol * idx  = *idxIt;
            Imop * i = new Imop(m_node, Imop::ASSIGN, idx, v.first);
            push_imop(i);
            result.patchFirstImop(i);
            loopInfo.pushNop (idx);
        }

        ++ idxIt;
    }

    idxIt = loopInfo.begin();
    BOOST_FOREACH (const SPV::value_type & v, spv) {
        Symbol * idx  = *idxIt;
        if (v.second) {
            Imop * i = new Imop(m_node, Imop::ASSIGN, idx, v.first);
            push_imop(i);
            result.patchFirstImop(i);

            SymbolTemporary * temp_bool = m_st->appendTemporary(boolTy);
            Imop * test = new Imop(m_node, Imop::GE, temp_bool, idx, v.second);
            push_imop(test);

            Imop * jump = new Imop(m_node, Imop::JT, 0, temp_bool);
            push_imop(jump);

            loopInfo.pushJump(idx, test, jump);
        }

        ++ idxIt;
    }

    return result;
}

CGResult CodeGen::exitLoop(LoopInfo & loopInfo) {
    CGResult result;
    Imop * prevJump = NULL;
    Symbol * one = indexConstant(1);

    while (! loopInfo.empty ()) {
        if (! loopInfo.isTopNop ()) {
            LoopInfo::LoopCheck check = loopInfo.top();

            Imop * i = new Imop(m_node, Imop::ADD, check.index, check.index, one);
            push_imop(i);
            result.patchFirstImop(i);
            if (prevJump != NULL) {
                prevJump->setDest(m_st->label(i));
            }

            i = new Imop(m_node, Imop::JUMP, (Symbol *) 0);
            push_imop(i);

            i->setDest(m_st->label(check.test));
            prevJump = check.jump;
        }

        loopInfo.pop ();
    }

    if (prevJump != NULL) {
        result.addToNextList(prevJump);
    }

    return result;
}

CGResult CodeGen::codeGenSubscript(SubscriptInfo & subInfo, Symbol * tmp, TreeNode * node) {
    typedef SubscriptInfo::SPV SPV;

    assert(node != NULL);
    assert(tmp != NULL);
    assert(node->type() == NODE_SUBSCRIPT);
    assert(dynamic_cast<SymbolSymbol *>(tmp) != NULL);

    SubscriptInfo::SliceIndices & m_slices = subInfo.m_slices;
    SubscriptInfo::SPV & m_spv = subInfo.m_spv;
    m_slices.clear();
    m_spv.clear();

    CGResult result;
    unsigned count = 0;
    SymbolSymbol * x = static_cast<SymbolSymbol *>(tmp);

    // 1. evaluate the indices and manage the syntactic suggar
    BOOST_FOREACH (TreeNode * t, node->children()) {
        Symbol * r_lo = indexConstant(0);
        Symbol * r_hi = x->getDim(count);

        // lower bound
        TreeNode * t_lo = t->children().at(0);
        if (t_lo->type() != NODE_EXPR_NONE) {
            TreeNodeExpr * e_lo = static_cast<TreeNodeExpr *>(t_lo);
            const CGResult & eResult = codeGen(e_lo);
            append(result, eResult);
            if (result.isNotOk()) {
                return result;
            }

            r_lo = eResult.symbol();
        }

        // upper bound
        if (t->type() == NODE_INDEX_SLICE) {
            TreeNode * t_hi = t->children().at(1);
            if (t_hi->type() != NODE_EXPR_NONE) {
                TreeNodeExpr * e_hi = static_cast<TreeNodeExpr *>(t_hi);
                const CGResult & eResult = codeGen(e_hi);
                append(result, eResult);
                if (result.isNotOk()) {
                    return result;
                }

                r_hi = eResult.symbol();
            }

            m_slices.push_back(count);
        }
        else {
            r_hi = NULL;
        }

        m_spv.push_back (std::make_pair (r_lo, r_hi));
        ++ count;
    }

    // 2. check that indices are legal
    {
        std::stringstream ss;
        ss << "Index out of bounds at " << m_node->location() << '.';
        Imop * jmp = new Imop(m_node, Imop::JUMP, (Symbol *) 0);
        Imop * err = newError(m_node, ConstantString::get(getContext(), ss.str()));
        SymbolLabel * errLabel = m_st->label(err);

        TypeBasic * boolTy = TypeBasic::getPublicBoolType(getContext());
        SymbolTemporary * temp_bool = m_st->appendTemporary(boolTy);

        dim_iterator dit = dim_begin(x);
        for (SPV::iterator it = m_spv.begin(), e = m_spv.end ();
             it != e; ++ it, ++ dit)
        {
            Symbol * s_lo = it->first;
            Symbol * s_hi = it->second;
            Symbol * d = *dit;
            Imop * i = NULL;

            if (s_hi == NULL) {
                i = new Imop(m_node, Imop::GE, temp_bool, s_lo, d);
                pushImopAfter(result, i);

                i = new Imop(m_node, Imop::JT, 0, temp_bool);
                push_imop(i);
                i->setDest(errLabel);
            }
            else {
                i = new Imop(m_node, Imop::GT, temp_bool, s_lo, s_hi);
                pushImopAfter(result, i);

                i = new Imop(m_node, Imop::JT, 0, temp_bool);
                push_imop(i);
                i->setDest(errLabel);

                i = new Imop(m_node, Imop::GT, temp_bool, s_hi, d);
                push_imop(i);

                i = new Imop(m_node, Imop::JT, 0, temp_bool);
                push_imop(i);
                i->setDest(errLabel);
            }
        }

        pushImopAfter(result, jmp);
        result.addToNextList(jmp);
        push_imop(err);
        return result;
    }
}

void CodeGen::startLoop() {
    m_loops.push_back(m_st);
}

void CodeGen::endLoop() {
    m_loops.pop_back();
}

SymbolTable * CodeGen::loopST() const {
    return m_loops.empty() ? 0 : m_loops.back();
}

CGResult CodeGen::cgProcParam (SymbolSymbol* sym) {
    assert (sym != NULL && sym->secrecType () != NULL);

    TypeNonVoid* ty = sym->secrecType ();
    CGResult result;

    if (ty->secrecDataType ()->isComposite ()) {
        BOOST_FOREACH (SymbolSymbol* field, sym->fields ()) {
            append (result, cgProcParam (field));
            if (result.isNotOk ())
                return result;
        }

        return result;
    }
    else
    if (ty->isScalar ()) {
        pushImopAfter(result, new Imop(m_node, Imop::PARAM, sym));
    }
    else {
        SymbolSymbol * const tns = m_st->appendTemporary(sym->secrecType());
        pushImopAfter(result, new Imop(m_node, Imop::PARAM, tns));

        for (dim_iterator di = dim_begin(sym), de = dim_end(sym); di != de; ++ di)
            push_imop(new Imop(m_node, Imop::PARAM, *di));

        push_imop(new Imop(m_node, Imop::ASSIGN, sym->getSizeSym(), indexConstant(1)));

        for (dim_iterator di = dim_begin(sym), de = dim_end(sym); di != de; ++ di)
            push_imop(new Imop(m_node, Imop::MUL, sym->getSizeSym(), sym->getSizeSym(), *di));

        push_imop(new Imop(m_node, Imop::COPY, sym, tns, sym->getSizeSym()));
    }

    return result;
}

CGResult CodeGen::cgInitalizeToDefaultValue (SymbolSymbol* sym, bool hasShape) {
    assert (sym != NULL && sym->secrecType () != NULL);

    TypeNonVoid* ty = sym->secrecType ();
    CGResult result;

    if (ty->secrecDataType ()->isComposite ()) {
        BOOST_FOREACH (SymbolSymbol* field, sym->fields ()) {
            append (result, cgInitalizeToDefaultValue (field, false));
            if (result.isNotOk ())
                return result;
        }

        return result;
    }

    // Initialize the value of the shape (and size) if need be.
    if (! ty->isScalar () && ! hasShape) {
        SymbolConstant* defIdx = indexConstant (0);
        pushImopAfter(result, new Imop(m_node, Imop::ASSIGN, sym->getSizeSym(), defIdx));
        for (SecrecDimType it = 0; it < ty->secrecDimType(); ++it)
            push_imop(new Imop(m_node, Imop::ASSIGN, sym->getDim(it), defIdx));

    }

    SymbolConstant* def = defaultConstant (getContext (), ty->secrecDataType ());
    if (ty->isScalar ()) {
        Imop::Type iType = ty->secrecSecType ()->isPrivate () ? Imop::CLASSIFY : Imop::ASSIGN;
        pushImopAfter(result, new Imop(m_node, iType, sym, def));
    }
    else {
        pushImopAfter(result, new Imop(m_node, Imop::ALLOC, sym, def, getSizeOr(sym, 0)));
    }

    return result;
}

CGResult CodeGen::cgInitializeToSymbol (SymbolSymbol* lhs, Symbol* rhs, bool hasShape) {
    assert (lhs != NULL && lhs->secrecType () != NULL);
    assert (rhs != NULL && rhs->secrecType () != NULL);
    assert (m_node != NULL);

    TypeNonVoid* ty = lhs->secrecType ();
    TypeBasic* pubBoolTy = TypeBasic::getPublicBoolType(getContext());
    CGResult result;
    result.setResult (lhs);

    if (ty->secrecDataType ()->isComposite ()) {
        assert (ty == rhs->secrecType ());
        for (size_t i = 0; i < lhs->fields ().size (); ++ i) {
            SymbolSymbol* lhsField = lhs->fields ().at (i);
            SymbolSymbol* rhsField = static_cast<SymbolSymbol*>(rhs)->fields ().at (i);
            append (result, cgInitializeToSymbol (lhsField, rhsField, false));
            if (result.isNotOk ())
                return result;
        }

        return result;
    }

    if (hasShape) { // type[[n>0]] x(i_1,...,i_n) = foo;
        if (ty->secrecDimType() > rhs->secrecType()->secrecDimType()) {
            // fill lhs with constant value
            pushImopAfter(result, new Imop(m_node, Imop::ALLOC, lhs, rhs, lhs->getSizeSym()));
            releaseTemporary(result, rhs);
        }
        else {
            // check that shapes match and assign
            std::stringstream ss;
            ss << "Shape mismatch at " << m_node->location();
            Imop * err = newError(m_node, ConstantString::get(getContext(), ss.str()));
            SymbolLabel * const errLabel = m_st->label(err);
            dim_iterator lhsDimIter = dim_begin(lhs);
            BOOST_FOREACH (Symbol * rhsDim, dim_range(rhs)) {
                SymbolTemporary * const temp_bool = m_st->appendTemporary(pubBoolTy);
                pushImopAfter(result, new Imop(m_node, Imop::NE, temp_bool, rhsDim, *lhsDimIter));
                push_imop(new Imop(m_node, Imop::JT, errLabel, temp_bool));

                ++lhsDimIter;
            }

            Imop * const jmp = new Imop(m_node, Imop::JUMP, NULL);
            Imop * const i = new Imop(m_node, Imop::COPY, lhs, rhs, lhs->getSizeSym());
            jmp->setDest(m_st->label(i));
            pushImopAfter(result, jmp);
            push_imop(err);
            push_imop(i);
            releaseTemporary(result, rhs);
        }
    } else {
        if (ty->secrecDimType() > 0) { // type[[>0]] x = foo;
            if (ty->secrecDimType() > rhs->secrecType()->secrecDimType()) {
                pushImopAfter(result, new Imop(m_node, Imop::ALLOC, lhs, rhs, lhs->getSizeSym()));
                releaseTemporary(result, rhs);
            } else {
                assert(ty->secrecDimType() == rhs->secrecType()->secrecDimType());

                dim_iterator srcIter = dim_begin(rhs);
                BOOST_FOREACH (Symbol * const destSym, dim_range(lhs)) {
                    assert(srcIter != dim_end(rhs));
                    pushImopAfter(result, new Imop(m_node, Imop::ASSIGN, destSym, *srcIter));
                    ++srcIter;
                }

                // The type checker should ensure that the symbol is a symbol (and not a constant):
                assert(rhs->symbolType() == SYM_SYMBOL);
                SymbolSymbol * const s = static_cast<SymbolSymbol *>(rhs);
                pushImopAfter(result, newAssign(m_node,  lhs->getSizeSym(), s->getSizeSym()));
                pushImopAfter(result, new Imop(m_node, Imop::COPY, lhs, s, lhs->getSizeSym()));
                releaseTemporary(result, rhs);
            }
        } else { // scalar_type x = scalar;
            pushImopAfter(result, new Imop(m_node, Imop::ASSIGN, lhs, rhs));
            releaseTemporary(result, rhs);
        }
    }

    return result;
}

} // namespace SecreC
