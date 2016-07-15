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

#include "analysis/ConstantFolding.h"

#include "Constant.h"
#include "Context.h"
#include "ContextImpl.h"
#include "DataType.h"
#include "Imop.h"
#include "SecurityType.h"
#include "SymbolTable.h"
#include "StringTable.h"
#include "Types.h"

#include <iostream>
#include <list>
#include <memory>
#include <sharemind/abort.h>
#include <sstream>
#include <vector>


namespace SecreC {

/*******************************************************************************
  AbstractValue
*******************************************************************************/

/*
 * TODO: add FloatValue
 * TODO: split IntValue to signed and unsigned
 * TODO: globals are not hangled properly and even obvious constants end up as NAC.
 */

class ArrayValue;
class IntValue;
class StringValue;

enum ValueTag {
    VINT,
    VARR,
    VSTR,
    VFLOAT
};

class AbstractValue {
public: /* Methods: */
    explicit AbstractValue (ValueTag tag)
        : m_tag (tag)
    { }

    virtual ~AbstractValue () { }
    virtual std::string toString () const = 0;
    virtual SymbolConstant* toConstant (Context& cxt, StringTable& st, const DataType* t) const = 0;
    ValueTag tag () const { return m_tag; }

private: /* Fields: */
    const ValueTag m_tag;
};

/*******************************************************************************
  IntValue
*******************************************************************************/

class IntValue : public AbstractValue, public APInt {
public: /* Methods: */

    IntValue ()
        : AbstractValue (VINT)
        , isSigned (false)
    { }

    IntValue (bool isSigned, APInt value)
        : AbstractValue (VINT)
        , APInt (value)
        , isSigned (isSigned)
    { }

    IntValue (APInt value)
        : IntValue (false, value)
    { }

    IntValue (bool value)
        : IntValue (false, value)
    { }

    const APInt& getValue () const { return *this; }
    std::string toString () const override final;
    SymbolConstant* toConstant (Context& cxt, StringTable& st, const DataType* t) const override final;

public: /* Methods: */
    const bool isSigned;
};

bool operator < (const IntValue& x, const IntValue& y) {
    return std::tie (x.isSigned, (APInt&) x) < std::tie (y.isSigned, (APInt&) y);
}

std::string IntValue::toString () const {
    std::ostringstream ss;
    if (isSigned)
        sprint (ss);
    else
        uprint (ss);
    return ss.str ();
}

SymbolConstant* IntValue::toConstant (Context&, StringTable&, const DataType* t) const  {
    return ConstantInt::get (t, bits ());
}

/*******************************************************************************
  StringValue
*******************************************************************************/

class StringValue : public AbstractValue, public std::string {
public: /* Methods: */
    StringValue (std::string val)
        : AbstractValue (VSTR)
        , std::string (std::move (val))
    { }

    const std::string& getValue () const { return *this; }
    std::string toString () const override final;
    SymbolConstant* toConstant (Context& cxt, StringTable& st, const DataType* t) const override final;
};

std::string StringValue::toString () const {
    return *this;
}

SymbolConstant* StringValue::toConstant (Context& cxt, StringTable& st, const DataType*) const {
    return ConstantString::get (cxt, *st.addString (*this));
}

/*******************************************************************************
  FloatValue
*******************************************************************************/

class FloatValue : public AbstractValue, public APFloat {
public: /* Methods: */

    FloatValue (APFloat val)
        : AbstractValue (VFLOAT)
        , APFloat (std::move (val))
    { }

    const APFloat& getValue () const { return *this; }
    std::string toString () const override final;
    SymbolConstant* toConstant (Context& cxt, StringTable&, const DataType* t) const override final;
};

bool operator < (const FloatValue& x, const FloatValue& y) {
    return APFloat::BitwiseCmp()(x, y);
}

std::string FloatValue::toString () const {
    std::ostringstream os;
    os << *this;
    return os.str ();
}

SymbolConstant* FloatValue::toConstant (Context&, StringTable&, const DataType* t) const {
    return ConstantFloat::get (t, *this);
}

/*******************************************************************************
  ArrayValue
*******************************************************************************/

class ArrayValue : public AbstractValue, public std::vector<Value> {
public: /* Methods: */
    ArrayValue (size_t size, Value elem)
        : AbstractValue (VARR)
        , std::vector<Value> (size, elem)
    { }

