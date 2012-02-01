#ifndef SECREC_IMOP_H
#define SECREC_IMOP_H

#include <cassert>
#include <string>
#include <vector>
#include <boost/intrusive/list.hpp>

#include "parser.h"

namespace SecreC {

class Block;
class Symbol;
class SymbolProcedure;
class SymbolLabel;
template <SecrecDataType ty> class Constant;
class TreeNode;

typedef boost::intrusive::list_base_hook<
            boost::intrusive::link_mode<
                boost::intrusive::auto_unlink> > auto_unlink_hook;

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
 *
 * \todo no unsafe C style casting into Symbol* should ever happen.
 *       Only such use remaining is "COMMENT" instruction, I think this
 *       instruction should be removed and there should be another way of
 *       attaching comments to intermediate code.
 * \todo instead of "newing" instructions all the constructors should be
 *       private and there should be special functions for creating new
 *       instructions with proper type safe parameters.
 *       For example:
 *       static Imop* newJump (TreeNode* node, SymbolLabel* target) { ... }
 */
class Imop : public auto_unlink_hook {
    public: /* Types: */
        typedef std::vector<const Symbol* > OperandList;
        typedef OperandList::iterator OperandIterator;
        typedef OperandList::const_iterator OperandConstIterator;
        typedef std::pair<OperandIterator, OperandIterator> OperandRange;
        typedef std::pair<OperandConstIterator, OperandConstIterator> OperandConstRange;

        enum Type {
            //-------------
            // Expressions:
            //-------------
            ASSIGN = 0, /*   d = arg1 {arg2};                   */
            CAST,       /*   d = arg1 {arg2};                   */
            CLASSIFY,   /*   d = CLASSIFY(arg1 {, arg2});       */
            DECLASSIFY, /*   d = DECLASSIFY(arg1 {, arg2});     */
            UNEG,       /*   d = !arg1 {arg2};                  */
            UMINUS,     /*   d = -arg1 {arg2};                  */
            MUL,        /*   d = arg1 *  arg2 {arg3};           */
            DIV,        /*   d = arg1 /  arg2 {arg3};           */
            MOD,        /*   d = arg1 %  arg2 {arg3};           */
            ADD,        /*   d = arg1 +  arg2 {arg3};           */
            SUB,        /*   d = arg1 -  arg2 {arg3};           */
            EQ,         /*   d = arg1 == arg2 {arg3};           */
            NE,         /*   d = arg1 != arg2 {arg3};           */
            LE,         /*   d = arg1 <= arg2 {arg3};           */
            LT,         /*   d = arg1 <  arg2 {arg3};           */
            GE,         /*   d = arg1 >= arg2 {arg3};           */
            GT,         /*   d = arg1 >  arg2 {arg3};           */
            LAND,       /*   d = arg1 && arg2 {arg3};           */
            LOR,        /*   d = arg1 || arg2 {arg3};           */

            //-------------------
            // Array expressions:
            //-------------------
            STORE,     /*    d[arg1] = arg2;                    */
            LOAD,      /*    d = arg1[arg2];                    */
            ALLOC,     /*    d = ALLOC (arg1, arg2)             */
            COPY,      /*    d = COPY arg1 arg2                 */
            RELEASE,   /*    RELEASE arg1                       */

            //-------------------
            // Other expressions:
            //-------------------
            PARAM,     /*    d = PARAM                          */
            DOMAINID,  /*    d = DOMAINID((SymbolDomain*)arg1)  */
            CALL,      /*  arg_{n+2}, ..., arg_{n+m+1} = CALL arg0 (arg1, ..., argn, 0)  */

            //-------
            // Jumps:
            //-------
            JUMP,      /* GOTO d;                              */
            JT,        /* if (arg1) GOTO d;                    */
            JF,        /* if (!arg1) GOTO d;                   */
            JE,        /* if (arg1 == arg2) GOTO d;            */
            JNE,       /* if (arg1 != arg2) GOTO d;            */
            JLE,       /* if (arg1 <= arg2) GOTO d;            */
            JLT,       /* if (arg1 <  arg2) GOTO d;            */
            JGE,       /* if (arg1 >= arg2) GOTO d;            */
            JGT,       /* if (arg1 >  arg2) GOTO d;            */

            //-----------------------
            // Procedure terminating:
            //-----------------------
            ERROR,      /* ERROR ((ConstantString*) arg1)      */
            RETURNVOID, /* RETURN;               (Imop *arg2)  */
            RETURN,     /* RETURN ((SymbolLabel*) arg0), arg_1, ..., arg_n */
            END,        /* END PROGRAM                         */

            //--------------------
            // Misc. instructions:
            //--------------------
            COMMENT,    /* // (ConstantString*) arg1           */
            PRINT,      /* PRINT arg1; */
            SYSCALL,
            PUSH,
            PUSHREF,
            PUSHCREF,
            RETCLEAN,   /* RETCLEAN;             (Imop *arg2) */

            _NUM_INSTR  /* Number of instructions. Do not add anything after this. */
        };

    public: /* Methods: */
        explicit inline Imop(TreeNode *creator, Type type)
            : m_creator(creator), m_type(type), m_args() {}

