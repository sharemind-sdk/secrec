/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "CodeGen.h"

#include "CodeGenResult.h"
#include "TreeNode.h"
#include "SymbolTable.h"
#include "Symbol.h"
#include "Log.h"

namespace SecreC {

CGResult CodeGen::cgLValue (TreeNodeLValue* lval, SubscriptInfo& subInfo, bool& isIndexed) {
    assert (lval != nullptr);
    return lval->codeGenWith (*this, subInfo, isIndexed);
}


/*******************************************************************************
  TreeNodeLVariable
*******************************************************************************/

CGResult TreeNodeLVariable::codeGenWith (CodeGen& cg, SubscriptInfo& subInfo,
                                         bool& isIndexed)
{
    return cg.cgLVariable (this, subInfo, isIndexed);
}

CGResult CodeGen::cgLVariable (TreeNodeLVariable* lvar, SubscriptInfo&,
                               bool& isIndexed)
{
    TreeNodeIdentifier* ident = lvar->identifier ();
    SymbolSymbol * destSym = m_st->find<SYM_SYMBOL>(ident->value());
    isIndexed = false;
    CGResult result;
    result.setResult (destSym);
    return result;
}

/*******************************************************************************
  TreeNodeLIndex
*******************************************************************************/

CGResult TreeNodeLIndex::codeGenWith (CodeGen& cg, SubscriptInfo& subInfo,
                                      bool& isIndexed)
{
    return cg.cgLIndex (this, subInfo, isIndexed);
}

CGResult CodeGen::cgLIndex (TreeNodeLIndex* lindex, SubscriptInfo& subInfo,
                            bool& isIndexed)
{
    TreeNodeLValue* lval = lindex->lvalue ();
    CGResult result = cgLValue (lval, subInfo, isIndexed);
    assert (!isIndexed && "ICE: type checker should have caught that!");
    isIndexed = true;
    append (result, codeGenSubscript (subInfo, result.symbol (), lindex->indices ()));
    return result;
}

/*******************************************************************************
  TreeNodeLSelect
*******************************************************************************/

CGResult TreeNodeLSelect::codeGenWith (CodeGen& cg, SubscriptInfo& subInfo,
                                       bool& isIndexed)
{
    return cg.cgLSelect (this, subInfo, isIndexed);
}

CGResult CodeGen::cgLSelect (TreeNodeLSelect* lselect, SubscriptInfo& subInfo,
                             bool& isIndexed)
{
    TreeNodeLValue* lval = lselect->lvalue ();
    CGResult result;

    // Generate code for the subexpression:
    CGResult lvalResult = cgLValue (lval, subInfo, isIndexed);
    assert (!isIndexed);
    isIndexed = false;
    append (result, lvalResult);
    if (result.isNotOk ())
        return result;

    // Pick the proper field:
    assert (dynamic_cast<SymbolSymbol*>(lvalResult.symbol ()) != nullptr);
    SymbolSymbol* exprValue = static_cast<SymbolSymbol*>(lvalResult.symbol ());
    StringRef fieldName = lselect->identifier ()->value ();
    if (SymbolSymbol* fieldValue = lookupField (exprValue, fieldName)) {
        result.setResult (fieldValue);
        return result;
    }

    // The following should have been rules out by the type checker:
    m_log.fatal () << "ICE: invalid code generation for attribute selection expression at "
                   << lselect->location () << ".";
    return CGResult::ERROR_CONTINUE;
}

} // namespace SecreC