    ArrayValue (std::vector<Value> elems)
        : AbstractValue (VARR)
        , std::vector<Value>(std::move (elems))
    { }

    const std::vector<Value>& getValue () const { return *this; }
    std::string toString () const override final;
    SymbolConstant* toConstant (Context&, StringTable&, const DataType*) const override final {
        return nullptr;
    }
};

std::string ArrayValue::toString () const {
    std::stringstream ss;
    ss << '{';
    for (size_t i = 0; i < size (); ++ i) {
        if (i != 0)
            ss << ", ";
        ss << (*this)[i].toString ();
    }

    ss << '}';
    return ss.str ();
}

/*******************************************************************************
  Value
*******************************************************************************/

std::string Value::toString () const {
    std::ostringstream ss;
    switch (m_kind) {
        case UNDEF:    ss << "UNDEF"; break;
        case CONSTANT: ss << m_value->toString(); break;
        case NAC:      ss << "NAC"; break;
        default:       assert (false);
    }

    return ss.str ();
}

/*******************************************************************************
  ValueFactory
*******************************************************************************/

/**
 * Here we guarantee that each value is represented using a unique pointer.
 * This way we can compare two values for equality via pointer equality.
 * The implementation works because std::set is guaranteed not to invalidate
 * pointers (or iterators).
 */
class ValueFactory {
public: /* Methods: */

    Value get (const IntValue& val) { return get (m_intValues, val); }
    Value get (const ArrayValue& val) { return get (m_arrayValues, val); }
    Value get (const StringValue& val) { return get (m_stringValues, val); }
    Value get (const FloatValue& val) { return get (m_floatValues, val); }

private:

