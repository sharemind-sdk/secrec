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
