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

#ifndef SECREC_CODE_GEN_H
#define SECREC_CODE_GEN_H

#include "CodeGenState.h"
#include "ICodeList.h"
#include "SymbolFwd.h"
#include "TreeNodeFwd.h"

#include <map>
#include <set>
#include <vector>

namespace SecreC {

class CGBranchResult;
class CGResult;
class CGStmtResult;
class CodeGen;
class CompileLog;
class Context;
class ICode;
class ICodeList;
class Imop;
class ModuleInfo;
class ModuleMap;
class StringRef;
class StringTable;
class SymbolProcedure;
class SymbolSymbol;
class SymbolTable;
class Type;
class TypeChecker;
class TypeNonVoid;

/*******************************************************************************
  SubscriptInfo
*******************************************************************************/

class SubscriptInfo {
public: /* Types: */
    using SliceIndices = std::vector<unsigned>;
    using SPV = std::vector<std::pair<Symbol*, Symbol* > >;

    const SliceIndices& slices () const { return m_slices; }
    const SPV& spv () const { return m_spv; }

protected:

    friend class CodeGen;

private: /* Fields: */

    SliceIndices m_slices;  ///< Specifies which indices are slices.
    SPV          m_spv;     ///< List of ranges for every index.
};

/*******************************************************************************
  LoopInfo
*******************************************************************************/

class LoopInfo {
public: /* Types: */
    using IndexList = std::vector<Symbol*>;
    using iterator = IndexList::iterator;
    using const_iterator = IndexList::const_iterator;

    struct LoopCheck {
        Symbol* index;
        Imop* test;
        Imop* jump;
    };

public: /* Methods: */
    LoopInfo () { }
    ~LoopInfo () {
        assert (m_jumpStack.empty () && "The loop was not exited.");
    }

    void push_index (Symbol* symbol) { m_indices.push_back (symbol); }
    Symbol* at (IndexList::size_type n) const { return m_indices.at (n); }
    iterator begin () { return m_indices.begin (); }
    iterator end () { return m_indices.end (); }
    const_iterator begin () const { return m_indices.begin (); }
    const_iterator end () const { return m_indices.end (); }

protected:

    friend class CodeGen;

    void pushJump (Symbol* index, Imop* test, Imop* jump) {
        LoopCheck temp = { index, test, jump };
        m_jumpStack.push_back (temp);
    }

    void pushNop (Symbol* index) {
        LoopCheck temp = { index, nullptr, nullptr };
        m_jumpStack.push_back (temp);
    }

    bool isTopNop () const { return m_jumpStack.back ().test == nullptr; }
    bool empty () const { return m_jumpStack.empty (); }
    LoopCheck top () const { return m_jumpStack.back (); }
    void pop () { return m_jumpStack.pop_back (); }

private: /* Fields: */
    IndexList                m_indices;
    std::vector<LoopCheck >  m_jumpStack;
};

/*******************************************************************************
  ArrayStrideInfo
*******************************************************************************/

class ArrayStrideInfo {
private: /* Types: */
    using StrideList = std::vector<Symbol*>;
public:
    using iterator = StrideList::iterator;
    using const_iterator = StrideList::const_iterator;
    using size_type = StrideList::size_type;
public: /* Methods: */

    ArrayStrideInfo (Symbol* sym)
        : m_symbol (sym)
    { }

    const_iterator begin () const { return m_stride.begin (); }
    const_iterator end () const { return m_stride.end (); }
    unsigned size () const { return m_stride.size (); }
    Symbol* at (unsigned i) const { return m_stride.at (i); }

protected:

    void push_back (Symbol* s) { m_stride.push_back (s); }
    void clear () { m_stride.clear (); }
    void reserve (size_type n) { m_stride.reserve (n); }
    Symbol* symbol () const { return m_symbol; }

