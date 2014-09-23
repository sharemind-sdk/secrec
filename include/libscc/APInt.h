/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
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

    void uprint (std::ostream& os) const;
    void sprint (std::ostream& os) const;
    unsigned numBits () const { return m_numBits; }
    value_type bits () const { return m_bits; }

    friend bool operator < (APInt x, APInt y);

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

inline bool operator < (APInt x, APInt y) {
    return std::tie (x.m_numBits, x.m_bits) < std::tie (y.m_numBits, y.m_bits);
}

} // namespace SecreC

#endif // SECREC_APINT_H
