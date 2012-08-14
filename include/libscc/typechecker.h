#ifndef SECREC_TYPE_CHECKER_H
#define SECREC_TYPE_CHECKER_H

#include "treenode.h"

namespace SecreC {

struct InstanceInfo;
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
    ICode::Status visit (TreeNodeExprCast* root);
    ICode::Status visit (TreeNodeExprIndex* root);
    ICode::Status visit (TreeNodeExprSize* root);
    ICode::Status visit (TreeNodeExprShape* root);
    ICode::Status visit (TreeNodeExprCat* root);
    ICode::Status visit (TreeNodeExprReshape* root);
    ICode::Status visit (TreeNodeExprToString* root);
    ICode::Status visit (TreeNodeExprBinary* root);
    ICode::Status visit (TreeNodeExprUnary* root);
    ICode::Status visit (TreeNodeExprClassify* root);
    ICode::Status visit (TreeNodeExprProcCall* root);
    ICode::Status visit (TreeNodeExprPrefix* root);
    ICode::Status visit (TreeNodeExprPostfix* root);
    ICode::Status visit (TreeNodeExprDeclassify* e);
    ICode::Status visit (TreeNodeExprRVariable* e);
    ICode::Status visit (TreeNodeExprString* e);
    ICode::Status visit (TreeNodeExprFloat* e);
    ICode::Status visit (TreeNodeExprTernary* e);
    ICode::Status visit (TreeNodeExprAssign* e);
    ICode::Status visit (TreeNodeExprInt* e);
    ICode::Status visit (TreeNodeExprDomainID* e);
    ICode::Status visit (TreeNodeExprQualified* e);
    ICode::Status visit (TreeNodeExprBytesFromString* e);
    ICode::Status visit (TreeNodeExprStringFromBytes* e);

    ICode::Status visit (TreeNodeSecTypeF* ty);
    ICode::Status visit (TreeNodeType* _ty);

    ICode::Status visit (TreeNodeStmtDecl* decl);
    ICode::Status visit (TreeNodeStmtPrint* stmt);
    ICode::Status visit (TreeNodeStmtReturn* stmt);

    ICode::Status visit (TreeNodeProcDef* proc, SymbolTable* localScope);
    ICode::Status visit (TreeNodeTemplate* templ);

    /// \see TemplateInstantiator
    bool getForInstantiation (InstanceInfo&);
    TreeNodeExpr* classifyIfNeeded (TreeNodeExpr* child, SecurityType* need = 0);

    /// Check if given idenfier is in scope. Logs error message
    /// and returns NULL if not.
    SymbolSymbol* getSymbol (TreeNodeIdentifier* id);

    /// Return symbol for the main procedure (if exists).
    SymbolProcedure* mainProcedure ();

    ICode::Status checkVarInit (TypeNonVoid* ty, TreeNodeVarInit* varInit);


protected:

    /// Check if given idenfier is in scope.
    /// Logs error message and returns NULL if not.
    Symbol* findIdentifier (TreeNodeIdentifier* id) const;

    ICode::Status checkPostfixPrefixIncDec (TreeNodeExpr* root,
                                            bool isPrefix,
                                            bool isInc);
    ICode::Status checkIndices (TreeNode* node, SecrecDimType& destDim);
    bool checkAndLogIfVoid (TreeNodeExpr* e);
    ICode::Status populateParamTypes (std::vector<DataType*>& params,
                                      TreeNodeProcDef* proc);
    ICode::Status getInstance (SymbolProcedure*& proc,
                               const Instantiation& inst);

    ICode::Status checkParams (const std::vector<TreeNodeExpr*>& arguments,
                               DataTypeProcedureVoid*& argTypes);


    ICode::Status checkProcCall (SymbolProcedure* symProc,
                                 DataTypeProcedureVoid* argTypes,
                                 SecreC::Type*& resultType);

    /**
     * \brief Type check a procedure, and classify parameters if needed.
     * \param[in] name name of the procedure to call
     * \param[in] contextSecType context security type
     * \param[in] arguments argument expressions
     * \param[out] resultType resulting type of the procedure call
     * \param[out] symProc symbol of the procedure which will be called
     */
    ICode::Status checkProcCall (TreeNodeIdentifier* name,
                                 const TypeContext& tyCxt,
                                 const std::vector<TreeNodeExpr*>& arguments,
                                 SecreC::Type*& resultType,
                                 SymbolProcedure*& symProc);

    // Try to unify template with given parameter types. On success this
    // procedure returns true, and gives bindings to quantifiers. No
    // additional side effects are performed.
    bool unify (Instantiation& inst,
                const TypeContext& tyCxt,
                DataTypeProcedureVoid* argTypes) const;

    /**
     * \brief Looks for a best matching procedure or template.
     *
     * Raises error if multiple matches are found. If no matches are found
     * returns \a OK, and sets \a symProc to \a NULL.
     *
     * \pre argTypes != 0
     * \param[in] name name of the procedure/template
     * \param[in] argTypes types of arguments
     * \param[out] symProc best matching procedure if single best one was found
     */
    ICode::Status findBestMatchingProc (SymbolProcedure*& symProc,
                                        const std::string& name,
                                        const TypeContext& tyCxt,
                                        DataTypeProcedureVoid* argTypes);

private: /* Fields: */

    SymbolTable*            m_st;
    CompileLog&             m_log;
    Context&                m_context;
    TemplateInstantiator*   m_instantiator;
};

} // namespace SecreC

#endif // TYPE_CHECKER_H
