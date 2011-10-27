#include "treenode.h"
#include "symboltable.h"
#include "misc.h"

#include "codegen.h"

/**
 * Code generation for top level statements.
 */

namespace SecreC {

/*******************************************************************************
  TreeNodeGlobals
*******************************************************************************/

CGStmtResult TreeNodeGlobals::codeGenWith (CodeGen &cg) {
    return cg.cgGlobals (this);
}

CGStmtResult CodeGen::cgGlobals (TreeNodeGlobals* gs) {
    typedef TreeNode::ChildrenListConstIterator CLCI;
    CGStmtResult result;
    for (CLCI it (gs->children ().begin ()); it != gs->children ().end (); ++ it) {
        assert ((*it)->type () == NODE_DECL);
        assert (dynamic_cast<TreeNodeStmtDecl*> (*it) != 0);
        TreeNodeStmtDecl *decl = static_cast<TreeNodeStmtDecl*> (*it);
        decl->setGlobal ();
        const CGStmtResult& declResult (codeGenStmt (decl));
        append (result, declResult);
        if (result.isNotOk ()) {
            return result;
        }
    }

    return result;
}

/*******************************************************************************
  TreeNodeProcDef
*******************************************************************************/

CGStmtResult TreeNodeProcDef::codeGenWith (CodeGen &cg) {
    return cg.cgProcDef (this);
}

CGStmtResult CodeGen::cgProcDef (TreeNodeProcDef *def) {
    typedef TreeNodeIdentifier TNI;
    typedef TypeNonVoid TNV;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    assert(def->children ().size () >= 3);
    assert(dynamic_cast<const TNI*> (def->children ().at (0)) != 0);
    const TNI *id = static_cast<const TNI*> (def->children ().at (0));

    CGStmtResult result;
    ICode::Status s = def->calculateProcedureType (*st, log);
    if (s != ICode::OK) {
        result.setStatus (s);
        return result;
    }

    std::ostringstream os;
    os << "Start of function: " << id->value ();
    result.setFirstImop (code.push_comment (os.str ()));
    os.str("");

    // Add to symbol table:
    SymbolProcedure* ns = st->appendProcedure (*def);
    ns->setTarget (result.firstImop ());
    def->setSymbol (ns);

    // Generate local scope:
    SymbolTable& localScope = *st->newScope ();
    CodeGen local (code, localScope, log);

    if (def->children ().size () > 3) {
        for (CLCI it(def->children ().begin () + 3); it != def->children ().end (); ++ it) {
            assert ((*it)->type() == NODE_DECL);
            assert (dynamic_cast<TreeNodeStmtDecl*>(*it) != 0);
            TreeNodeStmtDecl* paramDecl = static_cast<TreeNodeStmtDecl*>(*it);
            paramDecl->setProcParam (true);
            const CGStmtResult& paramResult (local.codeGenStmt (paramDecl));
            append (result, paramResult);
            if (result.isNotOk ()) {
                return result;
            }
        }
    }

    // Generate code for function body:
    assert(dynamic_cast<TreeNodeStmt*>(def->children().at(2)) != 0);
    TreeNodeStmt *body = static_cast<TreeNodeStmt*>(def->children().at(2));
    const CGStmtResult& bodyResult (local.codeGenStmt (body));
    append (result, bodyResult);
    if (result.isNotOk ()) {
        return result;
    }

    assert (bodyResult.flags () != 0x0);
    assert ((bodyResult.flags () & ~CGStmtResult::MASK) == 0);
    assert (bodyResult.breakList ().empty ());
    assert (bodyResult.continueList ().empty ());

    // Static checking:
    assert(ns->secrecType().isVoid() == false);
    assert(dynamic_cast<const TNV*>(&ns->secrecType()) != 0);
    const TNV &fType = static_cast<const TNV&>(ns->secrecType());
    if (fType.kind() == TNV::PROCEDURE) {
        if (bodyResult.flags () != CGStmtResult::RETURN) {
            if ((bodyResult.flags () & CGStmtResult::BREAK) != 0x0) {
                log.fatal() << "Function at " << def->location()
                    << " contains a break statement outside of any loop!";
                result.setStatus (ICode::E_OTHER);
                return result;
            } else if ((bodyResult.flags () & CGStmtResult::CONTINUE) != 0x0) {
                log.fatal() << "Function at " << def->location()
                    << " contains a continue statement outside of any loop!";
                result.setStatus (ICode::E_OTHER);
                return result;
            } else {
                assert((bodyResult.flags () & CGStmtResult::FALLTHRU) != 0x0);
                log.fatal() << "Function at " << def->location()
                            << " does not always return a value!";
                result.setStatus (ICode::E_OTHER);
                return result;
            }
        }
        assert((bodyResult.flags () & CGStmtResult::RETURN) != 0x0);
    } else {
        assert(fType.kind() == TNV::PROCEDUREVOID);
        if (bodyResult.flags () != CGStmtResult::RETURN) {
            if ((bodyResult.flags () & CGStmtResult::BREAK) != 0x0) {
                log.fatal() << "Function at " << def->location()
                    << " contains a break statement outside of any loop!";
                result.setStatus (ICode::E_OTHER);
                return result;
            } else if ((bodyResult.flags () & CGStmtResult::CONTINUE) != 0x0) {
                log.fatal() << "Function at " << def->location()
                    << " contains a continue statement outside of any loop!";
                result.setStatus (ICode::E_OTHER);
                return result;
            }

            assert (fType.kind() == TNV::PROCEDUREVOID);
            Imop *i = new Imop (def, Imop::RETURNVOID, (Symbol*) 0);
            i->setReturnDestFirstImop (st->label (result.firstImop ()));
            pushImopAfter (result, i);
        }
    }

    assert (result.nextList ().empty ());

    os << "End of function: " << id->value ();
    code.push_comment (os.str ());
    return result;
}

/*******************************************************************************
  TreeNodeProcDefs
*******************************************************************************/

