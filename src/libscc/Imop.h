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

#ifndef SECREC_IMOP_H
#define SECREC_IMOP_H

#include <boost/intrusive/list_hook.hpp>
#include <boost/range/iterator_range.hpp>
#include <cassert>
#include <sharemind/Optional.h>
#include <vector>

#include "Syscall.h"

namespace SecreC {

class Block;
class Symbol;
class SymbolProcedure;
class SymbolLabel;
class ConstantString;
class TreeNode;

using imop_auto_unlink_hook =
    boost::intrusive::list_base_hook<
        boost::intrusive::link_mode<
            boost::intrusive::auto_unlink> > ;

/**
 * Intermediated code instructions.
 * Many of the instructions have optional argument denoting size
 * of an array. For example if addition operator gets arrays as argument
 * it will also get the size of argument and resulting arrays.
 * - Size param is always denoted between curly bracers { and } in comments.
 * - If instruction is performed on scalar no size argument is required.
 * - Destination is always denoted with letter d.
 * - RETURN instruction is now only used to return non-arrays, to return arrays
 *   one must use stack.
 * - ALLOC takes a size parameter and an optional default value.
 *
 * \todo instead of "newing" instructions all the constructors should be
 *       private and there should be special functions for creating new
 *       instructions with proper type safe parameters.
 *       For example:
 *       static Imop* newJump (TreeNode* node, SymbolLabel* target) { ... }
 */
class Imop : public imop_auto_unlink_hook {
public: /* Types: */
    using OperandList = std::vector<Symbol* >;
    using OperandIterator = OperandList::iterator;
    using OperandConstIterator = OperandList::const_iterator;
    using OperandRange = boost::iterator_range<OperandIterator>;
    using OperandConstRange = boost::iterator_range<OperandConstIterator>;

    enum Type {
        //-------------
        // Expressions:
        //-------------
        DECLARE = 0,//   d = UNDEF (marks the start of lifetime)
        ASSIGN,     //   d = arg1 {arg2}
        CAST,       //   d = arg1 {arg2}
        CLASSIFY,   //   d = CLASSIFY(arg1 {, arg2})
        DECLASSIFY, //   d = DECLASSIFY(arg1 {, arg2})
        UINV,       //   d = ~arg1 {arg2}
        UNEG,       //   d = !arg1 {arg2}
        UMINUS,     //   d = -arg1 {arg2}
        TOSTRING,   //   d = toString arg1
        STRLEN,     //   d = STRLEN s
        MUL,        //   d = arg1 *  arg2 {arg3}
        DIV,        //   d = arg1 /  arg2 {arg3}
        MOD,        //   d = arg1 %  arg2 {arg3}
        ADD,        //   d = arg1 +  arg2 {arg3}
        SUB,        //   d = arg1 -  arg2 {arg3}
        EQ,         //   d = arg1 == arg2 {arg3}
        NE,         //   d = arg1 != arg2 {arg3}
        LE,         //   d = arg1 <= arg2 {arg3}
        LT,         //   d = arg1 <  arg2 {arg3}
        GE,         //   d = arg1 >= arg2 {arg3}
        GT,         //   d = arg1 >  arg2 {arg3}
        LAND,       //   d = arg1 && arg2 {arg3}
        LOR,        //   d = arg1 || arg2 {arg3}
        BAND,       //   d = arg1 & arg2 {arg3}
        BOR,        //   d = arg1 | arg2 {arg3}
        XOR,        //   d = arg1 ^ arg2 {arg3}
        SHL,        //   d = arg1 << arg2 {arg3}
        SHR,        //   d = arg1 >> arg2 {arg3}

        //-------------------
        // Array expressions:
        //-------------------
        STORE,      //    d[arg1] = arg2;
        LOAD,       //    d = arg1[arg2];
        ALLOC,      //    d = ALLOC (arg1 {, arg2})
        COPY,       //    d = COPY arg1 arg2
        RELEASE,    //    RELEASE arg1 (marks the end of lifetime)

        //-------------------
        // Other expressions:
        //-------------------
        PARAM,      //    d = PARAM
        DOMAINID,   //    d = DOMAINID((SymbolDomain*)arg1)
        CALL,       //    arg_{n+2}, ..., arg_{n+m+1} = CALL arg0 (arg1, ..., argn, 0)

        //-------
        // Jumps:
        //-------
        JUMP,       //    GOTO d
        JT,         //    if (arg1) GOTO d
        JF,         //    if (!arg1) GOTO d

        //-----------------------
        // Procedure terminating:
        //-----------------------
        ERROR,      //    ERROR ((ConstantString*) arg1)
        RETURN,     //    RETURN ((SymbolLabel*) arg0), arg_1, ..., arg_n
        END,        //    END PROGRAM

        //--------------------
        // Misc. instructions:
        //--------------------
        COMMENT,    //    // (ConstantString*) arg1
        PRINT,      //    PRINT arg1
        SYSCALL,    //    d = __syscall arg1
        RETCLEAN,   //    RETCLEAN; (Imop *arg2)

        _NUM_INSTR  // Number of instructions. Do not add anything after this.
    };

public: /* Methods: */

