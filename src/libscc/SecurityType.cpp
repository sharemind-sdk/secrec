/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

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
