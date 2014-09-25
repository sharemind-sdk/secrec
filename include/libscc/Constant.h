#ifndef SECREC_CONSTANT_H
#define SECREC_CONSTANT_H

#include "Symbol.h"
#include "APFloat.h"
#include "APInt.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <stdint.h>
#include <utility>

namespace SecreC {

class Context;

SymbolConstant* defaultConstant (Context& cxt, SecrecDataType ty);
SymbolConstant* numericConstant (Context& cxt, SecrecDataType ty, uint64_t value);

SymbolConstant* defaultConstant (Context& cxt, DataType* ty);
SymbolConstant* numericConstant (Context& cxt, DataType* ty, uint64_t value);

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
