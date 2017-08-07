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

#ifndef SECREC_TYPE_CHECKER_H
#define SECREC_TYPE_CHECKER_H

#include "ParserEnums.h"
#include "SymbolFwd.h"
#include "TreeNodeFwd.h"
#include "TypeArgument.h"

#include <utility>
#include <vector>

namespace SecreC {

class CompileLog;
class Context;
class DataTypeStruct;
class Instantiation;
class Location;
class OperatorTable;
class OverloadableOperator;
class SecurityType;
class StringRef;
class SymbolTable;
class TemplateInstantiator;
class Type;
class TypeBasic;
class TypeContext;
class TypeNonVoid;
class TypeProc;
struct InstanceInfo;

#ifndef TCGUARD
#define TCGUARD(expr) \
    do { \
        const Status status = (expr); \
        if (status != OK) \
            return status; \
    } \
    while (false)
#endif

const TypeBasic* upperTypeBasic(const TypeBasic* a, const TypeBasic* b);

/*******************************************************************************
  TypeChecker
*******************************************************************************/

class TypeChecker {
public: /* Types: */

    enum Status { OK, E_TYPE, E_NOT_IMPLEMENTED, E_OTHER };

    using result_type = Status;

public: /* Methods: */

    TypeChecker (OperatorTable& ops, SymbolTable& st, CompileLog& log, Context& cxt);
    ~TypeChecker ();
    TypeChecker (const TypeChecker&) = delete;
    TypeChecker& operator = (const TypeChecker&) = delete;

    void setScope (SymbolTable& st) {
        m_st = &st;
    }

    Context& getContext () const {
        return m_context;
    }

    Status visitExpr(TreeNodeExpr * e);

    Status visitExprArrayConstructor(TreeNodeExprArrayConstructor * e);
    Status visitExprAssign(TreeNodeExprAssign * e);
    Status visitExprBinary(TreeNodeExprBinary * root);
    Status visitExprBool(TreeNodeExprBool * e);
    Status visitExprBytesFromString(TreeNodeExprBytesFromString * e);
    Status visitExprCast(TreeNodeExprCast * root);
    Status visitExprCat(TreeNodeExprCat * root);
    Status visitExprClassify(TreeNodeExprClassify * root);
    Status visitExprDeclassify(TreeNodeExprDeclassify * e);
    Status visitExprDomainID(TreeNodeExprDomainID * e);
    Status visitExprFloat(TreeNodeExprFloat * e);
    Status visitExprIndex(TreeNodeExprIndex * root);
    Status visitExprInt(TreeNodeExprInt * e);
    Status visitExprNone(TreeNodeExprNone* e);
    Status visitExprPostfix(TreeNodeExprPostfix * root);
    Status visitExprPrefix(TreeNodeExprPrefix * root);
    Status visitExprProcCall(TreeNodeExprProcCall * root);
    Status visitExprQualified(TreeNodeExprQualified * e);
    Status visitExprRVariable(TreeNodeExprRVariable * e);
    Status visitExprReshape(TreeNodeExprReshape * root);
    Status visitExprSelection(TreeNodeExprSelection * e);
    Status visitExprShape(TreeNodeExprShape * root);
    Status visitExprSize(TreeNodeExprSize * root);
    Status visitExprString(TreeNodeExprString * e);
    Status visitExprStringFromBytes(TreeNodeExprStringFromBytes * e);
    Status visitExprStrlen(TreeNodeExprStrlen* e);
    Status visitExprTernary(TreeNodeExprTernary * e);
    Status visitExprToString(TreeNodeExprToString * root);
    Status visitExprUnary(TreeNodeExprUnary * root);

    Status visitType(TreeNodeType * _ty);
    Status visitTypeVarF(TreeNodeTypeVarF* ty);
    Status visitDataTypeF(TreeNodeDataTypeF* ty, const SecurityType* secType);
    Status visitDataTypeConstF(TreeNodeDataTypeConstF * ty);
    Status visitDataTypeConstF(TreeNodeDataTypeConstF * ty, const SecurityType* secType);
    Status visitDataTypeTemplateF(TreeNodeDataTypeTemplateF* t);
    Status visitDataTypeTemplateF(TreeNodeDataTypeTemplateF* t, const SecurityType* secType);
    Status visitDataTypeVarF(TreeNodeDataTypeVarF * ty, const SecurityType* secType = nullptr);
    Status visitDimTypeConstF(TreeNodeDimTypeConstF *);
    Status visitDimTypeF(TreeNodeDimTypeF* ty);
    Status visitDimTypeVarF(TreeNodeDimTypeVarF * ty);
    Status visitSecTypeF(TreeNodeSecTypeF * ty);
    Status visitTypeF(TreeNodeTypeF* ty);

    Status visitTypeArg(TreeNodeTypeArg* t);
    Status visitTypeArgVar(TreeNodeTypeArgVar* t);
    Status visitTypeArgTemplate(TreeNodeTypeArgTemplate* t);
    Status visitTypeArgDataTypeConst(TreeNodeTypeArgDataTypeConst* t);
    Status visitTypeArgDimTypeConst(TreeNodeTypeArgDimTypeConst* t);
    Status visitTypeArgPublic(TreeNodeTypeArgPublic* t);

    Status visitQuantifier(TreeNodeQuantifier* q);
    Status visitQuantifierDim(TreeNodeQuantifierDim*);
    Status visitQuantifierData(TreeNodeQuantifierData* q);
    Status visitQuantifierDomain(TreeNodeQuantifierDomain* q);