    template <typename S, typename T>
    static Value get (S& values, const T& val) {
        return &*values.insert (val).first;
    }

private: /* Fields: */
    std::set<IntValue>    m_intValues;
    std::set<ArrayValue>  m_arrayValues;
    std::set<StringValue> m_stringValues;
    std::set<FloatValue>  m_floatValues;
};

namespace /* anonymous */ {

template <ValueTag tag> struct GetTagType { };
template <> struct GetTagType<VINT> { using Type = IntValue; };
template <> struct GetTagType<VFLOAT> { using Type = FloatValue; };
template <> struct GetTagType<VARR> { using Type = ArrayValue; };
template <> struct GetTagType<VSTR> { using Type = StringValue; };

template <ValueTag tag>
inline const typename GetTagType<tag>::Type& as (const AbstractValue* v) {
    assert (v != nullptr && v->tag () == tag);
    return *static_cast<const typename GetTagType<tag>::Type*>(v);
}

template <ValueTag tag>
inline std::vector<const typename GetTagType<tag>::Type*> as (const std::vector<const AbstractValue*>& vs) {
    std::vector<const typename GetTagType<tag>::Type*> result;
    result.reserve (vs.size ());
    for (auto v : vs) {
        assert (v != nullptr && v->tag () == tag);
        result.push_back (static_cast<const typename GetTagType<tag>::Type*>(v));
    }

    return std::move (result);
}

/*******************************************************************************
  StringValue
*******************************************************************************/

std::string strAdd (const std::string& x, const std::string& y) {
    return StringValue (x + y);
}

IntValue strCmp (Imop::Type iType, const std::string& x, const std::string& y) {
    switch (iType) {
    case Imop::EQ: return x == y;
    case Imop::NE: return x != y;
    case Imop::LT: return x <  y;
    case Imop::GT: return x >  y;
    case Imop::GE: return x >= y;
    case Imop::LE: return x <= y;
    default:
        assert (false && "Invalid string comparison!");
        return IntValue ();
    }
}

/*******************************************************************************
  ArrayValue
*******************************************************************************/

Value arrLoad (const ArrayValue& a, const IntValue& i) {
    if (i.bits () < a.size ())
        return a[i.bits ()];
    else
        return Value::undef ();
}

ArrayValue arrStore (const ArrayValue& a, const IntValue& i, Value v) {
    if (i.bits () < a.size ()) {
        std::vector<Value> elems = a;
        elems[i.bits ()] = v;
        return ArrayValue (std::move (elems));
    }
    else {
        return a;
    }
}

/*******************************************************************************
  Value
*******************************************************************************/

Value meetValue (ValueFactory& factory, Value x, Value y) {
    if (x.isUndef ()) return y;
    if (y.isUndef ()) return x;
    if (x.isNac () || y.isNac ())
        return Value::nac ();

    const auto xv = x.value ();
    const auto yv = y.value ();

    if (xv == yv)
        return xv;

    if (xv->tag () == VARR && yv->tag () == VARR) {
        const std::vector<Value>& xs = *static_cast<const ArrayValue*>(xv);
        const std::vector<Value>& ys = *static_cast<const ArrayValue*>(yv);
        if (xs.size () == ys.size ()) {
            std::vector<Value> elems;
            elems.reserve (xs.size ());
            for (size_t i = 0; i < xs.size (); ++ i) {
                elems.push_back (meetValue (factory, xs[i], ys[i]));
            }

            return factory.get (ArrayValue (std::move (elems)));
        }
    }

    return Value::nac ();
}

// Lattice comparison:
bool leValue (Value x, Value y) {
    if (x.isUndef () || y.isNac ()) return true;
    if (x.isNac () || y.isUndef ()) return false;

    assert (x.isConst () && y.isConst ());
    const auto xv = x.value ();
    const auto yv = y.value ();
    if (xv == yv) return true;

    if (xv->tag () == VARR && yv->tag () == VARR) {
        const std::vector<Value>& xs = *static_cast<const ArrayValue*>(xv);
        const std::vector<Value>& ys = *static_cast<const ArrayValue*>(yv);
        if (xs.size () == ys.size ()) {
            for (size_t i = 0; i < xs.size (); ++ i) {
                if (! leValue (xs[i], ys[i]))
                    return false;
            }

            return true;
        }
    }

    return false;
}

bool ltValue (Value x, Value y) {
    return x != y && leValue (x, y);
}

template <typename Iter>
Value meetRange (ValueFactory& factory, Iter begin, Iter end) {
    auto current = Value::undef ();
    for (auto i = begin; i != end; ++ i)
        current = meetValue (factory, current, *i);
    return current;
}

Value valueStore (ValueFactory& factory, Value a, Value i, Value v) {
    if (a.isUndef () || a.isNac ()) return a;
    if (i.isUndef () || i.isNac ()) return i;
    const auto av = static_cast<const ArrayValue*>(a.value ());
    const auto iv = static_cast<const IntValue*>(i.value ());
    return factory.get (arrStore (*av, *iv, v));
}

Value valueLoad (ValueFactory& factory, Value a, Value i) {
    if (a.isUndef () || a.isNac ()) return a;
    assert (a.value ()->tag () == VARR);
    const auto av = static_cast<const ArrayValue*>(a.value ());
    const std::vector<Value>& elems = *av;

    // For undefined, or non-constant indices return meet of all values
    // TODO: not sure if this is correct.
    if (i.isUndef () || i.isNac ())
        return meetRange (factory, elems.begin (), elems.end ());

    assert (i.value ()->tag () == VINT);
    const auto iv = static_cast<const IntValue*>(i.value ());
    return arrLoad (*av, *iv);
}

/**
 */

Value exprValue (ValueFactory& factory, const Imop& imop, const std::vector<Value>& args);

inline Value makeSigned (ValueFactory& factory, const APInt& x) {
    return factory.get (IntValue (true, x));
}

inline Value makeUnsigned (ValueFactory& factory, const APInt& x) {
    return factory.get (IntValue (false, x));
}

Value intEval (ValueFactory& f, const Imop& imop, const std::vector<const IntValue*>& args) {
    switch (imop.type ()) {
    case Imop::ADD:        return makeSigned (f, APInt::add (*args[0], *args[1]));
    case Imop::SUB:        return makeSigned (f, APInt::sub (*args[0], *args[1]));
    case Imop::MUL:        return makeSigned (f, APInt::mul (*args[0], *args[1]));
    case Imop::DIV:        return makeSigned (f, APInt::sdiv (*args[0], *args[1]));
    case Imop::MOD:        return makeSigned (f, APInt::srem (*args[0], *args[1]));
    case Imop::LE:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::SLE));
    case Imop::LT:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::SLT));
    case Imop::GE:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::SGE));
    case Imop::GT:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::SGT));
    case Imop::LAND:
    case Imop::BAND:       return makeSigned (f, APInt::AND (*args[0], *args[1]));
    case Imop::LOR:
    case Imop::BOR:        return makeSigned (f, APInt::OR (*args[0], *args[1]));
    case Imop::XOR:        return makeSigned (f, APInt::XOR (*args[0], *args[1]));
    case Imop::SHL:        return makeSigned (f, APInt::shl (*args[0], *args[1]));
    case Imop::SHR:        return makeSigned (f, APInt::ashr (*args[0], *args[1]));
    case Imop::EQ:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::EQ));
    case Imop::NE:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::NE));
    case Imop::UINV:
    case Imop::UNEG:       return makeSigned (f, APInt::inv (*args[0]));
    case Imop::UMINUS:     return makeSigned (f, APInt::minus (*args[0]));
    default:
        assert (false && "Invalid signed integer operation.");
        return Value::nac ();
    }
}

