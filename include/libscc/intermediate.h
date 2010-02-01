#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include <assert.h>

#include "secrec_types.h"


#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

struct TreeNode;
struct TNSymbols;


/*******************************************************************************
  struct Imop and related functions
*******************************************************************************/

enum ImopType {
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

struct Imop {
    enum ImopType type;
    void *dest;
    void *arg1;
    void *arg2;
    struct Imop *next;
};

struct Imop *imop_init(enum ImopType type);


/*******************************************************************************
  struct ImopList and related functions
*******************************************************************************/

/**
  A singly-linked list of items of type struct Imop with caching the tail.
*/
struct ImopList {
    struct Imop *head;
    struct Imop *tail;
};

struct ImopList *imop_list_init();

static inline int imop_list_empty(struct ImopList *list) {
    return list->head == 0;
}

#define imop_list_for_each(imop,imopList) \
    for (imop = list->head; imop; imop = imop->next)

static inline void imop_list_prepend(struct ImopList *list, struct Imop *op) {
    assert(list);
    assert(op);
    assert(!op->next);

    op->next = list->head;
    list->head = op;
    if (!list->tail) list->tail = op;
}

static inline void imop_list_append(struct ImopList *list, struct Imop *op) {
    assert(list);
    assert(op);
    assert(!op->next);

    if (list->tail) {
        assert(list->head);
        list->tail->next = op;
    } else {
        assert(!list->head);
        list->head = op;
    }

    list->tail = op;
}

static inline void imop_list_merge(struct ImopList *l, struct ImopList *l2) {
    assert(l);
    assert(l2);

    if (l2->head) {
        if (l->head) {
            assert(l->tail);
            l->tail->next = l2->head;
        } else {
            assert(!l->tail);
            l->head = l2->head;
        }
        l->tail = l2->tail;
    }
}

struct ImopList *imop_generate(const struct TreeNode *program);
struct ImopList *imop_generate_with_symbols(const struct TreeNode *node_p,
                                            const struct TNSymbols *symbols);

#ifdef __cplusplus
} /* extern "C" */
#endif /* #ifdef __cplusplus */

#endif // INTERMEDIATE_H
