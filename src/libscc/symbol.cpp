#include "symbol.h"

#include <string>
#include <boost/foreach.hpp>

#include "blocks.h"
#include "imop.h"
#include "treenode.h"

namespace SecreC {

/*******************************************************************************
  Symbol
*******************************************************************************/

bool Symbol::isGlobal () const {
    switch (symbolType ()) {
    case Symbol::SYMBOL:
        if (static_cast<const SymbolSymbol*>(this)->scopeType () == SymbolSymbol::GLOBAL)
            return true;
    default:
        break;
    }

    return false;
}

bool Symbol::isArray () const {
    switch (symbolType ()) {
    case Symbol::SYMBOL:
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
    os << "KIND " << name ();
}

/*******************************************************************************
  SymbolDomain
*******************************************************************************/

void SymbolDomain::print(std::ostream & os) const {
    os << "DOMAIN (" << name ();
    if (secrecType ())
        os << " : " << *secrecType ();
    os << ')';
}

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

SymbolSymbol::SymbolSymbol(StringRef name, TypeNonVoid* valueType)
    : Symbol (SYMBOL, valueType)
    , m_scopeType (LOCAL)
    , m_dims (valueType->secrecDimType())
    , m_size (0)
    , m_isTemporary (false)
{
    setName(name);
}

SymbolSymbol::SymbolSymbol(StringRef name, TypeNonVoid * valueType, bool)
    : Symbol (SYMBOL, valueType)
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
    if (from->symbolType () == Symbol::SYMBOL) {
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
    : Symbol(Symbol::PROCEDURE, procdef->procedureType())
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

namespace {
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
}

void SymbolProcedure::print(std::ostream & os) const {
    printProcDef(os, m_decl);
}

/*******************************************************************************
  SymbolLabel
*******************************************************************************/

SymbolLabel::SymbolLabel (Imop* target)
    : Symbol (Symbol::LABEL)
    , m_target (target)
    , m_block (0)
{ }

SymbolLabel::SymbolLabel (Block* block)
    : Symbol (Symbol::LABEL)
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

SymbolTemplate::SymbolTemplate(TreeNodeTemplate *templ)
    : Symbol (Symbol::TEMPLATE)
    , m_templ (templ)
{ }

const Location * SymbolTemplate::location() const {
    return &m_templ->location();
}

void SymbolTemplate::print(std::ostream & os) const {
    os << "template <domain ";

    bool first = true;
    BOOST_FOREACH (TreeNodeDomainQuantifier& q, m_templ->quantifiers ()) {
        if (! first)
            os << ", domain ";
        first = false;
        q.printQuantifier (os);
    }

    printProcDef(os << "> ", m_templ->body());
}

}
