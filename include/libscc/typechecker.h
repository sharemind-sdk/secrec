#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "log.h"
#include "types.h"
#include "symboltable.h"
#include "treenode.h"

namespace SecreC {

/*******************************************************************************
  TypeChecker
*******************************************************************************/

class TypeChecker {
public: /* Methods: */

    TypeChecker (SymbolTable& st, CompileLog& log)
        : m_st (&st)
        , m_log (log)
    { }

    ~TypeChecker () { }

    void setScope (SymbolTable& st) {
        m_st = &st;
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


protected:

    /// \todo write more and better utility methods:

    ICode::Status checkPostfixPrefixIncDec (TreeNodeExpr* root, bool isPrefix, bool isInc);
    TreeNodeExpr* classifyIfNeeded (TreeNode* node, unsigned index, const Type& ty);
    ICode::Status checkIndices (TreeNode* node, unsigned& destDim);
    bool checkAndLogIfVoid (TreeNodeExpr* e);

    inline static TypeVoid* voidTy () {
        return new TypeVoid ();
    }

    inline static TypeNonVoid* publicTy (SecrecDataType dTy, SecrecDimType dimTy = 0) {
        return new TypeNonVoid (dTy, dimTy);
    }

    inline static TypeNonVoid* publicBoolTy (SecrecDimType dimTy = 0)  {
        return publicTy (DATATYPE_BOOL, dimTy);
    }


private: /* Types: */

    SymbolTable*    m_st;
    CompileLog&     m_log;
};

}

#endif // TYPE_CHECKER_H
