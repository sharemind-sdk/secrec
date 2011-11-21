#ifndef SECREC_TYPE_CHECKER_H
#define SECREC_TYPE_CHECKER_H

#include "treenode.h"

namespace SecreC {

class CompileLog;
class SymbolTable;

/*******************************************************************************
  TypeChecker
*******************************************************************************/

class TypeChecker {
private:

    TypeChecker (const TypeChecker&); // DO NOT IMPLEMENT
    void operator = (const TypeChecker&); // DO NOT IMPLEMENT

public: /* Methods: */

    TypeChecker (SymbolTable& st, CompileLog& log, Context& cxt)
        : m_st (&st)
        , m_log (log)
        , m_context (cxt)
    { }

    ~TypeChecker () { }

    void setScope (SymbolTable& st) {
        m_st = &st;
    }

    Context& getContext () const {
        return m_context;
    }

    ICode::Status visitExpr (TreeNodeExpr* e) {
        return e->accept (*this);
    }

    ICode::Status visit (TreeNodeExprBool* e);
    ICode::Status visit (TreeNodeExprUInt* e);
    ICode::Status visit (TreeNodeExprCast* root);
    ICode::Status visit (TreeNodeExprIndex* root);
    ICode::Status visit (TreeNodeExprSize* root);
    ICode::Status visit (TreeNodeExprShape* root);
    ICode::Status visit (TreeNodeExprCat* root);
    ICode::Status visit (TreeNodeExprReshape* root);
    ICode::Status visit (TreeNodeExprBinary* root);
    ICode::Status visit (TreeNodeExprUnary* root);
    ICode::Status visit (TreeNodeExprClassify* root);
    ICode::Status visit (TreeNodeExprProcCall* root);
    ICode::Status visit (TreeNodeExprPrefix* root);
    ICode::Status visit (TreeNodeExprPostfix* root);
    ICode::Status visit (TreeNodeExprDeclassify* e);
    ICode::Status visit (TreeNodeExprRVariable* e);
    ICode::Status visit (TreeNodeExprString* e);
    ICode::Status visit (TreeNodeExprTernary* e);
    ICode::Status visit (TreeNodeExprAssign* e);
    ICode::Status visit (TreeNodeExprInt* e);

    ICode::Status visit (TreeNodeProcDef* proc);
    ICode::Status visit (TreeNodeStmtDecl* decl);
    ICode::Status visit (TreeNodeType* _ty);
    ICode::Status visit (TreeNodeStmtPrint* stmt);
    ICode::Status visit (TreeNodeStmtReturn* stmt);

    ICode::Status visit (TreeNodeTemplate* templ);

    TreeNodeExpr* classifyIfNeeded (TreeNode* node, unsigned index, Type* ty);

    /// TreeNode* findImpl (std::string& name, const std::vector<TypeNonVoid*>& argTys);
    bool match (TreeNodeTemplate* n, const std::vector<TypeNonVoid*>& argTys);

protected:

    /// \todo write more and better utility methods:

    ICode::Status checkPostfixPrefixIncDec (TreeNodeExpr* root, bool isPrefix, bool isInc);
    ICode::Status checkIndices (TreeNode* node, unsigned& destDim);
    bool checkAndLogIfVoid (TreeNodeExpr* e);
    ICode::Status populateParamTypes (std::vector<DataType*>& params, TreeNodeProcDef* proc);

private: /* Fields: */

    SymbolTable*    m_st;
    CompileLog&     m_log;
    Context&        m_context;
};

}

#endif // TYPE_CHECKER_H
