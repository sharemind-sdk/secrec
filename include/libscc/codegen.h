#ifndef SECREC_CODE_GEN_H
#define SECREC_CODE_GEN_H

#include <stack>

#include "codegenResult.h"
#include "imop.h"
#include "intermediate.h"
#include "treenode.h"
#include "typechecker.h"

/**
 * Some ideas:
 * - Represent ICode::Status enum with powers of two, this way
 *   appendWith could just OR the statuses together. In general little
 *   better error reporting facility could be helpful.
 */

namespace SecreC {

class CodeGen;
class ICodeList;
class SymbolTable;
class CompileLog;
class Context;

/*******************************************************************************
  SubscriptInfo
*******************************************************************************/

class SubscriptInfo {
public: /* Types: */
    typedef std::vector<unsigned> SliceIndices;
    typedef std::vector<std::pair<Symbol*, Symbol* > > SPV;

    SubscriptInfo () { }
    ~SubscriptInfo () { }

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
    typedef std::vector<Symbol* > IndexList;
    typedef IndexList::iterator iterator;
    typedef IndexList::const_iterator const_iterator;

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

    void pushJump (Imop* imop) { m_jumpStack.push (imop); }
    bool empty () const { return m_jumpStack.empty (); }
    Imop* top () const { return m_jumpStack.top (); }
    void pop () { return m_jumpStack.pop (); }

private: /* Fields: */
    IndexList          m_indices;
    std::stack<Imop* > m_jumpStack;
};

/*******************************************************************************
  ArrayStrideInfo
*******************************************************************************/

class ArrayStrideInfo {
private: /* Types: */
    typedef std::vector<Symbol*> StrideList;
public:
    typedef StrideList::iterator iterator;
    typedef StrideList::const_iterator const_iterator;
    typedef StrideList::size_type size_type;
public: /* Methods: */

    ArrayStrideInfo (Symbol* sym)
        : m_symbol (sym)
    { }

    ~ArrayStrideInfo () { }

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
  ScopedAllocations
*******************************************************************************/

class ScopedAllocations {
private:
    void operator = (const ScopedAllocations&); // DO NOT IMPLEMENT
    ScopedAllocations (const ScopedAllocations&); // DO NOT IMPLEMENT

private: /* Types: */

    typedef std::vector<Symbol* > Allocations;

public: /* Methods: */

    ScopedAllocations (CodeGen& base, CGResult& result)
        : m_codeGen (base)
        , m_result (result)
    { }

    ~ScopedAllocations () {
        freeAllocs ();
    }

    void allocTemporary (Symbol* dest, Symbol* def, Symbol* size);

private:

    void freeAllocs ();

private: /* Fields: */

    CodeGen&     m_codeGen;
    CGResult&    m_result;
    Allocations  m_allocs;
};

/*******************************************************************************
  CodeGenState
*******************************************************************************/

/**
 * The part of code generator that needs to be stored and resumed on demand.
 * For example we might pause code generation at some scope to type check and
 * generate code for some imported module. After the code generation previous
 * state needs to be resumed.
 */
class CodeGenState {
protected: /* Types: */

    typedef std::vector<SymbolSymbol*> AllocList;

public:

    typedef ImopList::const_iterator InsertPoint;

public: /* Methods: */

    CodeGenState ()
        : m_st (0)
        , m_node (0)
    { }

    CodeGenState (InsertPoint it, SymbolTable* st)
        : m_insertPoint (it)
        , m_st (st)
        , m_node (0)
    { }

    CodeGenState& operator = (CodeGenState state) {
        swapState (state);
        return *this;
    }

    ~CodeGenState () { }

    TreeNode* currentNode () const {
        return m_node;
    }

    void setImopInsertPoint (InsertPoint it) {
        m_insertPoint = it;
    }

    SymbolTable* st () const { return m_st; }

    void setSymbolTable (SymbolTable* st) { m_st = st; }

    friend class CodeGen;

private:

