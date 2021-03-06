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

#include "VirtualMachine.h"

#include "Blocks.h"
#include "Constant.h"
#include "DataType.h"
#include "SecurityType.h"
#include "SymbolTable.h"
#include "TreeNode.h"
#include "Types.h"

#include <array>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sharemind/abort.h>
#include <sstream>
#include <stack>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <utility>


#if 0
#define TRACE(...) do { std::fprintf(stderr, __VA_ARGS__); } while (false)
#else
#define TRACE(...) (void) 0
#endif

#define PP_IF_0(...)
#define PP_IF_1(...) __VA_ARGS__
#define PP_IF_(bit,...) PP_IF_ ## bit(__VA_ARGS__)
#define PP_IF(bit,...) PP_IF_(bit, __VA_ARGS__)

namespace SecreC {

namespace /* anonymous */ {

/// Primitive values of the VM.
union ValueUnion {
    double        un_float64_val;
    float         un_float32_val;
    uint64_t      un_uint_val;
    uint32_t      un_uint32_val;
    uint16_t      un_uint16_val;
    uint8_t       un_uint8_val;
    int64_t       un_int_val;
    int32_t       un_int32_val;
    int16_t       un_int16_val;
    int8_t        un_int8_val;
    bool          un_bool_val;

    ValueUnion*        un_ptr;
    std::string*  un_str_val;
};

template <SecrecDataType ty> struct secrec_type_traits;
template <> struct secrec_type_traits<DATATYPE_FLOAT64> { using type = double; };
template <> struct secrec_type_traits<DATATYPE_FLOAT32> { using type = float; };
template <> struct secrec_type_traits<DATATYPE_UINT64> { using type = uint64_t; };
template <> struct secrec_type_traits<DATATYPE_UINT32> { using type = uint32_t; };
template <> struct secrec_type_traits<DATATYPE_UINT16> { using type = uint16_t; };
template <> struct secrec_type_traits<DATATYPE_UINT8> { using type = uint8_t; };
template <> struct secrec_type_traits<DATATYPE_INT64> { using type = int64_t; };
template <> struct secrec_type_traits<DATATYPE_INT32> { using type = int32_t; };
template <> struct secrec_type_traits<DATATYPE_INT16> { using type = int16_t; };
template <> struct secrec_type_traits<DATATYPE_INT8> { using type = int8_t; };
template <> struct secrec_type_traits<DATATYPE_STRING> { using type = const std::string&; };
template <> struct secrec_type_traits<DATATYPE_BOOL> { using type = bool; };

/// Get value based on the data type.
template <SecrecDataType ty> typename secrec_type_traits<ty>::type getValue (const ValueUnion&);
template <> double getValue<DATATYPE_FLOAT64> (const ValueUnion& v) { return v.un_float64_val; }
template <> float getValue<DATATYPE_FLOAT32> (const ValueUnion& v) { return v.un_float32_val; }
template <> uint64_t getValue<DATATYPE_UINT64> (const ValueUnion& v) { return v.un_uint_val; }
template <> uint32_t getValue<DATATYPE_UINT32> (const ValueUnion& v) { return v.un_uint32_val; }
template <> uint16_t getValue<DATATYPE_UINT16> (const ValueUnion& v) { return v.un_uint16_val; }
template <> uint8_t getValue<DATATYPE_UINT8> (const ValueUnion& v) { return v.un_uint8_val; }
template <> int64_t getValue<DATATYPE_INT64> (const ValueUnion& v) { return v.un_int_val; }
template <> int32_t getValue<DATATYPE_INT32> (const ValueUnion& v) { return v.un_int32_val; }
template <> int16_t getValue<DATATYPE_INT16> (const ValueUnion& v) { return v.un_int16_val; }
template <> int8_t getValue<DATATYPE_INT8> (const ValueUnion& v) { return v.un_int8_val; }
template <> bool getValue<DATATYPE_BOOL> (const ValueUnion& v) { return v.un_bool_val; }
template <> const std::string& getValue<DATATYPE_STRING> (const ValueUnion& v) { return *v.un_str_val; }

/// Set value based on C data type.
inline void assignValue (ValueUnion& v, double r) { v.un_float64_val = r; }
inline void assignValue (ValueUnion& v, float r) { v.un_float32_val = r; }
inline void assignValue (ValueUnion& v, uint64_t r) { v.un_uint_val = r; }
inline void assignValue (ValueUnion& v, uint32_t r) { v.un_uint32_val = r; }
inline void assignValue (ValueUnion& v, uint16_t r) { v.un_uint16_val = r; }
inline void assignValue (ValueUnion& v, uint8_t r) { v.un_uint8_val = r; }
inline void assignValue (ValueUnion& v, int64_t r) { v.un_int_val = r; }
inline void assignValue (ValueUnion& v, int32_t r) { v.un_int32_val = r; }
inline void assignValue (ValueUnion& v, int16_t r) { v.un_int16_val = r; }
inline void assignValue (ValueUnion& v, int8_t r) { v.un_int8_val = r; }
inline void assignValue (ValueUnion& v, bool r) { v.un_bool_val = r; }
inline void assignValue (ValueUnion& v, const std::string& r) { v.un_str_val = new std::string (r); }

/// Statically typed value casting.
template <SecrecDataType toTy, SecrecDataType fromTy >
void castValue (ValueUnion& dest, const ValueUnion& from) {
    using type = typename secrec_type_traits<toTy>::type;
    assignValue (dest, static_cast<type>(getValue<fromTy>(from)));
}

/// Stack for values
class ValueStack {

public:

    ValueStack () : m_bptr (nullptr), m_offset (0), m_size (0) { }

