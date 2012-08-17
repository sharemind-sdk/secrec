#include "SecurityType.h"

#include "symbol.h"
#include "context.h"
#include "context_impl.h"

namespace SecreC {

/*******************************************************************************
  PublicSecType
*******************************************************************************/

std::ostream& PublicSecType::print (std::ostream & os) const {
    os << "public";
    return os;
}

PublicSecType* PublicSecType::get (Context& cxt) {
    ContextImpl& impl = *cxt.pImpl ();
    return impl.publicType ();
}

/*******************************************************************************
  PrivateSecType
*******************************************************************************/

std::ostream& PrivateSecType::print (std::ostream & os) const {
    os << m_name;
    return os;
}

PrivateSecType* PrivateSecType::get (Context& cxt, const std::string& name,
                                     SymbolKind* kind)
{
    ContextImpl& impl = *cxt.pImpl ();
    return impl.privateType (name, kind);
}


} // namespace SecreC
