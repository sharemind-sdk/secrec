#include "virtual_machine.h"

#include <string>
#include <map>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <cstring>

#include "icodelist.h"

using namespace SecreC;

/**
 * \todo we are leaking memory!
 * \todo array regression test 22-ternary-bug crashes the evaluator with double free or corruption
 * \todo way too many lookups!
 * \todo lock the state of interpreter
 * \todo performance tests
 *
 * Some ideas to improve performance:
 * \todo in VM move away from Symbol to something better, for that
 *       we need extra pass to compute offsets in stack, also figure
 *       out what to do with constants.
 * \todo don't use std::map or std::vector
 */

/**
 * Data structures:
 */

// primitive values
union Value {
    int m_int_val;
    unsigned m_uint_val;
    bool m_bool_val;
    std::string const* m_str_val;
};

// registers
struct Register {
    Value* m_arr;
    inline Register () : m_arr((Value*) malloc (sizeof(Value))) { }
    inline ~Register () { }
};

static inline Value& fetch_val (Register const& reg) {
    return reg.m_arr[0];
}

static inline void reserve (Register& reg, unsigned n) {
    reg.m_arr = (Value*) realloc (reg.m_arr, sizeof(Value) * n);
}

static inline void store (Register& reg, Value const& val) { *reg.m_arr = val; }
static inline void store (Register& reg, int val) { reg.m_arr->m_int_val = val; }
static inline void store (Register& reg, bool val) { reg.m_arr->m_bool_val = val; }
static inline void store (Register& reg, unsigned val) { reg.m_arr->m_uint_val = val; }
static inline void store (Register& reg, Register const& src, unsigned n) {
    reserve (reg, n);
    memcpy (reg.m_arr, src.m_arr, sizeof(Value) * n);
}
static inline void store (Register& reg, unsigned i, Value const& val) {
    reg.m_arr[i] = val;
}
static inline void store (Value& val, Register const& reg, unsigned i) {
    val = reg.m_arr[i];
}


// stack for values
class ValueStack {
        typedef std::vector<Value >::iterator iterator;
        typedef std::vector<Value >::const_reverse_iterator const_reverse_iterator;
        std::vector<Value > vals;
    public:

        ValueStack() { }
        void top (Value& out) const { out = vals.back(); }
        void top (Register& out, unsigned n) const {
          assert (vals.size() >= n);
          const_reverse_iterator i = vals.rbegin();
          reserve(out, n);
          for (Value* val = out.m_arr; val < out.m_arr + n; ++ val, ++ i) {
            *val = *i;
          }
        }

        void pop () {
            assert (!vals.empty());
            vals.pop_back();
        }

        void pop (unsigned const n) {
            assert (vals.size() >= n);
            vals.resize(vals.size() - n);
        }

        inline bool empty () const {
            return vals.empty();
        }

        void push (Value const& val) {
            vals.push_back(val);
        }

        void push (Register const& reg, unsigned n) {
          vals.reserve (vals.size() + n);
          for (Value* i = reg.m_arr + n - 1; i >= reg.m_arr; -- i) {
            vals.push_back(*i);
          }
        }
};

typedef std::map<SecreC::Symbol const*, Register> Store;

struct Instruction {
  Symbol const* args[4];
  void (*callback)(Instruction*);
};

struct Frame {
    Store m_local;
    Instruction* const m_old_ip;
    SecreC::Symbol const* const m_ret;
    Frame* m_prev_frame;

    Frame(Instruction* const old_ip, SecreC::Symbol const* ret)
        : m_old_ip(old_ip), m_ret(ret), m_prev_frame(0) { }

private:
    Frame(Frame const&);
    Frame& operator = (Frame const&);
};

/**
 * Macros to simplify code generation:
 */

#define FETCH1(name, i) Register& name = lookup((i)->args[0])
#define FETCH2(name, i) Register const& name = lookup((i)->args[1])
#define FETCH3(name, i) Register const& name = lookup((i)->args[2])
#define FETCH4(name, i) Register const& name = lookup((i)->args[3])
#define NEXT(i) (i + 1)->callback (i + 1)

