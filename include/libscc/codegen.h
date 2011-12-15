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
class CodeGen {
private:

    void operator = (const CodeGen&); // DO NOT IMPLEMENT
    CodeGen (const CodeGen&); // DO NOT IMPLEMENT

private: /* Types: */

    typedef std::map<const TreeNodeProcDef*, std::set<Imop*> > CallMap;
    typedef std::list<SymbolSymbol*> AllocList;

public: /* Methods: */

    inline CodeGen (ICodeList &code,
                    SymbolTable& st,
                    CompileLog& log,
                    TypeChecker& tyChecker)
        : code (code)
        , st (&st)
        , log (log)
        , m_node (0)
        , m_tyChecker (tyChecker)
    { }

    inline ~CodeGen () { }

    TreeNode* currentNode () const {
        return m_node;
    }

    inline Context& getContext () const {
        return m_tyChecker.getContext ();
    }

    void newScope () {
        st = st->newScope ();
        m_tyChecker.setScope (*st);
    }

    void popScope () {
        st = st->parent ();
        m_tyChecker.setScope (*st);
    }

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
            result.patchNextList (st->label (imop));
        code.push_imop (imop);
    }

    /**
     * Useful to chain many expressions together if the control flow is linear.
     * Care must be taken when using this to chain branching code together.
     */
    void append (CGResult& result, const CGResult& other) {
        result.patchFirstImop (other.firstImop ());
        // we check for empty next list to avoid creating label
        if (other.firstImop () && !result.nextList ().empty ()) {
            result.patchNextList (st->label (other.firstImop ()));
        }

        result.addTempAllocs (other.tempAllocs ());
        result.addToNextList (other.nextList ());
        if (other.isNotOk ()) {
            result.setStatus (other.status ());
        }
    }

    inline CGResult codeGen (TreeNodeExpr* e) {
        TreeNode* const oldNode = m_node;
        m_node = e;
        const CGResult& r (e->codeGenWith (*this));
        m_node = oldNode;
        return r;
    }

    inline CGBranchResult codeGenBranch (TreeNodeExpr* e) {
        TreeNode* oldNode = m_node;
        m_node = e;
        const CGBranchResult& r (e->codeGenBoolWith (*this));
        m_node = oldNode;
        return r;
    }

    CGStmtResult codeGenStmt (TreeNodeStmt* e) {
        TreeNode* oldNode = m_node;
        m_node = e;
        const CGStmtResult& r (e->codeGenWith (*this));
        m_node = oldNode;
        return r;
    }

    /**
     * \name Top level code.
     * Methods for top level program code generation.
     */
     /// \{
    CGStmtResult cgProgram (TreeNodeProgram* prog);
    CGStmtResult cgProcDef (TreeNodeProcDef* def);
    CGStmtResult cgDomain (TreeNodeDomain* dom);
    CGStmtResult cgKind (TreeNodeKind* kind);
    CGStmtResult cgGlobalDecl (TreeNode* decl);
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
    CGBranchResult cgBoolExprCast (TreeNodeExprCast* e);
    CGBranchResult cgBoolExprUnary (TreeNodeExprUnary* e);
    CGBranchResult cgBoolExprIndex (TreeNodeExprIndex* e);
    CGBranchResult cgBoolExprBinary (TreeNodeExprBinary* e);
    CGBranchResult cgBoolExprProcCall (TreeNodeExprProcCall* e);
    CGBranchResult cgBoolExprRVariable (TreeNodeExprRVariable* e);
    CGBranchResult cgBoolExprTernary (TreeNodeExprTernary* e);
    CGBranchResult cgBoolExprBool (TreeNodeExprBool* e);
    CGBranchResult cgBoolExprDeclassify (TreeNodeExprDeclassify* e);
    CGBranchResult cgBoolExprAssign (TreeNodeExprAssign* e);
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

    /// generate appropriately typed result symbol for given node
    SymbolSymbol* generateResultSymbol (CGResult& result, TreeNodeExpr* node);

protected: /* Fields: */

    ICodeList&    code;         ///< The code new instructions are emitted to.
    SymbolTable*  st;           ///< Symbol table.
    CompileLog&   log;          ///< Compiler log.
    TreeNode*     m_node;       ///< Current tree node. \todo get rid of it somehow
    TypeChecker&  m_tyChecker;  ///< Instance of type checker.
    CallMap       m_callsTo;    ///< Map
    AllocList     m_allocs;
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

}

#endif
