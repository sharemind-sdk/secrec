/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "context_impl.h"

#include <map>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/foreach.hpp>

#include "types.h"
#include "misc.h"

namespace /* anonymous */ {
template <typename Key, typename T, typename Cmp >
void deleteValues (const std::map<Key, T*, Cmp>& m) {
    typedef const typename std::map<Key, T*, Cmp>::value_type value_type;
    BOOST_FOREACH (const value_type& p, m) {
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
    deleteValues (m_primitiveTypes);
    deleteValues (m_structTypes);
}

/* Security types: */
PublicSecType* ContextImpl::publicType () {
    return &m_pubSecType;
}

PrivateSecType* ContextImpl::privateType (StringRef name,
                                          SymbolKind* kind)
{
    PrivateSecTypeMap::iterator i = m_privSecTypes.find (name);
    if (i == m_privSecTypes.end ()) {
        i = m_privSecTypes.insert (i,
            PrivateSecTypeMap::value_type (name, new PrivateSecType (name, kind)));
    }

    return i->second;
}

}
