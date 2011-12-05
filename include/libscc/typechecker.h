#ifndef SECREC_TYPE_CHECKER_H
#define SECREC_TYPE_CHECKER_H

#include "treenode.h"

namespace SecreC {

class CompileLog;
class SymbolTable;
class Instantiation;
class TemplateInstantiator;

/*******************************************************************************
  TypeChecker
*******************************************************************************/

class TypeChecker {
private:

    TypeChecker (const TypeChecker&); // DO NOT IMPLEMENT
    void operator = (const TypeChecker&); // DO NOT IMPLEMENT

public: /* Methods: */

    TypeChecker (SymbolTable& st, CompileLog& log, Context& cxt);
    ~TypeChecker ();

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

    /// \see TemplateInstantiator
    bool getForInstantiation (TreeNodeProcDef*& proc, SymbolTable*& st);
    TreeNodeExpr* classifyIfNeeded (TreeNodeExpr* child);

    /// Check if given idenfier is in scope. Logs error message
    /// and returns NULL if not.
    SymbolSymbol* getSymbol (TreeNodeIdentifier* id);

    /// Return symbol for the main procedure (if exists).
    SymbolProcedure* mainProcedure ();


protected:

    /// Check if given idenfier is in scope.
    /// Logs error message and returns NULL if not.
    Symbol* findIdentifier (TreeNodeIdentifier* id) const;

    ICode::Status checkPostfixPrefixIncDec (TreeNodeExpr* root,
                                            bool isPrefix,
                                            bool isInc);
    ICode::Status checkIndices (TreeNode* node, unsigned& destDim);
    bool checkAndLogIfVoid (TreeNodeExpr* e);
    ICode::Status populateParamTypes (std::vector<DataType*>& params,
                                      TreeNodeProcDef* proc);
    ICode::Status getInstance (SymbolProcedure*& proc,
                               const Instantiation& inst);

    // Try to unify template with given parameter types. On success this
    // procedure returns true, and gives bindings to quantifiers. No
    // addition side effect are performed.
    bool unify (Instantiation& inst, DataTypeProcedureVoid* argTypes) const;

    /**
     * \pre argTypes != 0
     * \post symProc != 0 if returned status is OK
     * \param[in] name name of the procedure/template
     * \param[in] argTypes types of arguments
     * \param[out] symProc best matching procedure if single best one was found
     * \retval ICode::OK if best matching procedure was found
     */
    ICode::Status findBestMatchingProc (SymbolProcedure*& symProc,
                                        const std::string& name,
                                        SecurityType* contextTy,
                                        DataTypeProcedureVoid* argTypes);

private: /* Fields: */

    SymbolTable*            m_st;
    CompileLog&             m_log;
    Context&                m_context;
    TemplateInstantiator*   m_instantiator;
};

}

#endif // TYPE_CHECKER_H
