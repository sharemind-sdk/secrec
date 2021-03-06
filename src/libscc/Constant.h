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
SymbolConstant* numericConstant (SecrecDataType ty, uint64_t value);

SymbolConstant* defaultConstant (Context& cxt, const DataType* ty);
SymbolConstant* numericConstant (const DataType* ty, uint64_t value);

/******************************************************************
  ConstantInt
******************************************************************/

class ConstantInt : public SymbolConstant {
private: /* Methods: */
    ConstantInt (const TypeNonVoid* type, APInt value)
        : SymbolConstant (type)
        , m_value (std::move(value))
    { }

public:

    static ConstantInt* get (SecrecDataType type, uint64_t value);
    static ConstantInt* get (const DataType* type, uint64_t value);
    static ConstantInt* getBool (bool value);

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

    ConstantFloat (const TypeNonVoid* type, const APFloat& value)
        : SymbolConstant (type)
        , m_value (value)
    { }

public:

    static ConstantFloat* get (const DataType* type, uint64_t value);
    static ConstantFloat* get (const DataType* type, StringRef str);
    static ConstantFloat* get (const DataType* type, const APFloat& value);

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

    ConstantString (const TypeNonVoid* type, StringRef value)
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