    ValueStack(ValueStack &&) = delete;
    ValueStack(ValueStack const &) = delete;

    ~ValueStack () { free (m_bptr); }

    ValueStack & operator=(ValueStack &&) = delete;
    ValueStack & operator=(ValueStack const &) = delete;

    void top (ValueUnion& out) const {
        assert (m_offset > 0);
        out = m_bptr[m_offset - 1];
        TRACE("<- %lu\n", out.un_uint_val);
    }

    void pop () {
        assert (m_offset > 0);
        -- m_offset;
    }

    void push (ValueUnion val) {
        if (m_offset >= m_size)
            increase_size ();
        TRACE("-> %lu\n", val.un_uint_val);
        m_bptr[m_offset ++] = val;
    }

private:
    ValueUnion*   m_bptr;
    size_t   m_offset;
    size_t   m_size;

    void increase_size () {
        m_size = ((m_size + 1) * 3) / 2;
        TRACE("RESIZE STACK TO %zu\n", m_size);
        auto * newBPtr =
                static_cast<ValueUnion *>(
                    realloc(m_bptr, m_size * sizeof(ValueUnion)));
        if (newBPtr != nullptr) {
            m_bptr = newBPtr;
        }
    }
};

struct Instruction;

/**
 * Virtual machine symbol. May be either IR symbol, or pointer to some other
 * instruction.  Additinally has a tag for if it's local or global symbol.
 */
struct VMSym {
    bool isLocal : 1;
    union {
        const Symbol* un_sym;
        Instruction* un_inst;
    };

    VMSym () { }

    VMSym (bool isLocal_, Symbol const * sym)
        : isLocal(isLocal_)
        , un_sym(sym)
    {}
};

/// Very naive implementation of a memory store.
using Store = std::map<const Symbol*, ValueUnion>;

/// Type of instantiated callback.
using CallbackTy = int (*)(const Instruction*);

/// Instructions are composed of callback, and 4 arguments.
struct Instruction {
    CallbackTy callback;
    std::array<VMSym, 4u> args;

    Instruction ()
        : callback (nullptr)
    { }

    explicit Instruction (CallbackTy cb)
        : callback (cb)
    { }
};

/// Each frame has local store, old instruction pointer, and pointer to previous frame.
struct Frame {
    Store                     m_local;
    const Instruction* const  m_old_ip;
    Frame*                    m_prev_frame;

    Frame(const Instruction* old_ip)
        : m_old_ip (old_ip)
        , m_prev_frame(nullptr)
    { }

private:

    Frame (const Frame&);
    Frame& operator = (const Frame&);
};

/**
 * State of the interpreter.
 * We have:
 * - stack for function arguments and return values
 * - frame stack
 * - one register for temporary values, for example constants are stored in it
 * - store for global variables
 */

ValueStack m_stack;
Frame* m_frames = nullptr;
Store m_global;
std::uint64_t m_fpuState = 0u;

void free_store (Store& store) {
    // TODO: not actually releasing any dynamically allocated memory
    // This is not a serious issue as this VM is only used for testing
    // and should be removed as soon as possible.
    store.clear();
}

inline void push_frame (const Instruction* old_ip) {
    auto new_frame = new (std::nothrow) Frame (old_ip);
    new_frame->m_prev_frame = m_frames;
    m_frames = new_frame;
}

inline void pop_frame (void) {
    assert (m_frames != nullptr && "No frames to pop!");
    Frame* temp = m_frames;
    m_frames = temp->m_prev_frame;
    free_store (temp->m_local);
    delete temp;
}

// Force to not inline to make output assembler nicer.
ValueUnion& lookup (VMSym sym) __attribute__ ((noinline));
ValueUnion& lookup (VMSym sym)  {
    TRACE ("%s ", (sym.isLocal ? "LOCAL" : "GLOBAL"));
    Store& store = sym.isLocal ? m_frames->m_local : m_global;
    assert (sym.un_sym != nullptr);
    return store[sym.un_sym];
}

// Force to not inline to make output assembler nicer.
void storeSym (VMSym sym, ValueUnion val) __attribute__ ((noinline));
void storeSym (VMSym sym, ValueUnion val) {
    Store& store = sym.isLocal ? m_frames->m_local : m_global;
    assert (sym.un_sym != nullptr);
    store[sym.un_sym] = val;
}

// Quick and dirty solution.
template <SecrecDataType fromTy >
void castValueDyn (const DataType* dataType, ValueUnion& dest, const ValueUnion& from) {
    assert (dataType != nullptr && dataType->isBuiltinPrimitive ());
    SecrecDataType toTy = static_cast<const DataTypeBuiltinPrimitive*>(dataType)->secrecDataType ();
    switch (toTy) {
    case DATATYPE_BOOL:   castValue<DATATYPE_BOOL,fromTy>(dest, from); break;
    case DATATYPE_INT8:   castValue<DATATYPE_INT8,fromTy>(dest, from); break;
    case DATATYPE_UINT8:  castValue<DATATYPE_UINT8,fromTy>(dest, from); break;
    case DATATYPE_INT16:  castValue<DATATYPE_INT16,fromTy>(dest, from); break;
    case DATATYPE_UINT16: castValue<DATATYPE_UINT16,fromTy>(dest, from); break;
    case DATATYPE_INT32:  castValue<DATATYPE_INT32,fromTy>(dest, from); break;
    case DATATYPE_UINT32: castValue<DATATYPE_UINT32,fromTy>(dest, from); break;
    case DATATYPE_INT64:  castValue<DATATYPE_INT64,fromTy>(dest, from); break;
    case DATATYPE_UINT64: castValue<DATATYPE_UINT64,fromTy>(dest, from); break;
    case DATATYPE_FLOAT32: castValue<DATATYPE_FLOAT32,fromTy>(dest, from); break;
    case DATATYPE_FLOAT64: castValue<DATATYPE_FLOAT64,fromTy>(dest, from); break;
    default:
        assert (false);
        exit (1);
    }
}


/**
 * Macros to simplify code generation:
 */

/// Just to make vim syntax highlighter quiet
#define BLOCK(CODE) { CODE }

#define FETCH(name,i) ValueUnion & name = lookup((ip)->args[i])

// Note that returns after callback explicitly tell compiler that callbacks
// don't return. That should make tail call detection trivial.
#define NEXT do { return ((ip + 1)->callback (ip + 1)); } while (0)

#define CUR do { return ip->callback (ip); } while (0)

#define MKCALLBACK_(NAME, PDEST, PDN, PARG1, P1N, PARG2, P2N, PARG3, P3N, ...) \
    template <SecrecDataType ty = DATATYPE_UNDEFINED> \
    inline int NAME##_callback (const Instruction* ip) \
    BLOCK( \
        TRACE("%p: ", (void*) ip); \
        TRACE("%s ",#NAME); \
        PP_IF (PDEST, FETCH (PDN, 0); ) \
        PP_IF (PARG1, FETCH (P1N, 1); TRACE("0x%lx ", P1N.un_uint_val);) \
        PP_IF (PARG2, FETCH (P2N, 2); TRACE("0x%lx ", P2N.un_uint_val);) \
        PP_IF (PARG3, FETCH (P3N, 3); TRACE("0x%lx ", P3N.un_uint_val);) \
        TRACE ("%s", "\n"); \
        __VA_ARGS__; \
        NEXT; \
    )

#define MKCALLBACK(NAME, PDEST, PARG1, PARG2, PARG3, ...) \
    MKCALLBACK_(NAME, PDEST, dest, PARG1, arg1, PARG2, arg2, PARG3, arg3, \
                __VA_ARGS__)

#define DECLOP1(NAME,...) \
    MKCALLBACK(NAME, 1, 1, 0, 0, __VA_ARGS__) \
    MKCALLBACK_(NAME ## _vec, 1, dest_, 1, arg1_, 1, arg2_, 0,, BLOCK( \
        const size_t s = arg2_.un_uint_val; \
        ValueUnion* desti = dest_.un_ptr; \
        ValueUnion* end = dest_.un_ptr + s; \
        ValueUnion* arg1i = arg1_.un_ptr; \
        for (; desti != end; ++ desti, ++ arg1i) \
        BLOCK( \
            ValueUnion& dest = *desti; \
            ValueUnion arg1 = *arg1i; \
            __VA_ARGS__ \
        ) \
    ) \
    )

#define DECLOP2(NAME,...) \
    MKCALLBACK(NAME, 1, 1, 1, 0, __VA_ARGS__) \
    MKCALLBACK_(NAME ## _vec, 1, dest_, 1, arg1_, 1, arg2_, 1, arg3_, BLOCK( \
        const size_t s = arg3_.un_uint_val; \
        ValueUnion* desti = dest_.un_ptr; \
        ValueUnion* end = dest_.un_ptr + s; \
        ValueUnion* arg1i = arg1_.un_ptr; \
        ValueUnion* arg2i = arg2_.un_ptr; \
        for (; desti != end; ++ desti, ++ arg1i, ++ arg2i) \
        BLOCK( \
            ValueUnion& dest = *desti; \
            ValueUnion arg1 = *arg1i; \
            ValueUnion arg2 = *arg2i; \
            __VA_ARGS__ \
        ) \
    ) \
    )

/// build instruction body for conditional jump
#define JUMPCOND(COND) \
    const Instruction* newIp = ip + 1; \
    if (COND) BLOCK( \
        newIp = ip->args[0].un_inst; \
    ) \
    ip = newIp; \
    CUR;

/**
 * Unary and binary regular and vectorized ops:
 */

//DECLOP1 (DECLARE, (void) dest; (void) arg1)
DECLOP1 (ASSIGN, assignValue (dest, getValue<ty>(arg1));)
DECLOP1 (CLASSIFY, assignValue (dest, getValue<ty>(arg1));)
DECLOP1 (DECLASSIFY, assignValue (dest, getValue<ty>(arg1));)
DECLOP1 (CAST, castValueDyn<ty>(ip->args[0].un_sym->secrecType ()->secrecDataType (), dest, arg1);)
DECLOP1 (UINV, assignValue (dest, ~getValue<ty>(arg1));)
DECLOP1 (UNEG, assignValue (dest, !getValue<DATATYPE_BOOL>(arg1));)
DECLOP2 (LAND, assignValue (dest, arg1.un_bool_val && arg2.un_bool_val);)
DECLOP2 (LOR,  assignValue (dest, arg1.un_bool_val || arg2.un_bool_val);)
DECLOP2 (BAND, assignValue (dest, getValue<ty>(arg1) & getValue<ty>(arg2));)
DECLOP2 (BOR,  assignValue (dest, getValue<ty>(arg1) | getValue<ty>(arg2));)
DECLOP2 (XOR,  assignValue (dest, getValue<ty>(arg1) ^ getValue<ty>(arg2));)
DECLOP1 (UMINUS, assignValue (dest, -getValue<ty>(arg1));)
DECLOP2 (EQ,  assignValue (dest, getValue<ty>(arg1) == getValue<ty>(arg2));)
DECLOP2 (NE,  assignValue (dest, getValue<ty>(arg1) != getValue<ty>(arg2));)
DECLOP2 (ADD, assignValue (dest, getValue<ty>(arg1) +  getValue<ty>(arg2));)
DECLOP2 (SUB, assignValue (dest, getValue<ty>(arg1) -  getValue<ty>(arg2));)
DECLOP2 (MUL, assignValue (dest, getValue<ty>(arg1) *  getValue<ty>(arg2));)
DECLOP2 (DIV, assignValue (dest, getValue<ty>(arg1) /  getValue<ty>(arg2));)
DECLOP2 (MOD, assignValue (dest, getValue<ty>(arg1) %  getValue<ty>(arg2));)
DECLOP2 (LE,  assignValue (dest, getValue<ty>(arg1) <= getValue<ty>(arg2));)
DECLOP2 (LT,  assignValue (dest, getValue<ty>(arg1) <  getValue<ty>(arg2));)
DECLOP2 (GE,  assignValue (dest, getValue<ty>(arg1) >= getValue<ty>(arg2));)
DECLOP2 (GT,  assignValue (dest, getValue<ty>(arg1) >  getValue<ty>(arg2));)


/**
 * Various jumps:
 */

MKCALLBACK(JT,  0, 1, 0, 0, JUMPCOND (arg1.un_bool_val))
MKCALLBACK(JF,  0, 1, 0, 0, JUMPCOND (!arg1.un_bool_val))

/**
 * Miscellaneous or more complicated instructions:
 */

MKCALLBACK (ERROR, 1, 0, 0, 0,
    fprintf (stderr, "%s\n", dest.un_str_val->c_str());
    return EXIT_FAILURE;
)

MKCALLBACK (PRINT, 1, 0, 0, 0,
    fprintf (stdout, "%s\n", dest.un_str_val->c_str());
)

MKCALLBACK (TOSTRING, 1, 1, 0, 0,
    std::stringstream ss;
    ss << getValue<ty>(arg1);
    assignValue (dest, ss.str ());
)

MKCALLBACK(CALL, 0, 0, 0, 0,
    push_frame (ip + 1);
    ip = ip->args[0].un_inst;
    CUR;
)

MKCALLBACK(RETCLEAN, 0, 0, 0, 0, { })

MKCALLBACK(RETVOID, 0, 0, 0, 0,
    assert (m_frames != nullptr);
    const Instruction* new_i = m_frames->m_old_ip;
    pop_frame();
    ip = new_i;
    CUR;
)

MKCALLBACK(DOMAINID, 1, 0, 0, 0,
    const SymbolDomain* dom = static_cast<const SymbolDomain*>(ip->args[1].un_sym);
    uint64_t c = reinterpret_cast<uint64_t>(dom->securityType ());
    /// hopefully correct function will be selected
    assignValue (dest, c);
)

MKCALLBACK(STRLEN, 1, 1, 0, 0,
    dest.un_uint_val = arg1.un_str_val->size ();
)

MKCALLBACK(GETFPUSTATE, 1, 0, 0, 0,
    dest.un_uint_val = m_fpuState;
)

MKCALLBACK(SETFPUSTATE, 1, 0, 0, 0,
    m_fpuState = getValue<DATATYPE_UINT64>(dest);
)

MKCALLBACK(PUSH, 0, 1, 0, 0,
    m_stack.push(arg1);
)

MKCALLBACK(PARAM, 1, 0, 0, 0,
    m_stack.top(dest);
    m_stack.pop();
)

MKCALLBACK(POP, 1, 0, 0, 0,
    m_stack.top(dest);
    m_stack.pop();
)

MKCALLBACK(JUMP, 0, 0, 0, 0,
    ip = ip->args[0].un_inst;
    CUR;
)

MKCALLBACK(ALLOC, 1, 1, 1, 0,
    const ValueUnion& v = arg2;
    const unsigned n = arg1.un_uint_val;
    dest.un_ptr = static_cast<ValueUnion *>(malloc(sizeof(ValueUnion) * n));
    for (ValueUnion* it(dest.un_ptr); it < dest.un_ptr + n; ++ it)
      *it = v;
)

MKCALLBACK(COPY, 1, 1, 1, 0,
    const size_t n = arg2.un_uint_val;
    dest.un_ptr = static_cast<ValueUnion*>(malloc (sizeof (ValueUnion) * n));
    memcpy (dest.un_ptr, arg1.un_ptr, sizeof (ValueUnion) * n);
)

MKCALLBACK(RELEASE, 1, 0, 0, 0,
    if (ip->args[0].un_sym->secrecType ()->secrecDataType ()->equals(DATATYPE_STRING)) {
        assert (dest.un_str_val != nullptr);
        delete dest.un_str_val;
        dest.un_str_val = nullptr;
    }
    else
    if (ip->args[0].un_sym->secrecType ()->secrecSecType ()->isPublic ()) {
        assert (dest.un_ptr != nullptr);
        free (dest.un_ptr);
        dest.un_ptr = nullptr;
    }
)

template <SecrecDataType ty>
ValueUnion loadArray (ValueUnion& arg, uint64_t index) { return arg.un_ptr[index]; }

template <>
ValueUnion loadArray<DATATYPE_STRING>(ValueUnion& arg, uint64_t index) {
    ValueUnion out;
    out.un_uint8_val = arg.un_str_val->at (index);
    return out;
}

MKCALLBACK(LOAD, 0, 1, 1, 0,
    storeSym (ip->args[0], loadArray<ty>(arg1, arg2.un_uint_val));
)

template <SecrecDataType ty>
void storeArray (ValueUnion& dest, uint64_t i, ValueUnion v) {
    dest.un_ptr[i] = v;
}

template <>
void storeArray<DATATYPE_STRING>(ValueUnion& dest, uint64_t i, ValueUnion v) {
    dest.un_str_val->at (i) = v.un_uint8_val;
}

MKCALLBACK(STORE, 1, 1, 1, 0,
    storeArray<ty>(dest, arg1.un_uint_val, arg2);
)

MKCALLBACK(NOP, 0, 0, 0, 0, { })

MKCALLBACK(END, 0, 0, 0, 0, return EXIT_SUCCESS; )

/**
 * Following macros deal with selecting proper specialization for
 * instruction callbacks. Care must be taken to not instantiate callbacks
 * on type arguments that would raise compil-time errors. For example we must
 * not instantiate DIV callback on strings.
 * Many of the macros assume that variables "callback", "ty", and "isVec" are set.
 * All callbacks are SecrecDataType generic, even those that strictly do not need
 * to be (those are simply instantiated with DATATYPE_UNDEFINED).
 */

#define GET_CALLBACK(NAME,TYPE) (NAME ## _callback<TYPE>)
#define SET_CALLBACK(NAME,TYPE) do {\
        callback = GET_CALLBACK(NAME,TYPE);\
    } while (0)