Value uintEval (ValueFactory& f, const Imop& imop, const std::vector<const IntValue*>& args) {
    switch (imop.type ()) {
    case Imop::ADD:        return makeUnsigned (f, APInt::add (*args[0], *args[1]));
    case Imop::SUB:        return makeUnsigned (f, APInt::sub (*args[0], *args[1]));
    case Imop::MUL:        return makeUnsigned (f, APInt::mul (*args[0], *args[1]));
    case Imop::DIV:        return makeUnsigned (f, APInt::udiv (*args[0], *args[1]));
    case Imop::MOD:        return makeUnsigned (f, APInt::urem (*args[0], *args[1]));
    case Imop::LE:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::ULE));
    case Imop::LT:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::ULT));
    case Imop::GE:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::UGE));
    case Imop::GT:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::UGT));
    case Imop::LAND:
    case Imop::BAND:       return makeUnsigned (f, APInt::AND (*args[0], *args[1]));
    case Imop::LOR:
    case Imop::BOR:        return makeUnsigned (f, APInt::OR (*args[0], *args[1]));
    case Imop::XOR:        return makeUnsigned (f, APInt::XOR (*args[0], *args[1]));
    case Imop::SHL:        return makeUnsigned (f, APInt::shl (*args[0], *args[1]));
    case Imop::SHR:        return makeUnsigned (f, APInt::lshr (*args[0], *args[1]));
    case Imop::EQ:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::EQ));
    case Imop::NE:         return makeUnsigned (f, APInt::cmp (*args[0], *args[1], APInt::NE));
    case Imop::UINV:
    case Imop::UNEG:       return makeUnsigned (f, APInt::inv (*args[0]));
    case Imop::UMINUS:     return makeUnsigned (f, APInt::minus (*args[0]));
    default:
        assert (false && "Invalid unsigned integer operation.");
        return Value::nac ();
    }
}

Value strEval (ValueFactory& f, const Imop& imop, const std::vector<const StringValue*>& args) {
    switch (imop.type ()) {
    case Imop::LE:
    case Imop::LT:
    case Imop::GE:
    case Imop::GT:
    case Imop::EQ:
    case Imop::NE:
        return makeUnsigned (f, strCmp (imop.type (), *args[0], *args[1]));
    case Imop::ADD:
        return f.get (strAdd (*args[0], *args[1]));
    default:
        assert (false && "Invalid string operation.");
        return Value::nac ();
    }
}

