#include "treenode.h"
#include "symboltable.h"
#include "misc.h"

/**
 * Code generation for top level statements.
 */

namespace SecreC {

/*******************************************************************************
  TreeNodeGlobals
*******************************************************************************/

ICode::Status TreeNodeGlobals::generateCode(ICodeList &code, SymbolTable &st,
                                            CompileLog &log)
{
    typedef ChildrenListConstIterator CLCI;

    TreeNodeStmtDecl *last = 0;
    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_DECL);
        assert(dynamic_cast<TreeNodeStmtDecl*>(*it) != 0);
        TreeNodeStmtDecl *decl = static_cast<TreeNodeStmtDecl*>(*it);
        decl->setGlobal();
        ICode::Status s = decl->generateCode(code, st, log);
        if (s != ICode::OK) return s;

        if (decl->firstImop() != 0) {
            if (last == 0) {
                setFirstImop(decl->firstImop());
            } else {
                last->patchNextList(decl->firstImop(), st);
            }
            last = decl;
        } else {
            assert(decl->nextList().empty());
        }
    }

    if (last != 0) {
        addToNextList(last->nextList());
    }

    return ICode::OK;
}


/*******************************************************************************
  TreeNodeProcDef
*******************************************************************************/

ICode::Status TreeNodeProcDef::generateCode(ICodeList &code, SymbolTable &st,
                                           CompileLog &log)
{
    typedef TreeNodeIdentifier TNI;
    typedef TypeNonVoid TNV;
    typedef ChildrenListConstIterator CLCI;

    assert(children().size() >= 3);
    assert(dynamic_cast<const TNI*>(children().at(0)) != 0);
    const TNI *id = static_cast<const TNI*>(children().at(0));

    ICode::Status s = calculateProcedureType(st, log);
    if (s != ICode::OK) return s;

    std::ostringstream os;
    os << "Start of function: " << id->value();
    setFirstImop(code.push_comment(os.str()));
    os.str("");

    // Add to symbol table:
    /// \todo Check whether already defined
    SymbolProcedure *ns = st.appendProcedure(*this);

    // Generate local scope:
    SymbolTable &localScope = *st.newScope();
    if (children().size() > 3) {
        for (CLCI it(children().begin() + 3); it != children().end(); it++) {
            assert((*it)->type() == NODE_DECL);
            assert(dynamic_cast<TreeNodeStmtDecl*>(*it) != 0);
            TreeNodeStmtDecl *paramDecl = static_cast<TreeNodeStmtDecl*>(*it);
            paramDecl->setProcParam(true);
            ICode::Status s = paramDecl->generateCode(code, localScope, log);
            if (s != ICode::OK) return s;
        }
    }

    // Generate code for function body:
    assert(dynamic_cast<TreeNodeStmt*>(children().at(2)) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(children().at(2));
    s = body->generateCode(code, localScope, log);
    if (s != ICode::OK) return s;
    assert(body->resultFlags() != 0x0);
    assert((body->resultFlags() & ~TreeNodeStmt::MASK) == 0);

    // Static checking:
    assert(ns->secrecType().isVoid() == false);
    assert(dynamic_cast<const TNV*>(&ns->secrecType()) != 0);
    const TNV &fType = static_cast<const TNV&>(ns->secrecType());
    if (fType.kind() == TNV::PROCEDURE) {
        if (body->resultFlags() != TreeNodeStmt::RETURN) {
            if ((body->resultFlags() & TreeNodeStmt::BREAK) != 0x0) {
                log.fatal() << "Function at " << location()
                    << " contains a break statement outside of any loop!";
                return ICode::E_OTHER;
            } else if ((body->resultFlags() & TreeNodeStmt::CONTINUE) != 0x0) {
                log.fatal() << "Function at " << location()
                    << " contains a continue statement outside of any loop!";
                return ICode::E_OTHER;
            } else {
                assert((body->resultFlags() & TreeNodeStmt::FALLTHRU) != 0x0);
                log.fatal() << "Function at " << location()
                            << " does not always return a value!";
                return ICode::E_OTHER;
            }
        }
        assert((body->resultFlags() & TreeNodeStmt::RETURN) != 0x0);
    } else {
        assert(fType.kind() == TNV::PROCEDUREVOID);
        if (body->resultFlags() != TreeNodeStmt::RETURN) {
            if ((body->resultFlags() & TreeNodeStmt::BREAK) != 0x0) {
                log.fatal() << "Function at " << location()
                    << " contains a break statement outside of any loop!";
                return ICode::E_OTHER;
            } else if ((body->resultFlags() & TreeNodeStmt::CONTINUE) != 0x0) {
                log.fatal() << "Function at " << location()
                    << " contains a continue statement outside of any loop!";
                return ICode::E_OTHER;
            }
            assert(fType.kind() == TNV::PROCEDUREVOID);
            Imop *i = new Imop(this, Imop::RETURNVOID, (Symbol*) 0, (Symbol*) 0, (Symbol*) 0);
            i->setReturnDestFirstImop(
                        st.label(firstImop()));
            body->patchNextList(i, st);
            code.push_imop(i);
        }
    }

    assert(body->breakList().empty());
    assert(body->continueList().empty());
    assert(body->nextList().empty());

    os << "End of function: " << id->value();
    code.push_comment(os.str());
    return ICode::OK;
}

/*******************************************************************************
  TreeNodeProcDefs
*******************************************************************************/

ICode::Status TreeNodeProcDefs::generateCode(ICodeList &code, SymbolTable &st,
                                             CompileLog &log)
{
    typedef ChildrenListConstIterator CLCI;

    for (CLCI it(children().begin()); it != children().end(); it++) {
        assert((*it)->type() == NODE_PROCDEF);
        assert(dynamic_cast<TreeNodeProcDef*>(*it) != 0);
        TreeNodeProcDef *procdef = static_cast<TreeNodeProcDef*>(*it);

        // Generate code:
        ICode::Status s = procdef->generateCode(code, st, log);
        if (s != ICode::OK) return s;
    }

    return ICode::OK;
}

/*******************************************************************************
  TreeNodeProgram
*******************************************************************************/

ICode::Status TreeNodeProgram::generateCode(ICodeList &code, SymbolTable &st,
                                            CompileLog &log)
{
    typedef SymbolProcedure SP;

    if (children().empty()) {
        log.fatal() << "Program is empty";
        return ICode::E_EMPTY_PROGRAM;
    }

    assert(children().size() < 3);

    TreeNodeProcDefs *ps;

    /**
      \todo In contrast with grammar we don't allow mixed declarations of
            variables and functions in global scope.
    */

    // Handle global declarations:
    if (children().size() == 2) {
        code.push_comment("Start of global declarations:");
        assert(children().at(0)->type() == NODE_GLOBALS);
        assert(dynamic_cast<TreeNodeGlobals*>(children().at(0)) != 0);
        TreeNodeGlobals *gs = static_cast<TreeNodeGlobals*>(children().at(0));

        ICode::Status s = gs->generateCode(code, st, log);
        if (s != ICode::OK) return s;
        code.push_comment("End of global declarations.");

        assert(children().at(1)->type() == NODE_PROCDEFS);
        assert(dynamic_cast<TreeNodeProcDefs*>(children().at(1)) != 0);
        ps = static_cast<TreeNodeProcDefs*>(children().at(1));
    } else {
        assert(children().at(0)->type() == NODE_PROCDEFS);
        assert(dynamic_cast<TreeNodeProcDefs*>(children().at(0)) != 0);
        ps = static_cast<TreeNodeProcDefs*>(children().at(0));
    }

    // Insert main call into the beginning of the program:
    Imop *mainCall = new Imop(this, Imop::CALL, 0, 0, 0);
    Imop *retClean = new Imop(this, Imop::RETCLEAN, 0, 0, 0);
    code.push_imop(mainCall);
    code.push_imop(retClean);
    code.push_imop(new Imop(this, Imop::END));

    // Handle functions:
    ICode::Status s = ps->generateCode(code, st, log);
    if (s != ICode::OK) return s;

    // Check for "void main()":
    SP *mainProc = st.findGlobalProcedure("main", DataTypeProcedureVoid());
    if (mainProc == 0) {
        log.fatal() << "No function \"void main()\" found!";
        return ICode::E_NO_MAIN;
    }

    // Bind call to main(), i.e. mainCall:
    mainCall->setCallDest(mainProc, st.label(retClean));
    retClean->setArg2(st.label(mainCall));

    return ICode::OK;
}

}
