#include "virtual_machine.h"
#include "icodelist.h"

#include <string>
#include <map>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdint.h>

#include <boost/preprocessor/control/if.hpp>


#define LEAVETRACE 0
#define PP_IF(bit,arg) BOOST_PP_IF(bit, arg, (void) 0)
#define TRACE(format,msg) PP_IF(LEAVETRACE, fprintf(stderr, format, msg))

namespace { // anonymous namespace

using namespace SecreC;

/**
 * \todo translate this to C
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

void reserve (Value& arr, unsigned n) {
    arr.un_ptr = (Value*) malloc (sizeof (Value) * n);
}

// stack for values
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
    unsigned m_offset;
    unsigned m_size;

    void increase_size () {
        m_size = ((m_size + 1) * 3) / 2;
        TRACE("RESIZE STACK TO %u\n", m_size);
        m_bptr = (Value*) realloc (m_bptr, m_size * sizeof (Value));
    }
};


struct Instruction;

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

typedef std::map<const Symbol*, Value> Store;
typedef void (*CallbackTy)(const Instruction*);

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
 * - pointer to code
 */

ValueStack m_stack;
Frame* m_frames = 0;
Store m_global;

/// Read a table and push it into a stack as a array
void fread_table (const std::string& str) {
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
}

void free_store (Store& store) {
    store.clear();
}

inline void push_frame (const Instruction* old_ip)
{
    Frame* new_frame = new (std::nothrow) Frame (old_ip);
    new_frame->m_prev_frame = m_frames;
    m_frames = new_frame;
}

inline void pop_frame (void)
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
Value& lookup (VMSym sym) __attribute__ ((noinline));
Value& lookup (VMSym sym)  {
    TRACE ("%s ", (sym.isLocal ? "LOCAL" : "GLOBAL"));
    Store& store = sym.isLocal ? m_frames->m_local : m_global;
    return store[sym.un_sym];
}

#define MKSTORESYM(sym, ARGTY, CODE) \
    void storeSym (VMSym sym, ARGTY val) __attribute__ ((noinline)); \
    void storeSym (VMSym sym, ARGTY val) { \
        Store& store = sym.isLocal ? m_frames->m_local : m_global; \
        CODE; \
    }

MKSTORESYM(sym, Value, store[sym.un_sym] = val)
MKSTORESYM(sym, int64_t, store[sym.un_sym].un_int_val = val)
MKSTORESYM(sym, int32_t, store[sym.un_sym].un_int32_val = val)
MKSTORESYM(sym, uint64_t, store[sym.un_sym].un_uint_val = val)
MKSTORESYM(sym, uint32_t, store[sym.un_sym].un_uint32_val = val)
MKSTORESYM(sym, bool, store[sym.un_sym].un_bool_val = val)
MKSTORESYM(sym, std::string*, store[sym.un_sym].un_str_val = val)


/**
 * Macros to simplify code generation:
 */

/// Just to make vim syntax highlighter quiet
#define BLOCK(CODE) { CODE }

#define FETCH(name,i) Value& name = lookup((ip)->args[i])

#define NEXT do ((ip + 1)->callback (ip + 1)); while (0)

#define CUR do ip->callback (ip); while (0)

#define MKCALLBACK(NAME, PDEST, PARG1, PARG2, PARG3, CODE) \
    inline void NAME##_callback (const Instruction* ip) \
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
        unsigned s = arg2.un_uint_val; \
        reserve (dest, s); /* \todo remove this */ \
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
        unsigned s = arg3.un_uint_val; \
        /* reserve (dest, s); */ \
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
DECLOP1 (UNEG, dest.un_bool_val = !arg1.un_bool_val)
DECLOP1 (UMINUS, dest.un_int_val  = -arg1.un_int_val)
DECLOP2 (ADD, dest.un_int_val  = arg1.un_int_val + arg2.un_int_val)
DECLOP2 (ADDSTR, dest.un_str_val = new (std::nothrow) std::string (*arg1.un_str_val + *arg2.un_str_val)) ///< \todo memory leak
DECLOP2 (SUB, dest.un_int_val  = arg1.un_int_val - arg2.un_int_val)
DECLOP2 (MUL, dest.un_int_val  = arg1.un_int_val * arg2.un_int_val)
DECLOP2 (DIV, dest.un_int_val  = arg1.un_int_val / arg2.un_int_val)
DECLOP2 (MOD, dest.un_int_val  = arg1.un_int_val % arg2.un_int_val)
DECLOP2 (LE, dest.un_bool_val = arg1.un_int_val <= arg2.un_int_val)
DECLOP2 (LT, dest.un_bool_val = arg1.un_int_val < arg2.un_int_val)
DECLOP2 (GE, dest.un_bool_val = arg1.un_int_val >= arg2.un_int_val)
DECLOP2 (GT, dest.un_bool_val = arg1.un_int_val > arg2.un_int_val)
DECLOP2 (LAND, dest.un_bool_val = arg1.un_bool_val && arg2.un_bool_val)
DECLOP2 (LOR, dest.un_bool_val = arg1.un_bool_val || arg2.un_bool_val)
DECLOP2 (EQINT, dest.un_bool_val = arg1.un_int_val == arg2.un_int_val)
DECLOP2 (EQUINT, dest.un_bool_val = arg1.un_uint_val == arg2.un_uint_val)
DECLOP2 (EQBOOL, dest.un_bool_val = arg1.un_bool_val == arg2.un_bool_val)
DECLOP2 (EQSTRING, dest.un_bool_val = *(arg1.un_str_val) == *(arg2.un_str_val))
DECLOP2 (NEQINT, dest.un_bool_val = arg1.un_int_val != arg2.un_int_val)
DECLOP2 (NEQUINT, dest.un_bool_val = arg1.un_uint_val != arg2.un_uint_val)
DECLOP2 (NEQBOOL, dest.un_bool_val = arg1.un_bool_val != arg2.un_bool_val)
DECLOP2 (NEQSTRING, dest.un_bool_val = *(arg1.un_str_val) != *(arg2.un_str_val))

/**
 * Various jumps:
 */

MKCALLBACK(JT, 0, 1, 0, 0, JUMPCOND (arg1.un_bool_val))
MKCALLBACK(JF, 0, 1, 0, 0, JUMPCOND (!arg1.un_bool_val))
MKCALLBACK(JEBOOL, 0, 1, 1, 0, JUMPCOND (arg1.un_bool_val == arg2.un_bool_val))
MKCALLBACK(JEINT, 0, 1, 1, 0, JUMPCOND (arg1.un_int_val == arg2.un_int_val))
MKCALLBACK(JEUINT, 0, 1, 1, 0, JUMPCOND (arg1.un_uint_val == arg2.un_uint_val))
MKCALLBACK(JESTR, 0, 1, 1, 0, JUMPCOND (*arg1.un_str_val == *arg2.un_str_val))
MKCALLBACK(JNEBOOL, 0, 1, 1, 0, JUMPCOND (arg1.un_bool_val != arg2.un_bool_val))
MKCALLBACK(JNEINT, 0, 1, 1, 0, JUMPCOND (arg1.un_int_val != arg2.un_int_val))
MKCALLBACK(JNEUINT, 0, 1, 1, 0, JUMPCOND (arg1.un_uint_val != arg2.un_uint_val))
MKCALLBACK(JNESTR, 0, 1, 1, 0, JUMPCOND (*arg1.un_str_val != *arg2.un_str_val))
MKCALLBACK(JLE, 0, 1, 1, 0, JUMPCOND (arg1.un_int_val <= arg2.un_int_val))
MKCALLBACK(JLT, 0, 1, 1, 0, JUMPCOND (arg1.un_int_val < arg2.un_int_val))
MKCALLBACK(JGE, 0, 1, 1, 0, JUMPCOND (arg1.un_int_val >= arg2.un_int_val))
MKCALLBACK(JGT, 0, 1, 1, 0, JUMPCOND (arg1.un_int_val > arg2.un_int_val))

/**
 * Miscellaneous or more complicated instructions:
 */

MKCALLBACK (ERROR, 0, 1, 0, 0,
    fprintf (stderr, "%s\n", arg1.un_str_val->c_str());
    exit (EXIT_FAILURE);
)

MKCALLBACK (PRINT, 0, 1, 0, 0,
    fprintf (stdout, "%s\n", arg1.un_str_val->c_str());
)

MKCALLBACK(FREAD, 0, 0, 0, 0,
    FETCH (arg, 1);
    fread_table (*arg.un_str_val);
)

MKCALLBACK(CALL, 0, 0, 0, 0,
    push_frame (ip + 1);
    ip = ip->args[0].un_inst;
    CUR;
)

MKCALLBACK(RETCLEAN, 0, 0, 0, 0, {} )

MKCALLBACK(RETVOID, 0, 0, 0, 0,
  assert (m_frames != 0);
  const Instruction* new_i = m_frames->m_old_ip;
  pop_frame();
  ip = new_i;
  CUR;
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
    Value const& v = arg1;
    unsigned const n = arg2.un_uint_val;
    reserve (dest, n);
    for (Value* it(dest.un_ptr); it < dest.un_ptr + n; ++ it)
      *it = v;
)

MKCALLBACK(LOAD, 0, 1, 1, 0,
    storeSym (ip->args[0], arg1.un_ptr [arg2.un_uint_val]);
)

MKCALLBACK(STORE, 1, 1, 1, 0,
    dest.un_ptr[arg1.un_uint_val] = arg2;
)

MKCALLBACK(NOP, 0, 0, 0, 0, { })

MKCALLBACK(END, 0, 0, 0, 0, { exit (EXIT_SUCCESS); } )

// vectorized callback if condition holds
#define CONDVCALLBACK(NAME, cond) ((cond) ? NAME##_vec_callback : NAME##_callback)

/**
 * Compiler:
 */

class Compiler {
public: /* Types: */

