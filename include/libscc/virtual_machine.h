#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <string>
#include <map>
#include <stack>
#include <iostream>

#include "symboltable.h"
#include "imop.h"
#include "types.h"
#include "log.h"

namespace SecreC {

class ICodeList;

/// \todo a lot of polymorphic operations are still broken
/// \todo write tests and test
/// \todo performance tests
/// \todo desnailify
class VirtualMachine {
private:
    union Value {
        int m_int_val;
        unsigned m_uint_val;
        bool m_bool_val;
        std::string const* m_str_val;
    };

    typedef std::map<Symbol const*, Value> Store;
    typedef std::stack<Value> ArgStack;

public:

    inline VirtualMachine() : m_pc(0) { }

    void run (ICodeList const&);

    std::string toString(void);

private:

    inline
    void error (Symbol const* arg) {
        std::string const* str = (std::string const*) arg;
        std::cout << *str << std::endl;
        m_log.fatal() << *str;
        m_pc = -1;
    }

    inline
    void assign (Symbol const* dest, Symbol const* arg) {
        m_store[dest] = lookup(arg);
        ++ m_pc;
    }

    inline
    void classify (Symbol const* dest, Symbol const* arg) {
        m_store[dest] = lookup(arg);
        ++ m_pc;
    }

    inline
    void declassify (Symbol const* dest, Symbol const* arg) {
        m_store[dest] = lookup(arg);
        ++ m_pc;
    }

    inline
    void call (Symbol const* dest, Symbol const* arg1) {
        assert (dynamic_cast<SymbolProcedure const*>(arg1) != 0);
        SymbolProcedure const* proc = static_cast<SymbolProcedure const*>(arg1);
        size_t const label = proc->target()->index();
        m_ret_stack.push(std::make_pair(dest, m_pc + 1));
        m_pc = label - 1;
    }

    inline
    void pushparam (Symbol const* arg) {
        m_arg_stack.push(lookup(arg));
        ++ m_pc;
    }

    inline
    void popparam (Symbol const* dest) {
        assert (!m_arg_stack.empty());
        m_store[dest] = m_arg_stack.top();
        m_arg_stack.pop();
        ++ m_pc;
    }

    inline
    void ret (Symbol const* val) {
        assert (!m_ret_stack.empty());
        Symbol const* dest = m_ret_stack.top().first;
        size_t const label = m_ret_stack.top().second;
        m_ret_stack.pop();
        m_store[dest] = lookup(val);
        m_pc = label;
    }