#define SET_SIMPLE_CALLBACK(NAME) SET_CALLBACK(NAME,)
#define SWITCH_ONE(NAME,TYPE) case TYPE: SET_CALLBACK(NAME,TYPE); break;
#define SWITCH_SIGNED(NAME)\
    SWITCH_ONE (NAME, DATATYPE_INT8)\
    SWITCH_ONE (NAME, DATATYPE_INT16)\
    SWITCH_ONE (NAME, DATATYPE_INT32)\
    SWITCH_ONE (NAME, DATATYPE_INT64)
#define SWITCH_UNSIGNED(NAME)\
    SWITCH_ONE (NAME, DATATYPE_UINT8)\
    SWITCH_ONE (NAME, DATATYPE_UINT16)\
    SWITCH_ONE (NAME, DATATYPE_UINT32)\
    SWITCH_ONE (NAME, DATATYPE_UINT64)
#define SWITCH_FLOAT(NAME)\
    SWITCH_ONE (NAME, DATATYPE_FLOAT32)\
    SWITCH_ONE (NAME, DATATYPE_FLOAT64)
#define SWITCH_ARITH(NAME)\
    SWITCH_SIGNED(NAME)\
    SWITCH_UNSIGNED(NAME)
#define SWITCH_INTEGRAL(NAME)\
    SWITCH_ARITH(NAME)\
    SWITCH_ONE (NAME, DATATYPE_BOOL)
