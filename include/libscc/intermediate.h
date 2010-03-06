#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include "sccpointer.h"


class Symbol: public SccObject {
    //asdf
};

class Imop {
    public:
        enum Type {
            IMOP_DECLARE,       /* arg1 d;           (TreeNode *arg2) */
            IMOP_SYMBOL,        /*   d = arg1;  (SSA; TreeNode *arg2) */

            IMOP_ASSIGN,        /*   d = arg1;                        */
            IMOP_CAST,          /*   d = (arg1) arg2;                 */
            IMOP_PUTPARAM,      /* PUTPARAM arg1;                     */
            IMOP_FUNCALL,       /*   d = arg1(PARAMS);                */
            IMOP_WILDCARD,      /*   d = arg1[*];                     */
            IMOP_SUBSCRIPT,     /*   d = arg1[arg2];                  */
            IMOP_UNEG,          /*   d = !arg1;                       */
            IMOP_UMINUS,        /*   d = -arg1;                       */
            IMOP_MATRIXMUL,     /*   d = arg1 #  arg2;                */
            IMOP_MUL,           /*   d = arg1 *  arg2;                */
            IMOP_DIV,           /*   d = arg1 /  arg2;                */
            IMOP_MOD,           /*   d = arg1 %  arg2;                */
            IMOP_ADD,           /*   d = arg1 +  arg2;                */
            IMOP_SUB,           /*   d = arg1 -  arg2;                */
            IMOP_EQ,            /*   d = arg1 == arg2;                */
            IMOP_NE,            /*   d = arg1 != arg2;                */
            IMOP_LE,            /*   d = arg1 <= arg2;                */
            IMOP_LT,            /*   d = arg1 <  arg2;                */
            IMOP_GE,            /*   d = arg1 >= arg2;                */
            IMOP_GT,            /*   d = arg1 >  arg2;                */
            IMOP_LAND,          /*   d = arg1 && arg2;                */
            IMOP_LOR,           /*   d = arg1 || arg2;                */
            IMOP_JT,            /* if (arg1) GOTO d;                  */
            IMOP_JF,            /* if (!arg1) GOTO d;                 */
            IMOP_JUMP,          /* GOTO d;                            */
            IMOP_LABEL,         /* label:                       (NOP) */
            IMOP_RETURN,        /* RETURN;                            */
                                /* RETURN arg1;                       */
            IMOP_END            /* END PROGRAM                        */
        };

    public:
        explicit inline Imop(Type type)
            : m_type(type) {}

        explicit inline Imop(Type type, Symbol *dest, Symbol *arg1, Symbol *arg2)
            : m_type(type), m_dest(dest), m_arg1(arg1), m_arg2(arg2) {}

        inline Type    type() const { return m_type; }
        inline Symbol *dest() const { return m_dest; }
        inline Symbol *arg1() const { return m_arg1; }
        inline Symbol *arg2() const { return m_arg2; }

    private:
        Type               m_type;
        SccPointer<Symbol> m_dest;
        SccPointer<Symbol> m_arg1;
        SccPointer<Symbol> m_arg2;
};

#endif // INTERMEDIATE_H