    inline
    void uneg (Symbol const* dest, Symbol const* arg) {
        Value const val = { ! lookup(arg).m_bool_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void uminus (Symbol const* dest, Symbol const* arg) {
        Value const val = { - lookup(arg).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void mul (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_int_val * lookup(arg2).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void div (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_int_val / lookup(arg2).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void mod (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_int_val % lookup(arg2).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void add (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_int_val + lookup(arg2).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void sub (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_int_val - lookup(arg2).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void eq (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        SecrecDataType const& dtype = arg1->secrecType().secrecDataType();
        switch (dtype) {
            case DATATYPE_BOOL:
                m_store[dest].m_bool_val = lookup(arg1).m_bool_val == lookup(arg2).m_bool_val;
                ++ m_pc;
                return;
            case DATATYPE_INT:
                m_store[dest].m_bool_val = lookup(arg1).m_int_val == lookup(arg2).m_int_val;
                ++ m_pc;
                return;
            case DATATYPE_UINT:
                m_store[dest].m_bool_val = lookup(arg1).m_uint_val == lookup(arg2).m_uint_val;
                ++ m_pc;
                return;
            case DATATYPE_STRING:
                m_store[dest].m_bool_val =  *lookup(arg1).m_str_val == *lookup(arg2).m_str_val;
                ++ m_pc;
                return;
            case DATATYPE_INVALID: assert (false);
        }
    }

    inline
    void ne (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        SecrecDataType const& dtype = arg1->secrecType().secrecDataType();
        switch (dtype) {
            case DATATYPE_BOOL:
                m_store[dest].m_bool_val = lookup(arg1).m_bool_val != lookup(arg2).m_bool_val;
                ++ m_pc;
                return;
            case DATATYPE_INT:
                m_store[dest].m_bool_val = lookup(arg1).m_int_val != lookup(arg2).m_int_val;
                ++ m_pc;
                return;
            case DATATYPE_UINT:
                m_store[dest].m_bool_val = lookup(arg1).m_uint_val != lookup(arg2).m_uint_val;
                ++ m_pc;
                return;
            case DATATYPE_STRING:
                m_store[dest].m_bool_val =  *lookup(arg1).m_str_val != *lookup(arg2).m_str_val;
                ++ m_pc;
                return;
            case DATATYPE_INVALID: assert (false);
        }
    }

    inline
    void le (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_int_val <= lookup(arg2).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void lt (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_int_val < lookup(arg2).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void ge (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_int_val >= lookup(arg2).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void gt (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_int_val > lookup(arg2).m_int_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void land (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_bool_val && lookup(arg2).m_bool_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void lor (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Value const val = { lookup(arg1).m_bool_val || lookup(arg2).m_bool_val };
        m_store[dest] = val;
        ++ m_pc;
    }

    inline
    void ret (void) {
        assert (!m_ret_stack.empty());
        size_t label = m_ret_stack.top().second;
        m_ret_stack.pop();
        m_pc = label;
    }

    inline
    void jump (Symbol const* dest) {
        Imop const* imop = (Imop const*) dest;
        m_pc = imop->index() - 1;
    }

    inline
    void je (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Imop const* imop = (Imop const*) dest;
        Value const v1 = lookup(arg1);
        Value const v2 = lookup(arg2);
        SecrecDataType const& dtype = arg1->secrecType().secrecDataType();
        size_t const jumpto = imop->index() - 1;
        ++ m_pc;
        switch (dtype) {
            case DATATYPE_BOOL:   if (v1.m_bool_val == v2.m_bool_val) m_pc = jumpto; break;
            case DATATYPE_INT:    if (v1.m_int_val  == v2.m_int_val)  m_pc = jumpto; break;
            case DATATYPE_UINT:   if (v1.m_uint_val == v2.m_uint_val) m_pc = jumpto; break;
            case DATATYPE_STRING: if (*v1.m_str_val == *v2.m_str_val) m_pc = jumpto; break;
            case DATATYPE_INVALID: assert (false);
        }
    }

    inline
    void jne (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Imop const* imop = (Imop const*) dest;
        Value const v1 = lookup(arg1);
        Value const v2 = lookup(arg2);
        SecrecDataType const& dtype = arg1->secrecType().secrecDataType();
        size_t const jumpto = imop->index() - 1;
        ++ m_pc;
        switch (dtype) {
            case DATATYPE_BOOL:   if (v1.m_bool_val != v2.m_bool_val) m_pc = jumpto; break;
            case DATATYPE_INT:    if (v1.m_int_val  != v2.m_int_val)  m_pc = jumpto; break;
            case DATATYPE_UINT:   if (v1.m_uint_val != v2.m_uint_val) m_pc = jumpto; break;
            case DATATYPE_STRING: if (*v1.m_str_val != *v2.m_str_val) m_pc = jumpto; break;
            case DATATYPE_INVALID: assert (false);
        }
    }

    inline
    void jle (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Imop const* imop = (Imop const*) dest;
        Value const v1 = lookup(arg1);
        Value const v2 = lookup(arg2);
        ++ m_pc;
        if (v1.m_int_val <= v2.m_int_val) {
            m_pc = imop->index() - 1;
        }
    }

    inline
    void jlt (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Imop const* imop = (Imop const*) dest;
        Value const v1 = lookup(arg1);
        Value const v2 = lookup(arg2);
        ++ m_pc;
        if (v1.m_int_val < v2.m_int_val) {
            m_pc = imop->index() - 1;
        }
    }

    inline
    void jge (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Imop const* imop = (Imop const*) dest;
        Value const v1 = lookup(arg1);
        Value const v2 = lookup(arg2);
        ++ m_pc;
        if (v1.m_int_val >= v2.m_int_val) {
            m_pc = imop->index() - 1;
        }
    }

    inline
    void jgt (Symbol const* dest, Symbol const* arg1, Symbol const* arg2) {
        Imop const* imop = (Imop const*) dest;
        Value const v1 = lookup(arg1);
        Value const v2 = lookup(arg2);
        ++ m_pc;
        if (v1.m_int_val > v2.m_int_val) {
            m_pc = imop->index() - 1;
        }
    }

    inline
    void jt (Symbol const* dest, Symbol const* arg) {
        Imop const* imop = (Imop const*) dest;
        Value const v = lookup(arg);
        ++ m_pc;
        if (v.m_bool_val) {
            m_pc = imop->index() - 1;
        }
    }

    inline
    void jf (Symbol const* dest, Symbol const* arg) {
        Imop const* imop = (Imop const*) dest;
        Value const v = lookup(arg);
        ++ m_pc;
        if (!v.m_bool_val) {
            m_pc = imop->index() - 1;
        }
    }

    inline void nop (void) {
        ++ m_pc;
    }

    inline
    Value lookup (Symbol const* sym) const {
        SecrecDataType const& dtype = sym->secrecType().secrecDataType();
        Value out;
        switch (sym->symbolType()) {
            case Symbol::CONSTANT:
                assert (sym->secrecType().tnvDataType().kind() == DataType::BASIC);
                switch (dtype) {
                    case DATATYPE_BOOL:
                        assert (dynamic_cast<SymbolConstantBool const*>(sym));
                        out.m_bool_val = static_cast<SymbolConstantBool const*>(sym)->value();
                        break;
                    case DATATYPE_INT:
                        assert (dynamic_cast<SymbolConstantInt const*>(sym));
                        out.m_int_val = static_cast<SymbolConstantInt const*>(sym)->value();
                        break;
                    case DATATYPE_UINT:
                        assert (dynamic_cast<SymbolConstantUInt const*>(sym));
                        out.m_uint_val = static_cast<SymbolConstantUInt const*>(sym)->value();
                        break;
                    case DATATYPE_STRING:
                        assert (dynamic_cast<SymbolConstantString const*>(sym));
                        out.m_str_val = &static_cast<SymbolConstantString const*>(sym)->value();
                        break;
                    case DATATYPE_INVALID:
                        assert (false);
                }

                break;

            case Symbol::TEMPORARY:
            case Symbol::SYMBOL:
                out = m_store.find(sym)->second;
                break;

            case Symbol::PROCEDURE:
                assert (false);
        }

        return out;
    }

private:
    ArgStack m_arg_stack;
    std::stack<std::pair<Symbol const*, size_t> > m_ret_stack;
    Store m_store;
    size_t m_pc;
    CompileLog m_log;
};

}

#endif // VIRTUAL_MACHINE_H
