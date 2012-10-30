/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_STRINGREF_H
#define SECREC_STRINGREF_H

#include <boost/functional/hash_fwd.hpp>
#include <cstring>
#include <iosfwd>
#include <string>

namespace SecreC {

/*******************************************************************************
  StringRef
*******************************************************************************/

/**
 * Reference to some string.
 * \note Small object -- always pass by value.
 */
class StringRef {
public: /* Types: */
    typedef const char* iterator;
    typedef const char* const_iterator;
    typedef char value_type;
    typedef size_t size_type;

public: /* Methods: */

    StringRef ()
        : m_data ("")
        , m_size (0)
    { }

    StringRef (const char* data, size_type size)
        : m_data (data)
        , m_size (size)
    { }

    StringRef (const char* data)
        : m_data (data)
        , m_size (std::strlen (data))
    { }

    StringRef (const std::string& str)
        : m_data (str.c_str ())
        , m_size (str.length ())
    { }

    size_type size () const { return m_size; }
    iterator begin () const { return m_data; }
    iterator end () const { return m_data + m_size; }
    value_type operator [] (size_type i) const { return m_data[i]; }
    const char* data () const { return m_data; }
    bool empty () const { return m_size == 0; }
    value_type front () const { return m_data[0]; }
    value_type back () const { return m_data[m_size - 1]; }
    std::string str () const { return std::string (m_data, m_size); }

    bool fastEquality (StringRef other) const {
        return m_data == other.m_data && m_size == other.m_size;
    }

    bool isPrefixOf (StringRef ref) const {
        if (m_size > ref.m_size)
            return false;

        return std::strncmp (m_data, ref.m_data, m_size);
    }

    /**
     * \note Only use if you know what you're doing.
     */
    struct FastCmp {
        inline bool operator () (StringRef r1, StringRef r2) const {
            return std::make_pair (r1.m_data, r1.m_size) <
                   std::make_pair (r2.m_data, r2.m_size);
        }
    };

    static void free (StringRef ref) {
        std::free (const_cast<value_type*>(ref.m_data));
    }

private: /* Fields: */
    const value_type*  m_data;
    size_type          m_size;
};

std::ostream& operator << (std::ostream& os, StringRef sref);

inline bool operator == (StringRef r1, StringRef r2) {
    if (r1.size () != r2.size ())
        return false;

    return std::strncmp (r1.data(), r2.data(), r1.size()) == 0;
}

inline bool operator < (StringRef r1, StringRef r2) {
    return std::lexicographical_compare (r1.begin(), r1.end(), r2.begin(), r2.end());
}

} // namespace SecreC

namespace boost {
    template<>
    struct hash<SecreC::StringRef> : public std::unary_function<SecreC::StringRef, std::size_t> {
      std::size_t operator()(SecreC::StringRef sref) const {
          return boost::hash_range (sref.begin(), sref.end());
      }
    };
} // namespace boost

#endif /* SECREC_STRINGREF_H */
