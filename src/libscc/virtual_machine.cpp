#include "virtual_machine.h"

#include <string>
#include <sstream>
#include <map>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdint.h>
#include <boost/preprocessor/control/if.hpp>
#include <boost/foreach.hpp>

#include "symboltable.h"
#include "blocks.h"
#include "constant.h"
#include "treenode.h"

#define LEAVETRACE 0
#define PP_IF(bit,arg) BOOST_PP_IF(bit, arg, (void) 0)
#define TRACE(format,msg) PP_IF(LEAVETRACE, fprintf(stderr, format, msg))

/**
 * \todo Simple GC to clear allocated strings and vectors.
 */

namespace { // anonymous namespace

using namespace SecreC;

std::vector<std::string*> stringHeap;
void releaseStringHeap () {
    BOOST_FOREACH (std::string* str, stringHeap) {
        delete str;
    }
}

/// Primitive values of the VM.
union Value {
    uint64_t            un_uint_val;
    uint32_t            un_uint32_val;
    uint16_t            un_uint16_val;
    uint8_t             un_uint8_val;
    int64_t             un_int_val;
    int32_t             un_int32_val;
    int16_t             un_int16_val;
    int8_t              un_int8_val;
    Value*              un_ptr;
    const std::string*  un_str_val;
    bool                un_bool_val;
};

/// Get value based on the data type.
template <SecrecDataType ty> typename SecrecTypeInfo<ty>::CType getValue (const Value&);
template <> bool getValue<DATATYPE_BOOL> (const Value& v) { return v.un_bool_val; }
template <> uint64_t getValue<DATATYPE_UINT64> (const Value& v) { return v.un_uint_val; }
template <> int64_t getValue<DATATYPE_INT64> (const Value& v) { return v.un_int_val; }
template <> uint32_t getValue<DATATYPE_UINT32> (const Value& v) { return v.un_uint32_val; }
template <> int32_t getValue<DATATYPE_INT32> (const Value& v) { return v.un_int32_val; }
template <> uint16_t getValue<DATATYPE_UINT16> (const Value& v) { return v.un_uint16_val; }
template <> int16_t getValue<DATATYPE_INT16> (const Value& v) { return v.un_int16_val; }
template <> uint8_t getValue<DATATYPE_UINT8> (const Value& v) { return v.un_uint8_val; }
template <> int8_t getValue<DATATYPE_INT8> (const Value& v) { return v.un_int8_val; }
template <> std::string getValue<DATATYPE_STRING> (const Value& v) { return *v.un_str_val; }

/// Set value based on C data type.
inline void assignValue (Value& v, bool r) { v.un_bool_val = r; }
inline void assignValue (Value& v, int8_t r) { v.un_int8_val = r; }
inline void assignValue (Value& v, uint8_t r) { v.un_uint8_val = r; }
inline void assignValue (Value& v, int16_t r) { v.un_int16_val = r; }
inline void assignValue (Value& v, uint16_t r) { v.un_uint16_val = r; }
inline void assignValue (Value& v, int32_t r) { v.un_int32_val = r; }
inline void assignValue (Value& v, uint32_t r) { v.un_uint32_val = r; }
inline void assignValue (Value& v, int64_t r) { v.un_int_val = r; }
inline void assignValue (Value& v, uint64_t r) { v.un_uint_val = r; }
inline void assignValue (Value& v, const std::string& r) {
    std::string* copy =  new std::string (r);
    v.un_str_val = copy;
    stringHeap.push_back (copy);
}

/// Statically typed value casting.
template <SecrecDataType toTy, SecrecDataType fromTy >
void castValue (Value& dest, const Value& from) {
    typedef typename SecrecTypeInfo<toTy>::CType toCType;
    assignValue (dest, static_cast<toCType>(getValue<fromTy>(from)));
}

/// Stack for values
class ValueStack {
    ValueStack (const ValueStack&); // not copyable
    ValueStack& operator = (const ValueStack&); // not assignable
public:

    ValueStack () : m_bptr (0), m_offset (0), m_size (0) { }
    ~ValueStack () { free (m_bptr); }

    void top (Value& out) const {
        assert (m_offset > 0);
        out = m_bptr[m_offset - 1];
        TRACE("<- %lu\n", out.un_uint_val);
    }

    void pop () {
        assert (m_offset > 0);
        -- m_offset;
    }

    inline bool empty () const {
        return m_offset <= 0;
    }

    void push (Value val) {
        if (m_offset >= m_size)
            increase_size ();
        TRACE("-> %lu\n", val.un_uint_val);
        m_bptr[m_offset ++] = val;
    }

    void push (int n) {
        const Value val = { n };
        TRACE("-> %d\n", n);
        push (val);
    }

private:
    Value*   m_bptr;
    size_t   m_offset;
    size_t   m_size;

    void increase_size () {
        m_size = ((m_size + 1) * 3) / 2;
        TRACE("RESIZE STACK TO %u\n", m_size);
        m_bptr = (Value*) realloc (m_bptr, m_size * sizeof (Value));
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

    VMSym (bool isLocal, Instruction* inst)
        : isLocal (isLocal)
        , un_inst (inst) { }

    VMSym (bool isLocal, const Symbol* sym)
        : isLocal(isLocal)
        , un_sym (sym) { }
};

/// Very naive implementation of a memory store.
typedef std::map<const Symbol*, Value> Store;

/// Type of instantiated callback.
typedef int (*CallbackTy)(const Instruction*);

/// Instructions are composed of callback, and 4 arguments.
struct Instruction {
    CallbackTy callback;
    VMSym args[4];

    Instruction ()
        : callback (0)
    { }

    explicit Instruction (CallbackTy cb)
        : callback (cb)
    { }

    ~Instruction () { }
};

/// Each frame has local store, old instruction pointer, and pointer to previous frame.
struct Frame {
    Store                     m_local;
    const Instruction* const  m_old_ip;
    Frame*                    m_prev_frame;