#define SWITCH_NUM(NAME)\
    SWITCH_ARITH(NAME)\
    SWITCH_FLOAT(NAME)
#define SWITCH_NONSTRING(NAME)\
    SWITCH_FLOAT(NAME)\
    SWITCH_INTEGRAL(NAME)
#define SWITCH_ANY(NAME)\
    SWITCH_ONE (NAME, DATATYPE_STRING)\
    SWITCH_NONSTRING(NAME)
#define SET_SPECIALIZE_CALLBACK(NAME, SWITCHER) do {\
    assert (ty != DATATYPE_UNDEFINED);\
    switch (ty) {\
    SWITCHER(NAME)\
    default:\
        assert (false && #NAME " is not declared on given type!");\
        callback = nullptr; /* fallback failure if DEBUG is not set */ \
    } } while (0)
#define SET_SPECIALIZE_CALLBACK_V(NAME, SWITCHER) do {\
    if (isVec) {\
        SET_SPECIALIZE_CALLBACK(NAME ## _vec, SWITCHER);\
    } else {\
        SET_SPECIALIZE_CALLBACK(NAME, SWITCHER);\
    }} while (0)
#define SIMPLE_CALLBACK(NAME) GET_CALLBACK(NAME,)
#define SET_SIMPLE_CALLBACK_V(NAME) do {\
    if (isVec) {\
        SET_SIMPLE_CALLBACK(NAME ## _vec);\
    } else {\
        SET_SIMPLE_CALLBACK(NAME);\
    }} while (0)

bool matchTypes (const Type* ty1, const Type* ty2) {
    return ty1->secrecDataType () == ty2->secrecDataType () &&
           ty1->secrecSecType () == ty2->secrecSecType ();
}

/**
 * Select instruction based on intermediate operator.
 * Does not handle multi-callback operators.
 */
