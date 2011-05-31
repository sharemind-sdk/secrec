#include "treenode.h"
#include "symboltable.h"
#include "misc.h"

/**
 * Code generation for statements.
 */

namespace SecreC {

/*******************************************************************************
  TreeNodeStmtCompound
*******************************************************************************/

ICode::Status TreeNodeStmtCompound::generateCode(ICodeList &code,
                                                 SymbolTable &st,
                                                 CompileLog &log)
{
    typedef ChildrenListConstIterator CLCI;

    TreeNodeStmt *last = 0;
    setResultFlags(TreeNodeStmt::FALLTHRU);

    SymbolTable& innerScope = *st.newScope();

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert(dynamic_cast<TreeNodeStmt*>(*it) != 0);
        TreeNodeStmt *c = static_cast<TreeNodeStmt*>(*it);
        ICode::Status s = c->generateCode(code, innerScope, log);
        if (s != ICode::OK) return s;

        assert(c->resultFlags() != 0x0);

        if (c->firstImop() == 0) {
            if (c->type() != NODE_DECL) {
                log.fatal() << "Statement with no effect at " << c->location();
                return ICode::E_OTHER;
            }
        } else {
            patchFirstImop (c->firstImop ());
        }

        addToBreakList(c->breakList());
        addToContinueList(c->continueList());
        if (last != 0) {
            // Static checking:
            if ((resultFlags() & TreeNodeStmt::FALLTHRU) == 0x0) {
                log.fatal() << "Unreachable statement at " << c->location();
                return ICode::E_OTHER;
            } else {
                setResultFlags((resultFlags() & ~TreeNodeStmt::FALLTHRU)
                               | c->resultFlags());
            }

            last->patchNextList(c->firstImop(), st);
        } else {
            setResultFlags(c->resultFlags());
        }
        last = c;
    }
    if (last != 0) {
        addToNextList(last->nextList());
    }

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtBreak
*******************************************************************************/

ICode::Status TreeNodeStmtBreak::generateCode(ICodeList &code, SymbolTable &,
                                              CompileLog &)
{
    Imop *i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    setFirstImop(i);
    addToBreakList(i);
    setResultFlags(TreeNodeStmt::BREAK);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtContinue
*******************************************************************************/

ICode::Status TreeNodeStmtContinue::generateCode(ICodeList &code, SymbolTable &,
                                                 CompileLog &)
{
    Imop *i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    setFirstImop(i);
    addToContinueList(i);
    setResultFlags(TreeNodeStmt::CONTINUE);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtDecl
*******************************************************************************/

/*
 * We have 6 cases depending on wether shape is present and if
 * right hand side is scalar or array. In addition we have to take account that
 * RHS may be missing. The cases are as follows:
 * - type x = scalar;
 * - type x = array;
 * - type x shape = scalar;
 * - type x shape = array;
 * - type x;
 * - type x shape;
 */
ICode::Status TreeNodeStmtDecl::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log)
{
    assert(children().size() > 0 && children().size() <= 4);

    ICode::Status s = calculateResultType(st, log);
    if (s != ICode::OK) return s;

    // Initialize the new symbol (for initializer target)
    SymbolSymbol *ns = new SymbolSymbol(resultType());
    SymbolSymbol::ScopeType scopeType = m_global ? SymbolSymbol::GLOBAL : SymbolSymbol::LOCAL;
    ns->setScopeType(scopeType);
    ns->setName(variableName());

    bool isScalar = resultType().isScalar();
    unsigned n = 0;

    // Initialize temporaries for all variables corresponding to shape
    TypeNonVoid const& dimType = TypeNonVoid(DataTypeVar(DataTypeBasic(SECTYPE_PUBLIC, DATATYPE_INT)));
    for (unsigned i = 0; i < resultType().secrecDimType(); ++ i) {
        SymbolSymbol* sym = new SymbolSymbol(dimType);
        sym->setScopeType(scopeType);
        std::stringstream ss;
        ss << variableName() << "{d" << i << "}";
        sym->setName(ss.str());
        st.appendSymbol(sym);
        ns->setDim(i, sym);
    }

    { // set size symbol
        SymbolSymbol* sizeSym = new SymbolSymbol(dimType);
        sizeSym->setScopeType(scopeType);
        std::stringstream ss;
        ss << variableName() << "{size}";
        sizeSym->setName(ss.str());
        st.appendSymbol(sizeSym);
        ns->setSizeSym(sizeSym);
    }

    // evaluate shape if given, also compute size
    if (children().size() > 2) {
        if (!isScalar) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), st.constantInt(1));
            code.push_imop(i);
            patchFirstImop(i);
        }

