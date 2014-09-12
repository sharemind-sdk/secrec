#ifndef SECREC_TYPE_CHECKER_H
#define SECREC_TYPE_CHECKER_H

#include "ParserEnums.h"
#include "SymbolFwd.h"
#include "TreeNodeFwd.h"

#include <vector>

namespace SecreC {

class StringRef;
class CompileLog;
class Context;
class TypeArgument;
class TypeBasic;
class TypeProc;
class Instantiation;
class SecurityType;
class SymbolTable;
class TemplateInstantiator;
class Type;
class TypeContext;
class TypeNonVoid;
struct InstanceInfo;
class Location;
class DataTypeStruct;

#ifndef TCGUARD
#define TCGUARD(expr) \
    do { \
        const Status status = (expr); \
        if (status != OK) \
            return status; \
    } \
    while (false)
#endif

/*******************************************************************************
  TypeChecker
*******************************************************************************/

class TypeChecker {
private:

    TypeChecker (const TypeChecker&) = delete;
    TypeChecker& operator = (const TypeChecker&) = delete;

public: /* Types: */

    enum Status { OK, E_TYPE, E_NOT_IMPLEMENTED, E_OTHER };

public: /* Methods: */

    TypeChecker (SymbolTable& st, CompileLog& log, Context& cxt);
    ~TypeChecker ();

    void setScope (SymbolTable& st) {
        m_st = &st;
    }

    Context& getContext () const {
        return m_context;
    }

    Status visitExpr(TreeNodeExpr * e);

    Status visit(TreeNodeExprArrayConstructor * e);
    Status visit(TreeNodeExprBool * e);
    Status visit(TreeNodeExprCast * root);
    Status visit(TreeNodeExprIndex * root);
    Status visit(TreeNodeExprSize * root);
    Status visit(TreeNodeExprShape * root);
    Status visit(TreeNodeExprCat * root);
    Status visit(TreeNodeExprReshape * root);
    Status visit(TreeNodeExprToString * root);
    Status visit(TreeNodeExprBinary * root);
    Status visit(TreeNodeExprUnary * root);
    Status visit(TreeNodeExprClassify * root);
    Status visit(TreeNodeExprProcCall * root);
    Status visit(TreeNodeExprPrefix * root);
    Status visit(TreeNodeExprPostfix * root);
    Status visit(TreeNodeExprDeclassify * e);
    Status visit(TreeNodeExprRVariable * e);
    Status visit(TreeNodeExprString * e);
    Status visit(TreeNodeExprFloat * e);
    Status visit(TreeNodeExprTernary * e);
    Status visit(TreeNodeExprAssign * e);
    Status visit(TreeNodeExprInt * e);
    Status visit(TreeNodeExprDomainID * e);
    Status visit(TreeNodeExprQualified * e);
    Status visit(TreeNodeExprBytesFromString * e);
    Status visit(TreeNodeExprStringFromBytes * e);
    Status visit(TreeNodeExprSelection * e);

    Status visit(TreeNodeTypeF* ty);
    Status visit(TreeNodeTypeVarF* ty);
    Status visit(TreeNodeSecTypeF * ty);
    Status visit(TreeNodeDimTypeF* ty);
    Status visit(TreeNodeDimTypeVarF * ty);
    Status visit(TreeNodeDataTypeVarF * ty);
    Status visit(TreeNodeDataTypeTemplateF* t);
    Status visit(TreeNodeType * _ty);

    Status visit(TreeNodeTypeArg* t);
    Status visit(TreeNodeTypeArgVar* t);
    Status visit(TreeNodeTypeArgTemplate* t);
    Status visit(TreeNodeTypeArgDataTypeConst* t);
    Status visit(TreeNodeTypeArgDimTypeConst* t);
    Status visit(TreeNodeTypeArgPublic* t);

    Status visit(TreeNodeQuantifier* q);
    Status visit(TreeNodeDimQuantifier*);
    Status visit(TreeNodeDomainQuantifier* q);
    Status visit(TreeNodeDataQuantifier* q);

    Status visit(TreeNodeStmtIf * stmt);
    Status visit(TreeNodeStmtWhile * stmt);
    Status visit(TreeNodeStmtDoWhile * stmt);
    Status visit(TreeNodeStmtDecl * decl);
    Status visit(TreeNodeStmtPrint * stmt);
    Status visit(TreeNodeStmtReturn * stmt);
    Status visit(TreeNodeStmtSyscall * stmt);
    Status visit(TreeNodeStmtAssert * stmt);

