#include <boost/foreach.hpp>
#include <boost/filesystem/fstream.hpp>

#include "treenode.h"
#include "symboltable.h"
#include "misc.h"
#include "codegen.h"
#include "ModuleInfo.h"
#include "typechecker/templates.h"

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
    case NODE_IMPORT:
        return static_cast<TreeNodeImport*>(decl)->codeGenWith (*this);
    default:
        assert (false && "UNREACHABLE");
        return CGStmtResult (ICode::E_OTHER);
    }
}

/*******************************************************************************
  TreeNodeKind
*******************************************************************************/

CGStmtResult TreeNodeKind::codeGenWith (CodeGen &cg) {
    return cg.cgKind (this);
}

CGStmtResult CodeGen::cgKind (TreeNodeKind *kind) {
    typedef TreeNodeIdentifier TNI;
    const TNI* id = static_cast<const TNI*>(kind->children ().at (0));
    SymbolTable* st = m_st->globalScope (); // kinds live in global scope

    if (st->find (id->value ()) != 0) {
        m_log.error () << "Redefining global symbol at " << kind->location ();
        return CGResult (ICode::E_TYPE);
    }

    SymbolKind* pdk = new SymbolKind ();
    pdk->setName (id->value ());
    st->appendSymbol (pdk);
    return CGStmtResult ();
}

/*******************************************************************************
  TreeNodeDomain
*******************************************************************************/

CGStmtResult TreeNodeDomain::codeGenWith (CodeGen &cg) {
    return cg.cgDomain (this);
}

CGStmtResult CodeGen::cgDomain (TreeNodeDomain *dom) {
    typedef TreeNodeIdentifier TNI;
    const TNI* idDomain = static_cast<const TNI*>(dom->children ().at (0));
    const TNI* idKind = static_cast<const TNI*>(dom->children ().at (1));
    SymbolTable* st = m_st->globalScope ();
    SymbolKind* kind = dynamic_cast<SymbolKind*>(st->find (idKind->value ()));
    if (kind == 0) {
        m_log.error () << "Undefined domain kind at " << dom->location () << ".";
        return CGResult (ICode::E_TYPE);
    }

    if (st->find (idDomain->value ()) != 0) {
        m_log.error () << "Redeclaration of security domain at " << dom->location () << ".";
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

CGStmtResult CodeGen::cgProcDef (TreeNodeProcDef *def, SymbolTable* localScope) {
    assert (localScope->parent () == m_st);
    typedef TreeNodeIdentifier TNI;
    typedef TypeNonVoid TNV;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    assert (def != 0);

    m_allocs.clear ();
    const TNI *id = def->identifier ();

    CGStmtResult result;
    ICode::Status s = m_tyChecker.visit (def, localScope);
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

    setScope (localScope);

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
                m_log.fatal() << "Function at " << def->location()
                    << " contains a break statement outside of any loop!";
                result.setStatus (ICode::E_OTHER);
                return result;
            } else if ((bodyResult.flags () & CGStmtResult::CONTINUE) != 0x0) {
                m_log.fatal() << "Function at " << def->location()
                    << " contains a continue statement outside of any loop!";
                result.setStatus (ICode::E_OTHER);
                return result;
            } else {
                assert((bodyResult.flags () & CGStmtResult::FALLTHRU) != 0x0);
                m_log.fatal() << "Function at " << def->location()
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
                m_log.fatal() << "Function at " << def->location()
                    << " contains a break statement outside of any loop!";
                result.setStatus (ICode::E_OTHER);
                return result;
            } else if ((bodyResult.flags () & CGStmtResult::CONTINUE) != 0x0) {
                m_log.fatal() << "Function at " << def->location()
                    << " contains a continue statement outside of any loop!";
                result.setStatus (ICode::E_OTHER);
                return result;
            }

            assert (fType->kind() == TNV::PROCEDUREVOID);
            releaseLocalAllocs (result);

            Imop *i = new Imop (def, Imop::RETURNVOID, (Symbol*) 0);
            i->setReturnDestFirstImop (m_st->label (result.firstImop ()));
            pushImopAfter (result, i);
        }
    }

    assert (result.nextList ().empty ());

    os << "End of function: " << id->value ();
    pushComment (os.str ());
    return result;
}

/*******************************************************************************
  TreeNodeModule
*******************************************************************************/

CGStmtResult TreeNodeModule::codeGenWith (CodeGen& cg) {
    return cg.cgModule (this);
}