        TreeNode::ChildrenListConstIterator
                ei = children().at(2)->children().begin(),
                ee = children().at(2)->children().end();
        for (; ei != ee; ++ ei, ++ n) {
            assert (dynamic_cast<TreeNodeExpr*>(*ei) != 0);
            TreeNodeExpr* e = static_cast<TreeNodeExpr*>(*ei);
            s = generateSubexprCode(e, code, st, log, ns->getDim(n));
            if (s != ICode::OK) return s;
            Imop* i = new Imop(this, Imop::MUL, ns->getSizeSym(), ns->getSizeSym(), e->result());
            code.push_imop(i);
            patchFirstImop(i);
            prevPatchNextList(i, st);
        }
    }
    else {
        if (!isScalar) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), st.constantInt(0));
            code.push_imop(i);
            patchFirstImop(i);
        }

        for (unsigned it = 0; it < resultType().secrecDimType(); ++ it) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns->getDim(it), st.constantInt(0));
            code.push_imop(i);
        }
    }

    if (m_procParam) {
        Imop *i = 0;
        Symbol::dim_iterator
                di = ns->dim_begin(),
                de = ns->dim_end();

        if (!isScalar) {
            i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), st.constantInt(1));
            code.push_imop(i);
            patchFirstImop(i);
        }

        for (; di != de; ++ di) {
            i = new Imop(this, Imop::POP, *di);
            code.push_imop(i);
            i = new Imop(this, Imop::MUL, ns->getSizeSym(), ns->getSizeSym(), *di);
            code.push_imop(i);
        }

        if (isScalar)
            i = new Imop(this, Imop::POP, ns);
        else
            i = new Imop(this, Imop::POP, ns, ns->getSizeSym());
        code.push_imop(i);
        patchFirstImop(i);
    } else if (children().size() > 3) {

        // evaluate rhs
        TreeNode *t = children().at(3);
        assert((t->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(t) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(t);
        ICode::Status s = generateSubexprCode(e, code, st, log);
        if (s != ICode::OK) return s;

        // type x = foo;
        if (resultType().secrecDimType() > 0 && n == 0) {
            if (resultType().secrecDimType() > e->resultType().secrecDimType()) {
                Imop* i = new Imop(this, Imop::FILL, ns, e->result(), ns->getSizeSym());
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i, st);
            }
            else {
                assert (resultType().secrecDimType() == e->resultType().secrecDimType());

                Symbol::dim_iterator
                        di = ns->dim_begin(),
                        dj = e->result()->dim_begin(),
                        de = ns->dim_end();
                for (; di != de; ++ di, ++ dj) {
                    Imop* i = new Imop(this, Imop::ASSIGN, *di, *dj);
                    code.push_imop(i);
                    patchFirstImop(i);
                    prevPatchNextList(i, st);
                }
                if (!isScalar) {
                    Imop* i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), e->result()->getSizeSym());
                    code.push_imop(i);
                }

                Imop* i = new Imop(this, Imop::ASSIGN, ns, e->result(), e->result()->getSizeSym());
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i, st);
            }
        }

        // arr x[e1,...,en] = foo;
        if (n > 0 && resultType().secrecDimType() == n) {
            if (resultType().secrecDimType() > e->resultType().secrecDimType()) {
                // fill lhs with constant value
                Imop* i = new Imop(this, Imop::FILL, ns, e->result(), ns->getSizeSym());
                code.push_imop(i);
                patchFirstImop(i);
                prevPatchNextList(i, st);
            }
            else {
                // check that shapes match and assign
                std::stringstream ss;
                ss << "Shape mismatch at " << location();
                Imop* err = newError (this, st.constantString (ss.str ()));
                // new Imop(this, Imop::ERROR, (Symbol*) 0, (Symbol*) new std::string(ss.str()));
                SymbolLabel* errLabel = st.label (err);
                Symbol::dim_iterator
                        di = e->result()->dim_begin(),
                        dj = ns->dim_begin(),
                        de = e->result()->dim_end();
                for (; di != de; ++ di, ++ dj) {
                    Imop* i = new Imop(this, Imop::JNE, (Symbol*) 0, *di, *dj);
                    i->setJumpDest(errLabel);
                    code.push_imop(i);
                    patchFirstImop(i);
                    prevPatchNextList(i, st);
                }
                Imop* jmp = new Imop(this, Imop::JUMP, (Symbol*) 0);
                Imop* i = new Imop(this, Imop::ASSIGN, ns, e->result(), ns->getSizeSym());
                jmp->setJumpDest(st.label(i));
                code.push_imop(jmp);
                code.push_imop(err);
                code.push_imop(i);
                patchFirstImop(jmp);
                prevPatchNextList(jmp, st);
            }  
        }

        // scalar_type x = scalar;
        if (resultType().isScalar()) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns, e->result());
            code.push_imop(i);
            patchFirstImop(i);
            prevPatchNextList(i, st);
        }

        if (prevSubexpr() != 0) addToNextList(prevSubexpr()->nextList()); // uh oh...
    } else {

        if (!isScalar && n == 0) {
            Imop* i = new Imop(this, Imop::ASSIGN, ns->getSizeSym(), st.constantInt(0));
            code.push_imop(i);
            patchFirstImop(i);

            for (unsigned it = 0; it < resultType().secrecDimType(); ++ it) {
                Imop* i = new Imop(this, Imop::ASSIGN, ns->getDim(it), st.constantInt(0));
                code.push_imop(i);
            }
        }

        Imop *i = 0;
        if (isScalar) {
            i = new Imop(this, Imop::ASSIGN, ns, (Symbol*) 0);
        }
        else {
            i = new Imop(this, Imop::FILL, ns, (Symbol*) 0, (Symbol*) 0);
            if (n == 0) {
                i->setArg2(st.constantInt(0));
            }
            else {
                i->setArg2(ns->getSizeSym());
            }
        }

        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i, st);

        typedef DataTypeBasic DTB;
        typedef DataTypeVar DTV;
        assert(m_type->dataType().kind() == DataType::VAR);
        assert(dynamic_cast<const DTV*>(&m_type->dataType()) != 0);
        const DTV &dtv(static_cast<const DTV&>(m_type->dataType()));
        assert(dtv.dataType().kind() == DataType::BASIC);
        assert(dynamic_cast<const DTB*>(&dtv.dataType()) != 0);
        const DTB &dtb(static_cast<const DTB&>(dtv.dataType()));

        Symbol *def = st.defaultConstant (dtb.dataType ());
        i->setArg1(def);
    }

    // Add the symbol to the symbol table for use in later expressions:
    st.appendSymbol(ns);
    setResultFlags(TreeNodeStmt::FALLTHRU);
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtFor
*******************************************************************************/