Value floatEval (ValueFactory& f, const Imop& imop, const std::vector<const FloatValue*>& args) {
    switch (imop.type ()) {
    case Imop::ADD:    return f.get (APFloat::add (*args[0], *args[1]));
    case Imop::SUB:    return f.get (APFloat::sub (*args[0], *args[1]));
    case Imop::MUL:    return f.get (APFloat::mul (*args[0], *args[1]));
    case Imop::DIV:    return f.get (APFloat::div (*args[0], *args[1]));
    case Imop::LE:     return makeUnsigned (f, APFloat::cmp (*args[0], *args[1], APFloat::LE));
    case Imop::LT:     return makeUnsigned (f, APFloat::cmp (*args[0], *args[1], APFloat::LT));
    case Imop::GE:     return makeUnsigned (f, APFloat::cmp (*args[0], *args[1], APFloat::GE));
    case Imop::GT:     return makeUnsigned (f, APFloat::cmp (*args[0], *args[1], APFloat::GT));
    case Imop::EQ:     return makeUnsigned (f, APFloat::cmp (*args[0], *args[1], APFloat::EQ));
    case Imop::NE:     return makeUnsigned (f, APFloat::cmp (*args[0], *args[1], APFloat::NE));
    case Imop::UMINUS: return f.get (APFloat::minus (*args[0]));
    default:
        assert (false && "Invalid unsigned integer operation.");
        return Value::nac ();
    }
}

Value arrEval (ValueFactory& factory, const Imop& imop, const std::vector<const ArrayValue*>& args) {
    std::vector<Value> vs;
    std::vector<Value> elems (args.at(0)->size ());
    for (size_t i = 0; i < elems.size (); ++ i) {
        vs.clear ();
        for (auto arg : args)
            vs.push_back (arg->at (i));

        elems[i] = exprValue (factory, imop, vs);
    }

    return factory.get (elems);
}

Value exprValue (ValueFactory& factory, const Imop& imop, const std::vector<Value>& args) {
    assert (args.size () > 0);

    /*
     * The generic case:
     *  - if all arguments are const apply the operation
     *  - return NAC if any arguments are NAC
     *  - return UNDEF otherwise
     */
    bool anyIsUndef = false;
    std::vector<const AbstractValue*> vs;
    for (const auto v : args) {
        if (v.isConst ()) {
            assert (v.value () != nullptr);
            vs.push_back (v.value ());
            continue;
        }

        if (v.isNac ())
            return Value::nac ();

        if (v.isUndef ())
            anyIsUndef = true;
    }

    if (anyIsUndef) {
        return Value::undef ();
    }

    assert (vs.size () > 0);

    if (imop.type () == Imop::TOSTRING) {
        assert (vs.size () == 1);
        return factory.get (vs[0]->toString ());
    }

    const bool isSigned = isSignedNumericDataType (imop.arg1 ()->secrecType ()->secrecDataType ());
    ValueTag const valueTag = vs.at(0)->tag();
    switch (valueTag) {
    case VINT:
        if (isSigned)
            return intEval (factory, imop, as<VINT>(vs));
        else
            return uintEval (factory, imop, as<VINT>(vs));
    case VFLOAT: return floatEval (factory, imop, as<VFLOAT>(vs));
    case VSTR: return strEval (factory, imop, as<VSTR>(vs));
    case VARR: return arrEval (factory, imop, as<VARR>(vs));
    #ifdef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcovered-switch-default"
    #endif
    default: SHAREMIND_ABORT("eV %d", static_cast<int>(valueTag));
    #ifdef __clang__
    #pragma GCC diagnostic pop
    #endif
    }
}

/**
 * @brief Abstract value casting.
 * Need to consider the following cases:
 * - floatToFloatCast
 * - floatToIntCast
 * - intToFloatCast
 * - intToIntCast
 * - arrayCast
 */
Value castValue (ValueFactory& factory, const TypeNonVoid* resultType, const AbstractValue* arg);

Value castInt (ValueFactory& factory, const TypeNonVoid* resultType, const IntValue& x) {
    const auto dataType = resultType->secrecDataType ();
    assert (dataType->isBuiltinPrimitive ());
    const auto secrecDataType = static_cast<const DataTypeBuiltinPrimitive*>(dataType)->secrecDataType ();

    if (resultType->isFloat ()) {
        const auto prec = floatPrec (secrecDataType);
        if (x.isSigned)
            return factory.get (APFloat::makeSigned (prec, x.bits ()));
        else
            return factory.get (APFloat::makeUnsigned (prec, x.bits ()));
    }
    else {
        const bool destIsSigned = isSignedNumericDataType (dataType);
        const auto destWidth = widthInBitsDataType (secrecDataType);

        if (secrecDataType == DATATYPE_BOOL)
            return factory.get (IntValue (false, x.bits () != 0));

        if (destWidth == x.numBits ())
            return factory.get (IntValue (destIsSigned, x));

        if (destWidth < x.numBits ())
            return factory.get (IntValue (destIsSigned, APInt::trunc (x, destWidth)));

        if (x.isSigned)
            return factory.get (IntValue (destIsSigned, APInt::sextend (x, destWidth)));

        return factory.get (IntValue (destIsSigned, APInt::extend (x, destWidth)));
    }
}

