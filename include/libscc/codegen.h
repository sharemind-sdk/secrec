#ifndef CODEGEN_H
#define CODEGEN_H

#include "codegenResult.h"
#include "imop.h"
#include "intermediate.h"
#include "treenode.h"
#include "typechecker.h"

#include <stack>

/**
 * Some ideas:
 * - Represent ICode::Status enum with powers of two, this way
 *   appendWith could just OR the statuses together. In general little
 *   better error reporting facility could be helpful.
 */

namespace SecreC {

class ICodeList;
class SymbolTable;
class CompileLog;

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
    CodeGen& operator = (const CodeGen&); // do not implement

public:

    inline explicit CodeGen (const CodeGen& other)
        : code (other.code)
        , st (other.st)
        , log (other.log)
        , m_node (other.m_node)
        , m_tyChecker (other.m_tyChecker)
    { }

    inline CodeGen (ICodeList &code, SymbolTable &st, CompileLog &log, TypeChecker& tyChecker)
        : code (code)
        , st (&st)
        , log (log)
        , m_node (0)
        , m_tyChecker (tyChecker)
    { }

    inline ~CodeGen () { }

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

    /// Given result computes size of it
    void codeGenSize (CGResult& result);

    void allocResult (CGResult& result);

    /// Copy shape from another symbol
    void copyShapeFrom (CGResult& result, Symbol* sym);

    /// generate appropriately typed result symbol for given node
    SymbolSymbol* generateResultSymbol (CGResult& result, TreeNodeExpr* node);

protected:

    ICodeList&    code;         ///< The code new instructions are emitted to.
    SymbolTable*  st;           ///< Symbol table.
    CompileLog&   log;          ///< Compiler log.
    TreeNode*     m_node;       ///< Current tree node. \todo get rid of it somehow
    TypeChecker&  m_tyChecker;  ///< Instance of type checker.
};

/*******************************************************************************
  CodeGenStride
*******************************************************************************/

/**
 * \brief Code generation of a stride.
 *
 * Given multi-dimensional that has dimensionalities in
 * d_1, d_2, ..., d_n the stride is computed as follows:
 * 1, d_1, d_1 d_2, ..., d_1 d_2 ... d_{n-1}
 *
 * Note that this is stride for column major order.
 */
class CodeGenStride : public CodeGen {
public:
    typedef std::vector<Symbol*> StrideList;

public:
    inline explicit CodeGenStride (CodeGen& base)
        : CodeGen (base) { }

    inline ~CodeGenStride () { }

    unsigned size () const {
        return m_stride.size ();
    }

    Symbol* at (unsigned i) const {
        return m_stride.at (i);
    }

    CGResult codeGenStride (Symbol* sym);

private:
    StrideList m_stride;
};


/*******************************************************************************
  CodeGenLoop
*******************************************************************************/

/**
 * \brief Generate code to loop over entire range of multi-dimensional value or limited slice of one.
 * \code
 * CodeGenLoop loop (*this);
 * append (result, loop.enterLoop (spv, indices));
 * // check result, and generate code for loop body
 * append (result, loop.exitLoop ());
 * return result;
 * \endcode
 */
class CodeGenLoop : public CodeGen {
public:
    typedef std::vector<std::pair<Symbol*, Symbol* > > SPV;
    typedef std::vector<Symbol* > IndexList;

public:

    inline explicit CodeGenLoop (const CodeGen& base)
        : CodeGen (base)
    { }

    inline ~CodeGenLoop () {
        assert (m_jumpStack.empty () && "The loop was destroyed before exiting!");
    }

    /// loop over entire range
    CGResult enterLoop (Symbol* sym, const IndexList& indices);

    /// loop over limited range
    CGResult enterLoop (const SPV& spv, const IndexList& indices);

    /// exit the loop
    CGResult exitLoop (const IndexList& indices);

private:
    std::stack<Imop* > m_jumpStack;  ///< Jump stack. Entering loop builds it, exiting loop consumes it.
};

/*******************************************************************************
  CodeGenSubscript
*******************************************************************************/

/**
 * \brief Code generation of a subscript (general indexing).
 */
class CodeGenSubscript : public CodeGen {
public:
    typedef std::vector<unsigned> SliceIndices;
    typedef std::vector<std::pair<Symbol*, Symbol* > > SPV;
public:

    inline explicit CodeGenSubscript (const CodeGen& base)
        : CodeGen (base)
    { }

    inline ~CodeGenSubscript () { }

    const SliceIndices& slices () const {
        return m_slices;
    }

    const SPV& spv () const {
        return m_spv;
    }

    CGResult codeGenSubscript (Symbol* x, TreeNode* node);

private:
    SliceIndices m_slices;  ///< Specifies which indices are slices.
    SPV          m_spv;     ///< List of ranges for every index.
};

}

#endif