ICode::Status TreeNodeStmtFor::generateCode(ICodeList &code, SymbolTable &st,
                                            CompileLog &log)
{
    assert(children().size() == 4);
    TreeNode *c0 = children().at(0);
    TreeNode *c1 = children().at(1);
    TreeNode *c2 = children().at(2);
    TreeNode *c3 = children().at(3);
    assert((c0->type() & NODE_EXPR_MASK) != 0);
    assert((c1->type() & NODE_EXPR_MASK) != 0);
    assert((c2->type() & NODE_EXPR_MASK) != 0);

    // Initialization expression:
    TreeNodeExpr *e0 = 0;
    if (children().at(0)->type() != NODE_EXPR_NONE) {
        assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
        e0 = static_cast<TreeNodeExpr*>(c0);
        ICode::Status s = e0->generateCode(code, st, log);
        if (s != ICode::OK) return s;
        setFirstImop(e0->firstImop());
    }

    // Conditional expression:
    TreeNodeExpr *e1 = 0;
    if (children().at(1)->type() != NODE_EXPR_NONE) {
        assert(dynamic_cast<TreeNodeExpr*>(c1) != 0);
        e1 = static_cast<TreeNodeExpr*>(c1);
        ICode::Status s = e1->calculateResultType(st, log);
        if (s != ICode::OK) return s;
        if (!e1->havePublicBoolType()) {
            log.fatal() << "Conditional expression in if statement must be of "
                           "type public bool in " << e1->location();
            return ICode::E_TYPE;
        }
        s = e1->generateBoolCode(code, st, log);
        if (s != ICode::OK) return s;
        patchFirstImop(e1->firstImop());
        addToNextList(e1->falseList());
    }

    // Loop body:
    SymbolTable &innerScope = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(c3) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(c3);
    ICode::Status s = body->generateCode(code, innerScope, log);
    if (s != ICode::OK) return s;
    patchFirstImop(body->firstImop());
    if (e1 != 0 && body->firstImop() != 0)
        e1->patchTrueList(st.label(body->firstImop()));
    addToNextList(body->breakList());

    // Iteration expression:
    TreeNodeExpr *e2 = 0;
    if (children().at(2)->type() != NODE_EXPR_NONE) {
        assert(dynamic_cast<TreeNodeExpr*>(c2) != 0);
        e2 = static_cast<TreeNodeExpr*>(c2);
        ICode::Status s = e2->generateCode(code, st, log);
        SymbolLabel* jumpDest = st.label(e2->firstImop());
        if (s != ICode::OK) return s;
        if (e1 != 0 && body->firstImop() == 0) e1->patchTrueList(jumpDest);
        body->patchContinueList(jumpDest);
        body->patchNextList(jumpDest);
    } else {
        if (e1 != 0) {
            SymbolLabel* jumpDest = st.label(e1->firstImop());
            body->patchContinueList(jumpDest);
            body->patchNextList(jumpDest);
        } else {
            SymbolLabel* jumpDest = st.label(body->firstImop());
            body->patchContinueList(jumpDest);
            body->patchNextList(jumpDest);
        }
    }

    // Static checking:
    assert(body->resultFlags() != 0x0);
    if ((body->resultFlags()
        & (TreeNodeStmt::FALLTHRU | TreeNodeStmt::CONTINUE)) == 0x0)
    {
        log.fatal() << "For loop at " << location() << " wont loop!";
        return ICode::E_OTHER;
    }
    if (e1 == 0 && ((body->resultFlags()
        & (TreeNodeStmt::BREAK | TreeNodeStmt::RETURN)) == 0x0))
    {
        log.fatal() << "For loop at " << location() << " is clearly infinite!";
        return ICode::E_OTHER;
    }
    setResultFlags((body->resultFlags()
                    & ~(TreeNodeStmt::BREAK | TreeNodeStmt::CONTINUE))
                   | TreeNodeStmt::FALLTHRU);

    // Next iteration jump:
    Imop *j = new Imop(this, Imop::JUMP, 0);
    if (e1 != 0) {
        j->setJumpDest(st.label(e1->firstImop()));
    } else {
        j->setJumpDest(st.label(body->firstImop()));
    }
    code.push_imop(j);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtIf
*******************************************************************************/

ICode::Status TreeNodeStmtIf::generateCode(ICodeList &code, SymbolTable &st,
                                           CompileLog &log)
{
    assert(children().size() == 2 || children().size() == 3);
    TreeNode *c0 = children().at(0);

    // Type check the expression
    assert((c0->type() & NODE_EXPR_MASK) != 0);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in if statement must be of "
                       "type public bool in " << e->location();
        return ICode::E_TYPE;
    }

    // Generate code for conditional expression:
    s = e->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);
    setFirstImop(e->firstImop());

    // Generate code for first branch:
    SymbolTable &innerScope1 = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(children().at(1)) != 0);
    TreeNodeStmt *s1 = static_cast<TreeNodeStmt*>(children().at(1));
    s = s1->generateCode(code, innerScope1, log);
    if (s != ICode::OK) return s;

    if (s1->firstImop() != 0) {
        e->patchTrueList(
                    st.label(s1->firstImop()));
        addToNextList(s1->nextList());
        addToBreakList(s1->breakList());
        addToContinueList(s1->continueList());
        assert(s1->resultFlags() != 0x0);
    } else {
        addToNextList(e->trueList());
    }

    if (children().size() == 2) {
        addToNextList(e->falseList());
        setResultFlags(s1->resultFlags() | TreeNodeStmt::FALLTHRU);
    } else {
        // Generate jump out of first branch, if needed:
        if ((s1->resultFlags() & TreeNodeStmt::FALLTHRU) != 0x0) {
            Imop *i = new Imop(this, Imop::JUMP, 0);
            code.push_imop(i);
            addToNextList(i);
        }

        // Generate code for second branch:
        SymbolTable &innerScope2 = *st.newScope();
        assert(dynamic_cast<TreeNodeStmt*>(children().at(2)) != 0);
        TreeNodeStmt *s2 = static_cast<TreeNodeStmt*>(children().at(2));
        s = s2->generateCode(code, innerScope2, log);
        if (s != ICode::OK) return s;

        e->patchFalseList(
                    st.label(s2->firstImop()));
        addToNextList(s2->nextList());
        addToBreakList(s2->breakList());
        addToContinueList(s2->continueList());
        assert(s2->resultFlags() != 0x0);
        setResultFlags(s1->resultFlags() | s2->resultFlags());
    }

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtReturn
*******************************************************************************/

