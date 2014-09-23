#include "analysis/ConstantFolding.h"

#include "Constant.h"
#include "Context.h"
#include "ContextImpl.h"
#include "DataType.h"
#include "Imop.h"
#include "SecurityType.h"
#include "SymbolTable.h"
#include "Types.h"

#include <iostream>
#include <list>
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
    VSTR
};

class AbstractValue {
public: /* Methods: */
    explicit AbstractValue (ValueTag tag)
        : m_tag (tag)
    { }

    virtual ~AbstractValue () { }
    virtual std::string toString () const = 0;
    ValueTag tag () const { return m_tag; }

private: /* Fields: */
    const ValueTag m_tag;
};

/*******************************************************************************
  IntValue
*******************************************************************************/

class IntValue : public AbstractValue {
public: /* Methods: */

    IntValue ()
        : AbstractValue (VINT)
        , isSigned (false)
    { }

    IntValue (bool isSigned, APInt value)
        : AbstractValue (VINT)
        , isSigned (isSigned)
        , value (value)
    { }

    IntValue (APInt value)
        : IntValue (false, value)
    { }

    APInt::value_type bits () const { return value.bits (); }
    std::string toString () const override final;

public: /* Methods: */
    const bool  isSigned;
    const APInt value;
};

bool operator < (const IntValue& x, const IntValue& y) {
    return std::tie (x.isSigned, x.value) < std::tie (y.isSigned, y.value);
}

std::string IntValue::toString () const {
    std::ostringstream ss;
    if (isSigned)
        value.sprint (ss);
    else
        value.uprint (ss);
    return ss.str ();
}

/*******************************************************************************
  StringValue
*******************************************************************************/

class StringValue : public AbstractValue {
public: /* Methods: */
    StringValue (std::string val)
        : AbstractValue (VSTR)
        , str (std::move (val))
    { }

    std::string toString () const override final;

public: /* Fields: */
    const std::string str;
};

bool operator < (const StringValue& x, const StringValue& y) {
    return x.str < y.str;
}

std::string StringValue::toString () const {
    return str;
}

/*******************************************************************************
  ArrayValue
*******************************************************************************/

class ArrayValue : public AbstractValue {
public: /* Methods: */
    ArrayValue (size_t size, Value elem)
        : AbstractValue (VARR)
        , elems (size, elem)
    { }

    ArrayValue (std::vector<Value> elems)
        : AbstractValue (VARR)
        , elems (std::move (elems))
    { }

    std::string toString () const override final;

public: /* Fields: */
    const std::vector<Value> elems;
};

bool operator < (const ArrayValue& x, const ArrayValue& y) {
    return x.elems < y.elems;
}

