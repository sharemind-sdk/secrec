#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include <list>
#include <sstream>
#include "secrec/symboltable.h"
#include "secrec/types.h"


namespace SecreC {

class TreeNodeProgram;

class Imop {
    public:
        enum Type {
            DECLARE,       /* arg1 d;           (TreeNode *arg2) */
            SYMBOL,        /*   d = arg1;  (SSA; TreeNode *arg2) */

            ASSIGN,        /*   d = arg1;                        */
            CAST,          /*   d = (arg1) arg2;                 */
            PUTPARAM,      /* PUTPARAM arg1;                     */
            FUNCALL,       /*   d = arg1(PARAMS);                */
            WILDCARD,      /*   d = arg1[*];                     */
            SUBSCRIPT,     /*   d = arg1[arg2];                  */
            UNEG,          /*   d = !arg1;                       */
            UMINUS,        /*   d = -arg1;                       */
            MATRIXMUL,     /*   d = arg1 #  arg2;                */
            MUL,           /*   d = arg1 *  arg2;                */
            DIV,           /*   d = arg1 /  arg2;                */
            MOD,           /*   d = arg1 %  arg2;                */
            ADD,           /*   d = arg1 +  arg2;                */
            SUB,           /*   d = arg1 -  arg2;                */
            EQ,            /*   d = arg1 == arg2;                */
            NE,            /*   d = arg1 != arg2;                */
            LE,            /*   d = arg1 <= arg2;                */
            LT,            /*   d = arg1 <  arg2;                */
            GE,            /*   d = arg1 >= arg2;                */
            GT,            /*   d = arg1 >  arg2;                */
            LAND,          /*   d = arg1 && arg2;                */
            LOR,           /*   d = arg1 || arg2;                */
            JT,            /* if (arg1) GOTO d;                  */
            JF,            /* if (!arg1) GOTO d;                 */
            JE,            /* if (arg1 == arg2) GOTO d;          */
            JNE,           /* if (arg1 != arg2) GOTO d;          */
            JLE,           /* if (arg1 <= arg2) GOTO d;          */
            JLT,           /* if (arg1 <  arg2) GOTO d;          */
            JGE,           /* if (arg1 >= arg2) GOTO d;          */
            JGT,           /* if (arg1 >  arg2) GOTO d;          */
            JUMP,          /* GOTO d;                            */
            LABEL,         /* label:                       (NOP) */
            RETURN,        /* RETURN;                            */
                           /* RETURN arg1;                       */
            END,           /* END PROGRAM                        */
            COMMENT        /* // arg1                            */
        };

    public:
        explicit inline Imop(Type type)
            : m_type(type) {}
        explicit inline Imop(Type type, Symbol *dest)
            : m_type(type), m_dest(dest) {}
        explicit inline Imop(Type type, Symbol *dest, Symbol *arg1)
            : m_type(type), m_dest(dest), m_arg1(arg1) {}
        explicit inline Imop(Type type, Symbol *dest, Symbol *arg1,
                             Symbol *arg2)
            : m_type(type), m_dest(dest), m_arg1(arg1), m_arg2(arg2) {}

        inline Type    type() const { return m_type; }
        inline const Symbol *dest() const { return m_dest; }
        inline void setDest(const Symbol *dest) { m_dest = dest; }
        inline const Symbol *arg1() const { return m_arg1; }
        inline void setArg1(const Symbol *arg1) { m_arg1 = arg1; }
        inline const Symbol *arg2() const { return m_arg2; }
        inline void setArg2(const Symbol *arg2) { m_arg2 = arg2; }

        std::string toString() const;

    private:
        const Type    m_type;
        const Symbol *m_dest;
        const Symbol *m_arg1;
        const Symbol *m_arg2;
};

class ICode {
    public: /* Types: */
        class CodeList {
            public: /* Types: */
                typedef std::list<Imop*> List;
                typedef List::const_iterator const_iterator;
                typedef List::iterator iterator;

            public: /* Methods: */
                inline const_iterator begin() const { return m_list.begin(); }
                inline iterator begin() { return m_list.begin(); }
                inline const_iterator end() const { return m_list.end(); }
                inline iterator end() { return m_list.end(); }
                inline void push_back(Imop *i) {
                    m_list.push_back(i);
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