#define DECLOP1(NAME) static inline void NAME##_operator (Value& dest, Value const& arg1)
#define DECLOP2(NAME) static inline void NAME##_operator (Value& dest, Value const& arg1, Value const& arg2)

#define DECLVECOP1(NAME) static inline void NAME##_vec_operator (Register& dest, Register const& arg1, Register const& size) {\
unsigned s = size.m_arr->m_uint_val;\
reserve (dest, s);\
Value* desti = dest.m_arr;\
Value* end = dest.m_arr + s;\
Value* arg1i = arg1.m_arr;\
for (; desti != end; ++ desti, ++ arg1i)\
  NAME##_operator (*desti, *arg1i);\
}

#define DECLVECOP2(NAME) static inline void NAME##_vec_operator (Register& dest, Register const& arg1, Register const& arg2, Register const& size) {\
unsigned s = size.m_arr->m_uint_val;\
reserve (dest, s);\
Value* desti = dest.m_arr;\
Value* end = dest.m_arr + s;\
Value* arg1i = arg1.m_arr;\
Value* arg2i = arg2.m_arr;\
for (; desti != end; ++ desti, ++ arg1i, ++ arg2i)\
  NAME##_operator (*desti, *arg1i, *arg2i);\
}

#define CALLBACK(NAME, i) static void NAME##_callback (Instruction* const i)

/**
 * Regular primitive operations on data:
 */

DECLOP1 (ASSIGN)     { dest = arg1; }
DECLOP1 (CLASSIFY)   { dest = arg1; }
DECLOP1 (DECLASSIFY) { dest = arg1; }
DECLOP1 (UNEG)       { dest.m_bool_val = !arg1.m_bool_val; }
DECLOP1 (UMINUS)     { dest.m_int_val  = -arg1.m_int_val; }
DECLOP2 (ADD)        { dest.m_int_val  = arg1.m_int_val + arg2.m_int_val; }
DECLOP2 (SUB)        { dest.m_int_val  = arg1.m_int_val - arg2.m_int_val; }
DECLOP2 (MUL)        { dest.m_int_val  = arg1.m_int_val * arg2.m_int_val; }
DECLOP2 (DIV)        { dest.m_int_val  = arg1.m_int_val / arg2.m_int_val; }
DECLOP2 (MOD)        { dest.m_int_val  = arg1.m_int_val % arg2.m_int_val; }
DECLOP2 (LE)         { dest.m_bool_val = arg1.m_int_val <= arg2.m_int_val; }
DECLOP2 (LT)         { dest.m_bool_val = arg1.m_int_val < arg2.m_int_val; }
DECLOP2 (GE)         { dest.m_bool_val = arg1.m_int_val >= arg2.m_int_val; }
DECLOP2 (GT)         { dest.m_bool_val = arg1.m_int_val > arg2.m_int_val; }
DECLOP2 (LAND)       { dest.m_bool_val = arg1.m_bool_val && arg2.m_bool_val; }
DECLOP2 (LOR)        { dest.m_bool_val = arg1.m_bool_val || arg2.m_bool_val; }
DECLOP2 (EQINT)      { dest.m_bool_val = arg1.m_int_val == arg2.m_int_val; }
DECLOP2 (EQUINT)     { dest.m_bool_val = arg1.m_uint_val == arg2.m_uint_val; }
DECLOP2 (EQBOOL)     { dest.m_bool_val = arg1.m_bool_val == arg2.m_bool_val; }
DECLOP2 (EQSTRING)   { dest.m_bool_val = *(arg1.m_str_val) == *(arg2.m_str_val); }
DECLOP2 (NEQINT)     { dest.m_bool_val = arg1.m_int_val != arg2.m_int_val; }
DECLOP2 (NEQUINT)    { dest.m_bool_val = arg1.m_uint_val != arg2.m_uint_val; }
DECLOP2 (NEQBOOL)    { dest.m_bool_val = arg1.m_bool_val != arg2.m_bool_val; }
DECLOP2 (NEQSTRING)  { dest.m_bool_val = *(arg1.m_str_val) != *(arg2.m_str_val); }

/**
 * Vectorized versions of primitive operations:
 */