std::string ArrayValue::toString () const {
    std::stringstream ss;
    ss << '{';
    for (size_t i = 0; i < elems.size (); ++ i) {
        if (i != 0)
            ss << ", ";
        ss << elems[i].toString ();
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

private:

    template <typename S, typename T>
    static Value get (S& values, const T& val) {
        return &*values.insert (val).first;
    }

private: /* Fields: */
    std::set<IntValue>    m_intValues;
    std::set<ArrayValue>  m_arrayValues;
    std::set<StringValue> m_stringValues;
};

namespace /* anonymous */ {

/*******************************************************************************
  IntValue
*******************************************************************************/

template <typename F>
inline IntValue intLift (F op, const IntValue& x) {
    return IntValue (x.isSigned, op (x.value));
}

template <typename F, typename ...Args>
inline IntValue intLift (F op, const IntValue& x, const IntValue& y, Args&& ...args) {
    return IntValue (x.isSigned, op (x.value, y.value, std::forward<Args>(args)...));
}

IntValue intCast (TypeNonVoid* resultType, const IntValue& x) {
    assert (resultType != nullptr && resultType->secrecDataType () != nullptr);
    assert (resultType->secrecDataType ()->isPrimitive ());
    const auto dataType = resultType->secrecDataType ();
    const auto secrecDataType = static_cast<DataTypePrimitive*>(dataType)->secrecDataType ();
    const auto destWidth = widthInBitsDataType (secrecDataType);
    const bool destIsSigned = isSignedNumericDataType (dataType);

    // Cast to boolean:
    if (secrecDataType == DATATYPE_BOOL)
        return {false, x.bits () != 0};

    if (destWidth == x.value.numBits ())
        return {destIsSigned, x.value};

    // Trunc:
    if (destWidth < x.value.numBits ())
        return {destIsSigned, APInt::trunc (x.value, destWidth)};

    // Sign extend:
    if (x.isSigned)
        return {destIsSigned, APInt::sextend (x.value, destWidth)};

    // Extend:
    return {destIsSigned, APInt::extend (x.value, destWidth)};
}

IntValue intBinary (Imop::Type iType, const IntValue& x, const IntValue& y) {
    assert (x.isSigned == y.isSigned && "ICE: mismatching signs in binary operator!");

    const bool isSigned = x.isSigned;

    const auto div = isSigned ? APInt::sdiv : APInt::udiv;
    const auto rem = isSigned ? APInt::srem : APInt::urem;
    const auto shr = isSigned ? APInt::ashr : APInt::lshr;

    switch (iType) {
    case Imop::ADD:  return intLift (APInt::add, x, y);
    case Imop::SUB:  return intLift (APInt::sub, x, y);
    case Imop::MUL:  return intLift (APInt::mul, x, y);
    case Imop::DIV:  return intLift (div, x, y);
    case Imop::MOD:  return intLift (rem, x, y);
    case Imop::LE:   return APInt::cmp (x.value, y.value, isSigned ? APInt::SLE : APInt::ULE);
    case Imop::LT:   return APInt::cmp (x.value, y.value, isSigned ? APInt::SLT : APInt::ULT);
    case Imop::GE:   return APInt::cmp (x.value, y.value, isSigned ? APInt::SGE : APInt::UGE);
    case Imop::GT:   return APInt::cmp (x.value, y.value, isSigned ? APInt::SGT : APInt::UGT);
    case Imop::LAND:
    case Imop::BAND: return intLift (APInt::AND, x, y);
    case Imop::LOR:
    case Imop::BOR:  return intLift (APInt::OR, x, y);
    case Imop::XOR:  return intLift (APInt::XOR, x, y);
    case Imop::SHL:  return intLift (APInt::shl, x, y);
    case Imop::SHR:  return intLift (shr, x, y);
    case Imop::EQ:   return APInt::cmp (x.value, y.value, APInt::EQ);
    case Imop::NE:   return APInt::cmp (x.value, y.value, APInt::NE);
    default:
        assert (false && "Invalid binary integer operation.");
        return IntValue ();
    }
}

IntValue intUnary (Imop::Type iType, const IntValue& x) {
    switch (iType) {
    case Imop::CLASSIFY:   return x;
    case Imop::DECLASSIFY: return x;
    case Imop::UINV:
    case Imop::UNEG:       return intLift (APInt::inv, x);
    case Imop::UMINUS:     return intLift (APInt::minus, x);
    default:
        assert (false && "Invalid unary integer operation.");
        return IntValue ();
    }
}

StringValue intToString (const IntValue& x) {
    return StringValue (x.toString ());
}

/*******************************************************************************
  ArrayValue
*******************************************************************************/

Value arrLoad (const ArrayValue& a, const IntValue& i) {
    assert (i.bits () < a.elems.size ());
    return a.elems[i.bits ()];
}

ArrayValue arrStore (const ArrayValue& a, const IntValue& i, Value v) {
    assert (i.bits () < a.elems.size ());
    auto elems = a.elems;
    elems[i.bits ()] = v;
    return ArrayValue (std::move (elems));
}

ArrayValue arrAlloc (Value v, const IntValue& i) {
    return ArrayValue (i.bits (), v);
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
        const auto& xs = static_cast<const ArrayValue*>(xv)->elems;
        const auto& ys = static_cast<const ArrayValue*>(yv)->elems;
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
    const auto& elems = av->elems;

    // For undefined, or non-constant indices return meet of all values
    // TODO: not sure if this is correct.
    if (i.isUndef () || i.isNac ())
        return meetRange (factory, elems.begin (), elems.end ());

    assert (i.value ()->tag () == VINT);
    const auto iv = static_cast<const IntValue*>(i.value ());
    return arrLoad (*av, *iv);
}

Value valueScalarCast (ValueFactory& factory, TypeNonVoid* resultType, Value x) {
    if (x.isNac () || x.isUndef ())
        return x;

    assert (x.value () && x.value ()->tag () == VINT);
    const auto xv = static_cast<const IntValue*>(x.value ());
    return factory.get (intCast (resultType, *xv));
}

Value valueArrayCast (ValueFactory& factory, TypeNonVoid* resultType, Value x) {
    if (x.isNac () || x.isUndef ())
        return x;

    assert (x.value () && x.value ()->tag () == VARR);
    const auto xv = static_cast<const ArrayValue*>(x.value ());
    auto elems = xv->elems;
    for (size_t i = 0; i < elems.size (); ++ i) {
        elems[i] = valueScalarCast (factory, resultType, elems[i]);
    }

    return factory.get (ArrayValue (elems));
}

Value valueScalarUnaryArith (ValueFactory& factory, Imop::Type iType, Value x) {
    if (x.isNac () || x.isUndef ())
        return x;

    const auto xv = static_cast<const IntValue*>(x.value ());
    assert (xv != nullptr);
    return factory.get (intUnary (iType, *xv));
}

Value valueScalarBinaryArith (ValueFactory& factory, Imop::Type iType, Value x, Value y) {
    const auto xv = static_cast<const IntValue*>(x.value ());
    const auto yv = static_cast<const IntValue*>(y.value ());

    const bool xIsZero = xv ? xv->tag () == VINT && xv->bits () == 0 : false;
    const bool yIsZero = yv ? yv->tag () == VINT && yv->bits () == 0 : false;

    /**
     * Check if the result is constant no matter the other argument:
     */

    switch (iType) {
    case Imop::MUL:
    case Imop::BAND:
    case Imop::LAND:
        if (xIsZero) return x;
        if (yIsZero) return y;
        break;
    case Imop::DIV:
    case Imop::MOD:
    case Imop::SHL:
    case Imop::SHR:
        if (xIsZero) return x;
        break;
    default:
        break;
    }

    if (x.isUndef () || y.isUndef ())
        return Value::undef ();

    if (x.isNac () || y.isNac ())
        return Value::nac ();

    if (iType == Imop::EQ)
        return factory.get (IntValue (false, x == y));

    if (iType == Imop::NE)
        return factory.get (IntValue (false, ! (x == y)));

    assert (xv != nullptr && yv != nullptr);
    return factory.get (intBinary (iType, *xv, *yv));
}

Value valueArrayUnaryArith (ValueFactory& factory, Imop::Type iType, Value x) {
    if (x.isNac () || x.isUndef ())
        return x;

    const auto xv = static_cast<const ArrayValue*>(x.value ());
    assert (xv != nullptr);
    auto elems = xv->elems;
    for (size_t i = 0; i < elems.size (); ++ i) {
        elems[i] = valueScalarUnaryArith (factory, iType, elems[i]);
    }

    return factory.get (ArrayValue (std::move (elems)));
}

// Either returns x is it is constant or resizes it to the size of the other argument.
ArrayValue valueArrayLift (Value x, Value y) {
    assert (x.isConst () || y.isConst ());
    if (x.isConst ()) {
        return *static_cast<const ArrayValue*>(x.value ());
    }
    else {
        const size_t size = static_cast<const ArrayValue*>(y.value ())->elems.size ();
        return ArrayValue (size, x);
    }
}

Value valueArrayBinaryArith (ValueFactory& factory, Imop::Type iType, Value x, Value y) {

    if (!x.isConst () || !y.isConst ()) {
        if (x.isNac () && y.isNac ())
            return Value::nac ();
        return Value::undef ();
    }

    // At least one of the arguments is constant!

    const auto xv = valueArrayLift (x, y);
    const auto yv = valueArrayLift (y, x);
    const auto& xElems = xv.elems;
    const auto& yElems = yv.elems;
    std::vector<Value> elems;
    elems.reserve (xElems.size ());
    for (size_t i = 0; i < elems.size (); ++ i) {
        elems.push_back (valueScalarBinaryArith (factory, iType, xElems[i], yElems[i]));
    }

    return factory.get (ArrayValue (std::move (elems)));
}

Value valueAlloc (ValueFactory& factory, Value e, Value s) {
    if (s.isNac ()) return Value::nac ();
    if (s.isUndef ()) return Value::undef ();
    const auto sv = static_cast<const IntValue*>(s.value ());
    return factory.get (arrAlloc (e, *sv));
}

Value valueToString (ValueFactory& factory, Value x) {
    if (! x.isConst ())
        return x;

    assert (x.value ()->tag () == VINT);
    const auto xv = static_cast<const IntValue*>(x.value ());
    return factory.get (intToString (*xv));
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
    assert (dataType->isPrimitive () && "Non-primitive constant symbol!");
    const auto secrecDataType = static_cast<DataTypePrimitive*>(dataType)->secrecDataType ();
    if (const auto s = dynamic_cast<const ConstantInt*>(sym)) {
        const auto v = s->value ();
        const bool isSigned = isSignedNumericDataType (secrecDataType);
        m_constants.insert (it,
            std::make_pair (sym, m_values->get (IntValue (isSigned, v))));
        return;
    }

    if (const auto s = dynamic_cast<const ConstantString*>(sym)) {
        std::ostringstream os;
        os << s->value ();
        m_constants.insert (it,
            std::make_pair (sym, m_values->get (StringValue (os.str ()))));
    }
}

Value ConstantFolding::getVal (SVM& val, const Symbol* sym) {
    const auto cit = m_constants.find (sym);
    if (cit != m_constants.end ())
        return cit->second;

    // TODO: Bailing for constant floats. We should also consider those.
    if (sym->isConstant ())
        return Value::nac ();

    return val[sym];
}

void ConstantFolding::setVal (SVM& val, const Symbol* sym, Value x) {
    val[sym] = x;
}

void ConstantFolding::transfer (SVM& val, const Imop& imop) {
    const auto iType = imop.type ();

    if (iType == Imop::PARAM || iType == Imop::DOMAINID) {
        setVal (val, imop.dest (), Value::nac ());
        return;
    }

    if (iType == Imop::SYSCALL) {
        if (imop.dest () != nullptr)
            setVal (val, imop.dest (), Value::nac ());
        return;
    }

    if (iType == Imop::PUSHREF) {
        setVal (val, imop.arg1 (), Value::nac ());
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

    if (iType == Imop::CALL) {
        for (auto dest : imop.defRange ())
            setVal (val, dest, Value::nac ());
        return;
    }

    if (iType == Imop::CAST) {
        TypeNonVoid* resultType = imop.dest ()->secrecType ();
        const auto& x = getVal (val, imop.arg1 ());
        if (resultType->isScalar ())
            setVal (val, imop.dest (), valueScalarCast (*m_values, resultType, x));
        else
            setVal (val, imop.dest (), valueArrayCast (*m_values, resultType, x));

        return;
    }

    if (iType == Imop::TOSTRING) {
        const auto x = getVal (val, imop.arg1 ());
        setVal (val, imop.dest (), valueToString (*m_values, x));
        return;
    }

    if (iType == Imop::ASSIGN || iType == Imop::COPY) {
        setVal (val, imop.dest (), getVal (val, imop.arg1 ()));
        return;
    }

    if (iType == Imop::ALLOC) {
        const auto e = getVal (val, imop.arg1 ());
        const auto s = getVal (val, imop.arg2 ());
        setVal (val, imop.dest (), valueAlloc (*m_values, e, s));
        return;
    }

    if (iType == Imop::LOAD) {
        const auto& a = getVal (val, imop.arg1 ());
        const auto& i = getVal (val, imop.arg2 ());
        setVal (val, imop.dest (), valueLoad (*m_values, a, i));
        return;
    }

    if (iType == Imop::STORE) {
        const auto a = getVal (val, imop.dest ());
        const auto i = getVal (val, imop.arg1 ());
        const auto v = getVal (val, imop.arg2 ());
        setVal (val, imop.dest (), valueStore (*m_values, a, i, v));
        return;
    }

    if (imop.isVectorized ()) {
        if (imop.nArgs () == 3) {
            const auto x = getVal (val, imop.arg1 ());
            setVal (val, imop.dest (), valueArrayUnaryArith (*m_values, iType, x));
            return;
        }

        if (imop.nArgs () == 4) {
            const auto x = getVal (val, imop.arg1 ());
            const auto y = getVal (val, imop.arg2 ());
            setVal (val, imop.dest (), valueArrayBinaryArith (*m_values, iType, x, y));
            return;
        }

        assert (false && "Unhandled vectorized operation!");
        return;
    }

    if (imop.isExpr ()) {
        if (imop.nArgs () == 2) {
            const auto x = getVal (val, imop.arg1 ());
            setVal (val, imop.dest (), valueScalarUnaryArith (*m_values, iType, x));
            return;
        }

        if (imop.nArgs () == 3) {
            const auto x = getVal (val, imop.arg1 ());
            const auto y = getVal (val, imop.arg2 ());
            setVal (val, imop.dest (), valueScalarBinaryArith (*m_values, iType, x, y));
            return;
        }

        assert (false && "Unhandled expression operation!");
        return;
    }

    switch (iType) {
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
        return;
    default:
        assert (false && "Unhandled operation!");
        return;
    }
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
    for (auto it = b.begin (); it != b.end (); ++ it)
        transfer (out, *it);

    return out != old;
}

bool ConstantFolding::finishBlock (const Block &b) {
    return makeOuts (b, m_ins[&b], m_outs[&b]);
}

void ConstantFolding::finish () { }

std::string ConstantFolding::toString (const Program& program) const {
    std::ostringstream os;
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