Value castFloat (ValueFactory& factory, const TypeNonVoid* resultType, const FloatValue& x) {
    const auto dataType = resultType->secrecDataType ();
    assert (dataType->isBuiltinPrimitive ());
    const auto secrecDataType = static_cast<const DataTypeBuiltinPrimitive*>(dataType)->secrecDataType ();
    if (resultType->isFloat ()) {
        const auto prec = floatPrec (secrecDataType);
        return factory.get (APFloat (prec, x));
    }
    else {
        const bool isSigned = isSignedNumericDataType (dataType);
        const auto destWidth = widthInBitsDataType (secrecDataType);
        const uint64_t bits = isSigned ? APFloat::getSigned (x) : APFloat::getUnsigned (x);
        return factory.get (IntValue (isSigned, APInt (destWidth, bits)));
    }
}

Value castArr (ValueFactory& factory, const TypeNonVoid* resultType, const ArrayValue& x) {
    std::vector<Value> elems;
    elems.reserve (x.size ());
    for (const auto& v : x) {
        if (v.isNac ()) elems.push_back (Value::nac ());
        if (v.isUndef ()) elems.push_back (Value::undef ());
        if (v.isConst ()) elems.push_back (castValue (factory, resultType, v.value ()));
    }

    return factory.get (elems);
}

Value castValue (ValueFactory& factory, const TypeNonVoid* resultType, const AbstractValue* arg) {
    switch (arg->tag ()) {
    case VINT:   return castInt (factory, resultType, as<VINT>(arg));
    case VFLOAT: return castFloat (factory, resultType, as<VFLOAT>(arg));
    case VARR:   return castArr (factory, resultType, as<VARR>(arg));
    default:
        assert (false && "ICE: invalid CAST argument!");
        return Value::nac ();
    }
}

} // namespace anonymous

/*******************************************************************************
  ConstantFolding
*******************************************************************************/

ConstantFolding::ConstantFolding ()
    : m_values (new ValueFactory ())
{ }

ConstantFolding::~ConstantFolding () {
    delete m_values;
}

void ConstantFolding::addConstant (const Symbol* sym) {
    assert (sym != nullptr);
    if (! sym->isConstant ())
        return;

    const auto it = m_constants.find (sym);
    if (it != m_constants.end ())
        return;

    const auto dataType = sym->secrecType ()->secrecDataType ();
    assert (dataType->isBuiltinPrimitive () && "Non-primitive constant symbol!");
    const auto secrecDataType = static_cast<const DataTypeBuiltinPrimitive*>(dataType)->secrecDataType ();
    if (const auto s = dynamic_cast<const ConstantInt*>(sym)) {
        const bool isSigned = isSignedNumericDataType (secrecDataType);
        const auto v = IntValue (isSigned, s->value ());
        m_constants.insert (it, std::make_pair (sym, m_values->get (v)));
        return;
    }

    if (const auto s = dynamic_cast<const ConstantFloat*>(sym)) {
        const auto v = FloatValue (s->value ());
        m_constants.insert (it, std::make_pair (sym, m_values->get (v)));
        return;
    }

    if (const auto s = dynamic_cast<const ConstantString*>(sym)) {
        std::ostringstream os;
        os << s->value ();
        const auto v = StringValue (os.str ());
        m_constants.insert (it, std::make_pair (sym, m_values->get (v)));
        return;
    }
}

Value ConstantFolding::getVal (const SVM& val, const Symbol* sym) const {
    const auto cit = m_constants.find (sym);
    if (cit != m_constants.end ())
        return cit->second;

    assert (! sym->isConstant () && "ICE: unhandled constant!");
    const auto it = val.find (sym);
    return it == val.end () ? Value::undef () : it->second;
}

