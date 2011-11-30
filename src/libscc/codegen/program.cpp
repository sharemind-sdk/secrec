#include <boost/foreach.hpp>

#include "treenode.h"
#include "symboltable.h"
#include "misc.h"
#include "codegen.h"

/**
 * Code generation for top level statements.
 */

namespace SecreC {

CGStmtResult CodeGen::cgGlobalDecl (TreeNode *decl) {
    switch (decl->type ()) {
    case NODE_DECL:
        static_cast<TreeNodeStmtDecl*>(decl)->setGlobal (true);
        return static_cast<TreeNodeStmtDecl*>(decl)->codeGenWith (*this);
    case NODE_KIND:
        return static_cast<TreeNodeKind*>(decl)->codeGenWith (*this);
    case NODE_DOMAIN:
        return static_cast<TreeNodeDomain*>(decl)->codeGenWith (*this);
    default:
        assert (false && "UNREACHABLE");
        return CGStmtResult (ICode::E_OTHER);
    }
}

/*******************************************************************************
  TreeNodeStmtKind
*******************************************************************************/

CGStmtResult TreeNodeKind::codeGenWith (CodeGen &cg) {
    return cg.cgKind (this);
}

CGStmtResult CodeGen::cgKind (TreeNodeKind *kind) {
    typedef TreeNodeIdentifier TNI;
    const TNI* id = static_cast<const TNI*>(kind->children ().at (0));
    if (st->find (id->value ()) != 0) {
        log.error () << "Redefining global symbol at " << kind->location ();
        return CGResult (ICode::E_TYPE);
    }

    SymbolKind* pdk = new SymbolKind ();
    pdk->setName (id->value ());
    st->appendSymbol (pdk);
    return CGStmtResult ();
}

/*******************************************************************************
  TreeNodeStmtDomain
*******************************************************************************/

CGStmtResult TreeNodeDomain::codeGenWith (CodeGen &cg) {
    return cg.cgDomain (this);
}

CGStmtResult CodeGen::cgDomain (TreeNodeDomain *dom) {
    typedef TreeNodeIdentifier TNI;
    const TNI* idDomain = static_cast<const TNI*>(dom->children ().at (0));
    const TNI* idKind = static_cast<const TNI*>(dom->children ().at (1));
    SymbolKind* kind = dynamic_cast<SymbolKind*>(st->find (idKind->value ()));
    if (kind == 0) {
        log.error () << "Undefined domain kind at " << dom->location () << ".";
        return CGResult (ICode::E_TYPE);
    }

    if (st->find (idDomain->value ()) != 0) {
        log.error () << "Redeclaration of security domain at " << dom->location () << ".";
        return CGResult (ICode::E_TYPE);
    }

    SymbolDomain* symDom = new SymbolDomain (
        PrivateSecType::get (getContext (), idDomain->value (), kind));
    symDom->setName (idDomain->value ());
    st->appendSymbol (symDom);
    return CGStmtResult ();
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

    const TNI *id = def->identifier ();

    CGStmtResult result;
    ICode::Status s = m_tyChecker.visit (def);
    if (s != ICode::OK) {
        result.setStatus (s);
        return result;
    }

    std::ostringstream os;
    os << "Start of function: " << id->value ();
    result.setFirstImop (pushComment (os.str ()));
    os.str("");

    // Add to symbol table:
    SymbolProcedure* ns = def->symbol ();
    ns->setTarget (result.firstImop ());

    // Generate local scope:

    newScope ();

    if (def->children ().size () > 3) {
        for (CLCI it(def->paramBegin ()); it != def->paramEnd (); ++ it) {
            assert ((*it)->type() == NODE_DECL);
            assert (dynamic_cast<TreeNodeStmtDecl*>(*it) != 0);
            TreeNodeStmtDecl* paramDecl = static_cast<TreeNodeStmtDecl*>(*it);
            paramDecl->setProcParam (true);
            const CGStmtResult& paramResult (codeGenStmt (paramDecl));
            append (result, paramResult);
            if (result.isNotOk ()) {
                return result;
            }
        }
    }

    // Generate code for function body:
    const CGStmtResult& bodyResult (codeGenStmt (def->body ()));
    append (result, bodyResult);
    if (result.isNotOk ()) {
        return result;
    }

    popScope ();

    assert (bodyResult.flags () != 0x0);
    assert ((bodyResult.flags () & ~CGStmtResult::MASK) == 0);
    assert (bodyResult.breakList ().empty ());
    assert (bodyResult.continueList ().empty ());

    // Static checking:
    assert(!ns->secrecType()->isVoid());
    assert(dynamic_cast<TNV*>(ns->secrecType()) != 0);
    TNV* fType = static_cast<TNV*>(ns->secrecType());
    if (fType->kind() == TNV::PROCEDURE) {
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
        assert(fType->kind() == TNV::PROCEDUREVOID);
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

            assert (fType->kind() == TNV::PROCEDUREVOID);
            Imop *i = new Imop (def, Imop::RETURNVOID, (Symbol*) 0);
            i->setReturnDestFirstImop (st->label (result.firstImop ()));
            pushImopAfter (result, i);
        }
    }

