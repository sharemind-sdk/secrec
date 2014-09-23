#ifndef SECREC_CONSTANT_H
#define SECREC_CONSTANT_H

#include "Symbol.h"
#include "APInt.h"

#include <cassert>
#include <iostream>
#include <mpfr.h>
#include <stdexcept>
#include <stdint.h>
#include <utility>


#if MPFR_VERSION >= 0x030000
#define SECREC_CONSTANT_MPFR_RNDN MPFR_RNDN
#else
#define SECREC_CONSTANT_MPFR_RNDN GMP_RNDN
#endif

namespace SecreC {

class Context;

SymbolConstant* defaultConstant (Context& cxt, SecrecDataType ty);
SymbolConstant* numericConstant (Context& cxt, SecrecDataType ty, uint64_t value);

SymbolConstant* defaultConstant (Context& cxt, DataType* ty);
SymbolConstant* numericConstant (Context& cxt, DataType* ty, uint64_t value);

/******************************************************************
  APFloat
******************************************************************/

class APFloat {
public: /* Types: */

    using prec_t = mpfr_prec_t;

    struct BitwiseCmp {
        inline bool operator () (const APFloat& x, const APFloat& y) const {
            return cmpMpfrStructs (x.m_value, y.m_value);
        }
    private:
        static bool cmpMpfrStructs (const mpfr_srcptr x, const mpfr_srcptr y);
    };

public: /* Methods: */

    APFloat (prec_t p, StringRef str) {
        mpfr_init2 (m_value, p);
        if (mpfr_set_str (m_value, str.str ().c_str (), 10, SECREC_CONSTANT_MPFR_RNDN) != 0) {
            mpfr_clear (m_value);
            throw std::logic_error ("Invalid floating point string literal!");
        }
    }

    APFloat (prec_t p, uint64_t value) {
        mpfr_init2 (m_value, p);
        mpfr_set_ui (m_value, value, SECREC_CONSTANT_MPFR_RNDN);
    }

    ~APFloat () {
        mpfr_clear (m_value);
    }

    APFloat (const APFloat& apf) {
        mpfr_init2 (m_value, apf.getPrec ());
        mpfr_set (m_value, apf.m_value, SECREC_CONSTANT_MPFR_RNDN);
    }

    APFloat& operator = (const APFloat& apf) {
        mpfr_set (m_value, apf.m_value, SECREC_CONSTANT_MPFR_RNDN);
        return *this;
    }

    friend std::ostream& operator << (std::ostream& os, const APFloat& apf);

    prec_t getPrec () const {
        return mpfr_get_prec (m_value);
    }

    uint32_t ieee32bits () const;
    uint64_t ieee64bits () const;

private: /* Fields: */
    mpfr_t m_value;
};

// TODO: respect float formatting?
inline std::ostream& operator << (std::ostream& os, const APFloat& apf) {
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


/******************************************************************
  ConstantInt
******************************************************************/

class ConstantInt : public SymbolConstant {
private: /* Methods: */
    ConstantInt (TypeNonVoid* type, APInt value)
        : SymbolConstant (type)
        , m_value (std::move(value))
    { }

public:

    static ConstantInt* get (Context& cxt, SecrecDataType type, uint64_t value);
    static ConstantInt* get (Context& cxt, DataType* type, uint64_t value);
    static ConstantInt* getBool (Context& cxt, bool value);

    const APInt& value () const { return m_value; }

protected:
    void print (std::ostream& os) const override;

private: /* Fields: */
    const APInt m_value;
};

/******************************************************************
  ConstantFloat
******************************************************************/

class ConstantFloat : public SymbolConstant {
private: /* Methods: */

    ConstantFloat (TypeNonVoid* type, const APFloat& value)
        : SymbolConstant (type)
        , m_value (value)
    { }

public:

    static ConstantFloat* get (Context& cxt, DataType* type, uint64_t value);
    static ConstantFloat* get (Context& cxt, DataType* type, StringRef str);
    static ConstantFloat* get (Context& cxt, DataType* type, const APFloat& value);

    const APFloat& value () const { return m_value; }

protected:
    void print (std::ostream& os) const override;

private: /* Fields: */
    const APFloat m_value;
};

/******************************************************************
  ConstantString
******************************************************************/

class ConstantString : public SymbolConstant {
private: /* Methods: */

    ConstantString (TypeNonVoid* type, StringRef value)
        : SymbolConstant (type)
        , m_value (std::move(value))
    { }

public:

    static ConstantString* get (Context& cxt, StringRef str);
    StringRef value () const { return m_value; }

protected:
    void print (std::ostream& os) const override;

private: /* Fields: */
    const StringRef m_value;
};

} // namespace SecreC

#endif
