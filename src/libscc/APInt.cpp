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

#include "APInt.h"

#include <ostream>
#include <sharemind/abort.h>


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
    return APInt (x.m_numBits, x.signedBits () / y.signedBits ());
}

APInt APInt::srem (APInt x, APInt y) {
    assert (x.m_numBits == y.m_numBits);
    assert (y.m_bits != 0 && "ICE: rem by zero!");
    return APInt (x.m_numBits, x.signedBits () % y.signedBits ());
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
    default: SHAREMIND_ABORT("AIC %d", static_cast<int>(mode));
    }
}

APInt APInt::getMax (unsigned numBits) {
    return APInt (numBits, ~ value_type (0));
}

APInt APInt::getNegativeOne (unsigned numBits) {
    return getMax (numBits);
}

APInt APInt::getNegativeMax (unsigned numBits) {
    return ashr (getMax (numBits), APInt (numBits, 1u));
}

APInt APInt::getNegativeMin (unsigned numBits) {
    assert (numBits > 0);
    return APInt (numBits, value_type (1) << (numBits - 1), bool ());
}

void APInt::uprint (std::ostream& os) const {
    os << m_bits;
}

void APInt::sprint (std::ostream& os) const {
    os << signedBits ();
}


} // namespace SecreC