    typedef std::vector<std::pair<Instruction, const Imop* > > UnlinkedCode;
    typedef std::map<const Imop*, unsigned > ImopAddrs;

public:

    Compiler () : m_codeSize (0) { }
    ~Compiler () { }

    Instruction* runOn (const ICodeList& code) {
        Instruction* out = 0;

        for (ICodeList::const_iterator it = code.begin (); it != code.end (); ++ it) {
            const Imop* imop = *it;
            compileInstruction (*imop);
        }


        out = (Instruction*) calloc(sizeof (Instruction), m_codeSize);
        for (unsigned i = 0; i != m_codeSize; ++ i) {
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
            case Symbol::CONSTANT: {
                SecrecDataType const& dtype = sym->secrecType().secrecDataType();
                out.isLocal = false;
                switch (dtype) {
                    case DATATYPE_BOOL:
                        assert (dynamic_cast<ConstantBool const*>(sym) != 0);
                        storeSym (out, static_cast<ConstantBool const*>(sym)->value());
                        break;
                    case DATATYPE_INT:
                        assert (dynamic_cast<ConstantInt const*>(sym) != 0);
                        storeSym (out, static_cast<ConstantInt const*>(sym)->value());
                        break;
                    case DATATYPE_UINT:
                        assert (dynamic_cast<ConstantUInt const*>(sym) != 0);
                        storeSym (out, static_cast<ConstantUInt const*>(sym)->value());
                        break;
                    case DATATYPE_STRING:
                        assert (dynamic_cast<ConstantString const*>(sym) != 0);
                        storeSym (out, (std::string*) &static_cast<ConstantString const*>(sym)->value());
                        break;
                    default: assert (false && "VM: reached invalid data type.");
                }}
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

        for (nArgs = 0; nArgs < imop.nArgs(); ++ nArgs) {
            Symbol const* sym = imop.arg(nArgs);
            if (sym == 0) continue;
            if (imop.type() == Imop::COMMENT) continue;
            i.args[nArgs] = toVMSym (sym);
        }

        // compute destinations for jumps
        if ((imop.type() & Imop::JUMP_MASK) != 0x0) {
            const Symbol* arg = imop.dest ();
            assert (dynamic_cast<const SymbolLabel*>(arg) != 0);
            dest = static_cast<const SymbolLabel*>(arg)->target ();
        }

        switch (imop.type ()) {
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
          case Imop::PARAM:          i.callback = PARAM_callback; break;
          case Imop::RETCLEAN:       i.callback = RETCLEAN_callback; break;
          case Imop::RETURNVOID:     i.callback = RETVOID_callback; break;
          case Imop::ALLOC:          i.callback = ALLOC_callback; break;
          case Imop::STORE:          i.callback = STORE_callback; break;
          case Imop::LOAD:           i.callback = LOAD_callback; break;
          case Imop::END:            i.callback = END_callback; break;
          case Imop::PRINT:          i.callback = PRINT_callback; break;
          case Imop::FREAD:          i.callback = FREAD_callback; break;
          default:
            assert (false && "VM: Reached unfamiliar instruction.");
        }

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
            Instruction i (PUSH_callback);
            i.args[1] = toVMSym (sym);
            emitInstruction (i);
        }

        assert (it != itEnd && *it == 0 &&
            "Malformed CALL instruction!");

        // CALL
        Instruction i (CALL_callback);
        emitInstruction (i, targetImop);

        // pop return values
        for (++ it; it != itEnd; ++ it) {
            Instruction i (POP_callback);
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
            Instruction i (PUSH_callback);
            i.args[1] = toVMSym (rev.top ());
            emitInstruction (i);
            rev.pop ();
        }

        Instruction i (RETVOID_callback);
        emitInstruction (i);
    }

