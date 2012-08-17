#ifndef SECREC_CODE_GEN_H
#define SECREC_CODE_GEN_H

#include <map>
#include <set>
#include <vector>

#include "codegenResult.h"
#include "CodeGenState.h"
#include "icodelist.h"
#include "treenode_fwd.h"

namespace SecreC {

class CodeGen;
class CompileLog;
class Context;
class ICode;
class ICodeList;
class Imop;
class ModuleInfo;
class ModuleMap;
class SymbolSymbol;
class SymbolProcedure;
class SymbolTable;
class Type;
class TypeChecker;
class TypeNonVoid;

/*******************************************************************************
  SubscriptInfo
*******************************************************************************/

class SubscriptInfo {
public: /* Types: */
    typedef std::vector<unsigned> SliceIndices;
    typedef std::vector<std::pair<Symbol*, Symbol* > > SPV;

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

    struct LoopCheck {
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

    void pushJump (Imop* test, Imop* jump) {
        LoopCheck temp = { test, jump };
        m_jumpStack.push_back (temp);
    }

    bool empty () const { return m_jumpStack.empty (); }
    LoopCheck top () const { return m_jumpStack.back (); }
    void pop () { return m_jumpStack.pop_back ();}

private: /* Fields: */
    IndexList                m_indices;
    std::vector<LoopCheck >  m_jumpStack;
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
private:

    void operator = (const CodeGen&); // DO NOT IMPLEMENT
    CodeGen (const CodeGen&); // DO NOT IMPLEMENT

private: /* Types: */

    typedef std::map<const TreeNodeProcDef*, std::set<Imop*> > CallMap;
    typedef std::vector<SymbolTable*> STList;

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
    /// \}

    /// Memory management
    /// \{
    void allocTemporaryResult (CGResult& result, Symbol* val = 0);
    void initSymbol (CGResult& result, Symbol* sym, Symbol* def = 0);
    void releaseResource (CGResult& result, Symbol* sym);
    void releaseTemporary (CGResult& result, Symbol* sym);
    void releaseScopeVariables (CGResult& result);
    void releaseProcVariables (CGResult& result, Symbol* ex = 0);
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

private:

    Context& getContext () const {
        return m_context;
    }

    void swap (CodeGenState& state) {
        swapState (state);
        updateTypeChecker ();
    }

    void setScope (SymbolTable* st) {
        m_st = st;
        updateTypeChecker ();
    }

    inline void push_imop (Imop* imop) {
        m_code.insert (m_insertPoint, imop);
    }

    void updateTypeChecker ();
    void newScope ();
    void popScope ();

    Imop* newComment (const std::string& commnet) const;
    Imop* pushComment (const std::string& comment);

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

    Symbol* getSizeOr (Symbol* sym, uint64_t val);
    Symbol* indexConstant (uint64_t value);

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
