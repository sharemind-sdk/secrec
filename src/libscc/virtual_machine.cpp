#include "virtual_machine.h"

#include <string>
#include <map>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdint.h>

#include "icodelist.h"

namespace { // anonymous namespace

using namespace SecreC;

/**
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
    uint64_t m_uint_val;
    uint32_t m_uint32_val;
    uint16_t m_uint16_val;
    uint8_t  m_uint8_val;
    int64_t m_int_val;
    int32_t m_int32_val;
    int16_t m_int16_val;
    int8_t  m_int8_val;
    std::string const* m_str_val;
    bool m_bool_val;
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
static inline void store (Register& reg, std::string const* val) { reg.m_arr->m_str_val = val; }
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

        void push (Value const& val) { vals.push_back(val); }
        void push (int n) {
            Value val = { n };
            vals.push_back(val);
        }

        void push (Register const& reg, unsigned n) {
            if (n == 0) return;
          vals.reserve (vals.size() + n);
          for (Value* i = reg.m_arr + n - 1; i >= reg.m_arr; -- i) {
            vals.push_back(*i);
          }
        }
};

typedef void* Location;

struct VMSym {
    bool isLocal : 1;
    Location loc;
    inline VMSym (bool isLocal, Location loc)
        : isLocal(isLocal), loc(loc) { }
};

typedef std::map<Location, Register> Store;

struct Instruction {
  VMSym args[4];
  void (*callback)();
};

struct Frame {
    Store m_local;
    Instruction* const m_old_ip;
    VMSym const m_ret;
    Frame* m_prev_frame;

    Frame(Instruction* const old_ip, VMSym ret)
        : m_old_ip(old_ip), m_ret(ret), m_prev_frame(0) { }

private:
    Frame(Frame const&);
    Frame& operator = (Frame const&);
};


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
Instruction* ip = 0;

static void free_store (Store& store) {
    Store::iterator it = store.begin();
    Store::iterator it_end = store.end();
    for (; it != it_end; ++ it) {
        Register& r = it->second;
        free (r.m_arr);
    }

    store.clear();
}

static inline void push_frame (Instruction* const old_ip, VMSym ret)
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
/// This would have to be constant time. In order to
/// achieve this the global and local stores would have to be
/// stacks and locations offsets in those stacks. The offsets would
/// have to be precomputed. This needs too much work for now.
static Register& lookup (VMSym const sym) {
    Store& store = sym.isLocal ? m_frames->m_local : m_global;
    return store[sym.loc];
}

/**
 * Macros to simplify code generation:
 */

#define FETCH1(name) Register& name = lookup((ip)->args[0])
#define FETCH2(name) Register const& name = lookup((ip)->args[1])
#define FETCH3(name) Register const& name = lookup((ip)->args[2])
#define FETCH4(name) Register const& name = lookup((ip)->args[3])
#define NEXT do ((++ ip)->callback ()); while (0)

#define DECLOP1(NAME) inline void NAME##_operator (Value& dest, Value const& arg1)
#define DECLOP2(NAME) inline void NAME##_operator (Value& dest, Value const& arg1, Value const& arg2)

#define DECLVECOP1(NAME) inline void NAME##_vec_operator (Register& dest, Register const& arg1, Register const& size) {\
unsigned s = size.m_arr->m_uint_val;\
reserve (dest, s);\
Value* desti = dest.m_arr;\
Value* end = dest.m_arr + s;\
Value* arg1i = arg1.m_arr;\
for (; desti != end; ++ desti, ++ arg1i)\
  NAME##_operator (*desti, *arg1i);\
}

#define DECLVECOP2(NAME) inline void NAME##_vec_operator (Register& dest, Register const& arg1, Register const& arg2, Register const& size) {\
unsigned s = size.m_arr->m_uint_val;\
reserve (dest, s);\
Value* desti = dest.m_arr;\
Value* end = dest.m_arr + s;\
Value* arg1i = arg1.m_arr;\
Value* arg2i = arg2.m_arr;\
for (; desti != end; ++ desti, ++ arg1i, ++ arg2i)\
  NAME##_operator (*desti, *arg1i, *arg2i);\
}

#define CALLBACK(NAME) inline void NAME##_callback ()

/**
 * Regular primitive operations on data:
 */

