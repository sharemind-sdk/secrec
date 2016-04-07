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

#include "CodeGen.h"
#include "CodeGenResult.h"
#include "DataType.h"
#include "Log.h"
#include "Misc.h"
#include "ModuleInfo.h"
#include "ModuleMap.h"
#include "SecurityType.h"
#include "StringTable.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "TypeChecker.h"
#include "Types.h"
#include "typechecker/Templates.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/optional.hpp>


/**
 * Code generation for top level statements.
 */

namespace SecreC {

/*******************************************************************************
  TreeNodeKind
*******************************************************************************/

CGStmtResult CodeGen::cgKind(TreeNodeKind * kind) {
    const TreeNodeIdentifier * id = kind->identifier ();
    SymbolTable * st = m_st->globalScope(); // kinds live in global scope

    if (findIdentifier (SYM_KIND, id) != nullptr) {
        m_log.error() << "Redefinition of kind '" << id->value()
                      << "' at " << kind->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    SymbolKind * skind = new SymbolKind (id->value ());

    for (const TreeNodeDataTypeDecl& tyDecl : kind->types ()) {
        if (skind->findType (tyDecl.typeName ()) != nullptr) {
            m_log.error () << "Redefinition of type '" << tyDecl.typeName ()
                           << "' of kind '" << id->value () << "' at "
                           << tyDecl.location () << "'";
            return CGResult::ERROR_CONTINUE;
        }

        // GCC claims that the boost::optional values are
        // uninitialized
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wuninitialized"
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        boost::optional<DataTypeBuiltinPrimitive*> publicType = boost::none;
        boost::optional<uint64_t> size = boost::none;

        for (const TreeNodeDataTypeDeclParam& param : tyDecl.parameters()) {
            if (param.isPublicParam ()) {
                if (publicType) {
                    m_log.error () << "Multiple public type parameters of declaration of type '"
                                   << tyDecl.typeName () << "' at " << param.location () << '.';
                    return CGResult::ERROR_CONTINUE;
                }

                SecrecDataType pubFund = static_cast<const TreeNodeDataTypeDeclParamPublic*> (&param)->secrecDataType ();
                publicType = DataTypeBuiltinPrimitive::get (getContext (), pubFund);
            }
            else {
                if (size) {
                    m_log.error () << "Multiple size parameters of declaration of type '"
                                   << tyDecl.typeName () << "' at " << param.location () << '.';
                    return CGResult::ERROR_CONTINUE;
                }

                uint64_t s = static_cast<const TreeNodeDataTypeDeclParamSize*> (&param)->size ();

                if (s == 0 || (s != 1 && s % 8 != 0)) {
                    m_log.error () << "Size parameter of declaration of type '"
                                   << tyDecl.typeName () << "' at " << param.location ()
                                   << " is not one or a multiple of eight.";
                    return CGResult::ERROR_CONTINUE;
                }

                size = s;
            }
        }

        // If the private type has the name of a public type, the
        // corresponding public type must be the same
        SecrecDataType tyFund = stringToSecrecFundDataType (tyDecl.typeName ().data ());
        if (tyFund != DATATYPE_UNDEFINED) {
            DataTypeBuiltinPrimitive* ty = DataTypeBuiltinPrimitive::get (getContext (), tyFund);
            if (! publicType) {
                publicType = ty;
            }
            else {
                if (*publicType != ty) {
                    m_log.error () << "A private type with the name of a built-in type must"
                                   << " have the built-in type as the corresponding public type"
                                   << " at " << tyDecl.location () << '.';
                    return CGResult::ERROR_CONTINUE;
                }
            }
        }
        #pragma GCC diagnostic pop

        DataTypeUserPrimitive* dt =
            DataTypeUserPrimitive::get (getContext (), tyDecl.typeName ());
        dt->addParameters (skind, publicType, size);
        skind->addType (dt);

        SymbolDataType* sym = st->find<SYM_TYPE> (tyDecl.typeName ());
        if (sym == nullptr) {
            st->appendSymbol (new SymbolDataType (tyDecl.typeName (), dt));
        }
    }

    st->appendSymbol(skind);
    return CGStmtResult();
}

/*******************************************************************************
  TreeNodeDomain
*******************************************************************************/

CGStmtResult CodeGen::cgDomain(TreeNodeDomain * dom) {
    const TreeNodeIdentifier * idDomain = dom->domainIdentifier ();
    const TreeNodeIdentifier * idKind = dom->kindIdentifier ();
    SymbolTable * st = m_st->globalScope();
    SymbolKind * kind = static_cast<SymbolKind *>(findIdentifier (SYM_KIND, idKind));
    if (kind == nullptr) {
        m_log.error() << "Undefined domain kind '" << idKind->value ()
                      << "' at " << dom->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    if (findIdentifier (SYM_DOMAIN, idDomain) != nullptr) {
        m_log.error() << "Redeclaration of domain '" << idDomain->value()
                      << "' at " << dom->location() << '.';
        return CGResult::ERROR_CONTINUE;
    }

    st->appendSymbol(new SymbolDomain(
                         idDomain->value(),
                         PrivateSecType::get(getContext(), idDomain->value(), kind),
                         &idDomain->location ()));
    return CGStmtResult();
}

/*******************************************************************************
  TreeNodeStructDecl
*******************************************************************************/

CGStmtResult CodeGen::cgStructDecl (TreeNodeStructDecl* decl) {
    if (m_tyChecker->visitStructDecl (decl))
        return CGResult::ERROR_CONTINUE;
    return CGResult::OK;
}

/*******************************************************************************
  TreeNodeProcDef
*******************************************************************************/

struct ScopedSetSymbolTable {
    ScopedSetSymbolTable(CodeGen & cg, SymbolTable * newScope)
        : m_oldScope(cg.m_st), m_cg(cg)
    {
        cg.setScope(newScope);
    }

    ~ScopedSetSymbolTable() {
        m_cg.setScope(m_oldScope);
    }

    SymbolTable * m_oldScope;
    CodeGen & m_cg;
};

CGStmtResult CodeGen::cgProcDef(TreeNodeProcDef * def, SymbolTable * localScope) {
    assert(localScope->parent() == m_st);
    assert(def != nullptr);

    if (def->isOperator()) {
        TreeNodeOpDef* opdef = static_cast<TreeNodeOpDef*>(def);
        if (m_tyChecker->visitOpDef(opdef, localScope) != TypeChecker::OK)
            return CGResult::ERROR_CONTINUE;
    }
    else if (def->isCast()) {
        TreeNodeCastDef* castdef = static_cast<TreeNodeCastDef*>(def);
        if (m_tyChecker->visitCastDef(castdef, localScope) != TypeChecker::OK)
            return CGResult::ERROR_CONTINUE;
    }
    else if (m_tyChecker->visitProcDef(def, localScope) != TypeChecker::OK) {
        return CGResult::ERROR_CONTINUE;
    }

    CGStmtResult result;
    std::ostringstream os;
    os << "Start of procedure: " << def->printableSignature();
    result.setFirstImop(pushComment(os.str()));
    os.str("");

    SymbolProcedure * ns = def->symbol();
    ns->setTarget(result.firstImop());

    // Generate local scope:
    ScopedSetSymbolTable s(*this, localScope);

    if (def->children().size() > 3) {
        for (TreeNodeStmtDecl& paramDecl : def->params()) {
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
    assert((bodyResult.flags() & ~CGStmtResult::MASK) == 0x0);
    assert(bodyResult.breakList().empty());
    assert(bodyResult.continueList().empty());

    // Static checking:
    assert(ns->secrecType()->kind () == Type::PROCEDURE);
    TypeProc * fType = static_cast<TypeProc*>(ns->secrecType());
    if (! fType->returnType ()->isVoid ()) {
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
    const StringRef* name = getStringTable ().addString ("Module " + mod->body()->name().str());

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
    for (TreeNode* decl : prog->children()) {
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
        case NODE_STRUCT_DECL:
            append(result, cgStructDecl(static_cast<TreeNodeStructDecl*>(decl)));
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
    for (TreeNode* decl : prog->children()) {
        switch (decl->type()) {
        case NODE_CASTDEF:
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
            assert(dynamic_cast<TreeNodeTemplate *>(decl) != nullptr);
            TreeNodeTemplate * templ = static_cast<TreeNodeTemplate *>(decl);
            templ->setContainingModule(*mod);
            if (m_tyChecker->visitTemplate(templ) != TypeChecker::OK) {
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

    auto modInfo = new ModuleInfo(getContext ());
    {
        std::unique_ptr<ModuleInfo> newMod(modInfo);
        if (! m_modules.addModule("__main", std::move(newMod))) {
            m_log.fatal() << "Error creating main module at "
                << mainModule->location() << '.';
            return CGResult::ERROR_FATAL;
        }

        assert(newMod.get() == nullptr);
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
    for (const CallMap::value_type& v : m_callsTo) {
        for (Imop * imop : v.second) {
            imop->setDest(v.first);
        }
    }

    // Insert main call after globals have been instantiated:
    TreeNodeProgram * prog = mainModule->program();
    Imop * mainCall = newCall(prog);
    auto retClean = new Imop(prog, Imop::RETCLEAN, nullptr, nullptr, nullptr);
    pushImopAfter(result, mainCall);
    push_imop(retClean);
    releaseAllVariables(result);
    push_imop(new Imop(prog, Imop::END));

    ScopedStateUse use(*this, cgState);  // we need to look in main module
    SymbolProcedure * mainProc = m_tyChecker->mainProcedure();
    if (mainProc == nullptr) {
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
    ModuleInfo * mod = m_modules.findModule(import->name().str ());
    if (mod == nullptr) {
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
    case ModuleInfo::CGNotStarted:
        if (!mod->read()) {
            result |= CGResult::ERROR_CONTINUE;
            return result;
        }

        append(result, cgModule(mod));
        if (result.isFatal()) {
            return result;
        }

        break;
    }

    modContext->codeGenState().st()->addImport(mod->codeGenState().st());
    return result;
}


} // namespace SecreC