CGStmtResult CodeGen::cgModule (TreeNodeModule* modNode) {
    TreeNode* nameNode = modNode->children ().at (0);
    ModuleInfo* modInfo = 0;
    if (modNode->hasName ()) {
        modInfo = m_modules.findModule (modNode->name ());

        if (modInfo == 0) {
            m_log.fatal () << "Module \"" << modNode->name () << "\" not found.";
            m_log.fatal () << "Error at " << nameNode->location () << ".";
            return CGResult (ICode::E_OTHER);
        }

        if (modNode->name () != modInfo->fileNameStem ()) {
            m_log.fatal () << "File name does not match with module name.";
            m_log.fatal () << "Error at " << nameNode->location () << ".";
            return CGResult (ICode::E_OTHER);
        }

        if (modInfo->status () != ModuleInfo::CGStarted) {
            m_log.fatal () << "ICE: attempting to generate code for module that's not yet parsed.";
            return CGResult (ICode::E_OTHER);
        }
    }
    else {
        modInfo = new ModuleInfo ();
        std::auto_ptr<ModuleInfo> newMod (modInfo);
        if (! m_modules.addModule ("__main", newMod)) {
            m_log.fatal () << "Error creating main module at " << modNode->location () << ".";
            return CGResult (ICode::E_OTHER);
        }

        assert (newMod.get () == 0 && "ModuleMap did not take ownership!");
        CodeGenState& cgState = modInfo->codeGenState ();
        modInfo->setStatus (ModuleInfo::CGStarted);
        cgState.m_insertPoint = m_code.end ();
        cgState.m_node = modNode;
        cgState.m_st = m_st->newScope ();
        cgState.m_st->setName ("Module");
    }


    assert (modInfo->codeGenState ().currentNode () == modNode);
    const CGStmtResult& result = cgMainModule (modNode->program (), modInfo);
    if (result.isNotOk ()) {
        return result;
    }

    modInfo->setStatus (ModuleInfo::CGDone);
    return result;
}

/*******************************************************************************
  TreeNodeImport
*******************************************************************************/

CGStmtResult TreeNodeImport::codeGenWith (CodeGen& cg) {
    return cg.cgImport (this);
}

CGStmtResult CodeGen::cgImport (TreeNodeImport* import) {
    ModuleInfo* mod = m_modules.findModule (import->name ());
    if (mod == 0) {
        m_log.fatal () << "Module \"" << import->name () << "\" not found within search path.";
        m_log.fatal () << "Error at " << import->location () << ".";
        return CGResult (ICode::E_OTHER);
    }

    switch (mod->status ()) {
    case ModuleInfo::CGDone: break;
    case ModuleInfo::CGStarted:
        /// \todo better error here
        m_log.fatal () << "Recursive modules.";
        m_log.fatal () << "Error at " << import->location () << ".";
        return CGResult (ICode::E_NOT_IMPLEMENTED);
    case ModuleInfo::CGNotStarted:
        break;
    }

    m_log.fatal () << "\\todo CodeGen::cgImport";
    return CGResult (ICode::E_NOT_IMPLEMENTED);
}

/*******************************************************************************
  TreeNodeProgram
*******************************************************************************/

CGStmtResult CodeGen::cgMainModule (TreeNodeProgram* prog, ModuleInfo* mod) {
    typedef std::map<const TreeNodeProcDef*, std::set<Imop*> > CallMap;

    CGStmtResult result;
    std::vector<TreeNodeProcDef*> procs;
    ScopedStateUse use (*this, mod->codeGenState ());

    if (prog->children().empty()) {
        m_log.fatal() << "Program is empty";
        result.setStatus (ICode::E_EMPTY_PROGRAM);
        return result;
    }

    BOOST_FOREACH (TreeNode* decl, prog->children ()) {
        switch (decl->type ()) {
        case NODE_PROCDEF:
        case NODE_OPDEF:
            assert (dynamic_cast<TreeNodeProcDef*>(decl) != 0);
            procs.push_back (static_cast<TreeNodeProcDef*>(decl));
            break;

        case NODE_TEMPLATE_DECL: {
            assert (dynamic_cast<TreeNodeTemplate*>(decl) != 0);
            TreeNodeTemplate* templ = static_cast<TreeNodeTemplate*>(decl);
            templ->setContainingModule (*mod);
            ICode::Status status = m_tyChecker.visit (templ);
            if (status != ICode::OK) {
                result.setStatus (status);
                return result;
            }

            break;
        }

        default:
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
    push_imop (retClean);
    push_imop (new Imop (prog, Imop::END));

    // Generate procedures:
    BOOST_FOREACH (TreeNodeProcDef* procDef, procs) {
        SymbolTable* localScope = m_st->newScope ();
        assert (localScope->parent () == m_st);
        localScope->setName ("Procedure");
        append (result, cgProcDef (procDef, localScope));
        if (result.isNotOk ()) {
            return result;
        }
    }

    // Instantiate templates:
    InstanceInfo info;
    while (m_tyChecker.getForInstantiation (info)) {
        ScopedStateUse use (*this, info.m_moduleInfo->codeGenState ());
        append (result, cgProcDef (info.m_generatedBody, info.m_localScope));
        if (result.isNotOk ()) {
            return result;
        }
    }

    // Patch up calls to template instances:
    BOOST_FOREACH (CallMap::value_type v, m_callsTo)
        BOOST_FOREACH (Imop* imop, v.second)
            imop->setCallDest (v.first->symbol ());

    // Check for "void main()":
    SymbolProcedure *mainProc = m_tyChecker.mainProcedure ();
    if (mainProc == 0) {
        m_log.fatal () << "No function \"void main()\" found!";
        result.setStatus (ICode::E_NO_MAIN);
        return result;
    }

    // Bind call to main(), i.e. mainCall:
    mainCall->setCallDest (mainProc);
    retClean->setArg2 (m_st->label (mainCall));
    releaseGlobalAllocs (result);
    return result;
}

} // namespace SecreC
