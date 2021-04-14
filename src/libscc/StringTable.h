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

#ifndef SECREC_STRINGTABLE_H
#define SECREC_STRINGTABLE_H

#include <boost/functional/hash.hpp>
#include <cstdlib>
#include <sharemind/StringView.h>
#include <string.h>
#include <unordered_set>


namespace SecreC {

/*******************************************************************************
  StringTable
*******************************************************************************/

/**
 * Extremly simple implementation of string table.
 */
class StringTable {
private: /* Types: */

    using impl_t = std::unordered_set<sharemind::StringView,
                                      boost::hash<sharemind::StringView> >;

public: /* Types: */

    using iterator = impl_t::iterator;
    using const_iterator = impl_t::const_iterator;

public: /* Methods: */

    StringTable () { }
    StringTable (const StringTable&) = delete;
    StringTable& operator = (const StringTable&) = delete;
    ~StringTable () {
        for (const auto & strRef : *this)
            std::free(const_cast<char *>(strRef.data()));
    }

    sharemind::StringView const * addString(std::string const & str)
    { return addString(str.c_str(), str.length()); }

    sharemind::StringView const * addString(sharemind::StringView str)
    { return addString(str.data(), str.size()); }

    sharemind::StringView const * addString(char const * str, std::size_t size)
    {
        auto i = m_impl.find(sharemind::StringView(str, size));
        if (i == end ()) {
            const char* copy = strndup (str, size);
            i = m_impl.insert(i, sharemind::StringView(copy, size));
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