void ConstantFolding::setVal (SVM& val, const Symbol* sym, Value x) {
    if (! x.isUndef ())
        val[sym] = x;
    else
        val.erase (sym);
}

void ConstantFolding::transfer (SVM& val, const Imop& imop) const {
    const auto iType = imop.type ();
    auto& factory = *m_values;

    switch (iType) {
    case Imop::PARAM:
    case Imop::DOMAINID:
    case Imop::SYSCALL:
        if (imop.dest () != nullptr)
            setVal (val, imop.dest (), Value::nac ());
        return;
    case Imop::PUSHREF:
        setVal (val, imop.arg1 (), Value::nac ());
        return;
    case Imop::CALL:
        for (auto dest : imop.defRange ())
            setVal (val, dest, Value::nac ());
        return;
    case Imop::COMMENT:
    case Imop::END:
    case Imop::ERROR:
    case Imop::JF:
    case Imop::JT:
    case Imop::JUMP:
    case Imop::PRINT:
    case Imop::PUSHCREF:
    case Imop::RELEASE:
    case Imop::RETCLEAN:
    case Imop::RETURN:
        /* No effect: */
        return;
    case Imop::ASSIGN:
    case Imop::COPY:
    case Imop::CLASSIFY:
    case Imop::DECLASSIFY:
        /* Trivial copy: */
        setVal (val, imop.dest (), getVal (val, imop.arg1 ()));
        return;
    default:
        break;
    }

    if (iType == Imop::DECLARE) {
        setVal (val, imop.dest (), Value::undef ());
        return;
    }

    if (iType == Imop::PUSH) {
        const auto t = imop.arg1 ()->secrecType ();
        const bool isString = t->secrecDataType ()->isString ();
        const bool isArray = t->secrecDimType () > 0;
        const bool isPrivate = t->secrecSecType ()->isPrivate ();
        if (isString || isArray || isPrivate)
            setVal (val, imop.arg1 (), Value::nac ());
        return;
    }

    if (iType == Imop::ALLOC) {
        if (imop.nArgs () == 3) {
            const auto e = getVal (val, imop.arg2 ());
            const auto s = getVal (val, imop.arg1 ());
            if (s.isConst ()) {
                auto r = ArrayValue (as<VINT>(s.value ()).bits (), e);
                setVal (val, imop.dest (), factory.get (r));
            }
            else
                setVal (val, imop.dest (), s);
        } else {
            setVal (val, imop.dest (), Value::undef ());
        }
        return;
    }

    if (iType == Imop::STORE) {
        const auto a = getVal (val, imop.dest ());
        const auto i = getVal (val, imop.arg1 ());
        const auto v = getVal (val, imop.arg2 ());
        setVal (val, imop.dest (), valueStore (factory, a, i, v));
        return;
    }

    if (iType == Imop::LOAD) {
        const auto a = getVal (val, imop.arg1 ());
        const auto i = getVal (val, imop.arg2 ());
        setVal (val, imop.dest (), valueLoad (factory, a, i));
        return;
    }

    if (iType == Imop::CAST) {
        const auto a = getVal (val, imop.arg1 ());
        const auto resultType = imop.dest ()->secrecType ();
        if (a.isConst ())
            setVal (val, imop.dest (), castValue (factory, resultType, a.value ()));
        else
            setVal (val, imop.dest (), a);
        return;
    }

    /*
     * The generic case:
     *  - if all arguments are const apply the operation
     *  - return NAC if any arguments are NAC
     *  - return UNDEF otherwise
     */
    std::vector<Value> args;

    if (imop.isVectorized ()) {
        for (size_t i = 1; i + 1 < imop.nArgs (); ++ i)
            args.push_back (getVal (val, imop.arg (i)));
    }
    else {
        for (const Symbol* sym : imop.useRange ())
            args.push_back (getVal (val, sym));
    }

    setVal (val, imop.dest (), exprValue (factory, imop, args));
    return;
}

void ConstantFolding::start (const Program& program) {
    for (const auto& proc : program)
        for (const auto& block : proc)
            for (const auto& imop : block)
                for (const Symbol* use : imop.useRange ())
                    addConstant (use);
}