DECLVECOP1 (ASSIGN)
DECLVECOP1 (CLASSIFY)
DECLVECOP1 (DECLASSIFY)
DECLVECOP1 (UNEG)
DECLVECOP1 (UMINUS)
DECLVECOP2 (ADD)
DECLVECOP2 (SUB)
DECLVECOP2 (MUL)
DECLVECOP2 (DIV)
DECLVECOP2 (MOD)
DECLVECOP2 (LE)
DECLVECOP2 (LT)
DECLVECOP2 (GE)
DECLVECOP2 (GT)
DECLVECOP2 (LAND)
DECLVECOP2 (LOR)
DECLVECOP2 (EQINT)
DECLVECOP2 (EQUINT)
DECLVECOP2 (EQBOOL)
DECLVECOP2 (EQSTRING)
DECLVECOP2 (NEQINT)
DECLVECOP2 (NEQUINT)
DECLVECOP2 (NEQBOOL)
DECLVECOP2 (NEQSTRING)

/**
 * State of the interpreter.
 * We have:
 * - stack for function arguments and return values
 * - frame stack
 * - one register for temporary values, for example constants are stored in it
 * - store for global variables
 * - pointer to code
 */

ValueStack m_stack;
Frame* m_frames = 0;
Store m_global;
Instruction* m_code = 0;

static void free_store (Store& store) {
    Store::iterator it = store.begin();
    Store::iterator it_end = store.end();
    for (; it != it_end; ++ it) {
        Register& r = it->second;
        free (r.m_arr);
    }

    store.clear();
}

static inline void push_frame (Instruction* const old_ip, SecreC::Symbol const* ret)
{
    Frame* new_frame = new Frame (old_ip, ret);
    new_frame->m_prev_frame = m_frames;
    m_frames = new_frame;
}

static inline void pop_frame (void)
{
    assert (m_frames != 0 && "No frames to pop!");
    Frame* temp = m_frames;
    m_frames = temp->m_prev_frame;
    free_store (temp->m_local);
    delete temp;
}

/// \todo this is too slow
static Register& lookup (Symbol const* const sym) {
  switch (sym->symbolType()) {
    case Symbol::TEMPORARY: return m_frames->m_local[sym];
    case Symbol::SYMBOL:
      assert (dynamic_cast<SymbolSymbol const*>(sym) != 0
          && "VM: Symbol::SYMBOL that isn't SymbolSymbol.");
      switch (static_cast<SymbolSymbol const*>(sym)->scopeType()) {
        case SymbolSymbol::LOCAL: return m_frames->m_local[sym];
        case SymbolSymbol::GLOBAL: return m_global[sym];
      }
    case Symbol::CONSTANT: return m_global[sym];
    case Symbol::PROCEDURE: assert (false && "Trying to look up procedure name.");
  }
}

/**
 * callbacks for instructions
 */

CALLBACK(ERROR, i) {
  std::string const* str = (std::string const*) i->args[1];
  std::cout << *str << std::endl;
}

CALLBACK(ASSIGN, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  ASSIGN_operator (fetch_val(dest), fetch_val(arg));
  NEXT(i);
}

CALLBACK(ASSIGN_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  FETCH3(size, i);
  ASSIGN_vec_operator (dest, arg, size);
  NEXT(i);
}

CALLBACK(CLASSIFY, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  CLASSIFY_operator (fetch_val(dest), fetch_val(arg));
  NEXT(i);
}

CALLBACK(CLASSIFY_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  FETCH3(size, i);
  CLASSIFY_vec_operator (dest, arg, size);
  NEXT(i);
}

CALLBACK(DECLASSIFY, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  DECLASSIFY_operator (fetch_val(dest), fetch_val(arg));
  NEXT(i);
}

CALLBACK(DECLASSIFY_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  FETCH3(size, i);
  DECLASSIFY_vec_operator (dest, arg, size);
  NEXT(i);
}

CALLBACK(CALL, i) {
  Instruction* new_ip = (Instruction*) i->args[1];
  Symbol const* dest = i->args[0];
  push_frame(i + 1, dest);
  new_ip->callback (new_ip);
}

CALLBACK(RETCLEAN, i) {
  NEXT(i);
}