    friend class CodeGen;

private: /* Fields: */
    Symbol*    m_symbol;
    StrideList m_stride;
};

/*******************************************************************************
  CodeGen
*******************************************************************************/

/**
 * \class CodeGen
 * \brief CodeGen class handles the logic of code generation.
 *
 * Do note that abstract syntax tree (TreeNode) handles how control
 * flows through CodeGen methods. There's curious recursion between those two
 * but this indirection should be inlined and makes possible to keep everything
 * related to intermediate code generation outside TreeNode.
 *
 * \todo It should be possible to remove the m_node member. Figure that out.
 */
class CodeGen : public CodeGenState {
    friend class ScopedStateUse;
    friend class ScopedScope;
    friend class ScopedLoop;
    friend class ScopedSetNode;
    friend struct ScopedSetSymbolTable;
private:

    CodeGen& operator = (const CodeGen&) = delete;
    CodeGen (const CodeGen&) = delete;

private: /* Types: */

    using CallMap = std::map<SymbolProcedure*, std::set<Imop*>>;
    using STList = std::vector<SymbolTable*>;

public: /* Methods: */

    CodeGen (ICodeList& code, ICode& icode);
    ~CodeGen ();

    CGResult codeGen (TreeNodeExpr* e);
    CGBranchResult codeGenBranch (TreeNodeExpr* e);
    CGStmtResult codeGenStmt (TreeNodeStmt* s);

    /**
     * \name Top level code.
     * Methods for top level program code generation.
     */
     /// \{
    CGStmtResult cgMain (TreeNodeModule* mainModule);
    CGStmtResult cgModule (ModuleInfo* mod);
    CGStmtResult cgProgram (TreeNodeProgram* prog);
    CGStmtResult cgDomain (TreeNodeDomain* dom);
    CGStmtResult cgKind (TreeNodeKind* kind);
    CGStmtResult cgImport (TreeNodeImport* imp, ModuleInfo* modContext);
    CGStmtResult cgProcDef (TreeNodeProcDef* def, SymbolTable* localScope);
    CGStmtResult cgStructDecl (TreeNodeStructDecl* decl);
    /// \}

    /**
     * \name Statements.
     * Methods for statement code generation.
     */
    /// \{
    CGStmtResult cgStmtBreak (TreeNodeStmtBreak* s);
    CGStmtResult cgStmtCompound (TreeNodeStmtCompound* s);
    CGStmtResult cgStmtContinue (TreeNodeStmtContinue* s);
    CGStmtResult cgStmtDecl (TreeNodeStmtDecl* s);
    CGStmtResult cgStmtDoWhile (TreeNodeStmtDoWhile* s);
    CGStmtResult cgStmtExpr (TreeNodeStmtExpr* s);
    CGStmtResult cgStmtAssert (TreeNodeStmtAssert* s);
    CGStmtResult cgStmtFor (TreeNodeStmtFor* s);
    CGStmtResult cgStmtIf (TreeNodeStmtIf* s);
    CGStmtResult cgStmtReturn (TreeNodeStmtReturn* s);
    CGStmtResult cgStmtWhile (TreeNodeStmtWhile* s);
    CGStmtResult cgStmtPrint (TreeNodeStmtPrint* s);
    CGStmtResult cgStmtSyscall (TreeNodeStmtSyscall* s);
    /// \}

    /**
     * \name Branching expressions.
     * Code generation for expressions that can have boolean type and occur
     * in branching context such as \a if statement and \a ternary  expressions.
     */
    /// \{
    CGBranchResult cgBoolSimple (TreeNodeExpr *e);
    CGBranchResult cgBoolExprCast (TreeNodeExprCast* e);
    CGBranchResult cgBoolExprUnary (TreeNodeExprUnary* e);
    CGBranchResult cgBoolExprBinary (TreeNodeExprBinary* e);
    CGBranchResult cgBoolExprRVariable (TreeNodeExprRVariable* e);
    CGBranchResult cgBoolExprTernary (TreeNodeExprTernary* e);
    CGBranchResult cgBoolExprBool (TreeNodeExprBool* e);
    CGBranchResult cgBoolExprAssign (TreeNodeExprAssign* e);
    CGBranchResult cgBoolExprQualified (TreeNodeExprQualified* e);
    CGBranchResult cgBoolExprSelection (TreeNodeExprSelection* e);
    /// \}