    Frame(const Instruction* old_ip)
        : m_old_ip (old_ip)
        , m_prev_frame(0)
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
Frame* m_frames = 0;
Store m_global;

void free_store (Store& store) {
    store.clear();
}

inline void push_frame (const Instruction* old_ip) {
    Frame* new_frame = new (std::nothrow) Frame (old_ip);
    new_frame->m_prev_frame = m_frames;
    m_frames = new_frame;
}

inline void pop_frame (void) {
    assert (m_frames != 0 && "No frames to pop!");
    Frame* temp = m_frames;
    m_frames = temp->m_prev_frame;
    free_store (temp->m_local);
    delete temp;
}

// Force to not inline to make output assembler nicer.
Value& lookup (VMSym sym) __attribute__ ((noinline));
Value& lookup (VMSym sym)  {
    TRACE ("%s ", (sym.isLocal ? "LOCAL" : "GLOBAL"));
    Store& store = sym.isLocal ? m_frames->m_local : m_global;
    assert (sym.un_sym != 0);
    return store[sym.un_sym];
}

// Force to not inline to make output assembler nicer.
void storeSym (VMSym sym, Value val) __attribute__ ((noinline));
void storeSym (VMSym sym, Value val) {
    Store& store = sym.isLocal ? m_frames->m_local : m_global;
    assert (sym.un_sym != 0);
    store[sym.un_sym] = val;
}

// Quick and dirty solution.
template <SecrecDataType fromTy >
void castValueDyn (SecrecDataType toTy, Value& dest, const Value& from) {
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

#define FETCH(name,i) Value& name = lookup((ip)->args[i])

// Note that returns after callback explicitly tell compiler that callbacks
// don't return. That should make tail call detection trivial.
#define NEXT do { return ((ip + 1)->callback (ip + 1)); } while (0)

#define CUR do { return ip->callback (ip); } while (0)

#define MKCALLBACK(NAME, PDEST, PARG1, PARG2, PARG3, CODE) \
    template <SecrecDataType ty>\
    inline int __##NAME##_callback (const Instruction* ip) \
    BLOCK( \
        TRACE("%p: ", (void*) ip); \
        TRACE("%s ",#NAME); \
        PP_IF (PDEST, FETCH (dest, 0)); \
        PP_IF (PARG1, FETCH (arg1, 1); TRACE("0x%lx ", arg1.un_uint_val)); \
        PP_IF (PARG2, FETCH (arg2, 2); TRACE("0x%lx ", arg2.un_uint_val)); \
        PP_IF (PARG3, FETCH (arg3, 3); TRACE("0x%lx ", arg3.un_uint_val)); \
        TRACE ("%s", "\n"); \
        CODE; \
        NEXT; \
    )

#define DECLOP1(NAME,CODE) \
    MKCALLBACK(NAME, 1, 1, 0, 0, CODE) \
    MKCALLBACK(NAME ## _vec, 1, 1, 1, 0, BLOCK( \
        const size_t s = arg2.un_uint_val; \
        Value* desti = dest.un_ptr; \
        Value* end = dest.un_ptr + s; \
        Value* arg1i = arg1.un_ptr; \
        for (; desti != end; ++ desti, ++ arg1i) \
        BLOCK( \
            Value& dest = *desti; \
            Value arg1 = *arg1i; \
            CODE; \
        ) \
    ) \
    )

#define DECLOP2(NAME,CODE) \
    MKCALLBACK(NAME, 1, 1, 1, 0, CODE) \
    MKCALLBACK(NAME ## _vec, 1, 1, 1, 1, BLOCK( \
        const size_t s = arg3.un_uint_val; \
        Value* desti = dest.un_ptr; \
        Value* end = dest.un_ptr + s; \
        Value* arg1i = arg1.un_ptr; \
        Value* arg2i = arg2.un_ptr; \
        for (; desti != end; ++ desti, ++ arg1i, ++ arg2i) \
        BLOCK( \
            Value& dest = *desti; \
            Value arg1 = *arg1i; \
            Value arg2 = *arg2i; \
            CODE; \
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

DECLOP1 (ASSIGN, dest = arg1)
DECLOP1 (CLASSIFY, dest = arg1)
DECLOP1 (DECLASSIFY, dest = arg1)
DECLOP1 (CAST, castValueDyn<ty>(ip->args[0].un_sym->secrecType ()->secrecDataType (), dest, arg1))
DECLOP1 (UNEG, assignValue (dest, !getValue<DATATYPE_BOOL>(arg1)))
DECLOP2 (LAND, assignValue (dest, arg1.un_bool_val && arg2.un_bool_val))
DECLOP2 (LOR, assignValue (dest, arg1.un_bool_val || arg2.un_bool_val))
DECLOP1 (UMINUS, assignValue (dest, -getValue<ty>(arg1)))
DECLOP2 (EQ,  assignValue (dest, getValue<ty>(arg1) == getValue<ty>(arg2)))
DECLOP2 (NE,  assignValue (dest, getValue<ty>(arg1) != getValue<ty>(arg2)))
DECLOP2 (ADD, assignValue (dest, getValue<ty>(arg1) +  getValue<ty>(arg2)))
DECLOP2 (SUB, assignValue (dest, getValue<ty>(arg1) -  getValue<ty>(arg2)))
DECLOP2 (MUL, assignValue (dest, getValue<ty>(arg1) *  getValue<ty>(arg2)))
DECLOP2 (DIV, assignValue (dest, getValue<ty>(arg1) /  getValue<ty>(arg2)))
DECLOP2 (MOD, assignValue (dest, getValue<ty>(arg1) %  getValue<ty>(arg2)))
DECLOP2 (LE,  assignValue (dest, getValue<ty>(arg1) <= getValue<ty>(arg2)))
DECLOP2 (LT,  assignValue (dest, getValue<ty>(arg1) <  getValue<ty>(arg2)))
DECLOP2 (GE,  assignValue (dest, getValue<ty>(arg1) >= getValue<ty>(arg2)))
DECLOP2 (GT,  assignValue (dest, getValue<ty>(arg1) >  getValue<ty>(arg2)))


/**
 * Various jumps:
 */

MKCALLBACK(JT,  0, 1, 0, 0, JUMPCOND (arg1.un_bool_val))
MKCALLBACK(JF,  0, 1, 0, 0, JUMPCOND (!arg1.un_bool_val))
MKCALLBACK(JE,  0, 1, 1, 0, JUMPCOND (getValue<ty>(arg1) == getValue<ty>(arg2)))
MKCALLBACK(JNE, 0, 1, 1, 0, JUMPCOND (getValue<ty>(arg1) != getValue<ty>(arg2)))
MKCALLBACK(JLE, 0, 1, 1, 0, JUMPCOND (getValue<ty>(arg1) <= getValue<ty>(arg2)))
MKCALLBACK(JLT, 0, 1, 1, 0, JUMPCOND (getValue<ty>(arg1) <  getValue<ty>(arg2)))
MKCALLBACK(JGE, 0, 1, 1, 0, JUMPCOND (getValue<ty>(arg1) >= getValue<ty>(arg2)))
MKCALLBACK(JGT, 0, 1, 1, 0, JUMPCOND (getValue<ty>(arg1) >  getValue<ty>(arg2)))

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
    assert (m_frames != 0);
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
    const Value& v = arg1;
    const unsigned n = arg2.un_uint_val;
    dest.un_ptr = (Value*) malloc (sizeof (Value) * n);
    for (Value* it(dest.un_ptr); it < dest.un_ptr + n; ++ it)
      *it = v;
)

MKCALLBACK(COPY, 1, 1, 1, 0,
    const size_t n = arg2.un_uint_val;
    dest.un_ptr = static_cast<Value*>(malloc (sizeof (Value) * n));
    memcpy (dest.un_ptr, arg1.un_ptr, sizeof (Value) * n);
)

MKCALLBACK(RELEASE, 1, 0, 0, 0,
    if (ip->args[0].un_sym->secrecType ()->secrecSecType ()->isPublic ()) {
        assert (dest.un_ptr != 0);
        free (dest.un_ptr);
        dest.un_ptr = 0;
    }
)

MKCALLBACK(LOAD, 0, 1, 1, 0,
    storeSym (ip->args[0], arg1.un_ptr [arg2.un_uint_val]);
)

MKCALLBACK(STORE, 1, 1, 1, 0,
    dest.un_ptr[arg1.un_uint_val] = arg2;
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

#define GET_CALLBACK(NAME,TYPE) (__ ## NAME ## _callback<TYPE>)
#define SET_CALLBACK(NAME,TYPE) do {\
        callback = GET_CALLBACK(NAME,TYPE);\
    } while (0)
#define SET_SIMPLE_CALLBACK(NAME) SET_CALLBACK(NAME,DATATYPE_UNDEFINED)
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
#define SWITCH_ARITH(NAME)\
    SWITCH_SIGNED(NAME)\
    SWITCH_UNSIGNED(NAME)
#define SWITCH_ANY(NAME)\
    SWITCH_ONE (NAME, DATATYPE_BOOL)\
    SWITCH_ONE (NAME, DATATYPE_STRING)\
    SWITCH_ARITH(NAME)
#define SWITCH_NONSTRING(NAME)\
    SWITCH_ONE (NAME, DATATYPE_BOOL)\
    SWITCH_ARITH(NAME)
#define SET_SPECIALIZE_CALLBACK(NAME, SWITCHER) do {\
    assert (ty != DATATYPE_UNDEFINED);\
    switch (ty) {\
    SWITCHER(NAME)\
    default:\
        assert (false && #NAME " is not declared on given type!");\
        callback = 0; /* fallback failure if DEBUG is not set */ \
    } } while (0)
#define SET_SPECIALIZE_CALLBACK_V(NAME, SWITCHER) do {\
    if (isVec) {\
        SET_SPECIALIZE_CALLBACK(NAME ## _vec, SWITCHER);\
    } else {\
        SET_SPECIALIZE_CALLBACK(NAME, SWITCHER);\
    }} while (0)
#define SIMPLE_CALLBACK(NAME) GET_CALLBACK(NAME,DATATYPE_UNDEFINED)
#define SIMPLE_CALLBACK_V(NAME) (isVec ? SIMPLE_CALLBACK(NAME ## _vec) : SIMPLE_CALLBACK(NAME))
#define SET_SIMPLE_CALLBACK_V(NAME) do {\
    if (isVec) {\
        SET_SIMPLE_CALLBACK(NAME ## _vec);\
    } else {\
        SET_SIMPLE_CALLBACK(NAME);\
    }} while (0)

bool matchTypes (Type* ty1, Type* ty2) {
    return  ty1->secrecDataType () == ty2->secrecDataType () &&
            ty1->secrecDimType () == ty2->secrecDimType () &&
            ty1->secrecSecType () == ty2->secrecSecType ();
}

/**
 * Select instruction based on intermediate operator.
 * Does not handle multi-callback operators.
 */
CallbackTy getCallback (const Imop& imop) {
    CallbackTy callback = 0;
    SecrecDataType ty = DATATYPE_UNDEFINED;
    const bool isVec = imop.isVectorized ();

    // figure out the data type
    switch (imop.type ()) {
    case Imop::MUL:
    case Imop::DIV:
    case Imop::MOD:
    case Imop::ADD:
    case Imop::SUB:
        assert (matchTypes (imop.dest ()->secrecType (), imop.arg1 ()->secrecType ()));
    case Imop::EQ:
    case Imop::NE:
    case Imop::LE:
    case Imop::LT:
    case Imop::GE:
    case Imop::GT:
    case Imop::JE:
    case Imop::JNE:
    case Imop::JLE:
    case Imop::JLT:
    case Imop::JGE:
    case Imop::JGT:
        assert (matchTypes (imop.arg2 ()->secrecType (), imop.arg1 ()->secrecType ()));
    case Imop::UMINUS:
    case Imop::CAST:
    case Imop::TOSTRING:
        ty = imop.arg1()->secrecType()->secrecDataType();
    default:
        break;
    }

    if (imop.type () == Imop::ASSIGN) {
        if (! matchTypes (imop.dest ()->secrecType (), imop.arg1 ()->secrecType ())) {
            std::cerr << imop.toString () << " // " << TreeNode::typeName (imop.creator ()->type ()) << std::endl;
            std::cerr << imop.dest ()->secrecType ()->toString () << std::endl;
            std::cerr << imop.arg1 ()->secrecType ()->toString () << std::endl;
            assert (false);
        }
    }

    switch (imop.type ()) {
    case Imop::ASSIGN:     SET_SIMPLE_CALLBACK_V(ASSIGN); break;
    case Imop::CLASSIFY:   SET_SIMPLE_CALLBACK_V(CLASSIFY); break;
    case Imop::DECLASSIFY: SET_SIMPLE_CALLBACK_V(DECLASSIFY); break;
    case Imop::CAST:       SET_SPECIALIZE_CALLBACK_V(CAST,SWITCH_NONSTRING); break;
    case Imop::UNEG:       SET_SIMPLE_CALLBACK_V(UNEG); break;
    case Imop::LAND:       SET_SIMPLE_CALLBACK_V(LAND); break;
    case Imop::LOR:        SET_SIMPLE_CALLBACK_V(LOR); break;
    case Imop::UMINUS:     SET_SPECIALIZE_CALLBACK_V(UMINUS,SWITCH_SIGNED); break;
    case Imop::ADD:        SET_SPECIALIZE_CALLBACK_V(ADD,SWITCH_ANY); break;
    case Imop::MUL:        SET_SPECIALIZE_CALLBACK_V(MUL,SWITCH_ARITH); break;
    case Imop::DIV:        SET_SPECIALIZE_CALLBACK_V(DIV,SWITCH_ARITH); break;
    case Imop::MOD:        SET_SPECIALIZE_CALLBACK_V(MOD,SWITCH_ARITH); break;
    case Imop::SUB:        SET_SPECIALIZE_CALLBACK_V(SUB,SWITCH_ARITH); break;
    case Imop::LE:         SET_SPECIALIZE_CALLBACK_V(LE,SWITCH_ANY); break;
    case Imop::LT:         SET_SPECIALIZE_CALLBACK_V(LT,SWITCH_ANY); break;
    case Imop::GE:         SET_SPECIALIZE_CALLBACK_V(GE,SWITCH_ANY); break;
    case Imop::GT:         SET_SPECIALIZE_CALLBACK_V(GT,SWITCH_ANY); break;
    case Imop::EQ:         SET_SPECIALIZE_CALLBACK_V(EQ,SWITCH_ANY); break;
    case Imop::NE:         SET_SPECIALIZE_CALLBACK_V(NE,SWITCH_ANY); break;
    case Imop::JE:         SET_SPECIALIZE_CALLBACK(JE, SWITCH_ANY); break;
    case Imop::JNE:        SET_SPECIALIZE_CALLBACK(JNE,SWITCH_ANY); break;
    case Imop::JLE:        SET_SPECIALIZE_CALLBACK(JLE,SWITCH_ANY); break;
    case Imop::JLT:        SET_SPECIALIZE_CALLBACK(JLT,SWITCH_ANY); break;
    case Imop::JGE:        SET_SPECIALIZE_CALLBACK(JGE,SWITCH_ANY); break;
    case Imop::JGT:        SET_SPECIALIZE_CALLBACK(JGT,SWITCH_ANY); break;
    case Imop::JUMP:       SET_SIMPLE_CALLBACK(JUMP); break;
    case Imop::JT:         SET_SIMPLE_CALLBACK(JT); break;
    case Imop::JF:         SET_SIMPLE_CALLBACK(JF); break;
    case Imop::COMMENT:    SET_SIMPLE_CALLBACK(NOP); break;
    case Imop::ERROR:      SET_SIMPLE_CALLBACK(ERROR); break;
    case Imop::PARAM:      SET_SIMPLE_CALLBACK(PARAM); break;
    case Imop::RETCLEAN:   SET_SIMPLE_CALLBACK(RETCLEAN); break;
    case Imop::RETURNVOID: SET_SIMPLE_CALLBACK(RETVOID); break;
    case Imop::ALLOC:      SET_SIMPLE_CALLBACK(ALLOC); break;
    case Imop::COPY:       SET_SIMPLE_CALLBACK(COPY); break;
    case Imop::RELEASE:    SET_SIMPLE_CALLBACK(RELEASE); break;
    case Imop::STORE:      SET_SIMPLE_CALLBACK(STORE); break;
    case Imop::LOAD:       SET_SIMPLE_CALLBACK(LOAD); break;
    case Imop::END:        SET_SIMPLE_CALLBACK(END); break;
    case Imop::PRINT:      SET_SIMPLE_CALLBACK(PRINT); break;
    case Imop::DOMAINID:   SET_SIMPLE_CALLBACK(DOMAINID); break;
    case Imop::TOSTRING:   SET_SPECIALIZE_CALLBACK(TOSTRING,SWITCH_ARITH); break;
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

template <SecrecDataType ty>
void storeConstantHelper (Value& out, const Symbol* c) {
    assignValue (out, static_cast<const Constant<ty>* >(c)->value ());
}

void storeConstant (VMSym sym, const Symbol* c) {
    SecrecDataType dtype = c->secrecType ()->secrecDataType ();
    Store& store = sym.isLocal ? m_frames->m_local : m_global;
    Value& out = store[sym.un_sym];
    switch (dtype) {
    case DATATYPE_BOOL: storeConstantHelper<DATATYPE_BOOL>(out, c); break;
    case DATATYPE_STRING: storeConstantHelper<DATATYPE_STRING>(out, c); break;
    case DATATYPE_INT8: storeConstantHelper<DATATYPE_INT8>(out, c); break;
    case DATATYPE_UINT8: storeConstantHelper<DATATYPE_UINT8>(out, c); break;
    case DATATYPE_INT16: storeConstantHelper<DATATYPE_INT16>(out, c); break;
    case DATATYPE_UINT16: storeConstantHelper<DATATYPE_UINT16>(out, c); break;
    case DATATYPE_INT32: storeConstantHelper<DATATYPE_INT32>(out, c); break;
    case DATATYPE_UINT32: storeConstantHelper<DATATYPE_UINT32>(out, c); break;
    case DATATYPE_INT64: storeConstantHelper<DATATYPE_INT64>(out, c); break;
    case DATATYPE_UINT64: storeConstantHelper<DATATYPE_UINT64>(out, c); break;
    default:
        assert (false && "Invalid data type.");
    }
}


/**
 * Compiler, only non-trivial thing it does is tracking of jump locations
 * because some intermediate code instructions compile into multiple callbacks.
 */
class Compiler {
public: /* Types: */

    typedef std::vector<std::pair<Instruction, const Imop* > > UnlinkedCode;
    typedef std::map<const Imop*, unsigned > ImopAddrs;

public: /* Methods: */

    Compiler () : m_codeSize (0) { }
    ~Compiler () { }

    Instruction* runOn (const Program& pr) {
        Instruction* out = 0;
        assert (! pr.empty ());
        for (Program::const_iterator pi = pr.begin (); pi != pr.end (); ++ pi) {
            for (Procedure::const_iterator bi = pi->begin (); bi != pi->end (); ++ bi) {
                assert (! bi->empty ());
                for (ImopList::const_iterator it = bi->begin (); it != bi->end (); ++ it) {
                    const Imop& imop = *it;
                    compileInstruction (imop);
                }
            }
        }

        out = (Instruction*) calloc(sizeof (Instruction), m_codeSize);
        for (size_t i = 0; i != m_codeSize; ++ i) {
            out[i] = m_code[i].first;
            const Imop* dest = m_code[i].second;
            if (dest != 0) {
                out[i].args[0].un_inst = &out[m_addrs[dest]];
            }
        }

        m_codeSize = 0;
        m_code.clear ();
        m_addrs.clear ();

        return out;
    }

private:

    VMSym toVMSym (const Symbol* sym) {
        assert (sym != 0);
        VMSym out (true, sym );

        switch (sym->symbolType()) {
        case Symbol::SYMBOL:
            assert (dynamic_cast<SymbolSymbol const*>(sym) != 0
                    && "VM: Symbol::SYMBOL that isn't SymbolSymbol.");
            if (static_cast<SymbolSymbol const*>(sym)->scopeType() == SymbolSymbol::GLOBAL)
                out.isLocal = false;
            break;
        case Symbol::CONSTANT:
            out.isLocal = false;
            storeConstant (out, sym);
        default: break;
        }

        return out;
    }

    //void compileInstruction (Instruction& i, const Imop& imop) {
    void compileInstruction (const Imop& imop) {
        // copy args
        m_addrs.insert (std::make_pair (&imop, m_codeSize));

        // handle multi instruction IR instructions
        switch (imop.type ()) {
        case Imop::CALL: compileCall (imop); return;
        case Imop::RETURN: compileReturn (imop); return;
        default:
            break;
        }

        unsigned nArgs = 0;
        Instruction i;
        const Imop* dest = 0;

        if (imop.isJump ()) {
            const Symbol* arg = imop.dest ();
            i.args[nArgs ++] = toVMSym (arg);
            assert (dynamic_cast<const SymbolLabel*>(arg) != 0);
            dest = static_cast<const SymbolLabel*>(arg)->target ();
        }
        else {
            BOOST_FOREACH (const Symbol* sym, imop.defRange ()) {
                i.args[nArgs ++] = toVMSym (sym);
            }
        }

        BOOST_FOREACH (const Symbol* sym, imop.useRange ()) {
            i.args[nArgs ++] = toVMSym (sym);
        }

        /// workaround as scc doesn't support strings yet
        switch (imop.type ()) {
        case Imop::ERROR:
        case Imop::PRINT:
        case Imop::DOMAINID:
            i.args[nArgs ++] = toVMSym (imop.arg1 ());
        default:
            break;
        }

        i.callback = getCallback (imop);
        emitInstruction (i, dest);
    }

    /// compile Imop::CALL instruction
    void compileCall (const Imop& imop) {
        assert (imop.type () == Imop::CALL);

        Imop::OperandConstIterator it, itBegin, itEnd;

        itBegin = imop.operandsBegin ();
        itEnd = imop.operandsEnd ();
        it = itBegin;

        // compute destinations for calls
        const Symbol* dest = *itBegin;
        assert (dynamic_cast<const SymbolProcedure*>(dest) != 0);
        const SymbolProcedure* proc = static_cast<const SymbolProcedure*> (dest);
        const Imop* targetImop = proc->target();

        // push arguments
        std::stack<const Symbol*> argList;
        for (++ it; it != itEnd && *it != 0; ++ it) {
            argList.push (*it);
        }

        while (!argList.empty ()) {
            const Symbol* sym = argList.top ();
            argList.pop ();
            Instruction i (SIMPLE_CALLBACK(PUSH));
            i.args[1] = toVMSym (sym);
            emitInstruction (i);
        }

        assert (it != itEnd && *it == 0 &&
            "Malformed CALL instruction!");

        // CALL
        Instruction i (SIMPLE_CALLBACK(CALL));
        emitInstruction (i, targetImop);

        // pop return values
        for (++ it; it != itEnd; ++ it) {
            Instruction i (SIMPLE_CALLBACK(POP));
            i.args[0] = toVMSym (*it);
            emitInstruction (i);
        }
    }


    /// compile Imop::RETURN instruction
    void compileReturn (const Imop& imop) {
        assert (imop.type () == Imop::RETURN);

        typedef Imop::OperandConstIterator OCI;

        assert (imop.operandsBegin () != imop.operandsEnd () &&
                "Malformed RETURN instruction!");

        std::stack<const Symbol* > rev;
        OCI it = imop.operandsBegin (),
            itEnd = imop.operandsEnd ();
        for (++ it; it != itEnd; ++ it)
            rev.push (*it);
        while (!rev.empty ()) {
            Instruction i (SIMPLE_CALLBACK(PUSH));
            i.args[1] = toVMSym (rev.top ());
            emitInstruction (i);
            rev.pop ();
        }

        Instruction i (SIMPLE_CALLBACK(RETVOID));
        emitInstruction (i);
    }

    void emitInstruction (Instruction i, const Imop* tar = 0) {
        m_code.push_back (std::make_pair (i, tar));
        ++ m_codeSize;
    }

private: /* Fields: */

    UnlinkedCode   m_code;
    size_t         m_codeSize;
    ImopAddrs      m_addrs;
};


} // end of anonymous namespace

namespace SecreC {

int VirtualMachine::run (const Program& pr) {
    Compiler comp;
    Instruction* code = comp.runOn (pr);

    // execute
    push_frame (0);
    int status = code->callback (code);

    // Program might exit from within a procedure, and if that
    // happens we nee to unwind all the frames to clear the memory.
    while (m_frames != 0) {
        pop_frame ();
    }

    free (code);
    free_store (m_global);
    releaseStringHeap ();

    return status;
}

}
