#include "symbol.h"

#include <string>
#include <boost/foreach.hpp>

#include "blocks.h"
#include "imop.h"
#include "treenode.h"

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
            assert (dynamic_cast<TreeNodeDomainQuantifier*>(&quant) != 0);
            if (static_cast<TreeNodeDomainQuantifier&>(quant).kind () == 0)
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
    os << procDef->returnType()->typeString()
       << ' ' << procDef->identifier()->value() << '(';

    bool first = true;
    BOOST_FOREACH (const TreeNodeStmtDecl& decl, procDef->params ()) {
        if (! first)
            os << ", ";
        first = false;
        os << decl.varType()->typeString() << ' ' << decl.variableName();
    }

    os << ')';
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
    os << "DIM (" << name () << " : " << dimType () << ')';
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
    if (securityType ())
        os << " : " << *securityType ();
}

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

SymbolSymbol::SymbolSymbol(StringRef name, TypeNonVoid* valueType)
    : Symbol (SYM_SYMBOL, valueType)
    , m_scopeType (LOCAL)
    , m_dims (valueType->secrecDimType())
    , m_size (0)
    , m_isTemporary (false)
{
    setName(name);
}

SymbolSymbol::SymbolSymbol(StringRef name, TypeNonVoid * valueType, bool)
    : Symbol (SYM_SYMBOL, valueType)
    , m_scopeType (LOCAL)
    , m_dims (valueType->secrecDimType ())
    , m_size (0)
    , m_isTemporary (true)
{
    setName(name);
}

const Location * SymbolSymbol::location() const {
    return NULL; // TODO
}

void SymbolSymbol::print(std::ostream & os) const {
    if (m_isTemporary) os << "TEMPORARY ";
    os << (m_scopeType == GLOBAL ? "GLOBAL" : "LOCAL") << ' '
       << *secrecType () << ' ' << name () << '{' << this << '}';
}


void SymbolSymbol::inheritShape (Symbol* from) {
    assert (from != 0);
    if (from->symbolType () == SYM_SYMBOL) {
        assert (dynamic_cast<SymbolSymbol*>(from) != 0);
        SymbolSymbol* t = static_cast<SymbolSymbol*>(from);
        setSizeSym(t->getSizeSym());
        std::copy (t->m_dims.begin (), t->m_dims.end (), m_dims.begin ());
    }
}

/*******************************************************************************
  SymbolProcedure
*******************************************************************************/

SymbolProcedure::SymbolProcedure(StringRef name,
                                 const TreeNodeProcDef * procdef,
                                 SymbolProcedure * shortOf)
    : Symbol(SYM_PROCEDURE, procdef->procedureType())
    , m_decl(procdef)
    , m_target(0)
    , m_shortOf(shortOf)
{
    setName(name);
}

const Location * SymbolProcedure::location() const {
    return &m_decl->location();
}

StringRef SymbolProcedure::procedureName () const {
    return m_decl->procedureName ();
}

void SymbolProcedure::print(std::ostream & os) const {
    printProcDef(os, m_decl);
}

/*******************************************************************************
  SymbolLabel
*******************************************************************************/

SymbolLabel::SymbolLabel (Imop* target)
    : Symbol (SYM_LABEL)
    , m_target (target)
    , m_block (0)
{ }

SymbolLabel::SymbolLabel (Block* block)
    : Symbol (SYM_LABEL)
    , m_target (0)
    , m_block (block)
{ }

const Imop* SymbolLabel::target () const {
    if (m_block == 0) {
        assert (m_target != 0);
        return m_target;
    }

    return &m_block->front ();
}

void SymbolLabel::print(std::ostream & os) const {
    os << "Lable to ";
    assert (m_target != 0);
    if (m_target->block () != 0) {
        os << "block " << m_target->block()->index ();
    }
    else {
        os << "imop " << m_target->index ();
    }
}

/*******************************************************************************
  SymbolTemplate
*******************************************************************************/

SymbolTemplate::SymbolTemplate(TreeNodeTemplate *templ, bool expectsSecType, bool expectsDimType)
    : Symbol (SYM_TEMPLATE)
    , m_templ (templ)
    , m_expectsSecType (expectsSecType)
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