    /**
     * \name Expressions.
     * Code generation for regular expressions.
     */
    /// \{
    CGResult cgExprArrayConstructor (TreeNodeExprArrayConstructor* e);
    CGResult cgExprCast (TreeNodeExprCast* e);
    CGResult cgExprBool (TreeNodeExprBool* e);
    CGResult cgExprIndex (TreeNodeExprIndex* e);
    CGResult cgExprShape (TreeNodeExprShape* e);
    CGResult cgExprReshape (TreeNodeExprReshape* e);
    CGResult cgExprToString (TreeNodeExprToString* e);
    CGResult cgExprCat (TreeNodeExprCat* e);
    CGResult cgExprBinary (TreeNodeExprBinary* e);
    CGResult cgExprProcCall (TreeNodeExprProcCall* e);
    CGResult cgExprRVariable (TreeNodeExprRVariable* e);
    CGResult cgExprDomainID (TreeNodeExprDomainID* e);
    CGResult cgExprQualified (TreeNodeExprQualified* e);
    CGResult cgExprBytesFromString (TreeNodeExprBytesFromString* e);
    CGResult cgExprStringFromBytes (TreeNodeExprStringFromBytes* e);
    CGResult cgExprString (TreeNodeExprString* e);
    CGResult cgExprFloat (TreeNodeExprFloat* e);
    CGResult cgExprTernary (TreeNodeExprTernary* e);
    CGResult cgExprInt (TreeNodeExprInt* e);
    CGResult cgExprClassify (TreeNodeExprClassify* e);
    CGResult cgExprDeclassify (TreeNodeExprDeclassify* e);
    CGResult cgExprUnary (TreeNodeExprUnary* e);
    CGResult cgExprPrefix (TreeNodeExprPrefix* e);
    CGResult cgExprPostfix (TreeNodeExprPostfix* e);
    CGResult cgExprSize (TreeNodeExprSize* e);
    CGResult cgExprAssign (TreeNodeExprAssign* e);
    CGResult cgExprSelection (TreeNodeExprSelection* e);
    CGResult cgExprStrlen (TreeNodeExprStrlen* e);
    /// \}

    /// \{
    CGResult cgStringPart (TreeNodeStringPart* p);
    CGResult cgStringPartIdentifier (TreeNodeStringPartIdentifier* p);
    CGResult cgStringPartFragment (TreeNodeStringPartFragment* p);
    /// \}

    /// \{
    CGResult cgLValue (TreeNodeLValue* lval, SubscriptInfo& subInfo, bool& isIndexed);
    CGResult cgLVariable (TreeNodeLVariable* lvar, SubscriptInfo& subInfo, bool& isIndexed);
    CGResult cgLIndex (TreeNodeLIndex* lindex, SubscriptInfo& subInfo, bool& isIndexed);
    CGResult cgLSelect (TreeNodeLSelect* lselect, SubscriptInfo& subInfo, bool& isIndexed);
    /// \}

    /// Memory management
    /// \{
    void allocTemporaryResult (CGResult& result, Symbol* val = nullptr);
    void releaseResource (CGResult& result, Symbol* sym);
    void releaseTemporary (CGResult& result, Symbol* sym, Symbol* ex = nullptr);
    void releaseScopeVariables (CGResult& result);
    void releaseProcVariables (CGResult& result, Symbol* ex = nullptr);
    void releaseAllVariables (CGResult& result);
    /// \}

    /// Looping, and indexing.
    /// \{
    CGResult codeGenSubscript (SubscriptInfo& subInfo, Symbol* x, TreeNode* node);
    CGResult codeGenStride (ArrayStrideInfo& strideInfo);
    CGResult enterLoop (LoopInfo& loopInfo, Symbol* sym);
    CGResult enterLoop (LoopInfo& loopInfo, const SubscriptInfo::SPV& spv);
    CGResult exitLoop (LoopInfo& loopInfo);
    /// \}

