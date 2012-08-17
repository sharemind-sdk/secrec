#include "symbol.h"

#include <string>
#include <sstream>

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

std::string SymbolKind::toString () const {
    std::ostringstream os;
    os << "KIND " << name ();
    return os.str ();
}

/*******************************************************************************
  SymbolDomain
*******************************************************************************/

std::string SymbolDomain::toString () const {
    std::ostringstream os;
    os << "DOMAIN (" << name ();
    if (secrecType ())
        os << " : " << secrecType ()->toString ();
    os << ')';
    return os.str ();
}

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

std::string SymbolSymbol::toString() const {
    std::ostringstream os;
    if (m_isTemporary) os << "TEMPORARY ";
    os << (m_scopeType == GLOBAL ? "GLOBAL" : "LOCAL") << ' '
       << *secrecType () << ' ' << name () << '{' << this << '}';
    return os.str();
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

std::string SymbolProcedure::toString() const {
    std::ostringstream os;
    os << "PROCEDURE " << name () << ": " << *secrecType ();
    return os.str();
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

std::string SymbolLabel::toString() const {
    std::ostringstream os;
    os << "Lable to ";
    assert (m_target != 0);
    if (m_target->block () != 0) {
        os << "block " << m_target->block()->index ();
    }
    else {
        os << "imop " << m_target->index ();
    }

    return os.str ();
}

/*******************************************************************************
  SymbolTemplate
*******************************************************************************/

SymbolTemplate::SymbolTemplate(const TreeNodeTemplate *templ)
    : Symbol (Symbol::TEMPLATE)
    , m_templ (templ)
{ }

std::string SymbolTemplate::toString() const {
    std::ostringstream os;
    os << "TEMPLATE " << name ();
    return os.str ();
}


}
