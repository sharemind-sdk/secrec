/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_STRINGTABLE_H
#define SECREC_STRINGTABLE_H

#include "StringRef.h"

#include <unordered_set>

namespace SecreC {

/*******************************************************************************
  StringTable
*******************************************************************************/

/**
 * Extremly simple implementation of string table.
 */
class StringTable {
    StringTable (const StringTable&) = delete;
    StringTable& operator = (const StringTable&) = delete;
private: /* Types: */

    typedef std::unordered_set<StringRef> impl_t;

public: /* Types: */

    typedef impl_t::iterator iterator;
    typedef impl_t::const_iterator const_iterator;

public: /* Methods: */

    StringTable () { }

    ~StringTable () {
        for (const auto & strRef : *this) {
            StringRef::free (strRef);
        }
    }

    const StringRef* addString (const std::string& str) {
        return addString (str.c_str(), str.length());
    }

    const StringRef* addString (StringRef str) {
        return addString (str.data (), str.size ());
    }

    const StringRef* addString (const char* str, size_t size) {
        auto i = m_impl.find (StringRef (str, size));
        if (i == end ()) {
            const char* copy = strndup (str, size);
            i = m_impl.insert (i, StringRef (copy, size));
        }

        return &*i;
    }

    iterator begin () { return m_impl.begin (); }
    iterator end () { return m_impl.end (); }
    const_iterator begin () const { return m_impl.begin (); }
    const_iterator end () const { return m_impl.end (); }

private: /* Fields: */
    impl_t m_impl;
};

} // namespace SecreC

#endif /* SECREC_STRINGTABLE_H */