ICode::Status TreeNodeStmtReturn::generateCode(ICodeList &code, SymbolTable &st,
                                               CompileLog &log)
{
    if (children().empty()) {
        if (containingProcedure()->procedureType().kind()
            == TypeNonVoid::PROCEDURE)
        {
            log.fatal() << "Cannot return from non-void function without value "
                           "at " << location();
            return ICode::E_OTHER;
        }
        assert(containingProcedure()->procedureType().kind()
               == TypeNonVoid::PROCEDUREVOID);

        Imop *i = new Imop(this, Imop::RETURNVOID, 0, 0, 0);
        i->setReturnDestFirstImop(
                    st.label(containingProcedure()->firstImop()));
        code.push_imop(i);
        setFirstImop(i);
    } else {
        assert(children().size() == 1);
        if (containingProcedure()->procedureType().kind()
            == TypeNonVoid::PROCEDUREVOID)
        {
            log.fatal() << "Cannot return value from void function at"
                        << location();
            return ICode::E_OTHER;
        }
        assert(containingProcedure()->procedureType().kind()
               == TypeNonVoid::PROCEDURE);

        assert((children().at(0)->type() & NODE_EXPR_MASK) != 0x0);
        assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
        TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
        ICode::Status s = e->calculateResultType(st, log);
        if (s != ICode::OK) return s;
        const SecreC::TypeNonVoid& procType = containingProcedure()->procedureType();

        // check that we can assign return type to proc type and also verify that
        // dimensionalities check as we can not to implicit conversion to array
        // with return statement
        if (!procType.canAssign(e->resultType()) ||
             procType.secrecDimType() != e->resultType().secrecDimType())
        {
            log.fatal() << "Cannot return value of type " << e->resultType()
                        << " from function with type "
                        << containingProcedure()->procedureType() << ". At"
                        << location();
            return ICode::E_OTHER;
        }

        // Add implicit classify node if needed:
        e = classifyChildAtIfNeeded(0, containingProcedure()->procedureType().secrecSecType());
        s = generateSubexprCode(e, code, st, log);
        if (s != ICode::OK) {
            return s;
        }

        // Push data and then shape
        Imop* i = 0;
        if (e->resultType().isScalar())
            i = new Imop(this, Imop::PUSH, 0, e->result());
        else
            i = new Imop(this, Imop::PUSH, 0, e->result(), e->result()->getSizeSym());
        code.push_imop(i);
        patchFirstImop(i);
        prevPatchNextList(i, st);

        Symbol::dim_reverese_iterator
                di = e->result()->dim_rbegin(),
                de = e->result()->dim_rend();
        for (; di != de; ++ di) {
            i = new Imop(this, Imop::PUSH, 0, *di);
            code.push_imop(i);
        }

        i = new Imop(this, Imop::RETURNVOID, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
        i->setReturnDestFirstImop(
                    st.label(containingProcedure()->firstImop()));
        code.push_imop(i);
    }

    setResultFlags(TreeNodeStmt::RETURN);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtWhile
*******************************************************************************/

ICode::Status TreeNodeStmtWhile::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log)
{
    assert(children().size() == 2);
    TreeNode *c0 = children().at(0);
    TreeNode *c1 = children().at(1);
    assert((c0->type() & NODE_EXPR_MASK) != 0);

    // Conditional expression:
    assert(c0->type() != NODE_EXPR_NONE);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in while statement must be of "
                       "type public bool in " << e->location();
        return ICode::E_TYPE;
    }
    s = e->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);

    // Loop body:
    SymbolTable &innerScope = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(c1) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(c1);
    s = body->generateCode(code, innerScope, log);
    if (s != ICode::OK) return s;

    // Static checking:
    if (body->firstImop() == 0) {
        log.fatal() << "Empty loop body at " << body->location();
        return ICode::E_OTHER;
    }
    assert(body->resultFlags() != 0x0);
    if ((body->resultFlags()
        & (TreeNodeStmt::FALLTHRU | TreeNodeStmt::CONTINUE)) == 0x0)
    {
        log.fatal() << "While loop at " << location() << " wont loop!";
        return ICode::E_OTHER;
    }
    setResultFlags((body->resultFlags()
                    & ~(TreeNodeStmt::BREAK | TreeNodeStmt::CONTINUE))
                    | TreeNodeStmt::FALLTHRU);

    Imop *i = new Imop(this, Imop::JUMP, 0);
    code.push_imop(i);
    i->setJumpDest(st.label(e->firstImop()));

    // Patch jump lists:
    setFirstImop(e->firstImop());
    e->patchTrueList(st.label(body->firstImop()));
    setNextList(e->falseList());
    addToNextList(body->breakList());
    SymbolLabel* jumpDest = st.label(e->firstImop());
    body->patchContinueList(jumpDest);
    body->patchNextList(jumpDest);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtWhile
*******************************************************************************/

ICode::Status TreeNodeStmtPrint::generateCode(ICodeList &code, SymbolTable &st,
                                              CompileLog &log)
{
    assert(children().size() == 1);
    TreeNode* c0 = children().at(0);

    assert(c0->type() != NODE_EXPR_NONE);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);

    // Type check:
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (e->resultType().secrecDataType() != DATATYPE_STRING || e->resultType().secrecSecType() != SECTYPE_PUBLIC   ||
        !e->resultType().isScalar()) {
        log.fatal() << "Argument to print statement has to be public string scalar, got "
                    << e->resultType() << " at " << location();
        return ICode::E_TYPE;
    }

    setResultFlags(TreeNodeStmt::FALLTHRU);

    // Generate code:
    s = generateSubexprCode(e, code, st, log);
    if (s != ICode::OK) return s;
    Imop* i = new Imop(this, Imop::PRINT, (Symbol*) 0, e->result());
    code.push_imop(i);
    patchFirstImop(i);
    e->patchNextList(i, st);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtDoWhile
*******************************************************************************/

ICode::Status TreeNodeStmtDoWhile::generateCode(ICodeList &code,
                                                SymbolTable &st,
                                                CompileLog &log)
{
    assert(children().size() == 2);
    TreeNode *c0 = children().at(0);
    TreeNode *c1 = children().at(1);
    assert((c1->type() & NODE_EXPR_MASK) != 0);

    // Loop body:
    SymbolTable &innerScope = *st.newScope();
    assert(dynamic_cast<TreeNodeStmt*>(c0) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(c0);
    ICode::Status s = body->generateCode(code, innerScope, log);
    if (s != ICode::OK) return s;

    // Static checking:
    if (body->firstImop() == 0) {
        log.fatal() << "Empty loop body at " << body->location();
        return ICode::E_OTHER;
    }
    assert(body->resultFlags() != 0x0);
    if ((body->resultFlags()
         & (TreeNodeStmt::FALLTHRU | TreeNodeStmt::CONTINUE)) == 0x0)
    {
        log.fatal() << "Do-while loop at " << location() << " wont loop!";
        return ICode::E_OTHER;
    }
    setResultFlags((body->resultFlags()
                    & ~(TreeNodeStmt::BREAK | TreeNodeStmt::CONTINUE))
                   | TreeNodeStmt::FALLTHRU);

    // Conditional expression:
    assert(c1->type() != NODE_EXPR_NONE);
    assert(dynamic_cast<TreeNodeExpr*>(c1) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c1);
    s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in if statement must be of "
                       "type public bool in " << e->location();
        return ICode::E_TYPE;
    }
    s = e->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);

    // Patch jump lists:
    setFirstImop(body->firstImop());
    setNextList(body->breakList());
    e->patchTrueList(st.label(body->firstImop()));
    addToNextList(e->falseList());
    body->patchContinueList(st.label(e->firstImop()));
    body->patchNextList(e->firstImop(), st);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtExpr
