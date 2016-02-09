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

#include "ContextImpl.h"

#include "DataType.h"
#include "Misc.h"
#include "Types.h"

#include <map>

namespace /* anonymous */ {
template <typename Key, typename T, typename Cmp >
void deleteValues (const std::map<Key, T*, Cmp>& m) {
    for (const auto& p : m) {
        delete p.second;
    }
}
} // anonymous namespace


namespace SecreC {

ContextImpl::~ContextImpl () {
    deleteValues (m_privSecTypes);
    deleteValues (m_procTypes);
    deleteValues (m_basicTypes);
    deleteValues (m_stringLiterals);
    deleteValues (m_numericConstants[0]);
    deleteValues (m_numericConstants[1]);
    deleteValues (m_builtinPrimitiveTypes);
    deleteValues (m_userPrimitiveTypes);
    deleteValues (m_structTypes);
    deleteValues (m_floatConstants);
}

/* Security types: */
PublicSecType* ContextImpl::publicType () {
    return &m_pubSecType;
}

PrivateSecType* ContextImpl::privateType (StringRef name,
                                          SymbolKind* kind)
{
    auto i = m_privSecTypes.find (name);
    if (i == m_privSecTypes.end ()) {
        i = m_privSecTypes.insert (i,
            PrivateSecTypeMap::value_type (name, new PrivateSecType (name, kind)));
    }

    return i->second;
}

}