DECLOP1 (ASSIGN)     { dest = arg1; }
DECLOP1 (CLASSIFY)   { dest = arg1; }
DECLOP1 (DECLASSIFY) { dest = arg1; }
DECLOP1 (UNEG)       { dest.m_bool_val = !arg1.m_bool_val; }
DECLOP1 (UMINUS)     { dest.m_int_val  = -arg1.m_int_val; }
DECLOP2 (ADD)        { dest.m_int_val  = arg1.m_int_val + arg2.m_int_val; }
DECLOP2 (ADDSTR)     { dest.m_str_val  = new std::string (*arg1.m_str_val + *arg2.m_str_val); } ///< \todo memory leaks here
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
DECLVECOP2 (ADDSTR)
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
 * Callbacks for instructions:
 * \todo quite a lot of repeated code
 * \todo get rid of the rather stupid FETCH macros
 */

CALLBACK(ERROR) {
    FETCH2(arg);
    std::cerr << *fetch_val(arg).m_str_val << std::endl;
}

CALLBACK(PRINT) {
    FETCH2(arg);
    std::cout << *fetch_val(arg).m_str_val << std::endl;
    NEXT;
}

CALLBACK(FREAD) {
    FETCH2(arg);
    std::string const& str = *fetch_val(arg).m_str_val;
    std::ifstream fhandle;
    std::string line;
    std::vector<int > values;
    fhandle.open (str.c_str(), std::ios::in);
    int rowCount = 0;
    int colCount = 0;
    int tmp = 0;
    int n;


    if (!fhandle.is_open()) {
        std::cout << "Unable to open file named \"" << str << "\"." << std::endl;
        return;
    }

    while (std::getline (fhandle, line)) {
        std::stringstream ss (line.c_str());
        tmp = 0;
        while (ss.good()) {
            if (ss >> n) {
                values.push_back(n);
                ++ tmp;
            }
        }

        if (rowCount == 0 || tmp == rowCount) {
            rowCount = tmp;
        }
        else {
            fhandle.close();
            std::cout << "Every line of \"" << str << "\" has to have equal number of values." << std::endl;
            std::cout << "Mismatch at line " << colCount + 1 << std::endl;
            return;
        }

        ++ colCount;
    }

    fhandle.close();

    for (int row = 0; row < rowCount; ++ row) {
        for (int col = 0; col < colCount; ++ col) {
            m_stack.push(values[(rowCount - row - 1) + (colCount - col - 1)*rowCount]);
        }
    }

    m_stack.push(colCount);
    m_stack.push(rowCount);

    NEXT;
}

CALLBACK(ASSIGN) {
  FETCH1(dest);
  FETCH2(arg);
  ASSIGN_operator (fetch_val(dest), fetch_val(arg));
  NEXT;
}

CALLBACK(ASSIGN_vec) {
  FETCH1(dest);
  FETCH2(arg);
  FETCH3(size);
  ASSIGN_vec_operator (dest, arg, size);
  NEXT;
}

CALLBACK(CLASSIFY) {
  FETCH1(dest);
  FETCH2(arg);
  CLASSIFY_operator (fetch_val(dest), fetch_val(arg));
  NEXT;
}

CALLBACK(CLASSIFY_vec) {
  FETCH1(dest);
  FETCH2(arg);
  FETCH3(size);
  CLASSIFY_vec_operator (dest, arg, size);
  NEXT;
}

CALLBACK(DECLASSIFY) {
  FETCH1(dest);
  FETCH2(arg);
  DECLASSIFY_operator (fetch_val(dest), fetch_val(arg));
  NEXT;
}

CALLBACK(DECLASSIFY_vec) {
  FETCH1(dest);
  FETCH2(arg);
  FETCH3(size);
  DECLASSIFY_vec_operator (dest, arg, size);
  NEXT;
}

CALLBACK(CALL) {
  VMSym dest = ip->args[0];
  push_frame (ip + 1, dest);
  ip = (Instruction*) ip->args[1].loc;
  ip->callback ();
}

CALLBACK(RETCLEAN) {
  NEXT;
}

CALLBACK(RETVOID) {
  assert (m_frames != 0);
  Instruction* const new_i = m_frames->m_old_ip;
  pop_frame();
  ip = new_i;
  ip->callback ();
}

CALLBACK(PUSH) {
  FETCH2(arg);
  m_stack.push(fetch_val(arg));
  NEXT;
}

