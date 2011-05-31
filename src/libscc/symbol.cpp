#include "symbol.h"

#include "imop.h"
#include "treenode.h"

#include <string>
#include <sstream>

namespace SecreC {

/*******************************************************************************
  Symbol
*******************************************************************************/

void Symbol::inheritShape (Symbol* from) {
    setSizeSym(from->getSizeSym());
    std::copy (from->dim_begin(), from->dim_end(), dim_begin());
}

/*******************************************************************************
  SymbolSymbol
*******************************************************************************/

std::string SymbolSymbol::toString() const {
    std::ostringstream os;
    os << (m_scopeType == GLOBAL ? "GLOBAL" : "LOCAL") << ' ' << secrecType()
       << ' ' << name() << '{' << this << '}';
    return os.str();
}

/*******************************************************************************
  SymbolTemporary
*******************************************************************************/

std::string SymbolTemporary::toString() const {
    std::ostringstream os;
    os << "TEMPORARY " << (m_scopeType == GLOBAL ? "GLOBAL" : "LOCAL") << ' '
       << secrecType() << ' ' << name() << '{' << this << '}';
    return os.str();
}

/*******************************************************************************
  SymbolProcedure
*******************************************************************************/

SymbolProcedure::SymbolProcedure(const TreeNodeProcDef *procdef)
    : Symbol(Symbol::PROCEDURE, procdef->procedureType()), m_decl(procdef),
      m_target(0)
{
    // Intentionally empty
}

std::string SymbolProcedure::toString() const {
    std::ostringstream os;
    os << "PROCEDURE " << name() << ": " << secrecType();
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
        os << "block " << m_target->block()->index;
    }
    else {
        os << "imop " << m_target->index ();
    }

    return os.str ();
}

}