CALLBACK(RETVOID, i) {
  (void) i;
  assert (m_frames != 0);
  Instruction* const new_i = m_frames->m_old_ip;
  pop_frame();
  new_i->callback (new_i);
}

CALLBACK(PUSH, i) {
  FETCH2(arg, i);
  m_stack.push(fetch_val(arg));
  NEXT(i);
}

CALLBACK(PUSH_vec, i) {
  FETCH2(arg, i);
  FETCH3(size, i);
  m_stack.push(arg, fetch_val(size).m_uint_val);
  NEXT(i);
}

CALLBACK(POP, i) {
  FETCH1(dest, i);
  m_stack.top(fetch_val(dest));
  m_stack.pop();
  NEXT(i);
}

CALLBACK(POP_vec, i) {
  FETCH1(dest, i);
  FETCH2(size, i);
  unsigned const n = fetch_val(size).m_uint_val;
  m_stack.top(dest, n);
  m_stack.pop(n);
  NEXT(i);
}

CALLBACK(UNEG, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  UNEG_operator (fetch_val(dest), fetch_val(arg));
  NEXT(i);
}

CALLBACK(UNEG_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  FETCH3(size, i);
  UNEG_vec_operator (dest, arg, size);
  NEXT(i);
}

CALLBACK(UMINUS, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  UMINUS_operator (fetch_val(dest), fetch_val(arg));
  NEXT(i);
}

CALLBACK(UMINUS_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg, i);
  FETCH3(size, i);
  UMINUS_vec_operator (dest, arg, size);
  NEXT(i);
}

