#ifndef SECREC_CONSTANT_H
#define SECREC_CONSTANT_H

#include <cassert>
#include <stdint.h>
#include <mpfr.h>
#include <iostream>
#include <stdexcept>

#include "symbol.h"


#if MPFR_VERSION >= 0x030000
#define SECREC_CONSTANT_MPFR_RNDN MPFR_RNDN
#else
#define SECREC_CONSTANT_MPFR_RNDN GMP_RNDN
#endif

namespace SecreC {

class Context;

SymbolConstant* defaultConstant (Context& cxt, SecrecDataType ty);
SymbolConstant* numericConstant (Context& cxt, SecrecDataType ty, uint64_t value);

/******************************************************************
  APInt
******************************************************************/

class APInt {
public: /* Types: */

    struct BitwiseCmp {
        inline bool operator () (const APInt& x, const APInt& y) const {
            return x.m_numBits < y.m_numBits ||
                (x.m_numBits == y.m_numBits && x.m_value < y.m_value);
        }
    };

private: /* Types: */
    typedef uint64_t value_type;
    enum {
        BitsPerWord = static_cast<unsigned>(sizeof (value_type)) * 8,
        WordSize = static_cast<unsigned>(sizeof (value_type))
    };

public: /* Methods: */

    APInt (unsigned numBits, uint64_t value)
        : m_numBits (numBits)
        , m_value (value)
    { }

    APInt trunc (unsigned numBits) {
        assert (numBits && numBits < m_numBits);
        APInt val (numBits, m_value);
        val.clearUnusedBits ();
        return val;
    }

    APInt zeroExtend (unsigned numBits) {
        assert (numBits <= BitsPerWord);
        assert (numBits > m_numBits);
        APInt val (numBits, m_value);
        val.clearUnusedBits ();
        return val;
    }

    bool isNegative () const {
        assert (m_numBits > 0);
        return (*this)[m_numBits - 1];
    }

    bool operator [] (size_t i) const {
        assert (i < m_numBits);
        return m_value & (value_type (1) << i);
    }

    // TODO: temporary solution
    operator uint64_t () const {
        return m_value;
    }

private:

    APInt& clearUnusedBits () {
        const unsigned wordSize = m_numBits % BitsPerWord;
        if (wordSize == 0)
            return *this;

        const value_type mask = ~value_type(0) >> (BitsPerWord - wordSize);
        m_value &= mask;
        return *this;
    }

private: /* Fields: */
    unsigned   m_numBits;
    value_type m_value;
};

/******************************************************************
  APFloat
******************************************************************/

class APFloat {
public: /* Types: */

    typedef mpfr_prec_t prec_t;

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
    char buff [buff_size];
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
    ConstantInt (TypeNonVoid* type, const APInt& value)
        : SymbolConstant (type)
        , m_value (value)
    { }

public:

    static ConstantInt* get (Context& cxt, SecrecDataType type, uint64_t value);
    static ConstantInt* getBool (Context& cxt, bool value);

    const APInt& value () const { return m_value; }

protected:
    void print (std::ostream& os) const;

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

    static ConstantFloat* get (Context& cxt, SecrecDataType type, uint64_t value);
    static ConstantFloat* get (Context& cxt, SecrecDataType type, StringRef str);
    static ConstantFloat* get (Context& cxt, SecrecDataType type, const APFloat& value);

    const APFloat& value () const { return m_value; }

protected:
    void print (std::ostream& os) const;

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
        , m_value (value)
    { }

public:

    static ConstantString* get (Context& cxt, StringRef str);
    StringRef value () const { return m_value; }

protected:
    void print (std::ostream& os) const;

private: /* Fields: */
    const StringRef m_value;
};

} // namespace SecreC

#endif