    void emitInstruction (Instruction i, const Imop* tar = 0) {
        m_code.push_back (std::make_pair (i, tar));
        ++ m_codeSize;
    }

private:

    UnlinkedCode m_code;
    unsigned m_codeSize;
    ImopAddrs m_addrs;
};


} // end of anonymous namespace

namespace SecreC {

void VirtualMachine::run (const ICodeList& icode) {
    Compiler comp;
    Instruction* code = comp.runOn (icode);

    // execute
    push_frame (0);
    code->callback (code);
    pop_frame ();
    free (code);
    free_store (m_global);
}

std::string VirtualMachine::toString(void) {
    std::stringstream os;
    os << "Store:\n";
    for (Store::const_iterator i(m_global.begin()); i != m_global.end(); ++ i) {
        const Symbol* sym = (Symbol const*) i->first;
        const Value& val = i->second;
        os << sym->toString() << " -> ";
        switch (sym->secrecType().secrecDataType()) {
            case DATATYPE_BOOL: os << val.un_bool_val; break;
            case DATATYPE_INT: os << val.un_int_val; break;
            case DATATYPE_UINT: os << val.un_uint_val; break;
            case DATATYPE_STRING: os << *val.un_str_val; break;
            default: assert (false);
        }

        os << '\n';
    }

    return os.str();
}

}
