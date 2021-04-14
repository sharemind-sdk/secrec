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

#ifndef SECREC_APFLOAT_H
#define SECREC_APFLOAT_H

#include <sharemind/StringView.h>
#include "ParserEnums.h"

// NOTE: cstddef needs to be included before mpfr.h to support gcc-4.9
#include <cstddef>
#include <cstdint>
#include <mpfr.h>

#if MPFR_VERSION >= 0x030000
#define SECREC_CONSTANT_MPFR_RNDN MPFR_RNDN
#else
#define SECREC_CONSTANT_MPFR_RNDN GMP_RNDN
#endif

namespace SecreC {

class DataType;

/******************************************************************
  APFloat
******************************************************************/

class APFloat {
public: /* Types: */

    using prec_t = mpfr_prec_t;

    enum RoundMode {
        RNDN = 0,
        RNDZ,
        RNDU,
        RNDD,
        RNDA,
        RNDF,
        RNDNA=-1
    };

    enum CmpMode {
        EQ, NE, LT, GT, LE, GE
    };

    struct BitwiseCmp {
        inline bool operator () (const APFloat& x, const APFloat& y) const {
            return cmpMpfrStructs (x.m_value, y.m_value);
        }
    private:
        static bool cmpMpfrStructs (const mpfr_srcptr x, const mpfr_srcptr y);
    };

public: /* Methods: */

    explicit APFloat (prec_t p);
    APFloat(prec_t p, sharemind::StringView str);
    APFloat (prec_t p, uint64_t value);
    APFloat (prec_t p, const APFloat& x, RoundMode mode = RNDN);

    ~APFloat ();
    APFloat (const APFloat& apf);
    APFloat& operator = (const APFloat& apf);

    void assign (const APFloat& x, RoundMode mode = RNDN);

    static APFloat add (APFloat x, APFloat y, RoundMode mode = RNDN);
    static APFloat sub (APFloat x, APFloat y, RoundMode mode = RNDN);
    static APFloat mul (APFloat x, APFloat y, RoundMode mode = RNDN);
    static APFloat div (APFloat x, APFloat y, RoundMode mode = RNDN);
    static bool cmp (APFloat x, APFloat y, CmpMode mode);
    static APFloat minus (APFloat x, RoundMode mode = RNDN);
    static APFloat makeSigned (prec_t p, int64_t value, RoundMode mode = RNDN);
    static APFloat makeUnsigned (prec_t p, uint64_t value, RoundMode mode = RNDN);
    static uint64_t getUnsigned (APFloat x, RoundMode mode = RNDN);
    static int64_t getSigned (APFloat x, RoundMode mode = RNDN);

    friend std::ostream& operator << (std::ostream& os, const APFloat& apf);

    prec_t getPrec () const {
        return mpfr_get_prec (m_value);
    }

    uint32_t ieee32bits () const;
    uint64_t ieee64bits () const;

    mpfr_t& bits () { return m_value; }
    const mpfr_t& bits () const { return m_value; }

private: /* Fields: */
    mpfr_t m_value;
};

APFloat::prec_t floatPrec (SecrecDataType type);
APFloat::prec_t floatPrec (const DataType* type);

// TODO: respect float formatting?
std::ostream& operator << (std::ostream& os, const APFloat& apf);

} // namespace SecreC

#endif // SECREC_APFLOAT_H
