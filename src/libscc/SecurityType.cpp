#include "SecurityType.h"

#include "symbol.h"
#include "context.h"
#include "context_impl.h"

namespace SecreC {

/*******************************************************************************
  PublicSecType
*******************************************************************************/

std::string PublicSecType::toString () const {
    return "public";
}

PublicSecType* PublicSecType::get (Context& cxt) {
    ContextImpl& impl = *cxt.pImpl ();
    return impl.publicType ();
}

/*******************************************************************************
  PrivateSecType
*******************************************************************************/

std::string PrivateSecType::toString () const {
    return m_name;
}

PrivateSecType* PrivateSecType::get (Context& cxt, const std::string& name,
                                     SymbolKind* kind)
{
    ContextImpl& impl = *cxt.pImpl ();
    return impl.privateType (name, kind);
}


} // namespace SecreC