    Status visitStmt(TreeNodeStmt * stmt);
    Status visitStmtIf(TreeNodeStmtIf * stmt);
    Status visitStmtWhile(TreeNodeStmtWhile * stmt);
    Status visitStmtDoWhile(TreeNodeStmtDoWhile * stmt);
    Status visitStmtDecl(TreeNodeStmtDecl * decl);
    Status visitStmtPrint(TreeNodeStmtPrint * stmt);
    Status visitStmtReturn(TreeNodeStmtReturn * stmt);
    Status visitStmtSyscall(TreeNodeStmtSyscall * stmt);
    Status visitStmtAssert(TreeNodeStmtAssert * stmt);

    Status visitStringPart(TreeNodeStringPart * p);
    Status visitStringPartFragment(TreeNodeStringPartFragment * p);
    Status visitStringPartIdentifier(TreeNodeStringPartIdentifier * p);

    Status visitStructDecl(TreeNodeStructDecl* decl);
    Status visitProcDef(TreeNodeProcDef * proc, SymbolTable * localScope, bool append=true);
    Status visitOpDef(TreeNodeOpDef * def, SymbolTable * localScope, bool append=true);
    Status visitCastDef(TreeNodeCastDef * def, SymbolTable * localScope, bool append=true);
    Status visitTemplate(TreeNodeTemplate * templ);

    Status visitLValue(TreeNodeLValue* lvalue);
    Status visitLIndex(TreeNodeLIndex* lindex);
    Status visitLVariable(TreeNodeLVariable* lvar);
    Status visitLSelect(TreeNodeLSelect* lselect);

    /// \see TemplateInstantiator
    bool getForInstantiation (InstanceInfo&);

    /// Check if given identifier is in scope. Logs error message
    /// and returns NULL if not.
    SymbolSymbol* getSymbol (TreeNodeIdentifier* id);

    /// Return symbol for the main procedure (if exists).
    SymbolProcedure* mainProcedure ();

    Status checkVarInit(const TypeNonVoid * ty, TreeNodeVarInit * varInit);

    Status checkPublicBooleanScalar (TreeNodeExpr* e);

private: /* Methods: */

    Status checkTypeApplication (TreeNodeIdentifier* id,
                                 TreeNodeSeqView<TreeNodeTypeArg> args,
                                 const Location& loc,
                                 const DataTypeStruct*& result);

    Status checkStruct (TreeNodeStructDecl* decl,
                        const Location& loc,
                        const DataTypeStruct*& result,
                        const std::vector<TypeArgument>& args
                            = std::vector<TypeArgument>());

    const TypeNonVoid* checkSelect (const Location& loc, const Type* ty, TreeNodeIdentifier* id);


    TreeNodeExpr* classifyIfNeeded(TreeNodeExpr * child, const SecurityType * need);

    Symbol* findIdentifier (SymbolCategory type, const TreeNodeIdentifier* id) const;

    template <SymbolCategory type>
    typename SymbolTraits<type>::Type* findIdentifier (const TreeNodeIdentifier* id) const {
        return static_cast<typename SymbolTraits<type>::Type*>(findIdentifier (type, id));
    }

    Status checkPostfixPrefixIncDec(TreeNodeExpr * root,
                                    OverloadableOperator * op,
                                    bool isPrefix,
                                    bool isInc);
    Status checkIndices(TreeNode * node, SecrecDimType & destDim);
    bool checkAndLogIfVoid (TreeNodeExpr * e);
    Status populateParamTypes(std::vector<const TypeBasic *> & params,
                              TreeNodeProcDef * proc);
    Status getInstance(SymbolProcedure *& proc,
                       const Instantiation & inst);

    static bool canPrintValue (const Type* ty);

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
                         const TreeNodeSeqView<TreeNodeExpr> & arguments,
                         const SecreC::Type *& resultType,
                         SymbolProcedure *& symProc);

    // Try to unify template with given parameter types. On success this
    // procedure returns true. No additional side effects are performed.
    bool unify (Instantiation & inst,
                const TypeContext & tyCxt,
                const TypeProc * argTypes) const;

    bool unifyOperator (Instantiation & inst,
                        const TypeContext & tyCxt,
                        const TypeProc * argTypes) const;

    bool unifyCast (Instantiation & inst,
                    const TypeBasic * arg,
                    const TypeBasic * want) const;

    Status findRegularOpDef(SymbolProcedure *& symProc,
                            StringRef name,
                            const TypeProc * callTypeProc,
                            const TreeNode * errorCxt);

    Status findRegularProc(SymbolProcedure *& symProc,
                           StringRef name,
                           const TypeContext & tyCxt,
                           const TypeProc * argTypes,
                           const TreeNode * errorCxt);

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
                                const TypeProc* argTypes,
                                const TreeNode * errorCxt);

    Status findBestMatchingOpDef(SymbolProcedure *& symProc,
                                 StringRef name,
                                 const TypeContext & tyCxt,
                                 const TypeProc * callTypeProc,
                                 const TreeNode * errorCxt);

    Status findBestMatchingCastDef(SymbolProcedure *& symProc,
                                   const TypeBasic * arg,
                                   const TypeBasic * want,
                                   const TreeNode * errorCxt);

    Status checkRedefinitions(const TreeNodeProcDef& proc);

private: /* Fields: */

    OperatorTable*        m_operators;
    SymbolTable*          m_st;
    CompileLog&           m_log;
    Context&              m_context;
    TemplateInstantiator* m_instantiator;
};

} // namespace SecreC

#endif // TYPE_CHECKER_H
