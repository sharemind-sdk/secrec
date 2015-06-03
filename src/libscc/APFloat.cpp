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

#include "APFloat.h"
#include "DataType.h"

#include <ostream>
#include <sharemind/abort.h>
#include <stdexcept>


namespace SecreC {

namespace /* anonymous */ {

// Hopefully this function is compiled to a identity.
inline mpfr_rnd_t mpfrRoundMode (APFloat::RoundMode mode) {
    switch (mode) {
    case APFloat::RNDN:  return MPFR_RNDN;
    case APFloat::RNDZ:  return MPFR_RNDZ;
    case APFloat::RNDU:  return MPFR_RNDU;
    case APFloat::RNDD:  return MPFR_RNDD;
    case APFloat::RNDA:  return MPFR_RNDA;
    case APFloat::RNDF:  return MPFR_RNDF;
    case APFloat::RNDNA: return MPFR_RNDNA;
    default: SHAREMIND_ABORT("mRM %d", static_cast<int>(mode));
    }
}

template <typename T>
struct mpfrResultType {
    inline static T unwrap (const T& x) {
        return x;
    }
};

template <>
struct mpfrResultType<APFloat> {
    inline static const mpfr_t& unwrap (const APFloat& x) {
        return x.bits ();
    }
};

template <>
struct mpfrResultType<APFloat::RoundMode> {
    inline static mpfr_rnd_t unwrap (const APFloat::RoundMode& mode) {
        return mpfrRoundMode (mode);
    }
};

template <typename ...Args>
inline  APFloat::prec_t unwrapPrec (const APFloat& x, const Args&...) {
    return x.getPrec ();
}

// Apply a function with MPFR interface to our objects.
template <typename F, typename ...Args>
inline APFloat mpfrApply (F op, const Args& ...args) {
    APFloat result (unwrapPrec (args...));
    op (result.bits (), mpfrResultType<Args>::unwrap (args)...);
    return std::move (result);
}


} // namespace anonymous

/*******************************************************************************
  APFloat
*******************************************************************************/

// TODO: this function breaks MPFR abstraction
bool APFloat::BitwiseCmp::cmpMpfrStructs (const mpfr_srcptr x, const mpfr_srcptr y) {
    if (x->_mpfr_prec < y->_mpfr_prec) return true;
    if (x->_mpfr_prec > y->_mpfr_prec) return false;
    if (x->_mpfr_sign < y->_mpfr_sign) return true;
    if (x->_mpfr_sign > y->_mpfr_sign) return false;
    if (x->_mpfr_exp  < y->_mpfr_exp)  return true;
    if (x->_mpfr_exp  > y->_mpfr_exp)  return false;
    const size_t num_bytes = mpfr_custom_get_size (x->_mpfr_prec);
    return std::memcmp (x->_mpfr_d, y->_mpfr_d, num_bytes) < 0;
}

// TODO: don't rely on IEEE representation of float!
uint32_t APFloat::ieee32bits () const {
    assert (getPrec () == floatPrec (DATATYPE_FLOAT32));
    #if MPFR_VERSION >= 0x030000
    float float_result = mpfr_get_flt (m_value, SECREC_CONSTANT_MPFR_RNDN);
    #else
    float float_result = mpfr_get_ld (m_value, SECREC_CONSTANT_MPFR_RNDN);
    #endif
    auto result = new (&float_result) uint32_t;
    return *result;
}

// TODO: don't rely on IEEE representation of double!
uint64_t APFloat::ieee64bits () const {
    assert (getPrec () == floatPrec (DATATYPE_FLOAT64));
    double double_result = mpfr_get_d (m_value, SECREC_CONSTANT_MPFR_RNDN);
    auto result = new (&double_result) uint64_t;
    return *result;
}

APFloat::APFloat (prec_t p, StringRef str) {
    mpfr_init2 (m_value, p);
    if (mpfr_set_str (m_value, str.str ().c_str (), 10, SECREC_CONSTANT_MPFR_RNDN) != 0) {
        mpfr_clear (m_value);
        throw std::logic_error ("Invalid floating point string literal!");
    }
}

APFloat::APFloat (prec_t p) {
    mpfr_init2 (m_value, p);
}

APFloat::APFloat (prec_t p, uint64_t value) {
    mpfr_init2 (m_value, p);
    mpfr_set_ui (m_value, value, SECREC_CONSTANT_MPFR_RNDN);
}

APFloat::APFloat (prec_t p, const APFloat& x, RoundMode mode) {
    mpfr_init2 (m_value, p);
    mpfr_set (m_value, x.m_value, mpfrRoundMode (mode));
}

APFloat::~APFloat () {
    mpfr_clear (m_value);
}

APFloat::APFloat (const APFloat& apf) {
    mpfr_init2 (m_value, apf.getPrec ());
    mpfr_set (m_value, apf.m_value, SECREC_CONSTANT_MPFR_RNDN);
}

APFloat& APFloat::operator = (const APFloat& apf) {
    mpfr_set (m_value, apf.m_value, SECREC_CONSTANT_MPFR_RNDN);
    return *this;
}

void APFloat::assign (const APFloat& x, APFloat::RoundMode mode) {
    mpfr_set (m_value, x.m_value, mpfrRoundMode (mode));
}

APFloat APFloat::add (APFloat x, APFloat y, APFloat::RoundMode mode) {
    return mpfrApply (mpfr_add, x, y, mode);
}

APFloat APFloat::sub (APFloat x, APFloat y, APFloat::RoundMode mode) {
    return mpfrApply (mpfr_sub, x, y, mode);
}

APFloat APFloat::mul (APFloat x, APFloat y, APFloat::RoundMode mode) {
    return mpfrApply (mpfr_mul, x, y, mode);
}

APFloat APFloat::div (APFloat x, APFloat y, APFloat::RoundMode mode) {
    return mpfrApply (mpfr_div, x, y, mode);
}

bool APFloat::cmp (APFloat x, APFloat y, APFloat::CmpMode mode) {
    switch (mode) {
    case EQ: return mpfr_equal_p (x.bits (), y.bits ()) != 0;
    case NE: return mpfr_equal_p (x.bits (), y.bits ()) == 0;
    case LT: return mpfr_cmp (x.bits (), y.bits ()) < 0;
    case GT: return mpfr_cmp (x.bits (), y.bits ()) > 0;
    case LE: return mpfr_cmp (x.bits (), y.bits ()) <= 0;
    case GE: return mpfr_cmp (x.bits (), y.bits ()) >= 0;
    default: SHAREMIND_ABORT("AFC %d", static_cast<int>(mode));
    }
}

APFloat APFloat::minus (APFloat x, APFloat::RoundMode mode) {
    return mpfrApply (mpfr_neg, x, mode);
}

APFloat APFloat::makeUnsigned (prec_t p, uint64_t value, APFloat::RoundMode mode) {
    auto result = APFloat (p);
    mpfr_set_ui (result.bits (), value, mpfrRoundMode (mode));
    return result;
}

APFloat APFloat::makeSigned (prec_t p, int64_t value, APFloat::RoundMode mode) {
    auto result = APFloat (p);
    mpfr_set_si (result.bits (), value, mpfrRoundMode (mode));
    return result;
}

uint64_t APFloat::getUnsigned (APFloat x, APFloat::RoundMode mode) {
    return mpfr_get_ui (x.bits (), mpfrRoundMode (mode));
}

int64_t APFloat::getSigned (APFloat x, APFloat::RoundMode mode) {
    return mpfr_get_si (x.bits (), mpfrRoundMode (mode));
}

std::ostream& operator << (std::ostream& os, const APFloat& apf) {
    const size_t buff_size = 256;
    char buff [buff_size] = {};
    const int n = mpfr_snprintf (buff, buff_size, "%.RNg", apf.m_value);
    if (n < 0) {
        os.setstate (std::ios::failbit);
        assert (false);
        return os;
    }

    os.write (buff, n);
    return os;
}

APFloat::prec_t floatPrec (SecrecDataType type) {
    switch (type) {
    case DATATYPE_FLOAT32: return 24;
    case DATATYPE_FLOAT64: return 53;
    default:  assert (false && "floatPrec: Unsupported type!"); return 0;
    }
}

APFloat::prec_t floatPrec (DataType* type) {
    assert (type != nullptr && type->isPrimitive ());
    return floatPrec (static_cast<DataTypePrimitive*>(type)->secrecDataType ());
}


} // namespace SecreC
