/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_APFLOAT_H
#define SECREC_APFLOAT_H

#include "StringRef.h"
#include "ParserEnums.h"

#include <mpfr.h>
#include <cstddef>
#include <cstdint>

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
    APFloat (prec_t p, StringRef str);
    APFloat (prec_t p, uint64_t value);
    ~APFloat ();
    APFloat (const APFloat& apf);
    APFloat& operator = (const APFloat& apf);

    static APFloat add (APFloat x, APFloat y, RoundMode mode = RNDN);
    static APFloat sub (APFloat x, APFloat y, RoundMode mode = RNDN);
    static APFloat mul (APFloat x, APFloat y, RoundMode mode = RNDN);
    static APFloat div (APFloat x, APFloat y, RoundMode mode = RNDN);
    static bool cmp (APFloat x, APFloat y, CmpMode mode);
    static APFloat minus (APFloat x, RoundMode mode = RNDN);

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
APFloat::prec_t floatPrec (DataType* type);

// TODO: respect float formatting?
std::ostream& operator << (std::ostream& os, const APFloat& apf);

} // namespace SecreC

#endif // SECREC_APFLOAT_H
