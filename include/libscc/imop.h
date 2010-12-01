#ifndef IMOP_H
#define IMOP_H

#include <cassert>
#include <set>
#include <string>
#include <vector>


namespace SecreC {

class Block;
class Symbol;
class SymbolProcedure;
class TreeNode;

/**
 * Intermediated code instructions.
 * Many of the instructions have optional argument denoting size
 * of an array. For example if addition operator gets arrays as argument
 * it will also get the size of argument and resulting arrays.
 *
 * - Size param is always denoted between curly bracers { and } in comments.
 * - If instruction is performed on scalar no size argument is required.
 * - Destination is always denoted with letter d.
 * - RETURN instruction is now only used to return non-arrays, to return arrays
 *   one must use stack.
 */
class Imop {
    public: /* Types: */
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
            FILL       = 0x20, /*    d = FILL(arg1, arg2)               */

            /* For CALL, arg2 is the corresponding RETCLEAN instruction:*/
            CALL       = 0x21,  /*   d = arg1(PARAMS);   (Imop *arg2)   */
                EXPR_MASK = 0xff,

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
            POP        = 0x3000,  /* d = POP {arg1};                    */
            PUSH       = 0x4000,  /* PUSH arg1 {arg2};                  */

            /* For RETCLEAN, arg2 is the corresponding CALL instruction:*/
            RETCLEAN   = 0x5000,  /* RETCLEAN;             (Imop *arg2) */

            /*
              For RETURN and RETURNVOID, arg2 is the first Imop of the procedure
              to return from.
            */
            RETURNVOID = 0x6000,  /* RETURN;               (Imop *arg2) */
            RETURN     = 0x7000,  /* RETURN arg1;          (Imop *arg2) */
            END        = 0x8000   /* END PROGRAM                        */
        };

    public: /* Methods: */
        explicit inline Imop(TreeNode *creator, Type type)
            : m_creator(creator), m_type(type), m_args() {}

        explicit inline Imop(TreeNode *creator, Type type, Symbol *dest)
            : m_creator(creator), m_type(type), m_args(1, dest) {}

        explicit inline Imop(TreeNode *creator, Type type, Symbol *dest,
                             Symbol *arg1)
            : m_creator(creator), m_type(type), m_args(2)
        { m_args[0] = dest; m_args[1] = arg1; m_args[2] = 0; }

        explicit inline Imop(TreeNode *creator, Type type, Symbol *dest,
                             Symbol *arg1, Symbol *arg2)
            : m_creator(creator), m_type(type), m_args(3)
        { m_args[0] = dest; m_args[1] = arg1; m_args[2] = arg2; }

        explicit inline Imop(TreeNode *creator, Type type, Symbol *dest,
                             Symbol *arg1, Symbol *arg2, Symbol *arg3)
            : m_creator(creator), m_type(type), m_args(4)
        { m_args[0] = dest; m_args[1] = arg1; m_args[2] = arg2; m_args[3] = arg3; }

        ~Imop();

        inline const std::set<Imop*> &incoming() const { return m_incoming; }
        inline const std::set<Imop*> &incomingCalls() const { return m_incomingCalls; }
        inline const std::set<Imop*> &returns() const { return m_returns; }

        inline TreeNode *creator() const { return m_creator; }
        inline Type type() const { return m_type; }
        inline bool isJump() const { return (m_type & JUMP_MASK) != 0x0; }
        inline bool isCondJump() const { return ((m_type & JUMP_MASK) != 0x0) && (m_type != JUMP); }
        inline bool isExpr() const { return (m_type & EXPR_MASK) != 0x0; }
        unsigned nArgs() const { return m_args.size(); }

        inline const Symbol *dest() const { return arg(0); }
        inline const Symbol *arg1() const { return arg(1); }
        inline const Symbol *arg2() const { return arg(2); }
        inline const Symbol *arg3() const { return arg(3); }

        inline void setDest(const Symbol *dest) { setArg(0, dest); }
        inline void setArg1(const Symbol *arg1) { setArg(1, arg1); }
        inline void setArg2(const Symbol *arg2) { setArg(2, arg2); }
        inline void setArg3(const Symbol* arg3) { setArg(3, arg3); }

        inline const Symbol* arg(unsigned i) const {
            assert (i < m_args.size() &&
                    "Imop::arg(unsigned i): index i out of bounds.");
            return m_args[i];
        }

        inline void setArg(unsigned i, const Symbol* arg) {
            assert (i < m_args.size() &&
                    "Imop::setArg(unsigned i, const Symbol* arg): index i out of bounds.");
            m_args[i] = arg;
        }

        inline const Imop *jumpDest() const {
            return (Imop*) dest();
        }

        inline void setJumpDest(Imop *dest) {
            assert(dest != 0);
            assert((m_type & JUMP_MASK) != 0x0);
            setDest((SecreC::Symbol*) dest);
            dest->addIncoming(this);
        }

        const Imop *callDest() const;
        void setCallDest(SymbolProcedure *proc, Imop *clean);
        inline void setReturnDestFirstImop(Imop *firstImop) {
            assert(firstImop != 0);
            assert(firstImop->m_type == COMMENT);
            setArg2((SecreC::Symbol*) firstImop);
            firstImop->addReturn(this);
        }

        inline Block *block() const { return m_block; }
        inline void setBlock(Block *block) { m_block = block; }

        inline unsigned long index() const { return m_index; }
        inline void setIndex(unsigned long index) { m_index = index; }

        std::string toString() const;

    protected: /* Methods: */
        inline void addIncoming(Imop *jump) { m_incoming.insert(jump); }
        inline void addIncomingCall(Imop *jump) { m_incomingCalls.insert(jump); }
        inline void addReturn(Imop *r) { m_returns.insert(r); }
        inline void removeIncoming(Imop *jump) { m_incoming.erase(jump); }
        inline void removeIncomingCall(Imop *jump) { m_incomingCalls.erase(jump); }
        inline void removeReturn(Imop *r) { m_returns.erase(r); }

    private: /* Fields: */
        std::set<Imop*> m_incoming;
        std::set<Imop*> m_incomingCalls;
        std::set<Imop*> m_returns;

        TreeNode     *m_creator;
        const Type    m_type;
        std::vector<Symbol const* > m_args;
        Block        *m_block;
        unsigned long m_index;
};

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Imop &i);

#endif // IMOP_H