CallbackTy getCallback (const Imop& imop) {
    CallbackTy callback = nullptr;
    SecrecDataType ty = DATATYPE_UNDEFINED;
    const bool isVec = imop.isVectorized ();

    // figure out the data type
    switch (imop.type ()) {
    case Imop::MUL:
    case Imop::DIV:
    case Imop::MOD:
    case Imop::ADD:
    case Imop::SUB:
    case Imop::BAND:
    case Imop::BOR:
    case Imop::XOR:
        assert (matchTypes (imop.dest ()->secrecType (), imop.arg1 ()->secrecType ()));
    case Imop::EQ:
    case Imop::NE:
    case Imop::LE:
    case Imop::LT:
    case Imop::GE:
    case Imop::GT:
    case Imop::LAND:
    case Imop::LOR:
        assert (matchTypes (imop.arg2 ()->secrecType (), imop.arg1 ()->secrecType ()));
    case Imop::UINV:
    case Imop::UMINUS:
    case Imop::CAST:
    case Imop::TOSTRING:
    case Imop::CLASSIFY:
    case Imop::DECLASSIFY:
    case Imop::ASSIGN:
    case Imop::LOAD: {
        const DataType* dataType = imop.arg1()->secrecType()->secrecDataType();
        assert (dataType != nullptr);

        if (dataType->isBuiltinPrimitive()) {
            ty = static_cast<const DataTypeBuiltinPrimitive*>(dataType)->secrecDataType();
        }
        else if (dataType->isUserPrimitive()) {
            const SecurityType* sec = imop.arg1()->secrecType()->secrecSecType();
            assert(sec->isPrivate());
            SymbolKind* kind = static_cast<const PrivateSecType*>(sec)->securityKind();
            const auto pubTy = kind->findType (static_cast<const DataTypeUserPrimitive*> (dataType)->name ())->publicType;
            if (pubTy) {
                ty = pubTy.get()->secrecDataType();
            }
            else {
                SHAREMIND_ABORT("ICE: Attemping to emulate private only values.");
            }
        }
    }
    default:
        break;
    }

    if (imop.type () == Imop::STORE) {
        const DataType* dataType = imop.dest()->secrecType()->secrecDataType();
        assert (dataType != nullptr && dataType->isPrimitive ());

        if (dataType->isBuiltinPrimitive ()) {
            ty = static_cast<const DataTypeBuiltinPrimitive*>(dataType)->secrecDataType ();
        }
        else {
            const SecurityType* sec = imop.dest()->secrecType()->secrecSecType();
            assert (sec->isPrivate ());
            SymbolKind* kind = static_cast<const PrivateSecType*>(sec)->securityKind();
            const auto pubTy = kind->findType (static_cast<const DataTypeUserPrimitive*> (dataType)->name ())->publicType;
            if (pubTy) {
                ty = pubTy.get()->secrecDataType();
            }
            else {
                SHAREMIND_ABORT("ICE: Attemping to emulate private only values.");
            }
        }
    }

    if (imop.type () == Imop::ASSIGN) {
        if (! matchTypes (imop.dest ()->secrecType (), imop.arg1 ()->secrecType ())) {
            std::cerr << imop << " // " << TreeNode::typeName (imop.creator ()->type ()) << std::endl;
            std::cerr << *imop.dest ()->secrecType () << std::endl;
            std::cerr << *imop.arg1 ()->secrecType () << std::endl;
            SHAREMIND_ABORT("ICE: Ill constructed assignment.");
        }
    }

    switch (imop.type ()) {
    case Imop::DECLARE:    SET_SIMPLE_CALLBACK(NOP); break;
    case Imop::ASSIGN:     SET_SPECIALIZE_CALLBACK_V(ASSIGN,SWITCH_ANY); break;
    case Imop::CLASSIFY:   SET_SPECIALIZE_CALLBACK_V(CLASSIFY,SWITCH_ANY); break;
    case Imop::DECLASSIFY: SET_SPECIALIZE_CALLBACK_V(DECLASSIFY,SWITCH_ANY); break;
    case Imop::CAST:       SET_SPECIALIZE_CALLBACK_V(CAST,SWITCH_NONSTRING); break;
    case Imop::UNEG:       SET_SIMPLE_CALLBACK_V(UNEG); break;
    case Imop::LAND:       SET_SIMPLE_CALLBACK_V(LAND); break;
    case Imop::LOR:        SET_SIMPLE_CALLBACK_V(LOR); break;
    case Imop::BAND:       SET_SPECIALIZE_CALLBACK_V(BAND,SWITCH_INTEGRAL); break;
    case Imop::BOR:        SET_SPECIALIZE_CALLBACK_V(BOR,SWITCH_INTEGRAL); break;
    case Imop::XOR:        SET_SPECIALIZE_CALLBACK_V(XOR,SWITCH_INTEGRAL); break;
    case Imop::UINV:       SET_SPECIALIZE_CALLBACK_V(UINV,SWITCH_ARITH); break;
    case Imop::UMINUS:     SET_SPECIALIZE_CALLBACK_V(UMINUS,SWITCH_NUM); break;
    case Imop::ADD:        SET_SPECIALIZE_CALLBACK_V(ADD,SWITCH_ANY); break;
    case Imop::MUL:        SET_SPECIALIZE_CALLBACK_V(MUL,SWITCH_NUM); break;
    case Imop::DIV:        SET_SPECIALIZE_CALLBACK_V(DIV,SWITCH_NUM); break;
    case Imop::MOD:        SET_SPECIALIZE_CALLBACK_V(MOD,SWITCH_ARITH); break;
    case Imop::SUB:        SET_SPECIALIZE_CALLBACK_V(SUB,SWITCH_NUM); break;
    case Imop::LE:         SET_SPECIALIZE_CALLBACK_V(LE,SWITCH_ANY); break;
    case Imop::LT:         SET_SPECIALIZE_CALLBACK_V(LT,SWITCH_ANY); break;
    case Imop::GE:         SET_SPECIALIZE_CALLBACK_V(GE,SWITCH_ANY); break;
    case Imop::GT:         SET_SPECIALIZE_CALLBACK_V(GT,SWITCH_ANY); break;
    case Imop::EQ:         SET_SPECIALIZE_CALLBACK_V(EQ,SWITCH_ANY); break;
    case Imop::NE:         SET_SPECIALIZE_CALLBACK_V(NE,SWITCH_ANY); break;
    case Imop::JUMP:       SET_SIMPLE_CALLBACK(JUMP); break;
    case Imop::JT:         SET_SIMPLE_CALLBACK(JT); break;
    case Imop::JF:         SET_SIMPLE_CALLBACK(JF); break;
    case Imop::COMMENT:    SET_SIMPLE_CALLBACK(NOP); break;
    case Imop::ERROR:      SET_SIMPLE_CALLBACK(ERROR); break;
    case Imop::PARAM:      SET_SIMPLE_CALLBACK(PARAM); break;
    case Imop::RETCLEAN:   SET_SIMPLE_CALLBACK(RETCLEAN); break;
    case Imop::ALLOC:      SET_SIMPLE_CALLBACK(ALLOC); break;
    case Imop::COPY:       SET_SIMPLE_CALLBACK(COPY); break;
    case Imop::RELEASE:    SET_SIMPLE_CALLBACK(RELEASE); break;
    case Imop::STORE:      SET_SPECIALIZE_CALLBACK(STORE,SWITCH_ANY); break;
    case Imop::LOAD:       SET_SPECIALIZE_CALLBACK(LOAD,SWITCH_ANY); break;
    case Imop::END:        SET_SIMPLE_CALLBACK(END); break;
    case Imop::PRINT:      SET_SIMPLE_CALLBACK(PRINT); break;
    case Imop::DOMAINID:   SET_SIMPLE_CALLBACK(DOMAINID); break;
    case Imop::TOSTRING:   SET_SPECIALIZE_CALLBACK(TOSTRING,SWITCH_NONSTRING); break;
    case Imop::STRLEN:     SET_SIMPLE_CALLBACK(STRLEN); break;
    case Imop::GETFPUSTATE: SET_SIMPLE_CALLBACK(GETFPUSTATE); break;
    case Imop::SETFPUSTATE: SET_SIMPLE_CALLBACK(SETFPUSTATE); break;
    default:
        assert (false && "Reached unsupported instruction.");
        break;
    }

    return callback;
}

