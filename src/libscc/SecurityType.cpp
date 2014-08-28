#include "SecurityType.h"

#include "context.h"
#include "context_impl.h"
#include "symbol.h"

namespace SecreC {

/*******************************************************************************
  PublicSecType
*******************************************************************************/

void PublicSecType::print (std::ostream & os) const {
    os << "public";
}

PublicSecType* PublicSecType::get (Context& cxt) {
    ContextImpl& impl = *cxt.pImpl ();
    return impl.publicType ();
}

/*******************************************************************************
  PrivateSecType
*******************************************************************************/

void PrivateSecType::print (std::ostream & os) const {
    os << m_name;
}

PrivateSecType* PrivateSecType::get (Context& cxt, StringRef name,
                                     SymbolKind* kind)
{
    ContextImpl& impl = *cxt.pImpl ();
    return impl.privateType (name, kind);
}


} // namespace SecreC