    Imop(TreeNode *creator, Type type);
    Imop(TreeNode *creator, Type type, OperandList args);
    Imop(TreeNode* creator, ConstantString* name, SyscallOperands operands);
    Imop(TreeNode *creator, Type type, Symbol *dest);
    Imop(TreeNode *creator, Type type, Symbol *dest, Symbol *arg1);

    Imop(TreeNode *creator, Type type, Symbol *dest,
                         Symbol *arg1, Symbol *arg2);

    Imop(TreeNode *creator, Type type, Symbol *dest,
                  Symbol *arg1, Symbol *arg2, Symbol *arg3);


    inline TreeNode *creator() const { return m_creator; }

    inline Type type() const { return m_type; }

    inline unsigned nArgs() const { return m_args.size(); }

    OperandConstIterator operandsBegin () const { return m_args.begin (); }
    OperandConstIterator operandsEnd () const { return m_args.end (); }
    const OperandList& operands () const { return m_args; }

    inline Symbol* dest() const { return arg(0); }
    inline Symbol* arg1() const { return arg(1); }
    inline Symbol* arg2() const { return arg(2); }
    inline Symbol* arg3() const { return arg(3); }
    inline Symbol* arg(unsigned i) const {
        assert (i < m_args.size () &&
                "Imop::arg(unsigned i): index i out of bounds.");
        return m_args[i];
    }

    inline void setDest(Symbol *dest) { setArg(0, dest); }
    inline void setArg1(Symbol *arg1) { setArg(1, arg1); }
    inline void setArg2(Symbol *arg2) { setArg(2, arg2); }
    inline void setArg3(Symbol* arg3) { setArg(3, arg3); }
    inline void setArg(unsigned i, Symbol* arg) {
        assert (i < m_args.size () &&
                "Imop::setArg(unsigned i, const Symbol* arg): index i out of bounds.");
        m_args[i] = arg;
    }

    // everything to do with jumping to labels and calling functions
    SymbolLabel* jumpDest() const;
    const Imop *callDest() const;

    inline Block *block() const { return m_block; }
    inline void setBlock(Block *block) { m_block = block; }

    inline unsigned long index() const { return m_index; }
    inline void setIndex(unsigned long index) { m_index = index; }

    bool isJump () const;
    bool isCondJump () const;
    bool isExpr () const;
    bool isTerminator () const;
    bool isVectorized () const;
    bool isComment () const;
    bool writesDest () const;

    bool isSyscall() const;
    const SyscallOperands& syscallOperands() const;

    void replaceWith (Imop& imop) {
        assert (!imop.is_linked ());
        imop.m_index = m_index;
        imop.m_block = m_block;
        imop.m_creator = m_creator;
        swap_nodes (imop);
    }

    OperandConstRange useRange () const;
    OperandConstRange defRange () const;

    void print (std::ostream& os) const;

    template <typename Iter >
    friend Imop* newReturn (TreeNode *node, Iter begin, Iter end);

    template <typename Iter >
    friend Imop* newCall (TreeNode *node,
                          Iter beginRet, Iter endRet,
                          Iter beginArg, Iter endArg);
private: /* Fields: */

    TreeNode*               m_creator = nullptr;
    const Type              m_type;
    OperandList             m_args;
    sharemind::Optional<SyscallOperands const> const m_syscallOperands;
    Block*                  m_block = nullptr;
    unsigned long           m_index = 0;
};

using ImopList = boost::intrusive::list<Imop, boost::intrusive::constant_time_size<false>>;

/**
 * Convenience operators for Imop creation.
 */

Imop* newError (TreeNode* node, ConstantString* msg);
Imop* newAssign (TreeNode* node, Symbol* dest, Symbol* arg);
Imop* newBinary (TreeNode* node, Imop::Type iType, Symbol* dest, Symbol* arg1, Symbol* arg2);
Imop* newUnary (TreeNode* node, Imop::Type iType, Symbol* dest, Symbol* arg1);
Imop* newNullary (TreeNode* node, Imop::Type iType, Symbol* dest);

template <typename Iter >
Imop* newReturn (TreeNode* node, Iter begin, Iter end) {
    auto out = new Imop (node, Imop::RETURN);
    out->m_args.push_back (nullptr);
    out->m_args.insert(out->m_args.end(), begin, end);
    return out;
}

inline
Imop* newReturn (TreeNode* node) {
    return new Imop (node, Imop::RETURN, nullptr);
}

Imop* newCall (TreeNode *node);

template <typename Iter >
Imop* newCall (TreeNode* node, Iter beginRet, Iter endRet, Iter beginArg, Iter endArg) {
    auto out = new Imop (node, Imop::CALL);

    out->m_args.push_back (nullptr); // call destination
    out->m_args.insert (out->m_args.end(), beginArg, endArg); // arguments
    out->m_args.push_back (nullptr); // marker
    out->m_args.insert (out->m_args.end(), beginRet, endRet); // return values

    return out;
}

inline std::ostream & operator<<(std::ostream & out, const Imop & i) {
    i.print(out);
    return out;
}

} // namespace SecreC

#endif // IMOP_H
