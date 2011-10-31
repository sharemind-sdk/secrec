#ifndef IMOP_H
#define IMOP_H

#include <cassert>
#include <set>
#include <string>
#include <vector>

#include <boost/intrusive/list.hpp>

namespace SecreC {

class Block;
class Symbol;
class SymbolProcedure;
class SymbolLabel;
class ConstantString;
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
 * \todo all implicitly parallel instructions should be removed from IL
 *       public machine should operate on scalars and perform system calls
 *       to sharemind platform for private operations.
 *       This, amongst other implications, means that classify/declassify need
 *       to be removed.
 */
class Imop : public auto_unlink_hook {
    public: /* Types: */
        typedef std::vector<const Symbol* > OperandList;
        typedef OperandList::iterator OperandIterator;
        typedef OperandList::const_iterator OperandConstIterator;

        enum Type {
            //-------------
            // Expressions:
            //-------------
            ASSIGN     = 0x1,   /*   d = arg1 {arg2};                   */
            CLASSIFY   = 0x2,   /*   d = CLASSIFY(arg1 {, arg2});       */
            DECLASSIFY = 0x3,   /*   d = DECLASSIFY(arg1 {, arg2});     */
            UNEG       = 0x7,   /*   d = !arg1 {arg2};                  */
            UMINUS     = 0x8,   /*   d = -arg1 {arg2};                  */
            MUL        = 0xa,   /*   d = arg1 *  arg2 {arg3};           */
            DIV        = 0xb,   /*   d = arg1 /  arg2 {arg3};           */
            MOD        = 0xc,   /*   d = arg1 %  arg2 {arg3};           */
            ADD        = 0xd,   /*   d = arg1 +  arg2 {arg3};           */
            SUB        = 0xe,   /*   d = arg1 -  arg2 {arg3};           */
            EQ         = 0xf,   /*   d = arg1 == arg2 {arg3};           */
            NE         = 0x10,  /*   d = arg1 != arg2 {arg3};           */
            LE         = 0x11,  /*   d = arg1 <= arg2 {arg3};           */
            LT         = 0x12,  /*   d = arg1 <  arg2 {arg3};           */
            GE         = 0x13,  /*   d = arg1 >= arg2 {arg3};           */
            GT         = 0x14,  /*   d = arg1 >  arg2 {arg3};           */
            LAND       = 0x15,  /*   d = arg1 && arg2 {arg3};           */
            LOR        = 0x16,  /*   d = arg1 || arg2 {arg3};           */

            //-------------------
            // Array expressions:
            //-------------------
            STORE      = 0x17, /*    d[arg1] = arg2;                    */
            LOAD       = 0x18, /*    d = arg1[arg2];                    */
            ALLOC      = 0x20, /*    d = ALLOC (arg1, arg2)             */

            /* For CALL, arg2 is the corresponding RETCLEAN instruction:*/
            PARAM      = 0x22,  /*   d = PARAM                           */
            CALL       = 0x23,  /*  arg_{n+2}, ..., arg_{n+m+1} = CALL arg0 (arg1, ..., argn, 0)  */
                EXPR_MASK = 0xff, /* expressions always write to dest    */

            //-------
            // Jumps:
            //-------
            JUMP       = 0x100, /* GOTO d;                              */
            JT         = 0x200, /* if (arg1) GOTO d;                    */
            JF         = 0x300, /* if (!arg1) GOTO d;                   */
            JE         = 0x400, /* if (arg1 == arg2) GOTO d;            */
            JNE        = 0x500, /* if (arg1 != arg2) GOTO d;            */
            JLE        = 0x600, /* if (arg1 <= arg2) GOTO d;            */
            JLT        = 0x700, /* if (arg1 <  arg2) GOTO d;            */
            JGE        = 0x800, /* if (arg1 >= arg2) GOTO d;            */
            JGT        = 0x900, /* if (arg1 >  arg2) GOTO d;            */
                JUMP_MASK  = 0xf00,

            //--------------------
            // Misc. instructions:
            //--------------------
            COMMENT    = 0x1000,  /* // arg1                            */
            ERROR      = 0x2000,  /* // arg1                            */
            FREAD      = 0x3000,  /* FREAD                              */


            /* For RETCLEAN, arg2 is the corresponding CALL instruction:*/
            RETCLEAN   = 0x4000,  /* RETCLEAN;             (Imop *arg2) */

            /*
              For RETURN and RETURNVOID, arg2 is the first Imop of the procedure
              to return from.
            */
            RETURNVOID = 0x6000,  /* RETURN;               (Imop *arg2) */
            RETURN     = 0x7000,  /* RETURN ((SymbolLabel*) arg0), arg_1, ..., arg_n */
            END        = 0x8000,  /* END PROGRAM                        */

            PRINT      = 0x9000   /* PRINT arg1; */
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

        inline bool isJump() const { return (m_type & JUMP_MASK) != 0x0; }
        inline bool isCondJump() const { return ((m_type & JUMP_MASK) != 0x0) && (m_type != JUMP); }
        inline bool isExpr() const { return (m_type & EXPR_MASK) != 0x0; }

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


        bool isTerminator (void) const;
        bool isVectorized () const;
        inline bool isComment (void) const {
            return m_type == COMMENT;
        }

        void getUse (std::vector<const Symbol*>& use) const;
        void getDef (std::vector<const Symbol*>& def) const;

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

Imop* newError (TreeNode* node, ConstantString* msg);
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