    void swapState (CodeGenState& st) {
        std::swap (m_insertPoint, st.m_insertPoint);
        std::swap (m_st, st.m_st);
        std::swap (m_node, st.m_node);
        std::swap (m_allocs, st.m_allocs);
    }

private: /* Fields: */
    InsertPoint   m_insertPoint;  ///< Location before which to insert instructions.
    SymbolTable*  m_st;           ///< Pointer to symbol table of current scope.
    TreeNode*     m_node;         ///< Current tree node.
    AllocList     m_allocs;       ///< Current allocations.
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
private:

    void operator = (const CodeGen&); // DO NOT IMPLEMENT
    CodeGen (const CodeGen&); // DO NOT IMPLEMENT

private: /* Types: */

    typedef std::map<const TreeNodeProcDef*, std::set<Imop*> > CallMap;


public: /* Methods: */

    CodeGen (Context& cxt, ICodeList& code, ICode& icode)
        : CodeGenState (code.end (), &icode.symbols ())
        , m_code (code)
        , m_log (icode.compileLog ())
        , m_modules (icode.modules ())
        , m_tyChecker (icode.symbols (), icode.compileLog (), cxt)
    { }

    ~CodeGen () { }

    void push_imop (Imop* imop) {
        m_code.insert (m_insertPoint, imop);
    }

    Context& getContext () const {
        return m_tyChecker.getContext ();
    }

    void updateTypeChecker () {
        m_tyChecker.setScope (*m_st);
    }

    void swap (CodeGenState& state) {
        swapState (state);
        updateTypeChecker ();
    }

    void setScope (SymbolTable* st) {
        m_st = st;
        updateTypeChecker ();
    }

    void newScope () {
        m_st = m_st->newScope ();
        updateTypeChecker ();
    }

    void popScope () {
        m_st = m_st->parent ();
        updateTypeChecker ();
    }

    Imop* newComment (const std::string& commnet) const;
    Imop* pushComment (const std::string& comment);

    /**
     * \brief Used to push instruction right after code block.
     * \pre imop is not NULL
     * \post first imop is set and next list is empty
     */
    void pushImopAfter (CGResult& result, Imop* imop) {
        assert (imop != 0);
        result.patchFirstImop (imop);
        if (!result.nextList ().empty ())
            result.patchNextList (m_st->label (imop));
        push_imop (imop);
    }

    /**
     * Useful to chain many expressions together if the control flow is linear.
     * Care must be taken when using this to chain branching code together.
     */
    void append (CGResult& result, const CGResult& other) {
        result.patchFirstImop (other.firstImop ());
        // we check for empty next list to avoid creating label
        if (other.firstImop () && !result.nextList ().empty ()) {
            result.patchNextList (m_st->label (other.firstImop ()));
        }

        result.addTempAllocs (other.tempAllocs ());
        result.addToNextList (other.nextList ());
        if (other.isNotOk ()) {
            result.setStatus (other.status ());
        }
    }

    CGResult codeGen (TreeNodeExpr* e);
    CGBranchResult codeGenBranch (TreeNodeExpr* e);
    CGStmtResult codeGenStmt (TreeNodeStmt* s);

    /**
     * \name Top level code.
     * Methods for top level program code generation.
     */
     /// \{
    CGStmtResult cgMainModule (TreeNodeProgram* prog, ModuleInfo* mod);
    CGStmtResult cgModule (TreeNodeModule* mod);
    CGStmtResult cgDomain (TreeNodeDomain* dom);
    CGStmtResult cgKind (TreeNodeKind* kind);
    CGStmtResult cgImport (TreeNodeImport* imp);
    CGStmtResult cgGlobalDecl (TreeNode* decl);
    CGStmtResult cgProcDef (TreeNodeProcDef* def, SymbolTable* localScope);
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
    CGStmtResult cgStmtPush (TreeNodeStmtPush* s);
    CGStmtResult cgStmtPushRef (TreeNodeStmtPushRef* s);
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
    /// \}

