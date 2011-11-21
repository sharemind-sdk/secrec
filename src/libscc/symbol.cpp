#include "symbol.h"

#include <string>
#include <sstream>

#include "imop.h"
#include "treenode.h"


namespace SecreC {

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
    os << "DOMAIN (" << name () << " : " << m_kind->toString () << ")";
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
    SymbolSymbol* t = 0;
    if ((t = dynamic_cast<SymbolSymbol*>(from)) != 0) {
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
    : Symbol (Symbol::LABEL),
      m_target (target)
{ }

std::string SymbolLabel::toString() const {
    std::ostringstream os;
    os << "Lable to ";
    if (m_target->block () != 0) {
        os << "block " << m_target->block()->index ();
    }
    else {
        os << "imop " << m_target->index ();
    }

    return os.str ();
}

}
