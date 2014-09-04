#include "symbol.h"

#include "blocks.h"
#include "imop.h"
#include "treenode.h"

#include <boost/foreach.hpp>
#include <string>

namespace SecreC {

namespace /* anonymous */ {

SymbolTemplate::Weight computeTemplateWeight (TreeNodeTemplate* templ) {
    std::set<StringRef, StringRef::FastCmp > typeVariables;

    unsigned typeVariableCount = templ->quantifiers ().size ();
    unsigned qualifiedTypeVariableCount = 0;
    unsigned quantifiedParamCount = 0;

    BOOST_FOREACH (TreeNodeQuantifier& quant, templ->quantifiers ()) {
        switch (quant.type ()) {
        case NODE_TEMPLATE_DOMAIN_QUANT:
            assert (dynamic_cast<TreeNodeDomainQuantifier*>(&quant) != NULL);
            if (static_cast<TreeNodeDomainQuantifier&>(quant).kind () == NULL)
                ++ qualifiedTypeVariableCount;
        case NODE_TEMPLATE_DIM_QUANT:
            typeVariables.insert (quant.typeVariable ()->value ());
        default:
            break;
        }
    }

    TreeNodeProcDef* body = templ->body ();
    BOOST_FOREACH (TreeNodeStmtDecl& decl, body->params ()) {
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
    }

    return SymbolTemplate::Weight (typeVariableCount,
                                   qualifiedTypeVariableCount,
                                   quantifiedParamCount);
}

void printProcDef(std::ostream & os, const TreeNodeProcDef * procDef) {
    procDef->returnType()->typeString(os);
    os << ' ' << procDef->identifier()->value() << '(';

    bool first = true;
    BOOST_FOREACH (const TreeNodeStmtDecl& decl, procDef->params ()) {
        if (! first)
            os << ", ";
        first = false;
        decl.varType()->typeString (os);
        os << ' ' << decl.variableName();
    }

    os << ')';
}

void flattenSymbolLoop (std::vector<Symbol*>& acc, Symbol* sym) {
    assert (sym != NULL && sym->secrecType () != NULL);

    if (sym->secrecType ()->secrecDataType ()->isComposite ()) {
        BOOST_FOREACH (SymbolSymbol* field, static_cast<SymbolSymbol*>(sym)->fields ()) {
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

SymbolSymbol::SymbolSymbol(StringRef name, TypeNonVoid* valueType)
    : Symbol (SYM_SYMBOL, valueType)
    , m_scopeType (LOCAL)
    , m_dims (valueType->secrecDimType())
    , m_size (NULL)
    , m_isTemporary (false)
{
    setName(name);
}

SymbolSymbol::SymbolSymbol(StringRef name, TypeNonVoid * valueType, bool)
    : Symbol (SYM_SYMBOL, valueType)
    , m_scopeType (LOCAL)
    , m_dims (valueType->secrecDimType ())
    , m_size (NULL)
    , m_isTemporary (true)
{
    setName(name);
}

const Location * SymbolSymbol::location() const {
    return NULL; // TODO
}

void SymbolSymbol::print(std::ostream & os) const {
    os << name ();
}

void SymbolSymbol::inheritShape (Symbol* from) {
    assert (from != NULL);
    if (from->symbolType () == SYM_SYMBOL) {
        assert (dynamic_cast<SymbolSymbol*>(from) != NULL);
        SymbolSymbol* t = static_cast<SymbolSymbol*>(from);
        setSizeSym(t->getSizeSym());
        std::copy (t->m_dims.begin (), t->m_dims.end (), m_dims.begin ());
    }
}

SymbolSymbol* lookupField (SymbolSymbol* val, StringRef fieldName) {
    assert (val != NULL && val->secrecType () != NULL);

    TypeNonVoid* ty = val->secrecType ();
    if (ty->secrecDataType ()->isComposite ()) {
        const std::vector<DataTypeStruct::Field>& fields = static_cast<DataTypeStruct*>(ty->secrecDataType ())->fields ();
        for (size_t i = 0; i < fields.size (); ++ i) {
            if (fields[i].name == fieldName) {
                return val->fields ().at (i);
            }
        }
    }

    return NULL;
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
                                 TypeProc* type)
    : Symbol(SYM_PROCEDURE, type)
    , m_target(NULL)
{
    setName(name);
}

void SymbolProcedure::print(std::ostream & os) const {
    typedef SecreC::Type::PrettyPrint PrettyPrint;
    TypeProc* procType = static_cast<TypeProc*>(secrecType ());
    os << PrettyPrint (procType->returnType ());
    os << ' ' << name () << '(';
    bool first = true;
    BOOST_FOREACH (const TypeBasic* argType, procType->paramTypes ()) {
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
                                          const TreeNodeProcDef * decl,
                                          SymbolProcedure * shortOf)
    : SymbolProcedure (name, decl->procedureType ())
    , m_decl (decl)
    , m_shortOf (shortOf)
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
    , m_block (NULL)
{ }

SymbolLabel::SymbolLabel (Block* block)
    : Symbol (SYM_LABEL)
    , m_target (NULL)
    , m_block (block)
{ }

const Imop* SymbolLabel::target () const {
    if (m_block == NULL) {
        assert (m_target != NULL);
        return m_target;
    }

    return &m_block->front ();
}

void SymbolLabel::print(std::ostream & os) const {
    os << "Lable to ";
    assert (m_target != NULL);
    if (m_target->block () != NULL) {
        os << "block " << m_target->block()->index ();
    }
    else {
        os << "imop " << m_target->index ();
    }
}

/*******************************************************************************
  SymbolTemplate
*******************************************************************************/

SymbolTemplate::SymbolTemplate(TreeNodeTemplate *templ, bool expectsSecType, bool expectsDataType, bool expectsDimType)
    : Symbol (SYM_TEMPLATE)
    , m_templ (templ)
    , m_expectsSecType (expectsSecType)
    , m_expectsDataType (expectsDataType)
    , m_expectsDimType (expectsDimType)
    , m_weight (computeTemplateWeight (templ))
{ }

const Location * SymbolTemplate::location() const {
    return &m_templ->location();
}

void SymbolTemplate::print(std::ostream & os) const {
    os << "template <";

    bool first = true;
    BOOST_FOREACH (TreeNodeQuantifier& q, m_templ->quantifiers ()) {
        if (! first)
            os << ", ";
        first = false;
        q.printQuantifier (os);
    }

    printProcDef(os << "> ", m_templ->body());
}

}