    Context& getContext () const { return m_context; }
    SymbolTable* symbolTable () const { return m_st; }
    TreeNode* treeNode () const { return m_node; }

    inline void push_imop (Imop* imop) {
        m_code.insert (m_insertPoint, imop);
    }

    template <typename ...Args>
    inline void emplaceImopAfter (CGResult& result, Args&& ...args) {
        pushImopAfter (result, new Imop (std::forward<Args>(args)...));
    }

    template <typename ...Args>
    inline void emplaceImop (Args&& ...args) {
        push_imop (new Imop (std::forward<Args>(args)...));
    }

    /**
     * \brief Used to push instruction right after code block.
     * \pre imop is not NULL
     * \post first imop is set and next list is empty
     */
    void pushImopAfter (CGResult& result, Imop* imop);

    /**
     * Useful to chain many expressions together if the control flow is linear.
     * Care must be taken when using this to chain branching code together.
     */
    void append (CGResult& result, const CGResult& other);

private:

    StringTable& getStringTable () const;

    void swap (CodeGenState& state) {
        swapState (state);
        updateTypeChecker ();
    }

    void setScope (SymbolTable* st) {
        m_st = st;
        updateTypeChecker ();
    }

    void updateTypeChecker ();
    void newScope ();
    void popScope ();

    Imop* newComment (StringRef commnet) const;
    Imop* pushComment (StringRef comment);

    /// Given result computes size of it
    void codeGenSize (CGResult& result);
    void codeGenSize (CGResult& result, Symbol* sym);
    void codeGenSize (CGResult& result, SymbolSymbol* sym);

    /// Copy shape from another symbol
    void copyShapeFrom (CGResult& result, Symbol* sym);
    CGResult copyShape (Symbol* dest, Symbol* sym);
    CGResult cgAssign (SymbolSymbol* dest, Symbol* src);
    Symbol* copyNonTemporary (CGResult& result, Symbol* sym);

    LoopInfo prepareLoopInfo (const SubscriptInfo& subscript);

    /**
     * @brief CodeGen::cgProcParam Expect the given symbol as procedure parameter.
     * @param sym Expected procedure parameter (it's subsymbols are also expected as parameters).
     * @return Code generation result.
     */
    CGResult cgProcParam (SymbolSymbol* sym);

    /**
     * @brief CodeGen::cgInitializeToConstant Initialize the given symbol to the given constant.
     * @param sym The symbol that needs to be initialized.
     * @param def Symbol of the constant.
     * @return Code generatoin result.
     */
    CGResult cgInitializeToConstant (SymbolSymbol* sym, SymbolConstant* def);

    /**
     * @brief CodeGen::cgInitializeToDefaultValue Initialize the given symbol to the default value.
     * @param sym The symbol that needs to be initialized.
     * @param hasShape If the shape of the array has already been computed.
     * @return Code generation result.
     */
    CGResult cgInitializeToDefaultValue (SymbolSymbol* sym, bool hasShape = false);

    /**
     * @brief cgInitializeToSymbol Initialize the lhs to the value of rhs.
     * @param lhs The left hand side symbol.
     * @param rhs The right hand side symbol.
     * @param hasShape If the left hand side has fixed shape.
     * @return Code generation result.
     */
    CGResult cgInitializeToSymbol (SymbolSymbol* lhs, Symbol* rhs, bool hasShape = false);

    CGResult cgProcCall (SymbolProcedure* symProc,
                         SecreC::Type* returnType,
                         const std::vector<Symbol*>& args);
    CGResult cgProcCall (SymbolProcedure* symProc,
                         SecreC::Type* returnType,
                         const std::vector<TreeNodeExpr*>& args);

    /// generate appropriately typed result symbol for given node
    SymbolSymbol* generateResultSymbol (CGResult& result, TreeNodeExpr* node);