    Status visit(TreeNodeStringPart * p);
    Status visit(TreeNodeStringPartFragment * p);
    Status visit(TreeNodeStringPartIdentifier * p);

    Status visit(TreeNodeStructDecl* decl);
    Status visit(TreeNodeProcDef * proc, SymbolTable * localScope);
    Status visit(TreeNodeTemplate * templ);

    Status visit(TreeNodeLValue* lvalue);
    Status visit(TreeNodeLIndex* lindex);
    Status visit(TreeNodeLVariable* lvar);
    Status visit(TreeNodeLSelect* lselect);

    /// \see TemplateInstantiator
    bool getForInstantiation (InstanceInfo&);

    /// Check if given identifier is in scope. Logs error message
    /// and returns NULL if not.
    SymbolSymbol* getSymbol (TreeNodeIdentifier* id);

    /// Return symbol for the main procedure (if exists).
    SymbolProcedure* mainProcedure ();

    Status checkVarInit(TypeNonVoid * ty, TreeNodeVarInit * varInit);

    Status checkPublicBooleanScalar (TreeNodeExpr* e);
private:

    Status checkTypeApplication (TreeNodeIdentifier* id,
                                 TreeNodeSeqView<TreeNodeTypeArg> args,
                                 const Location& loc,
                                 DataTypeStruct*& result);

    Status checkStruct (TreeNodeStructDecl* decl,
                        const Location& loc,
                        DataTypeStruct*& result,
                        const std::vector<TypeArgument>& args
                            = std::vector<TypeArgument>());

    TypeNonVoid* checkSelect (const Location& loc, Type* ty, TreeNodeIdentifier* id);


    TreeNodeExpr* classifyIfNeeded(TreeNodeExpr * child, SecurityType * need);

    Symbol* findIdentifier (SymbolCategory type, const TreeNodeIdentifier* id) const;

    template <SymbolCategory type>
    typename SymbolTraits<type>::Type* findIdentifier (const TreeNodeIdentifier* id) const {
        return static_cast<typename SymbolTraits<type>::Type*>(findIdentifier (type, id));
    }

    Status checkPostfixPrefixIncDec(TreeNodeExpr * root,
                                    bool isPrefix,
                                    bool isInc);
    Status checkIndices(TreeNode * node, SecrecDimType & destDim);
    bool checkAndLogIfVoid (TreeNodeExpr * e);
    Status populateParamTypes(std::vector<TypeBasic *> & params,
                              TreeNodeProcDef * proc);
    Status getInstance(SymbolProcedure *& proc,
                       const Instantiation & inst);

    static bool canPrintValue (Type* ty);

    /**
     * \brief Type check a procedure, and classify parameters if needed.
     * \param[in] name name of the procedure to call
     * \param[in] contextSecType context security type
     * \param[in] arguments argument expressions
     * \param[out] resultType resulting type of the procedure call
     * \param[out] symProc symbol of the procedure which will be called
     */
    Status checkProcCall(TreeNodeIdentifier * name,
                         const TreeNodeExprProcCall & tyCxt,
                         const TreeNodeSeqView<TreeNodeExpr>& arguments,
                         SecreC::Type *& resultType,
                         SymbolProcedure *& symProc);

    // Try to unify template with given parameter types. On success this
    // procedure returns true. No additional side effects are performed.
    bool unify (Instantiation &inst,
                const TypeContext& tyCxt,
                TypeProc* argTypes) const;

    /**
     * \brief Looks for a best matching procedure or template.
     *
     * Raises error if multiple matches are found. If no matches are found
     * returns \a OK, and sets \a symProc to \a NULL.
     *
     * \pre argTypes != NULL
     * \param[in] name name of the procedure/template
     * \param[in] argTypes types of arguments
     * \param[out] symProc best matching procedure if single best one was found
     * \param[in] errorCtx the location about which to print errors.
     */
    Status findBestMatchingProc(SymbolProcedure *& symProc,
                                StringRef name,
                                const TypeContext & tyCxt,
                                TypeProc* argTypes,
                                const TreeNode * errorCxt);

private: /* Fields: */

    SymbolTable*            m_st;
    CompileLog&             m_log;
    Context&                m_context;
    TemplateInstantiator*   m_instantiator;
};

} // namespace SecreC

#endif // TYPE_CHECKER_H