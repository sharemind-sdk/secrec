/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TypeChecker.h"

#include "DataType.h"
#include "Log.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "TreeNode.h"

#include <set>

namespace SecreC {

namespace /* anonymous */ {

class ScopedPushSymbolTable {
public: /* Methods: */
    ScopedPushSymbolTable (SymbolTable*& st)
        : m_st (st)
    {
        m_st = m_st->newScope ();
    }

    ~ScopedPushSymbolTable () { m_st = m_st->parent (); }
private: /* Fields: */
    SymbolTable*& m_st;
};

} // namespace anonymous

/*******************************************************************************
  TreeNodeStructDecl
*******************************************************************************/

TypeChecker::Status TypeChecker::checkStruct (TreeNodeStructDecl* decl,
                                              const Location& loc,
                                              DataTypeStruct*& result,
                                              const std::vector<TypeArgument>& args)
{
    TreeNodeIdentifier* id = decl->identifier ();
    if (DataTypeStruct* t = DataTypeStruct::find (getContext (), id->value (), args)) {
        result = t;
        return OK;
    }

    if (args.size () != decl->quantifiers ().size ()) {
        m_log.fatal () << "Structure instantiated with invalid number of type arguments at " << loc << ".";
        return E_TYPE;
    }

    TreeNodeSeqView<TreeNodeQuantifier> quants = decl->quantifiers ();

    {
        std::set<StringRef> seen;
        for (TreeNodeQuantifier& q : quants) {
            TCGUARD (visitQuantifier (&q));
            const StringRef name = q.typeVariable ()->value ();
            if (! seen.insert (name).second) {
                m_log.fatal () << "Duplicate quantifier at " << q.location () << ".";
                return E_TYPE;
            }
        }
    }

    ScopedPushSymbolTable pushSymbolTable (m_st);

    for (size_t i = 0; i < args.size (); ++ i) {
        const TypeArgument& arg = args[i];
        TreeNodeQuantifier* quant = &quants[i];
        StringRef name = quant->typeVariable ()->value ();
        m_st->appendSymbol (arg.bind (name));
    }

    std::vector<DataTypeStruct::Field> fields;
    fields.reserve (decl->attributes ().size ());
    for (TreeNodeAttribute& attr : decl->attributes ()) {
        // TODO: This is incredibly ugly workaround! We are cloning in order to avoid using cache-d type.
        TreeNodeType* type = static_cast<TreeNodeType*>(attr.type ()->clone (nullptr));
        TCGUARD (visitType (type));
        if (type->secrecType ()->kind () != Type::BASIC) {
            m_log.fatal () << "Invalid structure field at " << type->location () << '.';
            delete type;
            return E_TYPE;
        }

        TypeBasic* fieldType = static_cast<TypeBasic*>(type->secrecType ());
        StringRef name = attr.identifier ()->value ();
        fields.push_back (make_field (fieldType, name));
        delete type;
    }

    result = DataTypeStruct::get (getContext (), id->value (), fields, args);
    return OK;
}

TypeChecker::Status TypeChecker::visitStructDecl (TreeNodeStructDecl* decl) {
    TreeNodeIdentifier* id = decl->identifier ();

    // Structure declaration must not overshadow another one:
    if (SymbolStruct* sym = m_st->find<SYM_STRUCT>(id->value ())) {
        m_log.fatal () << "Redeclaration of a struct \'" << id->value () << "\' at " << decl->location () << ".";
        if (sym->location ()) {
            m_log.fatal () << " Previous declaration at " << *sym->location () << ".";
        }

        return E_TYPE;
    }

    // We also check that we are not overshadowing another type:
    if (SymbolDataType* sym = m_st->find<SYM_TYPE>(id->value ())) {
        m_log.fatal () << "Redeclaration of a type \'" << id->value () << "\' at " << decl->location () << ".";
        if (sym->location ()) {
            m_log.fatal () << "Previous type declared at " << *sym->location () << ".";
        }

        return E_TYPE;
    }

    if (decl->isQuantified ()) {
        // Verify that there are not duplicate quantifiers.
        TreeNodeSeqView<TreeNodeQuantifier> quants = decl->quantifiers ();
        std::set<StringRef> seen;
        for (TreeNodeQuantifier& q : quants) {
            TCGUARD (visitQuantifier (&q));
            const StringRef name = q.typeVariable ()->value ();
            if (! seen.insert (name).second) {
                m_log.fatal () << "Duplicate quantifier at " << q.location () << ".";
                return E_TYPE;
            }
        }
    }
    else {
        // In case of monomorphic structures we also directly add the data type to symbol table:
        DataTypeStruct* structType = nullptr;
        TCGUARD (checkStruct (decl, decl->location (), structType));
        m_st->appendSymbol (new SymbolDataType (id->value (), structType));
    }

    m_st->appendSymbol (new SymbolStruct (id->value (), decl));
    return OK;
}

} // namespace SecreC