*******************************************************************************/

ICode::Status TreeNodeStmtExpr::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log)
{
    assert(children().size() == 1);
    assert(dynamic_cast<TreeNodeExpr*>(children().at(0)) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(children().at(0));
    ICode::Status s = e->generateCode(code, st, log);
    if (s != ICode::OK) return s;

    setFirstImop(e->firstImop());
    setNextList(e->nextList());
    setResultFlags(TreeNodeStmt::FALLTHRU);

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeStmtAssert
*******************************************************************************/

ICode::Status TreeNodeStmtAssert::generateCode(ICodeList &code, SymbolTable &st,
                                               CompileLog &log)
{
    assert(children().size() == 1);

    TreeNode *c0 = children().at(0);

    // Type check the expression
    assert((c0->type() & NODE_EXPR_MASK) != 0);
    assert(dynamic_cast<TreeNodeExpr*>(c0) != 0);
    TreeNodeExpr *e = static_cast<TreeNodeExpr*>(c0);
    ICode::Status s = e->calculateResultType(st, log);
    if (s != ICode::OK) return s;
    if (!e->havePublicBoolType()) {
        log.fatal() << "Conditional expression in assert statement must be of "
                       "type public bool in " << e->location();
        return ICode::E_TYPE;
    }

    // Generate code for conditional expression:
    s = e->generateBoolCode(code, st, log);
    if (s != ICode::OK) return s;
    assert(e->firstImop() != 0);
    if (e->firstImop()) {
        setFirstImop(e->firstImop());
    }

    std::ostringstream ss;
    ss << "assert failed at " << location();
    Imop *i = newError (this, st.constantString (ss.str ()));
    // new Imop(this, Imop::ERROR, 0, (Symbol*) new std::string(ss.str()));
    code.push_imop(i);
    patchFirstImop(i);

    e->patchNextList(i, st);
    e->patchFalseList(st.label(i));
    addToNextList(e->trueList());

    setResultFlags(FALLTHRU);

    return ICode::OK;
}

}