/**
 * All constants in VM are stored in global store.
 * This is pretty big hack, but makes the life a lot simpler.
 * Following 2 function offer a way to store libscc constant symbols
 * to the global scope.
 */

void storeConstantString (ValueUnion& out, const Symbol* c) {
    const ConstantString* str = static_cast<const ConstantString*>(c);
    out.un_str_val = new std::string (str->value ().str ());
}

template <SecrecDataType ty>
void storeConstantInt (ValueUnion& out, const Symbol* c) {
    using type = typename secrec_type_traits<ty>::type;
    const ConstantInt* intSym = static_cast<const ConstantInt*>(c);
    const uint64_t value = intSym->value ().bits ();
    assignValue (out, static_cast<type>(value));
}

void storeConstant (VMSym sym, const Symbol* c) {
    const DataType* dataType = c->secrecType ()->secrecDataType ();
    assert (dataType != nullptr && dataType->isBuiltinPrimitive ());
    SecrecDataType dtype = static_cast<const DataTypeBuiltinPrimitive*>(dataType)->secrecDataType ();
    Store& store = sym.isLocal ? m_frames->m_local : m_global;
    ValueUnion& out = store[sym.un_sym];
    switch (dtype) {
    case DATATYPE_STRING: storeConstantString(out, c); break;
    case DATATYPE_BOOL: storeConstantInt<DATATYPE_BOOL>(out, c); break;
    case DATATYPE_INT8: storeConstantInt<DATATYPE_INT8>(out, c); break;
    case DATATYPE_UINT8: storeConstantInt<DATATYPE_UINT8>(out, c); break;
    case DATATYPE_INT16: storeConstantInt<DATATYPE_INT16>(out, c); break;
    case DATATYPE_UINT16: storeConstantInt<DATATYPE_UINT16>(out, c); break;
    case DATATYPE_INT32: storeConstantInt<DATATYPE_INT32>(out, c); break;
    case DATATYPE_UINT32: storeConstantInt<DATATYPE_UINT32>(out, c); break;
    case DATATYPE_INT64: storeConstantInt<DATATYPE_INT64>(out, c); break;
    case DATATYPE_UINT64: storeConstantInt<DATATYPE_UINT64>(out, c); break;
    case DATATYPE_FLOAT32: {
        const ConstantFloat* floatSym = static_cast<const ConstantFloat*>(c);
        const uint32_t bits = floatSym->value ().ieee32bits ();
        float value;
        static_assert(sizeof(value) == sizeof(bits), "");
        std::memcpy(&value, &bits, sizeof(bits));
        assignValue (out, value);
        break;
    } case DATATYPE_FLOAT64: {
        const ConstantFloat* floatSym = static_cast<const ConstantFloat*>(c);
        const uint64_t bits = floatSym->value ().ieee64bits ();
        double value;
        static_assert(sizeof(value) == sizeof(bits), "");
        std::memcpy(&value, &bits, sizeof(bits));
        assignValue (out, value);
        break;
    } default:
        assert (false && "Invalid data type.");
    }
}


/**
 * Compiler, only non-trivial thing it does is tracking of jump locations
 * because some intermediate code instructions compile into multiple callbacks.
 */
class Compiler {

public: /* Types: */

    using Instructions = std::vector<Instruction>;

private: /* Types: */

    using JumpDestinations =
            std::vector<std::pair<Instructions::size_type, Imop const *> >;
    struct UnlinkedCode {
        Instructions instructions;
        JumpDestinations jumpDestinations;
    };

public: /* Methods: */

