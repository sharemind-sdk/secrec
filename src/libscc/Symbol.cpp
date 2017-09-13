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

#include "Symbol.h"

#include "Blocks.h"
#include "DataType.h"
#include "Imop.h"
#include "SecurityType.h"
#include "TreeNode.h"
#include "Types.h"

#include <string>

namespace SecreC {

namespace /* anonymous */ {

unsigned countQuantifiedParams (TreeNodeTemplate* templ) {
    std::set<StringRef, StringRef::FastCmp > typeVariables;
    unsigned quantifiedParamCount = 0;

    for (TreeNodeQuantifier& quant : templ->quantifiers ()) {
        typeVariables.insert (quant.typeVariable ()->value ());
    }

    TreeNodeProcDef* body = templ->body ();
    for (TreeNodeStmtDecl& decl : body->params ()) {
        TreeNodeType* t = decl.varType ();
        if (! t->secType ()->isPublic ()) {
            TreeNodeIdentifier* id = t->secType ()->identifier ();
            if (typeVariables.count (id->value ()) > 0) {
                ++ quantifiedParamCount;
            }
        }

        if (t->dimType ()->isVariable ()) {
            ++ quantifiedParamCount;
        }

        if (t->dataType ()->isVariable ()) {
            TreeNodeIdentifier* id = t->dataType ()->identifier ();
            if (typeVariables.count (id->value ()) > 0) {
                ++ quantifiedParamCount;
            }
        }

        if (t->dataType ()->type () == NODE_DATATYPE_TEMPLATE_F) {
            TreeNodeDataTypeTemplateF* templ = static_cast<TreeNodeDataTypeTemplateF*> (t->dataType ());
            std::vector<TreeNodeTypeArg*> typeArgs;

            for (TreeNodeTypeArg& arg : templ->arguments ()) {
                typeArgs.push_back (&arg);
            }

            while (! typeArgs.empty ()) {
                TreeNodeTypeArg* arg = typeArgs.back ();
                typeArgs.pop_back ();
                if (arg->type () == NODE_TYPE_ARG_VAR) {
                    TreeNodeTypeArgVar* argVar = static_cast<TreeNodeTypeArgVar*> (arg);
                    if (typeVariables.count (argVar->identifier ()->value ()) > 0)
                        ++ quantifiedParamCount;
                }
                else if (arg->type () == NODE_TYPE_ARG_TEMPLATE) {
                    TreeNodeTypeArgTemplate* argTempl = static_cast<TreeNodeTypeArgTemplate*> (arg);
                    for (TreeNodeTypeArg& child : argTempl->arguments ()) {
                        typeArgs.push_back (&child);
                    }
                }
            }
        }
    }

    return quantifiedParamCount;
}

SymbolProcedureTemplate::Weight computeTemplateWeight (TreeNodeTemplate* templ) {
    unsigned typeVariableCount = templ->quantifiers ().size ();
    unsigned qualifiedTypeVariableCount = 0;
    unsigned quantifiedParamCount = countQuantifiedParams (templ);

    for (TreeNodeQuantifier& quant : templ->quantifiers ()) {
        if (quant.type () == NODE_TEMPLATE_QUANTIFIER_DOMAIN) {
            assert (dynamic_cast<TreeNodeQuantifierDomain*>(&quant) != nullptr);
            if (static_cast<TreeNodeQuantifierDomain&>(quant).kind () == nullptr)
                ++ qualifiedTypeVariableCount;
        }
    }

    return SymbolProcedureTemplate::Weight (typeVariableCount,
                                            qualifiedTypeVariableCount,
                                            quantifiedParamCount);
}

void printProcDef(std::ostream & os, const TreeNodeProcDef * procDef) {
    procDef->returnType()->typeString(os);
    os << ' ' << procDef->identifier()->value() << '(';

    bool first = true;
    for (const TreeNodeStmtDecl& decl : procDef->params ()) {
        if (! first)
            os << ", ";
        first = false;
        decl.varType()->typeString (os);
        os << ' ' << decl.variableName();
    }

    os << ')';
}

void flattenSymbolLoop (std::vector<Symbol*>& acc, Symbol* sym) {
    assert (sym != nullptr && sym->secrecType () != nullptr);

    if (sym->secrecType ()->secrecDataType ()->isComposite ()) {
        for (SymbolSymbol* field : static_cast<SymbolSymbol*>(sym)->fields ()) {
            flattenSymbolLoop (acc, field);
        }
    }
    else {
        acc.insert (acc.end (), dim_begin (sym), dim_end (sym));
        acc.push_back (sym);
    }
}

} // namespace anonymous

/*******************************************************************************
  Symbol
*******************************************************************************/

bool Symbol::isGlobal () const {
    switch (symbolType ()) {
    case SYM_SYMBOL:
        if (static_cast<const SymbolSymbol*>(this)->scopeType () == SymbolSymbol::GLOBAL)
            return true;
    default:
        break;
    }

    return false;
}

bool Symbol::isArray () const {
    switch (symbolType ()) {
    case SYM_SYMBOL:
        if (static_cast<const SymbolSymbol*>(this)->secrecType ()->secrecDimType () > 0)
            return true;
    default:
        break;
    }

    return false;
}

bool Symbol::isString() const {
    switch (symbolType()) {
    case SYM_SYMBOL:
        if (static_cast<const SymbolSymbol*>(this)->secrecType()->secrecDataType()->isString())
            return true;
    default:
        break;
    }

    return false;
}

/*******************************************************************************
  SymbolDimensionality
*******************************************************************************/

void SymbolDimensionality::print(std::ostream & os) const {
    os << "dim " << name ();
}

void SymbolDimensionality::setTypeContext (TypeContext& cxt) const {
    cxt.setContextDimType (dimType ());
}

/*******************************************************************************
  SymbolDataType
*******************************************************************************/

void SymbolDataType::print (std::ostream& os) const {
    os << "type " << name ();
}

void SymbolDataType::setTypeContext (TypeContext& cxt) const {
    cxt.setContextDataType (dataType ());
}

/*******************************************************************************
  SymbolKind
*******************************************************************************/

const SymbolKind::Parameters*
SymbolKind::findType (StringRef name) const {
    auto it = m_types.find (name);
    if (it == m_types.end ())
        return nullptr;
    else
        return it->second;
}

void SymbolKind::addType (StringRef name,
                          const DataType* type,
                          boost::optional<const DataTypeBuiltinPrimitive*> publicType,
                          boost::optional<uint64_t> size)
{
    assert (m_types.find (name) == m_types.end ());
    auto params = new Parameters (type, publicType, size);
    m_types.insert (std::make_pair (name, params));
}

void SymbolKind::print(std::ostream & os) const {
    os << "kind " << name ();
}

/*******************************************************************************
  SymbolDomain
*******************************************************************************/

void SymbolDomain::print(std::ostream & os) const {
    os << "domain " << name ();
    if (securityType ()) // TODO: not correct printing
        os << " : " << *securityType ();
}

void SymbolDomain::setTypeContext (TypeContext& cxt) const {
    cxt.setContextSecType (securityType ());
}

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

SymbolSymbol::SymbolSymbol(StringRef name, const TypeNonVoid* valueType)
    : Symbol (SYM_SYMBOL, valueType)
    , m_scopeType (LOCAL)
    , m_dims (valueType->secrecDimType(), nullptr)
    , m_size (nullptr)
    , m_isTemporary (false)
    , m_parent (nullptr)
{
    setName(name);
}

SymbolSymbol::SymbolSymbol(StringRef name, const TypeNonVoid * valueType, bool)
    : Symbol (SYM_SYMBOL, valueType)
    , m_scopeType (LOCAL)
    , m_dims (valueType->secrecDimType (), nullptr)
    , m_size (nullptr)
    , m_isTemporary (true)
    , m_parent (nullptr)
{
    setName(name);
}

const Location * SymbolSymbol::location() const {
    return nullptr; // TODO
}

void SymbolSymbol::print(std::ostream & os) const {
    os << name ();
}

void SymbolSymbol::inheritShape (Symbol* from) {
    assert (from != nullptr);
    if (from->symbolType () == SYM_SYMBOL) {
        assert (dynamic_cast<SymbolSymbol*>(from) != nullptr);
        SymbolSymbol* t = static_cast<SymbolSymbol*>(from);
        setSizeSym (t->getSizeSym());
        for (size_t i = 0; i < m_dims.size (); ++ i) {
            setDim (i, t->getDim (i));
        }
    }
}

SymbolSymbol* lookupField (SymbolSymbol* val, StringRef fieldName) {
    assert (val != nullptr && val->secrecType () != nullptr);

    const TypeNonVoid* ty = val->secrecType ();
    if (ty->secrecDataType ()->isComposite ()) {
        const std::vector<DataTypeStruct::Field>& fields =
                static_cast<const DataTypeStruct*>(ty->secrecDataType ())->fields ();
        for (size_t i = 0; i < fields.size (); ++ i) {
            if (fields[i].name == fieldName) {
                return val->fields ().at (i);
            }
        }
    }

    return nullptr;
}

std::vector<Symbol*> flattenSymbol (Symbol* sym) {
    std::vector<Symbol*> result;
    flattenSymbolLoop (result, sym);
    return result;
}

/*******************************************************************************
  SymbolProcedure
*******************************************************************************/

SymbolProcedure::SymbolProcedure(StringRef name,
                                 const TypeProc* type)
    : Symbol(SYM_PROCEDURE, type)
    , m_target(nullptr)
{
    setName(name);
}

void SymbolProcedure::print(std::ostream & os) const {
    const auto procType = static_cast<const TypeProc*>(secrecType ());
    os << PrettyPrint (procType->returnType ());
    os << ' ' << name () << '(';
    bool first = true;
    for (const TypeBasic* argType : procType->paramTypes ()) {
        if (! first)
            os << ", ";
        first = false;
        os << PrettyPrint (argType);
    }

    os << ')';
}

/*******************************************************************************
  SymbolUserProcedure
*******************************************************************************/

SymbolUserProcedure::SymbolUserProcedure (StringRef name,
                                          const TreeNodeProcDef * decl)
    : SymbolProcedure (name, decl->procedureType ())
    , m_decl (decl)
{ }

const Location * SymbolUserProcedure::location() const {
    return &m_decl->location();
}

StringRef SymbolUserProcedure::procedureName () const {
    return m_decl->procedureName ();
}

void SymbolUserProcedure::print(std::ostream & os) const {
    printProcDef(os, m_decl);
}

/*******************************************************************************
  SymbolLabel
*******************************************************************************/

SymbolLabel::SymbolLabel (Imop* target)
    : Symbol (SYM_LABEL)
    , m_target (target)
    , m_block (nullptr)
{ }

SymbolLabel::SymbolLabel (Block* block)
    : Symbol (SYM_LABEL)
    , m_target (nullptr)
    , m_block (block)
{ }

const Imop* SymbolLabel::target () const {
    if (m_block == nullptr) {
        assert (m_target != nullptr);
        return m_target;
    }

    return &m_block->front ();
}

void SymbolLabel::print(std::ostream & os) const {
    os << "Lable to ";
    assert (m_target != nullptr);
    if (m_target->block () != nullptr) {
        os << "block " << m_target->block()->index ();
    }
    else {
        os << "imop " << m_target->index ();
    }
}

/*******************************************************************************
  SymbolStruct
*******************************************************************************/

SymbolStruct::SymbolStruct(StringRef name, TreeNodeStructDecl *structDecl)
    : Symbol (SYM_STRUCT, name)
    , m_structDecl (structDecl)
{ }

const Location* SymbolStruct::location () const {
    return &m_structDecl->location ();
}

void SymbolStruct::print (std::ostream &os) const {
    os << "TODO";
}

/*******************************************************************************
  SymbolTemplate
*******************************************************************************/

SymbolTemplate::SymbolTemplate(SymbolCategory cat, TreeNodeTemplate* templ)
    : Symbol (cat)
    , m_templ (templ)
{
    for (const TreeNodeQuantifier& quant : templ->quantifiers ()) {
        if (quant.isDataTypeQuantifier ())
            m_dataTypeQuantifiers.insert (quant.typeVariable ()->value ());
        else if (quant.isDomainQuantifier ())
            m_domainQuantifiers.insert (quant.typeVariable ()->value ());
    }
}

const Location * SymbolTemplate::location() const {
    return &m_templ->location();
}

void SymbolTemplate::print(std::ostream & os) const {
    os << "template <";

    bool first = true;
    for (TreeNodeQuantifier& q : m_templ->quantifiers ()) {
        if (! first)
            os << ", ";
        first = false;
        q.printQuantifier (os);
    }

    printProcDef(os << "> ", m_templ->body());
}

/*******************************************************************************
  SymbolProcedureTemplate
*******************************************************************************/

SymbolProcedureTemplate::SymbolProcedureTemplate(TreeNodeTemplate *templ,
                                                 bool expectsSecType,
                                                 bool expectsDataType,
                                                 bool expectsDimType)
    : SymbolTemplate (SYM_PROCEDURE_TEMPLATE, templ)
    , m_expectsSecType (expectsSecType)
    , m_expectsDataType (expectsDataType)
    , m_expectsDimType (expectsDimType)
    , m_weight (computeTemplateWeight (templ))
{ }

/*******************************************************************************
  SymbolOperatorTemplate
*******************************************************************************/

SymbolOperatorTemplate::SymbolOperatorTemplate(TreeNodeTemplate *templ,
                                               bool expectsDataType)
    : SymbolTemplate (SYM_OPERATOR_TEMPLATE, templ)
    , m_expectsDataType (expectsDataType)
{
    bool hasDomainQuant = false;
    bool hasConstraint = false;

    for (TreeNodeQuantifier& quant : templ->quantifiers ()) {
        if (quant.type () == NODE_TEMPLATE_QUANTIFIER_DOMAIN) {
            TreeNodeQuantifierDomain* dom = static_cast<TreeNodeQuantifierDomain*> (&quant);
            TreeNodeIdentifier* kind = dom->kind ();
            hasDomainQuant = true;
            if (kind != nullptr) {
                hasConstraint = true;
            }
            break;
        }
    }

    m_domainWeight = (!hasDomainQuant) ? 0 : (hasConstraint ? 1 : 2);
    m_quantifiedParamCount = countQuantifiedParams (templ);
}

} // namespace SecreC {