    /// generate symbol for given type
    SymbolSymbol* generateResultSymbol (CGResult& result, SecreC::Type* ty);


    CGStmtResult cgGlobalVarInit (TypeNonVoid* ty, TreeNodeVarInit* varInit);
    CGStmtResult cgLocalVarInit (TypeNonVoid* ty, TreeNodeVarInit* varInit);
    CGStmtResult cgProcParamInit (TypeNonVoid* ty, TreeNodeVarInit* varInit);
    CGStmtResult cgVarInit (TypeNonVoid* ty, TreeNodeVarInit* varInit, bool isProcParam);

    Symbol* toVector(CGResult & result,
                     TreeNodeExpr * e,
                     Symbol * eArgRes,
                     Symbol * size);
    void cgBinExprShapeCheck(TreeNodeExpr * e,
                             Symbol * e1result,
                             Symbol * e2result,
                             CGResult & result);
    CGResult cgOverloadedExpr (TreeNodeExpr * e,
                               Type * resTy,
                               std::vector<Symbol*> & operands,
                               SymbolProcedure * symProc);
    CGResult cgOverloadedPrefixPostfix(TreeNodeExpr* e,
                                       SymbolProcedure* procSymbol,
                                       Symbol* x,
                                       Symbol* one);

    Symbol* getSizeOr (Symbol* sym, uint64_t val);
    SymbolConstant* indexConstant (uint64_t value);

    Symbol* findIdentifier (SymbolCategory type, const TreeNodeIdentifier* id) const;

    void startLoop ();
    void endLoop ();
    SymbolTable* loopST () const;
    SymbolTable* procST () const;

private: /* Fields: */

    // Components owned by others:
    ICodeList&    m_code;         ///< Generated sequence of IR instructions.
    CompileLog&   m_log;          ///< Compiler log.
    ModuleMap&    m_modules;      ///< Mapping from names to modules.
    Context&      m_context;

    // Local components:
    STList        m_loops;
    TypeChecker*  m_tyChecker;    ///< Instance of the type checker.
    CallMap       m_callsTo;      ///< Unpatched procedure calls.
};


/*******************************************************************************
  ScopedSetNode
*******************************************************************************/

class ScopedSetNode {
public:
    ScopedSetNode (CodeGen& codeGen, TreeNode* node)
        : m_codeGen (codeGen)
        , m_node (node)
    {
        std::swap (m_codeGen.m_node, m_node);
    }

    ~ScopedSetNode () {
        std::swap (m_codeGen.m_node, m_node);
    }

private:
    CodeGen&   m_codeGen;
    TreeNode*  m_node;
};


/*******************************************************************************
  ScopedStateUse
*******************************************************************************/

/**
 * RAII class that on construction creates backup of CodeGen state
 * and switches to using given new state. On destruction restores
 * the backup.
 */
class ScopedStateUse {
public:
    ScopedStateUse (CodeGen& codeGen, const CodeGenState& state)
        : m_codeGen (codeGen)
        , m_state (state)
    {
        m_codeGen.swap (m_state);
    }

    ~ScopedStateUse () {
        m_codeGen.swap (m_state);
    }

private:
    CodeGen&      m_codeGen;
    CodeGenState  m_state;
};

class ScopedScope {
public:

    ScopedScope(CodeGen & codeGen)
        : m_codeGen(codeGen)
    {
        m_codeGen.newScope();
    }

    ~ScopedScope() {
        m_codeGen.popScope();
    }

private:
    CodeGen & m_codeGen;
};

class ScopedLoop {
public:

    ScopedLoop(CodeGen & codeGen)
        : m_codeGen(codeGen)
    {
        m_codeGen.startLoop();
    }

    ~ScopedLoop() {
        m_codeGen.endLoop();
    }

private:
    CodeGen & m_codeGen;
};

void initShapeSymbols (Context& cxt, SymbolTable* st, SymbolSymbol* sym);

} // namespace SecreC

#endif