CALLBACK(PUSH_vec) {
  FETCH2(arg);
  FETCH3(size);
  m_stack.push(arg, fetch_val(size).m_uint_val);
  NEXT;
}

CALLBACK(POP) {
  FETCH1(dest);
  m_stack.top(fetch_val(dest));
  m_stack.pop();
  NEXT;
}

CALLBACK(POP_vec) {
  FETCH1(dest);
  FETCH2(size);
  unsigned const n = fetch_val(size).m_uint_val;
  m_stack.top(dest, n);
  m_stack.pop(n);
  NEXT;
}

CALLBACK(UNEG) {
  FETCH1(dest);
  FETCH2(arg);
  UNEG_operator (fetch_val(dest), fetch_val(arg));
  NEXT;
}

CALLBACK(UNEG_vec) {
  FETCH1(dest);
  FETCH2(arg);
  FETCH3(size);
  UNEG_vec_operator (dest, arg, size);
  NEXT;
}

CALLBACK(UMINUS) {
  FETCH1(dest);
  FETCH2(arg);
  UMINUS_operator (fetch_val(dest), fetch_val(arg));
  NEXT;
}

CALLBACK(UMINUS_vec) {
  FETCH1(dest);
  FETCH2(arg);
  FETCH3(size);
  UMINUS_vec_operator (dest, arg, size);
  NEXT;
}