 CGStmtResult TreeNodeProcDefs::codeGenWith (CodeGen &cg) {
    return cg.cgProcDefs (this);
}

CGStmtResult CodeGen::cgProcDefs (TreeNodeProcDefs *defs) {
    typedef TreeNode::ChildrenListConstIterator CLCI;

    CGStmtResult result;
    for (CLCI it (defs->children().begin()); it != defs->children().end(); ++ it) {
        assert((*it)->type () == NODE_PROCDEF);
        assert(dynamic_cast<TreeNodeProcDef*> (*it) != 0);
        TreeNodeProcDef *procdef = static_cast<TreeNodeProcDef*> (*it);

        // Generate code:
        const CGStmtResult& defResult (procdef->codeGenWith (*this));
        append (result, defResult);
        if (result.isNotOk ()) {
            break;
        }
    }

    return result;
}

/*******************************************************************************
  TreeNodeProgram
*******************************************************************************/


ICode::Status TreeNodeProgram::codeGenWith (CodeGen &cg) {
    assert (children().size () < 3);
    const CGStmtResult& result = cg.cgProgram (this);
    return result.status ();
}

CGStmtResult CodeGen::cgProgram (TreeNodeProgram* prog) {
    typedef SymbolProcedure SP;

    TreeNodeProcDefs *ps;
    CGStmtResult result;

    /**
      \todo In contrast with grammar we don't allow mixed declarations of
            variables and functions in global scope.
    */

    if (prog->children().empty()) {
        log.fatal() << "Program is empty";
        result.setStatus (ICode::E_EMPTY_PROGRAM);
        return result;
    }


    // Handle global declarations:
    if (prog->children ().size () == 2) {
        code.push_comment ("Start of global declarations:");
        assert (prog->children ().at (0)->type () == NODE_GLOBALS);
        assert (dynamic_cast<TreeNodeGlobals*> (prog->children ().at (0)) != 0);
        TreeNodeGlobals *gs = static_cast<TreeNodeGlobals*> (prog->children ().at (0));
        const CGStmtResult& gsResult (gs->codeGenWith (*this));
        append (result, gsResult);
        if (result.isNotOk ()) {
            return result;
        }

        code.push_comment("End of global declarations.");

        assert (prog->children().at (1)->type () == NODE_PROCDEFS);
        assert (dynamic_cast<TreeNodeProcDefs*> (prog->children ().at (1)) != 0);
        ps = static_cast<TreeNodeProcDefs*> (prog->children ().at (1));
    } else {
        assert (prog->children().at (0)->type () == NODE_PROCDEFS);
        assert (dynamic_cast<TreeNodeProcDefs*> (prog->children().at (0)) != 0);
        ps = static_cast<TreeNodeProcDefs*> (prog->children().at (0));
    }

    // Insert main call into the beginning of the program:
    Imop *mainCall = newCall (prog);
    Imop *retClean = new Imop (prog, Imop::RETCLEAN, 0, 0, 0);
    pushImopAfter (result, mainCall);
    code.push_imop (retClean);
    code.push_imop (new Imop (prog, Imop::END));

    // Handle functions:
    const CGStmtResult& procResult (ps->codeGenWith (*this));
    append (result, procResult);
    if (result.isNotOk ()) {
        return result;
    }

    // Check for "void main()":
    SP *mainProc = st->findGlobalProcedure ("main", DataTypeProcedureVoid ());
    if (mainProc == 0) {
        log.fatal () << "No function \"void main()\" found!";
        result.setStatus (ICode::E_NO_MAIN);
        return result;
    }

    // Bind call to main(), i.e. mainCall:
//    mainCall->setCallDest (mainProc, st->label (retClean));
    mainCall->setCallDest (mainProc);
    retClean->setArg2 (st->label (mainCall));


    return result;
}

} // namespace SecreC