        explicit inline Imop(TreeNode *creator, Type type, Symbol *dest)
            : m_creator(creator), m_type(type), m_args(1, dest) {}

        explicit inline Imop(TreeNode *creator, Type type, Symbol *dest,
                             Symbol *arg1)
            : m_creator(creator), m_type(type), m_args(2)
        { m_args[0] = dest; m_args[1] = arg1; }

        explicit inline Imop(TreeNode *creator, Type type, Symbol *dest,
                             Symbol *arg1, Symbol *arg2)
            : m_creator(creator), m_type(type), m_args(3)
        { m_args[0] = dest; m_args[1] = arg1; m_args[2] = arg2; }

        template <typename Iter >
        explicit inline Imop (TreeNode *creator, Type type, Iter begin, Iter end) :
            m_creator (creator), m_type (type), m_args (begin, end) { }

        explicit inline Imop(TreeNode *creator, Type type, Symbol *dest,
                             Symbol *arg1, Symbol *arg2, Symbol *arg3)
            : m_creator(creator), m_type(type), m_args(4)
        { m_args[0] = dest; m_args[1] = arg1; m_args[2] = arg2; m_args[3] = arg3; }
        ~Imop();

        inline TreeNode *creator() const { return m_creator; }

        inline Type type() const { return m_type; }

        inline unsigned nArgs() const { return m_args.size(); }

        OperandConstIterator operandsBegin () const { return m_args.begin (); }
        OperandConstIterator operandsEnd () const { return m_args.end (); }

        inline const Symbol *dest() const { return arg(0); }
        inline const Symbol *arg1() const { return arg(1); }
        inline const Symbol *arg2() const { return arg(2); }
        inline const Symbol *arg3() const { return arg(3); }
        inline const Symbol* arg(unsigned i) const {
            assert (i < m_args.size () &&
                    "Imop::arg(unsigned i): index i out of bounds.");
            return m_args[i];
        }

        inline void setDest(const Symbol *dest) { setArg(0, dest); }
        inline void setArg1(const Symbol *arg1) { setArg(1, arg1); }
        inline void setArg2(const Symbol *arg2) { setArg(2, arg2); }
        inline void setArg3(const Symbol* arg3) { setArg(3, arg3); }
        inline void setArg(unsigned i, const Symbol* arg) {
            assert (i < m_args.size () &&
                    "Imop::setArg(unsigned i, const Symbol* arg): index i out of bounds.");
            m_args[i] = arg;
        }

        // everything to do with jumping to labels and calling functions
        SymbolLabel const* jumpDest() const;
        void setJumpDest (SymbolLabel *dest);
        const Imop *callDest() const;
        void setCallDest(SymbolProcedure *proc);
        void setReturnDestFirstImop(SymbolLabel *label);

        inline Block *block() const { return m_block; }
        inline void setBlock(Block *block) { m_block = block; }

        inline unsigned long index() const { return m_index; }
        inline void setIndex(unsigned long index) { m_index = index; }

        bool isJump () const;
        bool isCondJump () const;
        bool isExpr () const;
        bool isTerminator () const;
        bool isVectorized () const;

        inline bool isComment (void) const {
            return m_type == COMMENT;
        }

        OperandConstRange useRange () const;
        OperandConstRange defRange () const;

        std::string toString() const;

        template <typename Iter >
        friend Imop* newReturn (TreeNode *node, Iter begin, Iter end);

        template <typename Iter >
        friend Imop* newCall (TreeNode *node,
                              Iter beginRet, Iter endRet,
                              Iter beginArg, Iter endArg);

    private: /* Fields: */

        TreeNode*     m_creator;
        const Type    m_type;
        OperandList   m_args;
        Block*        m_block;
        unsigned long m_index;
};

typedef boost::intrusive::list<Imop, boost::intrusive::constant_time_size<false> > ImopList;

/**
 * Convenience operators for Imop creation.
 */

Imop* newError (TreeNode* node, Constant<DATATYPE_STRING>* msg);
Imop* newAssign (TreeNode* node, Symbol* dest, Symbol* arg);
Imop* newBinary (TreeNode* node, Imop::Type iType, Symbol* dest, Symbol* arg1, Symbol* arg2);
Imop* newUnary (TreeNode* node, Imop::Type iType, Symbol* dest, Symbol* arg1);
Imop* newNullary (TreeNode* node, Imop::Type iType, Symbol* dest);

template <typename Iter >
Imop* newReturn (TreeNode* node, Iter begin, Iter end) {
    Imop* out = new Imop (node, Imop::RETURN);
    out->m_args.push_back (0);
    out->m_args.insert(out->m_args.end(), begin, end);
    return out;
}

Imop* newCall (TreeNode *node);

template <typename Iter >
Imop* newCall (TreeNode* node, Iter beginRet, Iter endRet, Iter beginArg, Iter endArg) {
    Imop* out = new Imop (node, Imop::CALL);

    out->m_args.push_back (0); // call destination
    out->m_args.insert (out->m_args.end(), beginArg, endArg); // arguments
    out->m_args.push_back (0); // marker
    out->m_args.insert (out->m_args.end(), beginRet, endRet); // return values

    return out;
}


} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Imop &i);

#endif // IMOP_H