CALLBACK(MUL) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  MUL_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(MUL_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  MUL_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(DIV) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  DIV_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(DIV_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  DIV_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(MOD) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  MOD_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(MOD_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  MOD_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(ADD) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  ADD_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(ADD_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  ADD_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(ADDSTR) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  ADDSTR_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(ADDSTR_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  ADDSTR_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(SUB) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  SUB_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(SUB_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  SUB_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(EQBOOL) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  EQBOOL_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(EQBOOL_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  EQBOOL_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(EQINT) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  EQINT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(EQINT_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  EQINT_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(EQUINT) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  EQUINT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(EQUINT_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  EQUINT_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(EQSTRING) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  EQSTRING_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(EQSTRING_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  EQSTRING_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(NEQBOOL) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  NEQBOOL_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(NEQBOOL_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  NEQBOOL_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(NEQINT) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  NEQINT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(NEQINT_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  NEQINT_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(NEQUINT) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  NEQUINT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(NEQUINT_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  NEQUINT_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(NEQSTRING) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  NEQSTRING_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(NEQSTRING_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  NEQSTRING_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(LE) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  LE_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(LE_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  LE_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(LT) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  LT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(LT_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  LT_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(GE) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  GE_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(GE_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  GE_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(GT) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  GT_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(GT_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  GT_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(LAND) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  LAND_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(LAND_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  LAND_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(LOR) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  LOR_operator (fetch_val(dest), fetch_val(arg1), fetch_val(arg2));
  NEXT;
}

CALLBACK(LOR_vec) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(arg2);
  FETCH4(size);
  LOR_vec_operator (dest, arg1, arg2, size);
  NEXT;
}

CALLBACK(JUMP) {
  ip = (Instruction*) ip->args[0].loc;
  ip->callback ();
}

CALLBACK(JEBOOL) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_bool_val == fetch_val(arg2).m_bool_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JEINT) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_int_val == fetch_val(arg2).m_int_val) {
      newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JEUINT) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_uint_val == fetch_val(arg2).m_uint_val) {
      newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JESTR) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (*fetch_val(arg1).m_str_val == *fetch_val(arg2).m_str_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JNEBOOL) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_bool_val != fetch_val(arg2).m_bool_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JNEINT) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_int_val != fetch_val(arg2).m_int_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JNEUINT) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_uint_val != fetch_val(arg2).m_uint_val) {
        ip = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JNESTR) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (*fetch_val(arg1).m_str_val != *fetch_val(arg2).m_str_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JLE) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_int_val <= fetch_val(arg2).m_int_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JLT) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_int_val < fetch_val(arg2).m_int_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JGE) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_int_val >= fetch_val(arg2).m_int_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JGT) {
    FETCH2(arg1);
    FETCH3(arg2);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_int_val > fetch_val(arg2).m_int_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JT) {
    FETCH2(arg1);
    Instruction* newIp = ip + 1;
    if (fetch_val(arg1).m_bool_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(JF) {
    FETCH2(arg1);
    Instruction* newIp = ip + 1;
    if (!fetch_val(arg1).m_bool_val) {
        newIp = (Instruction*) ip->args[0].loc;
    }

    ip = newIp;
    ip->callback ();
}

CALLBACK(FILL) {
  FETCH1(dest);
  FETCH2(arg1);
  FETCH3(size);
  Value const& v = fetch_val(arg1);
  unsigned const n = fetch_val(size).m_uint_val;
  reserve (dest, n);
  for (Value* it(dest.m_arr); it < dest.m_arr + n; ++ it) *it = v;
  NEXT;
}

CALLBACK(LOAD) {
  FETCH1(dest);
  FETCH2(arr);
  FETCH3(index);
  store (fetch_val(dest), arr, fetch_val(index).m_uint_val);
  NEXT;
}

CALLBACK(STORE) {
  FETCH1(d);
  FETCH2(index);
  FETCH3(arg);
  store (d, fetch_val(index).m_uint_val, fetch_val(arg));
  NEXT;
}

CALLBACK(NOP) {
  NEXT;
}

CALLBACK(END) { }

} // end of anonymous namespace

// vectorized callback if condition holds
#define CONDVCALLBACK(NAME, cond) ((cond) ? NAME##_vec_callback : NAME##_callback)

namespace SecreC {

void VirtualMachine::run (ICodeList const& code) {
    Instruction* m_code = 0;
    m_code = (Instruction*) calloc(sizeof(Instruction), code.size());
    push_frame(0, VMSym(false, 0)); // for temporaries in global scope... ugh...

    for (unsigned k = 0; k < code.size(); ++ k) {
      Imop const& imop = *code.at(k);
      Instruction& i = m_code[k];
      
      // copy args
      unsigned nArgs;
      for (nArgs = 0; nArgs < imop.nArgs(); ++ nArgs) {
          Symbol const* sym = imop.arg(nArgs);
          Location loc = (void*) sym;
          i.args[nArgs].isLocal = true;
          i.args[nArgs].loc = loc;

          // Correct isLocal and store constants in global store:
          if (sym == 0) continue;
          if (imop.type() == Imop::COMMENT) continue;
          switch (sym->symbolType()) {
              case Symbol::SYMBOL:
                  assert (dynamic_cast<SymbolSymbol const*>(sym) != 0
                          && "VM: Symbol::SYMBOL that isn't SymbolSymbol.");
                  if (static_cast<SymbolSymbol const*>(sym)->scopeType() == SymbolSymbol::GLOBAL)
                      i.args[nArgs].isLocal = false;
                  break;
              case Symbol::CONSTANT: {
                  SecrecDataType const& dtype = sym->secrecType().secrecDataType();
                  Store::iterator it = m_global.find(loc);
                  i.args[nArgs].isLocal = false;
                  if (it != m_global.end()) continue;
                  Register& reg = m_global[loc];
                  switch (dtype) {
                      case DATATYPE_BOOL:
                          assert (dynamic_cast<ConstantBool const*>(sym) != 0);
                          store (reg, static_cast<ConstantBool const*>(sym)->value());
                          break;
                      case DATATYPE_INT:
                          assert (dynamic_cast<ConstantInt const*>(sym) != 0);
                          store (reg, static_cast<ConstantInt const*>(sym)->value());
                          break;
                      case DATATYPE_UINT:
                          assert (dynamic_cast<ConstantUInt const*>(sym) != 0);
                          store (reg, static_cast<ConstantUInt const*>(sym)->value());
                          break;
                      case DATATYPE_STRING:
                          assert (dynamic_cast<ConstantString const*>(sym) != 0);
                          store (reg, &static_cast<ConstantString const*>(sym)->value());
                          break;
                      default: assert (false && "VM: reached invalid data type.");
                  }}
              default: break;
          }
      }

      // compute destinations for calls
      if (imop.type() == Imop::CALL) {
          Symbol const* arg1 = (Symbol const*) i.args[1].loc;
          assert (dynamic_cast<SymbolProcedure const*>(arg1) != 0);
          SymbolProcedure const* proc = static_cast<SymbolProcedure const*>(arg1);
          size_t const label = proc->target()->index();
          i.args[1].loc = (void*) &m_code[label - 1];
      }

      // compute destinations for jumps
      if ((imop.type() & Imop::JUMP_MASK) != 0x0) {
          Symbol const* arg = (Symbol const*) i.args[0].loc;
          assert (dynamic_cast<SymbolLabel const*>(arg) != 0);
          Imop const* targetImop = static_cast<SymbolLabel const*>(arg)->target ();
          i.args[0].loc = (void*) &m_code[targetImop->index() - 1];
      }

      // compile the code into sequence of instructions
      // at compile time we know which instructions are vectorized and
      // which aren't, and we also know on which types instructions are called
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
        case Imop::ADD:
          switch (imop.arg1()->secrecType().secrecDataType()) {
            case DATATYPE_STRING:  i.callback = CONDVCALLBACK(ADDSTR, nArgs == 4); break;
            default:               i.callback = CONDVCALLBACK(ADD, nArgs == 4); break;
          }
          break;
        case Imop::SUB:            i.callback = CONDVCALLBACK(SUB, nArgs == 4); break;
        case Imop::EQ:
          switch (imop.arg1()->secrecType().secrecDataType()) {
            case DATATYPE_BOOL:    i.callback = CONDVCALLBACK(EQBOOL, nArgs == 4); break;
            case DATATYPE_INT:     i.callback = CONDVCALLBACK(EQINT, nArgs == 4); break;
            case DATATYPE_UINT:    i.callback = CONDVCALLBACK(EQUINT, nArgs == 4); break;
            case DATATYPE_STRING:  i.callback = CONDVCALLBACK(EQSTRING, nArgs == 4); break;
            default: assert (false && "VM: invalid data type");
          }
          break;
        case Imop::NE:
          switch (imop.arg1()->secrecType().secrecDataType()) {
            case DATATYPE_BOOL:    i.callback = CONDVCALLBACK(NEQBOOL, nArgs == 4); break;
            case DATATYPE_INT:     i.callback = CONDVCALLBACK(NEQINT, nArgs == 4); break;
            case DATATYPE_UINT:    i.callback = CONDVCALLBACK(NEQUINT, nArgs == 4); break;
            case DATATYPE_STRING:  i.callback = CONDVCALLBACK(NEQSTRING, nArgs == 4); break;
            default: assert (false && "VM: invalid data type");
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
          switch (imop.arg1()->secrecType().secrecDataType()) {
            case DATATYPE_BOOL:    i.callback = JEBOOL_callback; break;
            case DATATYPE_INT:     i.callback = JEINT_callback; break;
            case DATATYPE_UINT:    i.callback = JEUINT_callback; break;
            case DATATYPE_STRING:  i.callback = JESTR_callback; break;
            default: assert (false && "VM: invalid data type");
          }
          break;
        case Imop::JNE:
          switch (imop.arg1()->secrecType().secrecDataType()) {
            case DATATYPE_BOOL:    i.callback = JNEBOOL_callback; break;
            case DATATYPE_INT:     i.callback = JNEINT_callback; break;
            case DATATYPE_UINT:    i.callback = JNEUINT_callback; break;
            case DATATYPE_STRING:  i.callback = JNESTR_callback; break;
            default: assert (false && "VM: invalid data type");
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
        case Imop::PRINT:          i.callback = PRINT_callback; break;
        case Imop::FREAD:          i.callback = FREAD_callback; break;
        default: assert (false && "VM: Reached unfamiliar instruction.");
      }
    }

    // execute
    ip = m_code;
    ip->callback ();

    // free memory
    pop_frame ();
    free (m_code);
    free_store (m_global);
}

std::string VirtualMachine::toString(void) {
    std::stringstream os;
    os << "Store:\n";
    for (Store::const_iterator i(m_global.begin()); i != m_global.end(); ++ i) {
        Symbol const* sym = (Symbol const*) i->first;
        Value& val = *i->second.m_arr;
        os << sym->toString() << " -> ";
        switch (sym->secrecType().secrecDataType()) {
            case DATATYPE_BOOL: os << val.m_bool_val; break;
            case DATATYPE_INT: os << val.m_int_val; break;
            case DATATYPE_UINT: os << val.m_uint_val; break;
            case DATATYPE_STRING: os << *val.m_str_val; break;
            default: assert (false);
        }

        os << '\n';
    }

    return os.str();
}

}