void ConstantFolding::startBlock (const Block& block) {
    m_ins[&block].clear ();
}

void ConstantFolding::inFrom (const Block &from, Edge::Label label, const Block &to) {
    if (Edge::isGlobal (label))
        return;

    auto& in = m_ins[&to];
    for (auto const& v : m_outs[&from]) {
        auto it = in.find (v.first);
        if (it == in.end ()) {
            const auto r = meetValue (*m_values, Value::undef (), v.second);
            in.insert (it, std::make_pair (v.first, r));
        }
        else {
            it->second = meetValue (*m_values, it->second, v.second);
        }
    }
}

bool ConstantFolding::makeOuts (const Block& b, const SVM& in, SVM& out) {
    const SVM old = out;
    out = in;
    for (auto& imop : b)
        transfer (out, imop);

    // Check if we increased the (lattice) value of any of the symbols:

    for (const auto& v : old) {
        const auto x = v.second;
        const auto y = getVal (out, v.first);
        if (ltValue (x, y))
            return true;
    }

    for (const auto& v : out) {
        const auto sym = v.first;
        const auto x = getVal (old, sym);
        const auto y = v.second;
        if (ltValue (x, y))
            return true;
    }

    return false;
}

bool ConstantFolding::finishBlock (const Block &b) {
    return makeOuts (b, m_ins[&b], m_outs[&b]);
}

void ConstantFolding::finish () { }

size_t ConstantFolding::optimizeBlock (Context& cxt, StringTable& st,
                                       Block& block) const
{
    const auto it = m_ins.find (&block);
    if (it == m_ins.end ())
        return 0;

    std::vector<std::pair<std::unique_ptr<Imop>, Imop*>> replace;
    SVM val = it->second;

    for (Imop& imop : block) {
        transfer (val, imop);

        if (! imop.isExpr ())
            continue;

        // We can't or there's no reason to optimize the following:
        switch (imop.type ()) {
        case Imop::SYSCALL:
        case Imop::CALL:
        case Imop::PARAM:
            continue;
        case Imop::ASSIGN:
        case Imop::DECLASSIFY:
        case Imop::CLASSIFY:
            if (imop.arg1 ()->isConstant ())
                continue;
            break;
        case Imop::ALLOC:
            if (imop.nArgs () == 2 || imop.arg2 ()->isConstant ())
                continue;
        default:
            break;
        }

        // Can't optimize non-constants:
        const auto symb = imop.dest ();
        const auto dest = getVal (val, symb);
        if (! dest.isConst ())
            continue;

        // Can't optimize arrays as we don't have constant arrays:
        if (dest.value ()->tag () == VARR)
            continue;

        auto dataType = symb->secrecType ()->secrecDataType ();
        bool isPrivate = symb->secrecType ()->secrecSecType ()->isPrivate ();
        if (isPrivate)
            dataType = dtypeDeclassify (symb->secrecType ()->secrecSecType (), dataType);

        if (SymbolConstant* c = dest.value ()->toConstant (cxt, st, dataType)) {
            Imop::Type iType = Imop::ASSIGN;
            if (isPrivate)
                iType = Imop::CLASSIFY;
            Imop* newImop = new Imop (imop.creator (), iType, symb, c);
            replace.emplace_back (std::unique_ptr<Imop>(&imop), newImop);
        }
    }

    for (auto& p : replace)
        p.first->replaceWith (*p.second);

    return replace.size ();
}

std::string ConstantFolding::toString (const Program& program) const {
    std::ostringstream os;

    os << "Constant folding analysis results:\n";
    for (const Procedure& proc : program) {
        if (proc.name ())
            os << "[Proc " << proc.name ()->procedureName () << "]\n";
        else
            os << "[Internal Proc]\n";

        for (const Block& block : proc) {
            if (! block.reachable ())
                continue;

            os << "  [Block " << block.index () << "]\n";
            const auto it = m_outs.find (&block);
            if (it == m_outs.end ())
                continue;

            for (const auto& v : it->second)
                if (! v.second.isNac ())
                    os << "    " << *v.first << " --> " << v.second.toString () << std::endl;
        }
    }

    return os.str ();
}

} // namespace SecreC
