#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include <set>
#include <sstream>
#include "secrec/symboltable.h"
#include "secrec/types.h"


namespace SecreC {

class TreeNodeProgram;

class Imop {
    public: /* Types: */
        enum Type {
            COMMENT    = 0x0,   /* // arg1                            */
            VARINTRO   = 0x1,   /* d;          (variable declaration) */
            ASSIGN     = 0x2,   /*   d = arg1;                        */
            CAST       = 0x3,   /*   d = (arg1) arg2;                 */
            WILDCARD   = 0x4,   /*   d = arg1[*];                     */
            SUBSCRIPT  = 0x5,   /*   d = arg1[arg2];                  */
            UNEG       = 0x6,   /*   d = !arg1;                       */
            UMINUS     = 0x7,   /*   d = -arg1;                       */
            MATRIXMUL  = 0x8,   /*   d = arg1 #  arg2;                */
            MUL        = 0x9,   /*   d = arg1 *  arg2;                */
            DIV        = 0xa,   /*   d = arg1 /  arg2;                */
            MOD        = 0xb,   /*   d = arg1 %  arg2;                */
            ADD        = 0xc,   /*   d = arg1 +  arg2;                */
            SUB        = 0xd,   /*   d = arg1 -  arg2;                */
            EQ         = 0xe,   /*   d = arg1 == arg2;                */
            NE         = 0xf,   /*   d = arg1 != arg2;                */
            LE         = 0x10,  /*   d = arg1 <= arg2;                */
            LT         = 0x11,  /*   d = arg1 <  arg2;                */
            GE         = 0x12,  /*   d = arg1 >= arg2;                */
            GT         = 0x13,  /*   d = arg1 >  arg2;                */
            LAND       = 0x14,  /*   d = arg1 && arg2;                */
            LOR        = 0x15,  /*   d = arg1 || arg2;                */

            PUTPARAM   = 0x16,  /* PUTPARAM arg1;                     */
            /* For CALL, arg2 is the corresponding RETCLEAN instruction. */
            CALL       = 0x17,  /*   d = arg1(PARAMS);   (Imop *arg2) */
            /* For RETCLEAN, arg2 is the corresponding CALL instruction. */
            RETCLEAN   = 0x18,  /* RETCLEAN;             (Imop *arg2) */

            /*
              For RETURN and RETURNVOID, arg2 is the first Imop of the procedure
              to return from.
            */
            RETURNVOID = 0x19,  /* RETURN;               (Imop *arg2) */
            RETURN     = 0x20,  /* RETURN arg1;          (Imop *arg2) */
            END        = 0x21,  /* END PROGRAM                        */

            JUMP       = 0x100, /* GOTO d;                            */
            JT         = 0x200, /* if (arg1) GOTO d;                  */
            JF         = 0x300, /* if (!arg1) GOTO d;                 */
            JE         = 0x400, /* if (arg1 == arg2) GOTO d;          */
            JNE        = 0x500, /* if (arg1 != arg2) GOTO d;          */
            JLE        = 0x600, /* if (arg1 <= arg2) GOTO d;          */
            JLT        = 0x700, /* if (arg1 <  arg2) GOTO d;          */
            JGE        = 0x800, /* if (arg1 >= arg2) GOTO d;          */
            JGT        = 0x900, /* if (arg1 >  arg2) GOTO d;          */
                JUMP_MASK  = 0xf00
        };

    public: /* Methods: */
        explicit inline Imop(Type type)
            : m_type(type) {}
        explicit inline Imop(Type type, Symbol *dest)
            : m_type(type), m_dest(dest) {}
        explicit inline Imop(Type type, Symbol *dest, Symbol *arg1)
            : m_type(type), m_dest(dest), m_arg1(arg1) {}
        explicit inline Imop(Type type, Symbol *dest, Symbol *arg1,
                             Symbol *arg2)
            : m_type(type), m_dest(dest), m_arg1(arg1), m_arg2(arg2) {}
        ~Imop();

        inline const std::set<Imop*> &incoming() const { return m_incoming; }
        inline const std::set<Imop*> &incomingCalls() const { return m_incomingCalls; }
        inline const std::set<Imop*> &returns() const { return m_returns; }

        inline Type type() const { return m_type; }
        inline const Symbol *dest() const { return m_dest; }
        inline void setDest(const Symbol *dest) { m_dest = dest; }

        inline const Imop *jumpDest() const { return (Imop*) m_dest; }
        inline void setJumpDest(Imop *dest) {
            assert(dest != 0);
            assert((m_type & JUMP_MASK) != 0x0);
            m_dest = (SecreC::Symbol*) dest;
            dest->addIncoming(this);
        }

        const Imop *callDest() const;
        void setCallDest(SymbolProcedure *proc, Imop *clean);
        inline void setReturnDestFirstImop(Imop *firstImop) {
            assert(firstImop != 0);
            assert(firstImop->m_type == COMMENT);
            m_arg2 = (SecreC::Symbol*) firstImop;
            firstImop->addReturn(this);
        }

        inline const Symbol *arg1() const { return m_arg1; }
        inline void setArg1(const Symbol *arg1) { m_arg1 = arg1; }
        inline const Symbol *arg2() const { return m_arg2; }
        inline void setArg2(const Symbol *arg2) { m_arg2 = arg2; }

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

        const Type    m_type;
        const Symbol *m_dest;
        const Symbol *m_arg1;
        const Symbol *m_arg2;
        unsigned long m_index;
};

class ICode {
    public: /* Types: */
        class CodeList {
            public: /* Types: */
                typedef std::vector<Imop*> List;
                typedef List::const_iterator const_iterator;
                typedef List::iterator iterator;

            public: /* Methods: */
                ~CodeList();

                void resetIndexes() const;

                inline const_iterator begin() const { return m_list.begin(); }
                inline iterator begin() { return m_list.begin(); }
                inline const_iterator end() const { return m_list.end(); }
                inline iterator end() { return m_list.end(); }
                inline void push_back(Imop *i) {
                    m_list.push_back(i);
                    i->setIndex(m_list.size());
                }
                inline Imop *push_comment(const std::string &comment) {
                    Imop *c = new Imop(Imop::COMMENT, 0, (Symbol*) new std::string(comment));
                    push_back(c);
                    return c;
                }

            private: /* Fields: */
                List m_list;
        };
        enum Status { OK, E_NOT_IMPLEMENTED, E_EMPTY_PROGRAM, E_NO_MAIN,
                      E_TYPE, E_OTHER, E_NO_MEM };

    public: /* Methods: */
        explicit ICode(TreeNodeProgram *p);

        const CodeList &code() const { return m_code; }
        const SymbolTable &symbols() const { return m_symbols; }
        Status status() const { return m_status; }
        std::string messages() const { return m_errorStream.str(); }

    private: /* Fields: */
        SymbolTable        m_symbols;
        CodeList           m_code;
        Status             m_status;
        std::ostringstream m_errorStream;
};


} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Imop &i);
std::ostream &operator<<(std::ostream &out, const SecreC::ICode::Status &s);
std::ostream &operator<<(std::ostream &out, const SecreC::ICode::CodeList &c);
std::ostream &operator<<(std::ostream &out, const SecreC::ICode &icode);

#endif // INTERMEDIATE_H