    /**
     * \name Expressions.
     * Code generation for regular expressions.
     */
    /// \{
    CGResult cgExprCast (TreeNodeExprCast* e);
    CGResult cgExprBool (TreeNodeExprBool* e);
    CGResult cgExprIndex (TreeNodeExprIndex* e);
    CGResult cgExprShape (TreeNodeExprShape* e);
    CGResult cgExprReshape (TreeNodeExprReshape* e);
    CGResult cgExprCat (TreeNodeExprCat* e);
    CGResult cgExprBinary (TreeNodeExprBinary* e);
    CGResult cgExprProcCall (TreeNodeExprProcCall* e);
    CGResult cgExprRVariable (TreeNodeExprRVariable* e);
    CGResult cgExprDomainID (TreeNodeExprDomainID* e);
    CGResult cgExprQualified (TreeNodeExprQualified* e);
    CGResult cgExprString (TreeNodeExprString* e);
    CGResult cgExprTernary (TreeNodeExprTernary* e);
    CGResult cgExprInt (TreeNodeExprInt* e);
    CGResult cgExprUInt (TreeNodeExprUInt* e);
    CGResult cgExprClassify (TreeNodeExprClassify* e);
    CGResult cgExprDeclassify (TreeNodeExprDeclassify* e);
    CGResult cgExprUnary (TreeNodeExprUnary* e);
    CGResult cgExprPrefix (TreeNodeExprPrefix* e);
    CGResult cgExprPostfix (TreeNodeExprPostfix* e);
    CGResult cgExprSize (TreeNodeExprSize* e);
    CGResult cgExprAssign (TreeNodeExprAssign* e);
    /// \}

    /// Memory management
    /// \{
    void allocResult (CGResult& result, Symbol* val = 0, bool isVariable = false);
    void releaseTempAllocs (CGResult& result, Symbol* ex = 0);
    void addAlloc (SymbolSymbol* sym) { m_allocs.push_back (sym); }
    void releaseLocalAllocs (CGResult& result, Symbol* ex = 0);
    void releaseGlobalAllocs (CGResult& result);
    void clearAllocs () { m_allocs.clear (); }
    /// \}

    /// Looping, and indexing.
    /// \{
    CGResult codeGenSubscript (SubscriptInfo& subInfo, Symbol* x, TreeNode* node);
    CGResult codeGenStride (ArrayStrideInfo& strideInfo);
    CGResult enterLoop (LoopInfo& loopInfo, Symbol* sym);
    CGResult enterLoop (LoopInfo& loopInfo, const SubscriptInfo::SPV& spv);
    CGResult exitLoop (LoopInfo& loopInfo);
    /// \}

    /// Given result computes size of it
    void codeGenSize (CGResult& result);

    /// Copy shape from another symbol
    void copyShapeFrom (CGResult& result, Symbol* sym);

    CGResult cgProcCall (SymbolProcedure* symProc,
                         SecreC::Type* returnType,
                         const std::vector<TreeNodeExpr*>& args);

    /// generate appropriately typed result symbol for given node
    SymbolSymbol* generateResultSymbol (CGResult& result, TreeNodeExpr* node);

    /// generate symbol for given type
    SymbolSymbol* generateResultSymbol (CGResult& result, SecreC::Type* ty);

    CGStmtResult cgVarInit (TypeNonVoid* ty, TreeNodeVarInit* varInit,
                            bool isGlobal, bool isProcParam);

    Symbol* getSizeOr (Symbol* sym, int64_t val);

private: /* Fields: */

    // Components owned by others:
    ICodeList&    m_code;         ///< Generated sequence of IR instructions.
    CompileLog&   m_log;          ///< Compiler log.
    ModuleMap&    m_modules;      ///< Mapping from names to modules.

    // Local components:
    TypeChecker   m_tyChecker;    ///< Instance of type checker.
    CallMap       m_callsTo;      ///< Unpatched procedure calls.
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

} // namespace SecreC

#endif
