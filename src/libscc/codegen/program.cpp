#include <boost/foreach.hpp>
#include <boost/filesystem/fstream.hpp>

#include "codegen.h"
#include "log.h"
#include "misc.h"
#include "ModuleInfo.h"
#include "ModuleMap.h"
#include "symbol.h"
#include "symboltable.h"
#include "treenode.h"
#include "typechecker/templates.h"

/**
 * Code generation for top level statements.
 */

namespace SecreC {

/*******************************************************************************
  TreeNodeKind
*******************************************************************************/

CGStmtResult CodeGen::cgKind(TreeNodeKind * kind) {
    typedef TreeNodeIdentifier TNI;
    const TNI * id = static_cast<const TNI *>(kind->children().at(0));
    SymbolTable * st = m_st->globalScope(); // kinds live in global scope

    if (st->find(id->value()) != 0) {
        m_log.error() << "Redefinition of global symbol '" << id->value()
                      << "'' at " << kind->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    st->appendSymbol(new SymbolKind(id->value()));
    return CGStmtResult();
}

/*******************************************************************************
  TreeNodeDomain
*******************************************************************************/

CGStmtResult CodeGen::cgDomain(TreeNodeDomain * dom) {
    typedef TreeNodeIdentifier TNI;
    const TNI * idDomain = static_cast<const TNI *>(dom->children().at(0));
    const TNI * idKind = static_cast<const TNI *>(dom->children().at(1));
    SymbolTable * st = m_st->globalScope();
    SymbolKind * kind = dynamic_cast<SymbolKind *>(st->find(idKind->value()));
    if (kind == 0) {
        m_log.error() << "Undefined domain kind at " << dom->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    if (st->find(idDomain->value()) != 0) {
        m_log.error() << "Redeclaration of global symbol '" << idDomain->value()
                      << "'' at " << dom->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    st->appendSymbol(new SymbolDomain(
                         idDomain->value(),
                         PrivateSecType::get(getContext(), idDomain->value(), kind)));
    return CGStmtResult();
}

/*******************************************************************************
  TreeNodeProcDef
*******************************************************************************/

struct ScopePusher {
    ScopePusher(CodeGen & cg, SymbolTable * newScope)
        : m_oldScope(cg.m_st), m_cg(cg)
    {
        cg.setScope(newScope);
    }

    ~ScopePusher() {
        m_cg.setScope(m_oldScope);
    }

    SymbolTable * m_oldScope;
    CodeGen & m_cg;
};

CGStmtResult CodeGen::cgProcDef(TreeNodeProcDef * def, SymbolTable * localScope) {
    assert(localScope->parent() == m_st);
    typedef TreeNodeIdentifier TNI;
    typedef TypeNonVoid TNV;
    typedef TreeNode::ChildrenListConstIterator CLCI;

    assert(def != 0);

    if (m_tyChecker->visit(def, localScope) != TypeChecker::OK)
        return CGResult::ERROR_CONTINUE;

    CGStmtResult result;
    std::ostringstream os;
    os << "Start of procedure: " << def->printableSignature();
    result.setFirstImop(pushComment(os.str()));
    os.str("");

    SymbolProcedure * ns = def->symbol();
    ns->setTarget(result.firstImop());

    // Generate local scope:
    ScopePusher s(*this, localScope);

    if (def->children().size() > 3) {
        BOOST_FOREACH (TreeNodeStmtDecl& paramDecl, def->params()) {
            paramDecl.setProcParam(true);
            append(result, codeGenStmt(&paramDecl));
        }
    }
    if (result.isNotOk())
        return result;

    // Generate code for procedure body:
    const CGStmtResult & bodyResult(codeGenStmt(def->body()));
    append(result, bodyResult);
    if (result.isNotOk())
        return result;

    if (bodyResult.mayFallThrough()) {
        releaseProcVariables(result);
    }

    popScope();

    assert(bodyResult.flags() != 0x0);
    assert((bodyResult.flags() & ~CGStmtResult::MASK) == 0);
    assert(bodyResult.breakList().empty());
    assert(bodyResult.continueList().empty());

    // Static checking:
    assert(!ns->secrecType()->isVoid());
    assert(dynamic_cast<TNV *>(ns->secrecType()) != 0);
    TNV * fType = static_cast<TNV *>(ns->secrecType());
    if (fType->kind() == TNV::PROCEDURE) {
        if (bodyResult.flags() != CGStmtResult::RETURN) {
            if ((bodyResult.flags() & CGStmtResult::BREAK) != 0x0) {
                m_log.fatal() << "Procedure at " << def->location()
                    << " contains a break statement outside of any loop!";
                result |= CGResult::ERROR_CONTINUE;
                return result;
            } else if ((bodyResult.flags() & CGStmtResult::CONTINUE) != 0x0) {
                m_log.fatal() << "Procedure at " << def->location()
                    << " contains a continue statement outside of any loop!";
                result |= CGResult::ERROR_CONTINUE;
                return result;
            } else {
                assert((bodyResult.flags() & CGStmtResult::FALLTHRU) != 0x0);
                m_log.fatal() << "Procedure at " << def->location()
                    << " does not always return a value!";
                result |= CGResult::ERROR_CONTINUE;
                return result;
            }
        }
        assert((bodyResult.flags() & CGStmtResult::RETURN) != 0x0);
    } else {
        assert(fType->kind() == TNV::PROCEDUREVOID);
        if (bodyResult.flags() != CGStmtResult::RETURN) {
            if ((bodyResult.flags() & CGStmtResult::BREAK) != 0x0) {
                m_log.fatal() << "Procedure at " << def->location()
                    << " contains a break statement outside of any loop!";
                result |= CGResult::ERROR_CONTINUE;
                return result;
            } else if ((bodyResult.flags() & CGStmtResult::CONTINUE) != 0x0) {
                m_log.fatal() << "Procedure at " << def->location()
                    << " contains a continue statement outside of any loop!";
                result |= CGResult::ERROR_CONTINUE;
                return result;
            }

            assert(fType->kind() == TNV::PROCEDUREVOID);
            releaseProcVariables(result);

            Imop * i = newReturn(def);
            i->setDest (m_st->label(result.firstImop()));
            pushImopAfter(result, i);
        }
    }

    assert(result.nextList().empty());

    os << "End of procedure: " << def->printableSignature();
    pushComment(os.str());
    return result;
}

/*******************************************************************************
  TreeNodeModule
*******************************************************************************/

CGStmtResult CodeGen::cgModule(ModuleInfo * mod) {
    typedef std::map<const TreeNodeProcDef *, std::set<Imop *> > CallMap;

    const StringRef* name = m_stringTable.addString ("Module " + mod->body()->name().str());

    assert(mod->status() == ModuleInfo::CGNotStarted);
    SymbolTable * moduleScope = m_st->newScope();
    moduleScope->setName(*name);
    mod->codeGenState().m_node = mod->body();
    mod->codeGenState().m_st = moduleScope;
    mod->codeGenState().m_insertPoint = m_code.end();

    TreeNodeProgram * prog = mod->body()->program();

    if (prog->children().empty()) {
        m_log.fatal() << "Program is empty.";
        return CGResult::ERROR_FATAL;
    }

    mod->setStatus(ModuleInfo::CGStarted);

    CGStmtResult result;
    // Generate globals to global scope:
    BOOST_FOREACH(TreeNode* decl, prog->children()) {
        switch (decl->type()) {
        case NODE_DECL:
            static_cast<TreeNodeStmtDecl *>(decl)->setGlobal(true);
            append(result, cgStmtDecl(static_cast<TreeNodeStmtDecl *>(decl)));
            break;
        case NODE_KIND:
            append(result, cgKind(static_cast<TreeNodeKind *>(decl)));
            break;
        case NODE_DOMAIN:
            append(result, cgDomain(static_cast<TreeNodeDomain *>(decl)));
            break;
        case NODE_IMPORT:
            append(result, cgImport(static_cast<TreeNodeImport *>(decl), mod));
            break;
        default:
            /* Intentionally empty */
            break;
        }

        if (result.isFatal()) return result;
        if (result.isNotOk()) continue;
    }

    // Generate module local stuff:
    ScopedStateUse use(*this, mod->codeGenState());
    BOOST_FOREACH(TreeNode* decl, prog->children()) {
        switch (decl->type()) {
        case NODE_PROCDEF:
        case NODE_OPDEF: {
            TreeNodeProcDef * procDef = static_cast<TreeNodeProcDef *>(decl);
            SymbolTable * localScope = m_st->newScope();
            localScope->setName("Procedure");
            append(result, cgProcDef(procDef, localScope));
            assert(localScope->parent() == m_st);
            break;
        }
        case NODE_TEMPLATE_DECL: {
            assert(dynamic_cast<TreeNodeTemplate *>(decl) != 0);
            TreeNodeTemplate * templ = static_cast<TreeNodeTemplate *>(decl);
            templ->setContainingModule(*mod);
            if (m_tyChecker->visit(templ) != TypeChecker::OK) {
                result |= CGResult::ERROR_CONTINUE;
            }
            break;
        }
        default:
            /* Intentionally empty */
            break;
        }

        if (result.isFatal()) return result;
        if (result.isNotOk()) continue;
    }

    mod->setStatus(ModuleInfo::CGDone);

    return result;
}

CGStmtResult CodeGen::cgMain(TreeNodeModule * mainModule) {

    ModuleInfo * modInfo = new ModuleInfo(m_stringTable);
    {
        std::auto_ptr<ModuleInfo> newMod(modInfo);
        if (! m_modules.addModule("__main", newMod)) {
            m_log.fatal() << "Error creating main module at "
                << mainModule->location() << '.';
            return CGResult::ERROR_FATAL;
        }

        assert(newMod.get() == 0);
    }

    pushComment("Start of globals:");
    Imop * i = pushComment("Start of procedures:");

    // insert instructions for globals before this point
    m_insertPoint = m_code.iterator_to(*i);
    CodeGenState & cgState = modInfo->codeGenState();
    modInfo->setBody(mainModule);

    CGStmtResult result = cgModule(modInfo);
    if (result.isFatal()) {
        return result;
    }

    // Instantiate templates:
    InstanceInfo info;
    while (m_tyChecker->getForInstantiation(info)) {
        ScopedStateUse use(*this, info.m_moduleInfo->codeGenState());
        append(result, cgProcDef(info.m_generatedBody, info.m_localScope));
        if (result.isFatal()) return result;
        if (result.isNotOk()) continue;
    }

    // Patch up calls to template instances:
    BOOST_FOREACH (const CallMap::value_type& v, m_callsTo) {
        BOOST_FOREACH (Imop * imop, v.second) {
            imop->setDest(v.first->symbol ());
        }
    }

    // Insert main call after globals have been instantiated:
    TreeNodeProgram * prog = mainModule->program();
    Imop * mainCall = newCall(prog);
    Imop * retClean = new Imop(prog, Imop::RETCLEAN, 0, 0, 0);
    pushImopAfter(result, mainCall);
    push_imop(retClean);
    releaseAllVariables(result);
    push_imop(new Imop(prog, Imop::END));

    ScopedStateUse use(*this, cgState);  // we need to look in main module
    SymbolProcedure * mainProc = m_tyChecker->mainProcedure();
    if (mainProc == 0) {
        result |= CGResult::ERROR_CONTINUE;
        return result;
    }

    // Bind call to main(), i.e. mainCall:
    mainCall->setDest(mainProc);
    retClean->setArg2(m_st->label(mainCall));
    return result;
}

/*******************************************************************************
  TreeNodeImport
*******************************************************************************/

CGStmtResult CodeGen::cgImport(TreeNodeImport * import, ModuleInfo * modContext) {
    ModuleInfo * mod = m_modules.findModule(import->name());
    if (mod == 0) {
        m_log.fatal() << "Module \"" << import->name() << "\" not found within search path.";
        m_log.fatal() << "Error at " << import->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    CGStmtResult result;
    switch (mod->status()) {
    case ModuleInfo::CGDone:
        break;
    case ModuleInfo::CGStarted:
        /// \todo better error here
        m_log.fatal() << "Recursive modules.";
        m_log.fatal() << "Error at " << import->location() << '.';
        result |= CGResult::ERROR_CONTINUE;
        break;
    case ModuleInfo::CGNotStarted: {
        if (!mod->read()) {
            result |= CGResult::ERROR_CONTINUE;
            return result;
        }

        append(result, cgModule(mod));
        if (result.isFatal()) {
            return result;
        }
    }

        break;
    }

    modContext->codeGenState().st()->addImport(mod->codeGenState().st());
    return result;
}


} // namespace SecreC
