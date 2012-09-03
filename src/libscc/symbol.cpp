#include "symbol.h"

#include <string>

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
  SymbolKind
*******************************************************************************/

std::ostream & SymbolKind::print(std::ostream & os) const {
    os << "KIND " << name ();
    return os;
}

/*******************************************************************************
  SymbolDomain
*******************************************************************************/

std::ostream & SymbolDomain::print(std::ostream & os) const {
    os << "DOMAIN (" << name ();
    if (secrecType ())
        os << " : " << secrecType ()->toString ();
    os << ')';
    return os;
}

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

SymbolSymbol::SymbolSymbol(const std::string & name, TypeNonVoid* valueType)
    : Symbol (SYMBOL, valueType)
    , m_decl(NULL)
    , m_scopeType (LOCAL)
    , m_dims (valueType->secrecDimType())
    , m_size (0)
    , m_isTemporary (false)
{
    setName(name);
}

SymbolSymbol::SymbolSymbol(const std::string & name, TypeNonVoid * valueType, bool)
    : Symbol (SYMBOL, valueType)
    , m_decl(NULL)
    , m_scopeType (LOCAL)
    , m_dims (valueType->secrecDimType ())
    , m_size (0)
    , m_isTemporary (true)
{
    setName(name);
}

const YYLTYPE * SymbolSymbol::location() const {
    if (m_decl)
        return &m_decl->location();
    return NULL;
}

std::ostream & SymbolSymbol::print(std::ostream & os) const {
    if (m_isTemporary) os << "TEMPORARY ";
    os << (m_scopeType == GLOBAL ? "GLOBAL" : "LOCAL") << ' '
       << *secrecType () << ' ' << name () << '{' << this << '}';
    return os;
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

SymbolProcedure::SymbolProcedure(const TreeNodeProcDef *procdef)
    : Symbol(Symbol::PROCEDURE, procdef->procedureType())
    , m_decl(procdef)
    , m_target(0)
{
    // Intentionally empty
}

const YYLTYPE * SymbolProcedure::location() const {
    return &m_decl->location();
}

std::ostream & SymbolProcedure::print(std::ostream & os) const {
    os << "PROCEDURE " << name () << ": " << *secrecType ();
    return os;
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

std::ostream & SymbolLabel::print(std::ostream & os) const {
    os << "Lable to ";
    assert (m_target != 0);
    if (m_target->block () != 0) {
        os << "block " << m_target->block()->index ();
    }
    else {
        os << "imop " << m_target->index ();
    }

    return os;
}

/*******************************************************************************
  SymbolTemplate
*******************************************************************************/

SymbolTemplate::SymbolTemplate(const TreeNodeTemplate *templ)
    : Symbol (Symbol::TEMPLATE)
    , m_templ (templ)
{ }

const YYLTYPE * SymbolTemplate::location() const {
    return &m_templ->location();
}

std::ostream & SymbolTemplate::print(std::ostream & os) const {
    os << "TEMPLATE " << name ();
    return os;
}


}