CALLBACK(MUL, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  MUL_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(MUL_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  MUL_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(DIV, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  DIV_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(DIV_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  DIV_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(MOD, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  MOD_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(MOD_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  MOD_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(ADD, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  ADD_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(ADD_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  ADD_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(SUB, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  SUB_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(SUB_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  SUB_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(EQBOOL, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  EQBOOL_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(EQBOOL_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  EQBOOL_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(EQINT, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  EQINT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(EQINT_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  EQINT_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(EQUINT, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  EQUINT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(EQUINT_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  EQUINT_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(EQSTRING, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  EQSTRING_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(EQSTRING_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  EQSTRING_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(NEQBOOL, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  NEQBOOL_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(NEQBOOL_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  NEQBOOL_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(NEQINT, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  NEQINT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(NEQINT_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  NEQINT_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(NEQUINT, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  NEQUINT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(NEQUINT_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  NEQUINT_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(NEQSTRING, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  NEQSTRING_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(NEQSTRING_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  NEQSTRING_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(LE, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  LE_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(LE_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  LE_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(LT, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  LT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(LT_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  LT_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(GE, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  GE_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(GE_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  GE_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(GT, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  GT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(GT_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  GT_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(LAND, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  LAND_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(LAND_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  LAND_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(LOR, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  LOR_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT(i);
}

CALLBACK(LOR_vec, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(arg2, i);
  FETCH4(size, i);
  LOR_vec_operator (dest, arg1, arg2, size);
  NEXT(i);
}

CALLBACK(JUMP, i) {
  Instruction* new_i = (Instruction*) i->args[0];
  new_i->callback (new_i);
}

CALLBACK(JEBOOL, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_bool_val == fetch_val(arg2).m_bool_val) {
        new_i = (Instruction*) i->args[0];
    }
    new_i->callback (new_i);
}

CALLBACK(JEINT, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_int_val == fetch_val(arg2).m_int_val) {
      new_i = (Instruction*) i->args[0];
    }
    new_i->callback (new_i);
}

CALLBACK(JEUINT, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_uint_val == fetch_val(arg2).m_uint_val) {
      new_i = (Instruction*) i->args[0];
    }
    new_i->callback (new_i);
}

CALLBACK(JESTR, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (*fetch_val(arg1).m_str_val == *fetch_val(arg2).m_str_val) {
        new_i = (Instruction*) i->args[0];
    }
    new_i->callback (new_i);
}

CALLBACK(JNEBOOL, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_bool_val != fetch_val(arg2).m_bool_val) {
        new_i = (Instruction*) i->args[0];
    }
    new_i->callback (new_i);
}

CALLBACK(JNEINT, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_int_val != fetch_val(arg2).m_int_val) {
        new_i = (Instruction*) i->args[0];
    }
    new_i->callback (new_i);
}

CALLBACK(JNEUINT, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_uint_val != fetch_val(arg2).m_uint_val) {
        new_i = (Instruction*) i->args[0];
    }
    new_i->callback (new_i);
}

CALLBACK(JNESTR, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (*fetch_val(arg1).m_str_val != *fetch_val(arg2).m_str_val) {
        new_i = (Instruction*) i->args[0];
    }
    new_i->callback (new_i);
}

CALLBACK(JLE, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_int_val <= fetch_val(arg2).m_int_val) {
        new_i = (Instruction*) i->args[0];
    }

    new_i->callback (new_i);
}

CALLBACK(JLT, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_int_val < fetch_val(arg2).m_int_val) {
        new_i = (Instruction*) i->args[0];
    }

    new_i->callback (new_i);
}

CALLBACK(JGE, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_int_val >= fetch_val(arg2).m_int_val) {
        new_i = (Instruction*) i->args[0];
    }

    new_i->callback (new_i);
}

CALLBACK(JGT, i) {
    FETCH2(arg1, i);
    FETCH3(arg2, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_int_val > fetch_val(arg2).m_int_val) {
        new_i = (Instruction*) i->args[0];
    }

    new_i->callback (new_i);
}

CALLBACK(JT, i) {
    FETCH2(arg1, i);
    Instruction* new_i = i + 1;
    if (fetch_val(arg1).m_bool_val) {
        new_i = (Instruction*) i->args[0];
    }

    new_i->callback (new_i);
}

CALLBACK(JF, i) {
    FETCH2(arg1, i);
    Instruction* new_i = i + 1;
    if (!fetch_val(arg1).m_bool_val) {
        new_i = (Instruction*) i->args[0];
    }

    new_i->callback (new_i);
}

CALLBACK(FILL, i) {
  FETCH1(dest, i);
  FETCH2(arg1, i);
  FETCH3(size, i);
  Value const& v = fetch_val(arg1);
  unsigned const n = fetch_val(size).m_uint_val;
  reserve (dest, n);
  for (Value* it(dest.m_arr); it < dest.m_arr + n; ++ it) *it = v;
  NEXT(i);
}

CALLBACK(LOAD, i) {
  FETCH1(dest, i);
  FETCH2(arr, i);
  FETCH3(index, i);
  store (fetch_val(dest), arr, fetch_val(index).m_uint_val);
  NEXT(i);
}

CALLBACK(STORE, i) {
  FETCH1(d, i);
  FETCH2(index, i);
  FETCH3(arg, i);
  store (d, fetch_val(index).m_uint_val, fetch_val(arg));
  NEXT(i);
}

CALLBACK(NOP, i) {
  NEXT (i);
}

CALLBACK(END, i) {
  (void) i;
}

// vectorized callback if condition holds
#define CONDVCALLBACK(NAME, cond) ((cond) ? NAME##_vec_callback : NAME##_callback)

namespace SecreC {

void VirtualMachine::run (ICodeList const& code) {
    m_code = (Instruction*) calloc(sizeof(Instruction), code.size());
    push_frame(0, 0); // for temporaries in global scope... ugh...

    for (unsigned k = 0; k < code.size(); ++ k) {
      Imop const& imop = *code.at(k);
      Instruction& i = m_code[k];
      
      // copy args
      unsigned nArgs;
      for (nArgs = 0; nArgs < imop.nArgs(); ++ nArgs) {
          Symbol const* sym = imop.arg(nArgs);
          i.args[nArgs] = sym;

          // Store constants in global store:
          if (sym == 0) continue;
          if (imop.type() == Imop::COMMENT) continue;
          if (imop.type() == Imop::ERROR) continue;
          if (sym->symbolType() == Symbol::CONSTANT) {
              SecrecDataType const& dtype = sym->secrecType().secrecDataType();
              Store::iterator it = m_global.find(sym);
              if (it != m_global.end()) continue;
              Register& reg = m_global[sym];
              switch (dtype) {
                  case DATATYPE_BOOL:
                      assert (dynamic_cast<SymbolConstantBool const*>(sym) != 0);
                      store (reg, static_cast<SymbolConstantBool const*>(sym)->value());
                      break;
                  case DATATYPE_INT:
                      assert (dynamic_cast<SymbolConstantInt const*>(sym) != 0);
                      store (reg, static_cast<SymbolConstantInt const*>(sym)->value());
                      break;
                  case DATATYPE_UINT:
                      assert (dynamic_cast<SymbolConstantUInt const*>(sym) != 0);
                      store (reg, static_cast<SymbolConstantUInt const*>(sym)->value());
                      break;
                  case DATATYPE_STRING:
                      assert (dynamic_cast<SymbolConstantString const*>(sym) != 0);
                      store (reg, &static_cast<SymbolConstantString const*>(sym)->value());
                      break;
                  case DATATYPE_INVALID: assert (false && "VM: reached invalid data type.");
              }
          }
      }

      // compute jump destination for calls
      if (imop.type() == Imop::CALL) {
          Symbol const* arg1 = i.args[1];
          assert (dynamic_cast<SymbolProcedure const*>(arg1) != 0);
          SymbolProcedure const* proc = static_cast<SymbolProcedure const*>(arg1);
          size_t const label = proc->target()->index();
          i.args[1] = (Symbol const*) &m_code[label - 1];
      }

      // compute jump destination for jumps
      if ((imop.type() & Imop::JUMP_MASK) != 0x0) {
          Imop const* arg = (Imop const*) i.args[0];
          i.args[0] = (Symbol const*) &m_code[arg->index() - 1];
      }

      // compile the code into sequence of instructions
      // at compile time we know which instructions are vectorized and
      // which aren't, and we also know on which type instructions are called
      // on. for example we replace intermediat code equality with specialized
      // instructions depending on type of first argument
      switch (imop.type()) {
        case Imop::ASSIGN:         i.callback = CONDVCALLBACK(ASSIGN, nArgs == 3); break;
        case Imop::CLASSIFY:       i.callback = CONDVCALLBACK(CLASSIFY, nArgs == 3); break;
        case Imop::DECLASSIFY:     i.callback = CONDVCALLBACK(DECLASSIFY, nArgs == 3); break;
        case Imop::UNEG:           i.callback = CONDVCALLBACK(UNEG, nArgs == 3); break;
        case Imop::UMINUS:         i.callback = CONDVCALLBACK(UMINUS, nArgs == 3);break;
        case Imop::MUL:            i.callback = CONDVCALLBACK(MUL, nArgs == 4); break;
        case Imop::DIV:            i.callback = CONDVCALLBACK(DIV, nArgs == 4); break;
        case Imop::MOD:            i.callback = CONDVCALLBACK(MOD, nArgs == 4); break;
        case Imop::ADD:            i.callback = CONDVCALLBACK(ADD, nArgs == 4); break;
        case Imop::SUB:            i.callback = CONDVCALLBACK(SUB, nArgs == 4); break;
        case Imop::EQ:
          switch (i.args[1]->secrecType().secrecDataType()) {
            case DATATYPE_BOOL:    i.callback = CONDVCALLBACK(EQBOOL, nArgs == 4); break;
            case DATATYPE_INT:     i.callback = CONDVCALLBACK(EQINT, nArgs == 4); break;
            case DATATYPE_UINT:    i.callback = CONDVCALLBACK(EQUINT, nArgs == 4); break;
            case DATATYPE_STRING:  i.callback = CONDVCALLBACK(EQSTRING, nArgs == 4); break;
            case DATATYPE_INVALID: assert (false && "VM: invalid data type");
          }
          break;
        case Imop::NE:
          switch (i.args[1]->secrecType().secrecDataType()) {
            case DATATYPE_BOOL:    i.callback = CONDVCALLBACK(NEQBOOL, nArgs == 4); break;
            case DATATYPE_INT:     i.callback = CONDVCALLBACK(NEQINT, nArgs == 4); break;
            case DATATYPE_UINT:    i.callback = CONDVCALLBACK(NEQUINT, nArgs == 4); break;
            case DATATYPE_STRING:  i.callback = CONDVCALLBACK(NEQSTRING, nArgs == 4); break;
            case DATATYPE_INVALID: assert (false && "VM: invalid data type");
          }
          break;
        case Imop::LE:             i.callback = CONDVCALLBACK(LE, nArgs == 4); break;
        case Imop::LT:             i.callback = CONDVCALLBACK(LT, nArgs == 4); break;
        case Imop::GE:             i.callback = CONDVCALLBACK(GE, nArgs == 4); break;
        case Imop::GT:             i.callback = CONDVCALLBACK(GT, nArgs == 4); break;
        case Imop::LAND:           i.callback = CONDVCALLBACK(LAND, nArgs == 4); break;
        case Imop::LOR:            i.callback = CONDVCALLBACK(LOR, nArgs == 4); break;
        case Imop::CALL:           i.callback = CALL_callback; break;
        case Imop::JUMP:           i.callback = JUMP_callback; break;
        case Imop::JT:             i.callback = JT_callback; break;
        case Imop::JF:             i.callback = JF_callback; break;
        case Imop::JE:
          switch (i.args[1]->secrecType().secrecDataType()) {
            case DATATYPE_BOOL:    i.callback = JEBOOL_callback; break;
            case DATATYPE_INT:     i.callback = JEINT_callback; break;
            case DATATYPE_UINT:    i.callback = JEUINT_callback; break;
            case DATATYPE_STRING:  i.callback = JESTR_callback; break;
            case DATATYPE_INVALID: assert (false && "VM: invalid data type");
          }
          break;
        case Imop::JNE:
          switch (i.args[1]->secrecType().secrecDataType()) {
            case DATATYPE_BOOL:    i.callback = JNEBOOL_callback; break;
            case DATATYPE_INT:     i.callback = JNEINT_callback; break;
            case DATATYPE_UINT:    i.callback = JNEUINT_callback; break;
            case DATATYPE_STRING:  i.callback = JNESTR_callback; break;
            case DATATYPE_INVALID: assert (false && "VM: invalid data type");
          }
          break;
        case Imop::JLE:            i.callback = JLE_callback; break;
        case Imop::JLT:            i.callback = JLT_callback; break;
        case Imop::JGE:            i.callback = JGE_callback; break;
        case Imop::JGT:            i.callback = JGT_callback; break;
        case Imop::COMMENT:        i.callback = NOP_callback; break;
        case Imop::ERROR:          i.callback = ERROR_callback; break;
        case Imop::POP:            i.callback = CONDVCALLBACK (POP, nArgs == 2); break;
        case Imop::PUSH:           i.callback = CONDVCALLBACK (PUSH, nArgs == 3); break;
        case Imop::RETCLEAN:       i.callback = RETCLEAN_callback; break;
        case Imop::RETURNVOID:     i.callback = RETVOID_callback; break;
        case Imop::FILL:           i.callback = FILL_callback; break;
        case Imop::STORE:          i.callback = STORE_callback; break;
        case Imop::LOAD:           i.callback = LOAD_callback; break;
        case Imop::END:            i.callback = END_callback; break;
        default: assert (false && "VM: Reached unfamiliar instruction.");
      }
    }

    // execute
    m_code->callback (m_code);

    // free memory
    pop_frame ();
    free (m_code);
    free_store (m_global);
}

std::string VirtualMachine::toString(void) {
    std::stringstream os;
    os << "Store:\n";
    for (Store::const_iterator i(m_global.begin()); i != m_global.end(); ++ i) {
        Symbol const* sym = i->first;
        Value& val = *i->second.m_arr;
        os << sym->toString() << " -> ";
        switch (sym->secrecType().secrecDataType()) {
            case DATATYPE_BOOL: os << val.m_bool_val; break;
            case DATATYPE_INT: os << val.m_int_val; break;
            case DATATYPE_UINT: os << val.m_uint_val; break;
            case DATATYPE_STRING: os << *val.m_str_val; break;
            case DATATYPE_INVALID: assert (false);
        }

        os << '\n';
    }

    return os.str();
}

}
