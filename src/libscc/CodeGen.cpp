/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "CodeGen.h"

#include "CodeGenResult.h"
#include "Constant.h"
#include "ContextImpl.h"
#include "DataType.h"
#include "Intermediate.h"
#include "Misc.h"
#include "SecurityType.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Types.h"


namespace SecreC {

namespace /* anonymous */ {

/**
 * @brief isNontrivialResource Check if the given type is an array, a private variable, or a string.
 * @param tnv The type of the resource.
 * @return Wether a resource of the given type requires memory allocation to store.
 */
bool isNontrivialResource (const TypeNonVoid* tnv) {
    return tnv->secrecDimType () != 0
        || tnv->secrecSecType ()->isPrivate ()
        || tnv->secrecDataType ()->isString ();
}

// TODO: this is not quite correct place for this function.
SymbolSymbol* generateSymbol (Context& cxt, SymbolTable* st, const TypeNonVoid* ty) {
    assert (st != nullptr && ty != nullptr);

    SymbolSymbol * sym = st->appendTemporary(ty);

    if (ty->secrecDimType() != 0) {
        const TypeBasic * intTy = TypeBasic::getIndexType();
        for (SecrecDimType i = 0; i < ty->secrecDimType(); ++ i)
            sym->setDim(i, st->appendTemporary(intTy));
        sym->setSizeSym(st->appendTemporary(intTy));
    }

    if (ty->secrecDataType ()->isComposite ()) {
        const auto structType = static_cast<const DataTypeStruct*>(ty->secrecDataType ());
        for (const auto& field : structType->fields ()) {
            sym->appendField (generateSymbol (cxt, st, field.type));
        }
    }

    return sym;
}

SymbolSymbol* generateSymbol (Context& cxt, SymbolTable* st, const Type* ty) {
    if (ty->isVoid())
        return nullptr;
    else
        return generateSymbol(cxt, st, static_cast<const TypeNonVoid*>(ty));
}

void collectTemporariesLoop (std::vector<SymbolSymbol*>& acc, SymbolSymbol* sym) {
    assert (sym != nullptr);
    if (sym->secrecType ()->secrecDataType ()->isComposite ()) {
        for (SymbolSymbol* field : sym->fields ()) {
            collectTemporariesLoop (acc, field);
        }
    }
    else {
        // TODO: enormous hack!
        if (sym->isTemporary()) {
            if (sym->parent ()) {
                SymbolSymbol* root = sym;
                while (root->parent () != nullptr)
                    root = root->parent ();

                if (root->isTemporary ())
                    acc.push_back (sym);
            }
            else {
                acc.push_back (sym);
            }
        }
    }
}

std::vector<SymbolSymbol*> collectTemporaries (Symbol* sym) {
    assert (sym != nullptr);
    std::vector<SymbolSymbol*> temporaries;
    if (sym->symbolType () == SYM_SYMBOL) {
        assert(dynamic_cast<SymbolSymbol *>(sym) != nullptr);
        collectTemporariesLoop (temporaries, static_cast<SymbolSymbol *>(sym));
    }

    return temporaries;
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
    , m_tyChecker(nullptr)
{
    m_tyChecker = new TypeChecker(icode.operators(), icode.symbols(), m_log, m_context);
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
    assert(imop != nullptr);
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
    auto c = new Imop(m_node, Imop::COMMENT, nullptr, str);
    return c;
}

Imop * CodeGen::pushComment(StringRef comment) {
    Imop * c = newComment(comment);
    push_imop(c);
    return c;
}

Symbol * CodeGen::getSizeOr(Symbol * sym, uint64_t val) {
    assert(sym != nullptr);
    Symbol * sizeSym = indexConstant(val);
    if (sym->symbolType() == SYM_SYMBOL) {
        assert(dynamic_cast<SymbolSymbol *>(sym) != nullptr);
        SymbolSymbol * symsym = static_cast<SymbolSymbol *>(sym);
        if (symsym->getSizeSym() != nullptr) {
            sizeSym = symsym->getSizeSym();
        }
    }

    return sizeSym;
}

SymbolConstant * CodeGen::indexConstant(uint64_t value) {
    return ConstantInt::get(DATATYPE_UINT64, value);
}

Symbol* CodeGen::findIdentifier (SymbolCategory type, const TreeNodeIdentifier* id) const {
    return m_st->find (type, id->value ());
}

void CodeGen::allocTemporaryResult(CGResult & result, Symbol * val) {
    assert(dynamic_cast<SymbolSymbol *>(result.symbol()) != nullptr);
    SymbolSymbol * sym = static_cast<SymbolSymbol *>(result.symbol());
    bool noVal = val == nullptr;
    auto dt = sym->secrecType()->secrecDataType();

    if (result.symbol()->secrecType()->isScalar()) {
        emplaceImopAfter(result, m_node, Imop::DECLARE, sym);
    }
    else if (dt->isUserPrimitive()) {
        auto pubTy = dtypeDeclassify(sym->secrecType()->secrecSecType(),
                                     sym->secrecType()->secrecDataType());
        if (noVal && pubTy != nullptr)
            val = defaultConstant(getContext(), pubTy);

        if (val != nullptr)
            emplaceImopAfter(result, m_node, Imop::ALLOC, sym, sym->getSizeSym(), val);
        else
            emplaceImopAfter(result, m_node, Imop::ALLOC, sym, sym->getSizeSym());
    }
    else {
        if (noVal) {
            val = defaultConstant(getContext(), sym->secrecType()->secrecDataType());
        }
        emplaceImopAfter(result, m_node, Imop::ALLOC, sym, sym->getSizeSym(), val);
    }
}

void CodeGen::releaseScopeVariables (CGResult& result) {
    for (auto sym : m_st->variables()) {
        for (auto var : flattenSymbol (sym)) {
            releaseResource (result, var);
        }
    }
}

void CodeGen::releaseProcVariables(CGResult & result, Symbol * ex) {
    std::set<Symbol*> exclude;
    if (ex != nullptr) {
        const auto symVec = flattenSymbol (ex);
        exclude.insert (symVec.begin (), symVec.end ());
    }

    for (auto sym : m_st->variablesUpTo(m_st->globalScope())) {
        for (auto var : flattenSymbol (sym)) {
            if (exclude.find (var) == exclude.end ()) {
                releaseResource (result, var);
            }
        }
    }
}

void CodeGen::releaseAllVariables(CGResult & result) {
    for (auto sym : m_st->variablesUpTo(nullptr)) {
        for (auto var : flattenSymbol (sym)) {
            releaseResource (result, var);
        }
    }
}

void CodeGen::releaseResource(CGResult & result, Symbol * sym) {
    if (isNontrivialResource(sym->secrecType()))
        pushImopAfter(result, new Imop(m_node, Imop::RELEASE, nullptr, sym));
}

void CodeGen::releaseTemporary(CGResult & result, Symbol * sym, Symbol* ex) {
    assert(sym != nullptr);
    std::vector<Symbol*> exclude;
    if (ex != nullptr) {
        for (SymbolSymbol* temp : collectTemporaries (ex)) {
            exclude.push_back (temp);
        }
    }

    for (SymbolSymbol* temp : collectTemporaries (sym)) {
        if (std::find (exclude.begin (), exclude.end (), temp) == exclude.end ()) {
            releaseResource(result, temp);
        }
    }
}

void CodeGen::codeGenSize (CGResult &result, Symbol* sym) {
    assert(sym != nullptr);
    if (SymbolSymbol* resSym = dynamic_cast<SymbolSymbol *>(sym)) {
        codeGenSize (result, resSym);
    }
}

void CodeGen::codeGenSize (CGResult &result, SymbolSymbol* sym) {
    assert(sym != nullptr);
    if (sym->secrecType ()->secrecDataType ()->isComposite ()) {
        for (SymbolSymbol* field : sym->fields ()) {
            codeGenSize (result, field);
        }
    }
    else {
        Symbol * size = sym->getSizeSym();
        if (size == nullptr)
            return;
        Symbol * one = indexConstant(1);
        emplaceImopAfter (result, m_node, Imop::ASSIGN, size, one);
        for (auto dimSym : sym->dims())
            emplaceImop(m_node, Imop::MUL, size, size, dimSym);
    }
}

void CodeGen::codeGenSize(CGResult & result) {
    codeGenSize (result, result.symbol ());
}

Symbol* CodeGen::copyNonTemporary (CGResult& result, Symbol* sym) {
    assert (sym != nullptr);
    assert (! sym->secrecType ()->secrecDataType ()->isComposite ());

    if (sym->isConstant ())
        return sym;

    assert (dynamic_cast<SymbolSymbol*>(sym) != nullptr);
    SymbolSymbol* t = static_cast<SymbolSymbol*>(sym);
    if (t->isTemporary ())
        return t;

    if (t->isArray ()) {
        auto copy = m_st->appendTemporary (t->secrecType ());
        // Note that we don't need to assign the shape.
        copy->setSizeSym (t->getSizeSym ());
        emplaceImopAfter (result, m_node, Imop::COPY, copy, t, t->getSizeSym ());
        return copy;
    }

    if (t->isString() || t->secrecType()->secrecSecType()->isPrivate()) {
        auto copy = m_st->appendTemporary(t->secrecType());
        emplaceImopAfter(result, m_node, Imop::DECLARE, copy);
        emplaceImop(m_node, Imop::ASSIGN, copy, t);
        return copy;
    }

    return t;
}

CGResult CodeGen::copyShape (Symbol* dest, Symbol* sym) {
    assert(dynamic_cast<SymbolSymbol *>(dest) != nullptr);
    assert (dynamic_cast<SymbolSymbol *>(sym) != nullptr);
    SymbolSymbol* d = static_cast<SymbolSymbol *>(dest);
    SymbolSymbol* s = static_cast<SymbolSymbol *>(sym);
    assert(d->dims().size() == s->dims().size());
    auto dj(d->dims().begin());

    CGResult result;
    for (auto ss : s->dims()) {
        assert(dj != d->dims().end());
        emplaceImopAfter(result, m_node, Imop::ASSIGN, *dj, ss);
        ++dj;
    }

    if (s->getSizeSym() != nullptr) {
        emplaceImopAfter (result, m_node, Imop::ASSIGN, d->getSizeSym(), s->getSizeSym());
    }

    return result;
}

void CodeGen::copyShapeFrom(CGResult & result, Symbol * tmp) {
    append (result, copyShape (result.symbol (), tmp));
}

LoopInfo CodeGen::prepareLoopInfo (const SubscriptInfo& subscript) {
    LoopInfo loopInfo;
    const auto& spv = subscript.spv();
    const auto pubIntTy = TypeBasic::getIndexType();
    for (size_t i = 0; i < spv.size (); ++ i)
        loopInfo.push_index(m_st->appendTemporary(pubIntTy));
    return loopInfo;
}


SymbolSymbol * CodeGen::generateResultSymbol(CGResult& result, const SecreC::Type* ty) {
    assert (ty != nullptr);
    assert (! ty->isVoid());
    SymbolSymbol* sym = generateSymbol (getContext (), m_st, ty);
    assert (sym != nullptr);
    result.setResult(sym);
    return sym;
}

SymbolSymbol* CodeGen::generateResultSymbol (CGResult& result, TreeNodeExpr* node) {
    assert (node);
    assert (node->haveResultType ());
    assert (! node->resultType()->isVoid());
    return generateResultSymbol (result, node->resultType());
}

CGResult CodeGen::codeGenStride(ArrayStrideInfo & strideInfo) {
    const TypeBasic * ty = TypeBasic::getIndexType();
    CGResult result;
    Symbol * tmp = strideInfo.symbol();
    const unsigned n = tmp->secrecType()->secrecDimType();
    if (n == 0) { // scalar doesn't have stride
        return result;
    }

    assert(dynamic_cast<SymbolSymbol *>(tmp) != nullptr);
    SymbolSymbol * sym = static_cast<SymbolSymbol *>(tmp);
    strideInfo.clear();
    strideInfo.reserve(n);

    for (unsigned it = 0; it < n; ++ it) {
        strideInfo.push_back(m_st->appendTemporary(ty));
    }

    emplaceImopAfter (result, m_node, Imop::ASSIGN, strideInfo.at(n - 1), indexConstant(1));
    for (unsigned it = n - 1; it != 0; -- it) {
        emplaceImop (m_node, Imop::MUL,
            strideInfo.at(it - 1), strideInfo.at(it), sym->getDim(it));
    }

    return result;
}

CGResult CodeGen::enterLoop(LoopInfo & loopInfo, Symbol * tmp) {
    assert(loopInfo.empty());
    CGResult result;
    assert(dynamic_cast<SymbolSymbol *>(tmp) != nullptr);
    SymbolSymbol * sym = static_cast<SymbolSymbol *>(tmp);
    Symbol * zero = indexConstant(0);
    const TypeBasic * boolTy = TypeBasic::getPublicBoolType();
    unsigned count = 0;
    for (Symbol * idx : loopInfo) {
        auto i = new Imop(m_node, Imop::ASSIGN, idx, zero);
        push_imop(i);
        result.patchFirstImop(i);

        SymbolTemporary * temp_bool = m_st->appendTemporary(boolTy);
        auto test = new Imop(m_node, Imop::GE, temp_bool, idx, sym->getDim(count ++));
        push_imop(test);

        auto jump = new Imop(m_node, Imop::JT, nullptr, temp_bool);
        push_imop(jump);

        loopInfo.pushJump(idx, test, jump);
    }

    return result;
}

CGResult CodeGen::enterLoop(LoopInfo & loopInfo, const SubscriptInfo::SPV & spv) {
    assert(loopInfo.empty());
    const TypeBasic * boolTy = TypeBasic::getPublicBoolType();
    CGResult result;
    LoopInfo::const_iterator idxIt;

    idxIt = loopInfo.begin();
    for (const auto& v : spv) {
        if (! v.second) {
            Symbol * idx  = *idxIt;
            auto i = new Imop(m_node, Imop::ASSIGN, idx, v.first);
            push_imop(i);
            result.patchFirstImop(i);
            loopInfo.pushNop (idx);
        }

        ++ idxIt;
    }

    idxIt = loopInfo.begin();
    for (const auto& v : spv) {
        Symbol * idx  = *idxIt;
        if (v.second) {
            auto i = new Imop(m_node, Imop::ASSIGN, idx, v.first);
            push_imop(i);
            result.patchFirstImop(i);

            SymbolTemporary * temp_bool = m_st->appendTemporary(boolTy);
            auto test = new Imop(m_node, Imop::GE, temp_bool, idx, v.second);
            push_imop(test);

            auto jump = new Imop(m_node, Imop::JT, nullptr, temp_bool);
            push_imop(jump);

            loopInfo.pushJump(idx, test, jump);
        }

        ++ idxIt;
    }

    return result;
}

CGResult CodeGen::exitLoop(LoopInfo & loopInfo) {
    CGResult result;
    Imop * prevJump = nullptr;
    Symbol * one = indexConstant(1);

    while (! loopInfo.empty ()) {
        if (! loopInfo.isTopNop ()) {
            LoopInfo::LoopCheck check = loopInfo.top();

            auto i = new Imop(m_node, Imop::ADD, check.index, check.index, one);
            push_imop(i);
            result.patchFirstImop(i);
            if (prevJump != nullptr) {
                prevJump->setDest(m_st->label(i));
            }

            i = new Imop(m_node, Imop::JUMP, static_cast<Symbol *>(nullptr));
            push_imop(i);

            i->setDest(m_st->label(check.test));
            prevJump = check.jump;
        }

        loopInfo.pop ();
    }

    if (prevJump != nullptr) {
        result.addToNextList(prevJump);
    }

    return result;
}

CGResult CodeGen::codeGenSubscript(SubscriptInfo & subInfo, Symbol * tmp, TreeNode * node) {
    assert(node != nullptr);
    assert(tmp != nullptr);
    assert(node->type() == NODE_SUBSCRIPT);
    assert(dynamic_cast<SymbolSymbol *>(tmp) != nullptr);

    SubscriptInfo::SliceIndices & m_slices = subInfo.m_slices;
    SubscriptInfo::SPV & m_spv = subInfo.m_spv;
    m_slices.clear();
    m_spv.clear();

    CGResult result;
    unsigned count = 0;
    SymbolSymbol * x = static_cast<SymbolSymbol *>(tmp);

    // 1. evaluate the indices and manage the syntactic suggar
    for (TreeNode * t : node->children()) {
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
            r_hi = nullptr;
        }

        m_spv.push_back (std::make_pair (r_lo, r_hi));
        ++ count;
    }

    // 2. check that indices are legal
    {
        std::stringstream ss;
        ss << "Index out of bounds at " << m_node->location() << '.';
        auto jmp = new Imop(m_node, Imop::JUMP, static_cast<Symbol *>(nullptr));
        Imop * err = newError(m_node, ConstantString::get(getContext(), ss.str()));
        SymbolLabel * errLabel = m_st->label(err);

        const TypeBasic * boolTy = TypeBasic::getPublicBoolType();
        SymbolTemporary * temp_bool = m_st->appendTemporary(boolTy);

        auto dit = x->dims().begin();
        for (auto it = m_spv.begin(), e = m_spv.end ();
             it != e; ++ it, ++ dit)
        {
            Symbol * s_lo = it->first;
            Symbol * s_hi = it->second;
            Symbol * d = *dit;
            Imop * i = nullptr;

            if (s_hi == nullptr) {
                i = new Imop(m_node, Imop::GE, temp_bool, s_lo, d);
                pushImopAfter(result, i);

                i = new Imop(m_node, Imop::JT, nullptr, temp_bool);
                push_imop(i);
                i->setDest(errLabel);
            }
            else {
                i = new Imop(m_node, Imop::GT, temp_bool, s_lo, s_hi);
                pushImopAfter(result, i);

                i = new Imop(m_node, Imop::JT, nullptr, temp_bool);
                push_imop(i);
                i->setDest(errLabel);

                i = new Imop(m_node, Imop::GT, temp_bool, s_hi, d);
                push_imop(i);

                i = new Imop(m_node, Imop::JT, nullptr, temp_bool);
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
    return m_loops.empty() ? nullptr : m_loops.back();
}

CGResult CodeGen::cgProcParam (SymbolSymbol* sym) {
    assert (sym != nullptr && sym->secrecType () != nullptr);

    const TypeNonVoid* ty = sym->secrecType ();
    CGResult result;

    if (ty->secrecDataType ()->isComposite ()) {
        for (SymbolSymbol* field : sym->fields ()) {
            append (result, cgProcParam (field));
            if (result.isNotOk ())
                return result;
        }

        return result;
    }
    else
    if (ty->isScalar ()) {
        emplaceImopAfter(result, m_node, Imop::PARAM, sym);
    }
    else {
        // Pop parameters:
        for (auto symDim : sym->dims())
            emplaceImopAfter(result, m_node, Imop::PARAM, symDim);
        emplaceImopAfter(result, m_node, Imop::PARAM, sym);

        // Update size:
        emplaceImop(m_node, Imop::ASSIGN, sym->getSizeSym(), indexConstant(1));
        for (auto symDim : sym->dims())
            emplaceImop(m_node, Imop::MUL, sym->getSizeSym(), sym->getSizeSym(), symDim);
    }

    return result;
}

CGResult CodeGen::cgInitializeToConstant (SymbolSymbol* sym, SymbolConstant* def) {
    const TypeNonVoid* ty = sym->secrecType ();
    CGResult result;

    if (ty->isScalar ()) {
        Imop::Type iType = ty->secrecSecType ()->isPrivate () ? Imop::CLASSIFY : Imop::ASSIGN;
        emplaceImopAfter(result, m_node, Imop::DECLARE, sym);
        emplaceImopAfter(result, m_node, iType, sym, def);
    }
    else {
        emplaceImopAfter(result, m_node, Imop::ALLOC, sym, getSizeOr(sym, 0), def);
    }

    return result;
}

CGResult CodeGen::cgInitializeToDefaultValue (SymbolSymbol* sym, bool hasShape) {
    assert (sym != nullptr && sym->secrecType () != nullptr);

    const TypeNonVoid* ty = sym->secrecType ();
    CGResult result;

    if (ty->secrecDataType ()->isComposite ()) {
        for (SymbolSymbol* field : sym->fields ()) {
            append (result, cgInitializeToDefaultValue (field, false));
            if (result.isNotOk ())
                return result;
        }

        return result;
    }

    // Initialize the value of the shape (and size) if need be.
    if (! ty->isScalar () && ! hasShape) {
        SymbolConstant* defIdx = indexConstant (0);
        emplaceImopAfter(result, m_node, Imop::ASSIGN, sym->getSizeSym(), defIdx);
        for (SecrecDimType it = 0; it < ty->secrecDimType(); ++it)
            emplaceImop(m_node, Imop::ASSIGN, sym->getDim(it), defIdx);
    }

    if (ty->secrecDataType ()->isUserPrimitive ()) {
        assert (ty->secrecSecType ()->isPrivate ());
        const DataType* publicType = dtypeDeclassify (ty->secrecSecType (), ty->secrecDataType ());

        if (publicType) {
            SymbolConstant* def =
                defaultConstant (getContext (), publicType);
            append (result, cgInitializeToConstant (sym, def));
        }
        else if (ty->isScalar ()) {
            emplaceImopAfter (result, m_node, Imop::DECLARE, sym);
        }
        else {
            emplaceImopAfter(result, m_node, Imop::ALLOC, sym, getSizeOr (sym, 0));
        }
    }
    else {
        SymbolConstant* def = defaultConstant (getContext (), ty->secrecDataType ());
        append (result, cgInitializeToConstant (sym, def));
    }

    return result;
}

CGResult CodeGen::cgInitializeToSymbol (SymbolSymbol* lhs, Symbol* rhs, bool hasShape) {
    assert (lhs != nullptr && lhs->secrecType () != nullptr);
    assert (rhs != nullptr && rhs->secrecType () != nullptr);
    assert (m_node != nullptr);

    const TypeNonVoid* ty = lhs->secrecType ();
    const TypeBasic* pubBoolTy = TypeBasic::getPublicBoolType();
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
            emplaceImopAfter(result, m_node, Imop::ALLOC, lhs, lhs->getSizeSym(), rhs);
            releaseTemporary(result, rhs);
        }
        else {
            // check that shapes match and assign
            std::stringstream ss;
            ss << "Shape mismatch at " << m_node->location();
            Imop * err = newError(m_node, ConstantString::get(getContext(), ss.str()));
            SymbolLabel * const errLabel = m_st->label(err);
            auto lhsDimIter(lhs->dims().begin());
            if (auto rhsSs = dynamic_cast<SymbolSymbol *>(rhs)) {
                for (auto rhsDim : rhsSs->dims()) {
                    SymbolTemporary * const temp_bool = m_st->appendTemporary(pubBoolTy);
                    emplaceImopAfter(result, m_node, Imop::NE, temp_bool, rhsDim, *lhsDimIter);
                    emplaceImop(m_node, Imop::JT, errLabel, temp_bool);
                    ++lhsDimIter;
                }
            }

            const auto jmp = new Imop(m_node, Imop::JUMP, nullptr);
            const auto i = new Imop(m_node, Imop::COPY, lhs, rhs, lhs->getSizeSym());
            jmp->setDest(m_st->label(i));
            pushImopAfter(result, jmp);
            push_imop(err);
            push_imop(i);
            releaseTemporary(result, rhs);
        }
    } else {
        if (ty->secrecDimType() > 0) { // type[[>0]] x = foo;
            if (ty->secrecDimType() > rhs->secrecType()->secrecDimType()) {
                emplaceImopAfter(result, m_node, Imop::ALLOC, lhs, lhs->getSizeSym(), rhs);
                releaseTemporary(result, rhs);
            } else {
                assert(ty->secrecDimType() == rhs->secrecType()->secrecDimType());

                auto & rhsSs = *static_cast<SymbolSymbol *>(rhs);

                auto srcIter(rhsSs.dims().begin());
                for (auto destSym : lhs->dims()) {
                    assert(srcIter != rhsSs.dims().end());
                    emplaceImopAfter(result, m_node, Imop::ASSIGN, destSym, *srcIter);
                    ++srcIter;
                }

                // The type checker should ensure that the symbol is a symbol (and not a constant):
                assert(rhs->symbolType() == SYM_SYMBOL);
                SymbolSymbol * const s = static_cast<SymbolSymbol *>(rhs);
                emplaceImopAfter(result, m_node, Imop::ASSIGN, lhs->getSizeSym(), s->getSizeSym());
                emplaceImop(m_node, Imop::COPY, lhs, s, lhs->getSizeSym());
                releaseTemporary(result, rhs);
            }
        } else { // scalar_type x = scalar;
            emplaceImopAfter(result, m_node, Imop::DECLARE, lhs);
            emplaceImop(m_node, Imop::ASSIGN, lhs, rhs);
            releaseTemporary(result, rhs);
        }
    }

    return result;
}

} // namespace SecreC
