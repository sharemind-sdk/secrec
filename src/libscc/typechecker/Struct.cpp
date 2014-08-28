/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "typechecker.h"

#include "log.h"
#include "symbol.h"
#include "symboltable.h"
#include "treenode.h"

#include <boost/foreach.hpp>

namespace SecreC {

/*******************************************************************************
  TreeNodeStructDecl
*******************************************************************************/

TypeChecker::Status TypeChecker::visit(TreeNodeStructDecl* decl) {
    TreeNodeIdentifier* id = decl->identifier ();
    BOOST_FOREACH (Symbol* s, m_st->findAll (SYM_TYPE, id->value ())) {
        m_log.fatal () << "Redeclaration of type \'" << id->value () << "\' at " << decl->location () << ".";
        if (s->location ()) {
            m_log.fatal () << "Previous declaration at " << *s->location () << ".";
        }

        return E_TYPE;
    }

    std::vector<DataTypeStruct::Field> fields;
    fields.reserve (decl->attributes ().size ());
    BOOST_FOREACH (TreeNodeAttribute& attr, decl->attributes ()) {
        TreeNodeType* type = attr.type ();
        TCGUARD (visit (type));
        if (type->secrecType ()->kind () != Type::BASIC) {
            m_log.fatal () << "Invalid structure field at " << type->location () << '.';
            return E_TYPE;
        }

        TypeBasic* fieldType = static_cast<TypeBasic*>(type->secrecType ());
        StringRef name = attr.identifier ()->value ();
        fields.push_back (make_field (fieldType, name));
    }

    DataTypeStruct* structType = DataTypeStruct::get (getContext (), id->value (), fields);
    SymbolDataType* symDataType = new SymbolDataType (id->value (), structType);
    m_st->appendSymbol (symDataType);
    return OK;
}

} // namespace SecreC