    assert (result.nextList ().empty ());

    os << "End of function: " << id->value ();
    pushComment (os.str ());
    return result;
}

/*******************************************************************************
  TreeNodeProgram
*******************************************************************************/


ICode::Status TreeNodeProgram::codeGenWith (CodeGen &cg) {
    const CGStmtResult& result = cg.cgProgram (this);
    return result.status ();
}

CGStmtResult CodeGen::cgProgram (TreeNodeProgram* prog) {
    typedef SymbolProcedure SP;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    CGStmtResult result;
    std::list<TreeNodeProcDef*> procs;

    if (prog->children().empty()) {
        log.fatal() << "Program is empty";
        result.setStatus (ICode::E_EMPTY_PROGRAM);
        return result;
    }

    BOOST_FOREACH (TreeNode* decl, prog->children ()) {
        if (decl->type () == NODE_PROCDEF) {
            assert (dynamic_cast<TreeNodeProcDef*>(decl) != 0);
            procs.push_back (static_cast<TreeNodeProcDef*>(decl));
        }
        else
        if (decl->type () == NODE_TEMPLATE_DECL) {
            assert (dynamic_cast<TreeNodeTemplate*>(decl) != 0);
            ICode::Status status = m_tyChecker.visit (static_cast<TreeNodeTemplate*>(decl));
            if (status != ICode::OK) {
                result.setStatus (status);
                return result;
            }
        }
        else {
            append (result, cgGlobalDecl (decl));
            if (result.isNotOk ()) {
                return result;
            }
        }
    }

    // Insert main call after declarations:
    Imop *mainCall = newCall (prog);
    Imop *retClean = new Imop (prog, Imop::RETCLEAN, 0, 0, 0);
    pushImopAfter (result, mainCall);
    code.push_imop (retClean);
    code.push_imop (new Imop (prog, Imop::END));

    // Generate procedures:
    BOOST_FOREACH (TreeNodeProcDef* procDef, procs) {
        append (result, procDef->codeGenWith (*this));
        if (result.isNotOk ()) {
            return result;
        }
    }

    // Instantiate templates:
    SymbolTable* oldST = st;
    TreeNodeProcDef* procDef = 0;
    while (m_tyChecker.getForInstantiation (procDef, st)) {
        assert (procDef != 0);
        append (result, cgProcDef (procDef));
        if (result.isNotOk ()) {
            return result;
        }
    }

    // Patch up calls to template instances:
    typedef std::map<const TreeNodeProcDef*, std::set<Imop*> > CallMap;
    BOOST_FOREACH (CallMap::value_type v, m_callsTo)
        BOOST_FOREACH (Imop* imop, v.second)
            imop->setCallDest (v.first->symbol ());

    std::swap (oldST, st);

    // Check for "void main()":
    SP *mainProc = st->findGlobalProcedure ("main", DataTypeProcedureVoid::get (getContext ()));
    if (mainProc == 0) {
        log.fatal () << "No function \"void main()\" found!";
        result.setStatus (ICode::E_NO_MAIN);
        return result;
    }

    // Bind call to main(), i.e. mainCall:
    mainCall->setCallDest (mainProc);
    retClean->setArg2 (st->label (mainCall));
    return result;
}

} // namespace SecreC