    static Instructions runOn (const Program& pr) {
        assert (! pr.empty ());
        UnlinkedCode code;
        std::map<Imop const *, std::size_t> imopAddresses;
        for (const auto & func : pr) {
            for (const auto & block : func) {
                assert (! block.empty ());
                for (const auto & imop : block) {
                    imopAddresses.emplace(&imop, code.instructions.size());
                    compileInstruction(code, imop);
                }
            }
        }

        auto & is = code.instructions;
        for (auto const & jumpDestination : code.jumpDestinations) {
            assert(jumpDestination.first < is.size());
            auto const it(imopAddresses.find(jumpDestination.second));
            assert(it != imopAddresses.end());
            is[jumpDestination.first].args[0].un_inst = is.data() + it->second;
        }
        return std::move(code.instructions);
    }

private:

    static VMSym toVMSym (const Symbol* sym) {
        assert (sym != nullptr);
        VMSym out (true, sym );

        switch (sym->symbolType()) {
        case SYM_SYMBOL:
            if (static_cast<SymbolSymbol const*>(sym)->scopeType() == SymbolSymbol::GLOBAL)
                out.isLocal = false;
            break;
        case SYM_CONSTANT:
            out.isLocal = false;
            storeConstant (out, sym);
        default: break;
        }

        return out;
    }

    //void compileInstruction (Instruction& i, const Imop& imop) {
    static void compileInstruction (UnlinkedCode & code, const Imop& imop) {
        // handle multi instruction IR instructions
        switch (imop.type ()) {
        case Imop::CALL: return compileCall(code, imop);
        case Imop::RETURN: return compileReturn(code, imop);
        default:
            break;
        }

        Instruction i;
        auto appendSymbolArg =
                [it = i.args.begin(), end = i.args.end()](Symbol const * symbol)
                        mutable
                {
                    assert(it != end);
                    *it = toVMSym(symbol);
                    ++it;
                };
        const Imop* dest = nullptr;

        if (imop.isJump ()) {
            const Symbol* arg = imop.dest ();
            appendSymbolArg(arg);
            assert (dynamic_cast<const SymbolLabel*>(arg) != nullptr);
            dest = static_cast<const SymbolLabel*>(arg)->target ();
        }
        else {
            for (const Symbol* sym : imop.defRange ()) {
                appendSymbolArg(sym);
            }
        }

        for (const Symbol* sym : imop.useRange ()) {
            appendSymbolArg(sym);
        }

        /// workaround as scc doesn't support strings yet
        switch (imop.type ()) {
        case Imop::ERROR:
        case Imop::PRINT:
        case Imop::DOMAINID:
            appendSymbolArg(imop.arg1());
        default:
            break;
        }

        i.callback = getCallback (imop);
        emitInstruction (code, i, dest);
    }

    /// compile Imop::CALL instruction
    static void compileCall (UnlinkedCode & code, const Imop& imop) {
        assert (imop.type () == Imop::CALL);

        Imop::OperandConstIterator it, itBegin, itEnd;

        itBegin = imop.operandsBegin ();
        itEnd = imop.operandsEnd ();
        it = itBegin;

        // compute destinations for calls
        const Symbol* dest = *itBegin;
        assert (dynamic_cast<const SymbolProcedure*>(dest) != nullptr);
        const SymbolProcedure* proc = static_cast<const SymbolProcedure*> (dest);
        const Imop* targetImop = proc->target();

        // push arguments
        std::stack<const Symbol*> argList;
        for (++ it; it != itEnd && *it != nullptr; ++ it) {
            argList.push (*it);
        }

        while (!argList.empty ()) {
            const Symbol* sym = argList.top ();
            argList.pop ();
            Instruction i (SIMPLE_CALLBACK(PUSH));
            i.args[1] = toVMSym (sym);
            emitInstruction (code, i);
        }

        assert (it != itEnd && *it == nullptr &&
            "Malformed CALL instruction!");

        // CALL
        Instruction i (SIMPLE_CALLBACK(CALL));
        emitInstruction (code, i, targetImop);

        // pop return values
        for (++ it; it != itEnd; ++ it) {
            Instruction i2(SIMPLE_CALLBACK(POP));
            i2.args[0] = toVMSym(*it);
            emitInstruction(code, i2);
        }
    }


    /// compile Imop::RETURN instruction
    static void compileReturn (UnlinkedCode & code, const Imop& imop) {
        assert (imop.type () == Imop::RETURN);

        assert (imop.operandsBegin () != imop.operandsEnd () &&
                "Malformed RETURN instruction!");

        std::stack<const Symbol* > rev;
        auto it = imop.operandsBegin (),
             itEnd = imop.operandsEnd ();
        for (++ it; it != itEnd; ++ it)
            rev.push (*it);
        while (!rev.empty ()) {
            Instruction i (SIMPLE_CALLBACK(PUSH));
            i.args[1] = toVMSym (rev.top ());
            emitInstruction (code, i);
            rev.pop ();
        }

        Instruction i (SIMPLE_CALLBACK(RETVOID));
        emitInstruction (code, i);
    }

    static void emitInstruction(UnlinkedCode & code, Instruction i)
    { code.instructions.emplace_back(std::move(i)); }

    static void emitInstruction(UnlinkedCode & code,
                                Instruction i,
                                Imop const * tar)
    {
        if (tar)
            code.jumpDestinations.emplace_back(code.instructions.size(), tar);
        try {
            emitInstruction(code, std::move(i));
        } catch (...) {
            code.jumpDestinations.pop_back();
            throw;
        }
    }

};


} // namespace anonymous

int VirtualMachine::run (const Program& pr) {
    auto const code(Compiler::runOn(pr));

    // execute
    push_frame (nullptr);
    int status = code.front().callback(code.data());

    // Program might exit from within a procedure, and if that
    // happens we nee to unwind all the frames to clear the memory.
    while (m_frames != nullptr) {
        pop_frame ();
    }

    free_store (m_global);

    return status;
}

} // namespace SecreC
