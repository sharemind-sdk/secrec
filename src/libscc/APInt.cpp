/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "APInt.h"

#include <ostream>

namespace SecreC {

APInt APInt::add (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    return APInt (x.m_numBits, x.m_bits + y.m_bits);
}

APInt APInt::sub (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    return APInt (x.m_numBits, x.m_bits - y.m_bits);
}

APInt APInt::mul (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    return APInt (x.m_numBits, x.m_bits * y.m_bits);
}

APInt APInt::udiv (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    assert (y.m_bits != 0 && "ICE: division by zero.");
    return APInt::makeUnsafe (x.m_numBits, x.m_bits / y.m_bits);
}

APInt APInt::urem (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    assert (y.m_bits != 0 && "ICE: rem by zero!");
    return APInt::makeUnsafe (x.m_numBits, x.m_bits % y.m_bits);
}

APInt APInt::sdiv (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    assert (y.m_bits != 0 && "ICE: division by zero.");
    const bool flipR = x.negative() != y.negative();
    const value_type tx = x.negative() ? - x.m_bits : x.m_bits;
    const value_type ty = y.negative() ? - y.m_bits : y.m_bits;
    const value_type tr = tx / ty;
    return APInt (x.m_numBits, flipR ? - tr : tr);
}

APInt APInt::srem (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    assert (y.m_bits != 0 && "ICE: rem by zero!");
    const bool flipR = x.negative ();
    const value_type tx = x.negative() ? - x.m_bits : x.m_bits;
    const value_type ty = y.negative() ? - y.m_bits : y.m_bits;
    const value_type tr = tx % ty;
    return APInt (x.m_numBits, flipR ? - tr : tr);
}

APInt APInt::shl (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    return APInt (x.m_numBits, x.m_bits << y.m_bits);
}

APInt APInt::lshr (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    return APInt::makeUnsafe (x.m_numBits, x.m_bits >> y.m_bits);
}

APInt APInt::ashr (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    if (y.m_bits == 0)
        return x;

    return APInt (x.m_numBits, x.signedBits () >> y.m_bits);
}

APInt APInt::AND (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    return APInt::makeUnsafe (x.m_numBits, x.m_bits & y.m_bits);
}

APInt APInt::OR (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    return APInt::makeUnsafe (x.m_numBits, x.m_bits | y.m_bits);
}

APInt APInt::XOR (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    return APInt::makeUnsafe (x.m_numBits, x.m_bits ^ y.m_bits);
}

APInt APInt::inv (APInt x) {
    return APInt (x.m_numBits, ~ x.m_bits);
}

APInt APInt::minus (APInt x) {
    return APInt (x.m_numBits, - x.m_bits);
}

APInt APInt::trunc (APInt x, unsigned numBits) {
    assert (numBits < x.m_numBits);
    return APInt (numBits, x.m_bits);
}

APInt APInt::extend (APInt x, unsigned numBits) {
    assert (numBits > x.m_numBits);
    return APInt::makeUnsafe (numBits, x.m_bits);
}

APInt APInt::sextend (APInt x, unsigned numBits) {
    assert (numBits > x.m_numBits);
    return APInt (numBits, x.signedBits ());
}

APInt APInt::cmp (APInt x, APInt y, APInt::CmpMode mode) {
    assert (x.m_numBits == y.m_numBits);
    switch (mode) {
    case EQ:  return x.m_bits == y.m_bits;
    case NE:  return x.m_bits != y.m_bits;
    case ULT: return x.m_bits <  y.m_bits;
    case UGT: return x.m_bits >  y.m_bits;
    case ULE: return x.m_bits <= y.m_bits;
    case UGE: return x.m_bits >= y.m_bits;
    case SLT: return x.signedBits () <  y.signedBits ();
    case SGT: return x.signedBits () >  y.signedBits ();
    case SLE: return x.signedBits () <= y.signedBits ();
    case SGE: return x.signedBits () >= y.signedBits ();
    }
}

void APInt::uprint (std::ostream& os) const {
    os << m_bits;
}

void APInt::sprint (std::ostream& os) const {
    os << signedBits ();
}


} // namespace SecreC
