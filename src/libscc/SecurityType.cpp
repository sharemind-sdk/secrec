#include "SecurityType.h"

#include "Context.h"
#include "ContextImpl.h"
#include "Symbol.h"

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
