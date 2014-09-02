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
    Imop * c = new Imop(0, Imop::COMMENT, 0, str);
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

Symbol * CodeGen::indexConstant(uint64_t value) {
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
    BOOST_FOREACH (Symbol * var, m_st->variablesUpTo(0)) {
        releaseResource(result, var);
    }
}

void CodeGen::releaseResource(CGResult & result, Symbol * sym) {
    if (isNontrivialResource(sym->secrecType())) {
        pushImopAfter(result, new Imop(m_node, Imop::RELEASE, 0, sym));
    }
}

void CodeGen::releaseTemporary(CGResult & result, Symbol * sym) {
    assert(sym != NULL);
    if (sym->symbolType() == SYM_SYMBOL) {
        assert(dynamic_cast<SymbolSymbol *>(sym) != NULL);
        SymbolSymbol * ssym = static_cast<SymbolSymbol *>(sym);
        if (ssym->isTemporary()) {
            releaseResource(result, ssym);
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

/**
 * @brief CodeGen::cgProcParam Expect the given symbol as procedure parameter.
 * @param sym Expected procedure parameter (it's subsymbols are also expected as parameters).
 * @return Code generation result.
 */
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


} // namespace SecreC
