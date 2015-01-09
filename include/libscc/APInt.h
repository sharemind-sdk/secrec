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

#ifndef SECREC_APINT_H
#define SECREC_APINT_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <tuple>

namespace SecreC {

/******************************************************************
  APInt
******************************************************************/

/**
 * \brief Arbitrary precision integer class for modular arithmetic.
 * Each operation is performed modulo number of bits. This class can
 * be used to represent 8-, 16-, 32- and 64-bit integer.
 * TODO: support for wider integers might be needed in the future.
 */
class APInt {
public: /* Types: */

    enum CmpMode {
        EQ, NE, ULT, UGT, ULE, UGE, SLT, SGT, SLE, SGE
    };

    using value_type = uint64_t;
    using signed_value_type = int64_t;

    static constexpr unsigned WordSize = sizeof (value_type);
    static constexpr unsigned BitsPerWord = WordSize * 8u;

public: /* Methods: */

    APInt ()
        : m_numBits (0)
        , m_bits (0)
    { }

    APInt (unsigned numBits, value_type bits)
        : m_numBits (numBits)
        , m_bits (maskBits (numBits, bits))
    { }

    APInt (bool value)
        : m_numBits (1)
        , m_bits (value ? 1u : 0u)
    { }

    bool zero () const { return m_bits == 0; }
    bool negative () const {
        assert (m_numBits > 0);
        // return signedBits () < 0;
        return (m_bits & (1u << (m_numBits - 1u))) == 1u;
    }

    static APInt add (APInt x, APInt y);
    static APInt sub (APInt x, APInt y);
    static APInt mul (APInt x, APInt y);
    static APInt udiv (APInt x, APInt y);
    static APInt sdiv (APInt x, APInt y);
    static APInt urem (APInt x, APInt y);
    static APInt srem (APInt x, APInt y);
    static APInt shl (APInt x, APInt y);
    static APInt lshr (APInt x, APInt y);
    static APInt ashr (APInt x, APInt y);
    static APInt AND (APInt x, APInt y);
    static APInt OR (APInt x, APInt y);
    static APInt XOR (APInt x, APInt y);
    static APInt inv (APInt x);
    static APInt minus (APInt x);
    static APInt cmp (APInt x, APInt y, CmpMode mode);
    static APInt trunc (APInt x, unsigned numBits);
    static APInt extend (APInt x, unsigned numBits);
    static APInt sextend (APInt x, unsigned numBits);

    static APInt getMax (unsigned numBits);
    static APInt getNegativeOne (unsigned numBits);
    static APInt getNegativeMax (unsigned numBits);
    static APInt getNegativeMin (unsigned numBits);

    void uprint (std::ostream& os) const;
    void sprint (std::ostream& os) const;
    unsigned numBits () const { return m_numBits; }
    value_type bits () const { return m_bits; }

    friend bool operator < (APInt x, APInt y);
    friend bool operator == (APInt x, APInt y);
    friend bool operator != (APInt x, APInt y);

private:

    APInt (unsigned numBits, value_type bits, bool)
        : m_numBits (numBits)
        , m_bits (bits)
    { }

    static APInt makeUnsafe (unsigned numBits, value_type bits) {
        return APInt (numBits, bits, bool ());
    }

    static value_type maskBits (unsigned numBits, value_type bits) {
        if (numBits == 0)
            return 0;

        const auto mask = ~value_type (0) >> (BitsPerWord - numBits);
        return bits & mask;
    }

    signed_value_type signedBits () const {
        const auto signBit = BitsPerWord - m_numBits;
        return (signed_value_type(m_bits) << signBit) >> signBit;
    }

private: /* Fields: */
    const unsigned   m_numBits;
    const value_type m_bits;
};

inline bool operator == (APInt x, APInt y) {
    return x.m_numBits == y.m_numBits && x.m_bits == y.m_bits;
}

inline bool operator != (APInt x, APInt y) {
    return !(x == y);
}

inline bool operator < (APInt x, APInt y) {
    return std::tie (x.m_numBits, x.m_bits) < std::tie (y.m_numBits, y.m_bits);
}

} // namespace SecreC

#endif // SECREC_APINT_H
