#include "intermediate.h"

#include <stdlib.h>
#include <string.h>
#include "tnsymbols.h"
#include "treenode.h"

struct ImopExprPair {
    struct ImopList *code;
    struct Imop     *value;
};

struct Imop *imop_init(enum ImopType type) {
    struct Imop *op = malloc(sizeof(struct Imop));
    op->type = type;
    op->dest = 0;
    op->arg1 = 0;
    op->arg2 = 0;
    op->next = 0;
    return op;
}

static struct ImopExprPair imop_genExpr(const struct TreeNode *expr,
                                        const struct TNSymbols *symbols);

static struct ImopList *imop_generate_vsuffix(const struct TreeNode *vsuffix,
                                              const struct TNSymbols *symbols)
{
    struct ImopList *l = imop_list_init();
    /** \todo */
    return l;
}

static struct ImopExprPair imop_genExprUnary(const struct TreeNode *expr,
                                             const struct TNSymbols *symbols,
                                             enum ImopType type)
{
    struct ImopExprPair r;
    struct Imop *op;

    assert(expr && symbols);
    assert(treenode_numChildren(expr) == 1);
    r = imop_genExpr(treenode_childAt(expr, 0), symbols);
    op = imop_init(type);
    op->arg1 = r.value;
    imop_list_append(r.code, op);
    r.value = op;
    return r;
}

static struct ImopExprPair imop_genExprBinary(const struct TreeNode *expr,
                                              const struct TNSymbols *symbols,
                                              enum ImopType type)
{
    struct ImopExprPair r, p;
    struct Imop *op;

    assert(expr && symbols);
    assert(treenode_numChildren(expr) == 2);

    r = imop_genExpr(treenode_childAt(expr, 0), symbols);
    p = imop_genExpr(treenode_childAt(expr, 1), symbols);
    imop_list_merge(r.code, p.code);
    free(p.code);
    op = imop_init(type);
    op->arg1 = r.value;
    op->arg2 = p.value;
    imop_list_append(r.code, op);
    r.value = op;
    return r;
}

static struct ImopExprPair imop_genExpr(const struct TreeNode *expr,
                                        const struct TNSymbols *symbols)
{
    assert(expr && symbols);

    struct ImopExprPair r;

    switch (treenode_type(expr)) {
        case NODE_EXPR_WILDCARD:
            return imop_genExprUnary(expr, symbols, IMOP_WILDCARD);
        case NODE_EXPR_SUBSCRIPT:
            return imop_genExprBinary(expr, symbols, IMOP_SUBSCRIPT);
        case NODE_EXPR_UNEG:
            return imop_genExprUnary(expr, symbols, IMOP_UNEG);
        case NODE_EXPR_UMINUS:
            return imop_genExprUnary(expr, symbols, IMOP_UMINUS);
        case NODE_EXPR_MATRIXMUL:
            return imop_genExprBinary(expr, symbols, IMOP_MATRIXMUL);
        case NODE_EXPR_ASSIGN:
            r = imop_genExprBinary(expr, symbols, IMOP_ASSIGN);
            r.value->dest = r.value->arg1;
            r.value->arg1 = r.value->arg2;
            r.value->arg2 = 0;
            break;
        case NODE_EXPR_MUL:
            return imop_genExprBinary(expr, symbols, IMOP_MUL);
        case NODE_EXPR_ASSIGN_MUL:
            r = imop_genExprBinary(expr, symbols, IMOP_MUL);
            r.value->dest = r.value->arg1;
            break;
        case NODE_EXPR_MOD:
            return imop_genExprBinary(expr, symbols, IMOP_MOD);
        case NODE_EXPR_ASSIGN_MOD:
            r = imop_genExprBinary(expr, symbols, IMOP_MOD);
            r.value->dest = r.value->arg1;
            break;
        case NODE_EXPR_DIV:
            return imop_genExprBinary(expr, symbols, IMOP_DIV);
        case NODE_EXPR_ASSIGN_DIV:
            r = imop_genExprBinary(expr, symbols, IMOP_DIV);
            r.value->dest = r.value->arg1;
            break;
        case NODE_EXPR_ADD:
            return imop_genExprBinary(expr, symbols, IMOP_ADD);
        case NODE_EXPR_ASSIGN_ADD:
            r = imop_genExprBinary(expr, symbols, IMOP_ADD);
            r.value->dest = r.value->arg1;
            break;
        case NODE_EXPR_SUB:
            return imop_genExprBinary(expr, symbols, IMOP_SUB);
        case NODE_EXPR_ASSIGN_SUB:
            r = imop_genExprBinary(expr, symbols, IMOP_SUB);
            r.value->dest = r.value->arg1;
            break;
        case NODE_EXPR_EQ:
            return imop_genExprBinary(expr, symbols, IMOP_EQ);
        case NODE_EXPR_NE:
            return imop_genExprBinary(expr, symbols, IMOP_NE);
        case NODE_EXPR_LE:
            return imop_genExprBinary(expr, symbols, IMOP_LE);
        case NODE_EXPR_LT:
            return imop_genExprBinary(expr, symbols, IMOP_LT);
        case NODE_EXPR_GE:
            return imop_genExprBinary(expr, symbols, IMOP_GE);
        case NODE_EXPR_GT:
            return imop_genExprBinary(expr, symbols, IMOP_GT);
        case NODE_EXPR_LAND:
            return imop_genExprBinary(expr, symbols, IMOP_LAND);
        case NODE_EXPR_LOR:
            return imop_genExprBinary(expr, symbols, IMOP_LOR);
        case NODE_IDENTIFIER:
            r.code = imop_list_init();
            r.value = imop_init(IMOP_SYMBOL);
            r.value->arg2 = (void*) tnsymbols_symbol(symbols, expr);
            imop_list_append(r.code, r.value);
            break;
        case NODE_EXPR_CAST:
            r = imop_genExpr(treenode_childAt(expr, 1), symbols);
            {
                struct Imop *op = imop_init(IMOP_CAST);
                op->arg1 = secrecType_initFromType(treenode_childAt(expr, 0));
                op->arg2 = r.value;
                imop_list_append(r.code, op);
                r.value = op;
            }
            break;
        case NODE_EXPR_FUNCALL:
            r.code = imop_list_init();
            {
                unsigned i;
                struct Imop *op;
                struct ImopExprPair p;
                for (i = 1; i < treenode_numChildren(expr); i++) {
                    p = imop_genExpr(treenode_childAt(expr, i), symbols);
                    imop_list_merge(r.code, p.code);
                    free(p.code);

                    op = imop_init(IMOP_PUTPARAM);
                    op->arg1 = p.value;
                    imop_list_append(r.code, op);
                }
                p = imop_genExpr(treenode_childAt(expr, 0), symbols);
                imop_list_merge(r.code, p.code);
                free(p.code);
                op = imop_init(IMOP_FUNCALL);
                op->arg1 = p.value;
                r.value = op;
            }
            break;
        case NODE_EXPR_TERNIF:
            /**
              \todo Implement.

              a ? b : c
              ------------------------------------------------------------------
                    a.code;
                    (typeof(b,c)) v;      IMOP_DECLARE
                    IF (!a) GOTO do_c;    IMOP_JF
                    v = b.code;
                    GOTO end;             IMOP_JUMP
              do_b: v = c.code;
              end:  NOP;                  IMOP_LABEL
            */
        default:
            assert(0);
            r.code = 0;
            r.value = 0;
            break;
    }
    return r;
}

static struct ImopList *imop_genDecl(const struct TreeNode *decl,
                                           const struct TNSymbols *symbols)
{
    struct ImopList *l = imop_list_init();

    struct Imop *op = imop_init(IMOP_DECLARE);
    op->dest = strdup(treenode_value_string(treenode_childAt(decl, 0)));
    op->arg1 = secrecType_initFromDecl(decl);
    op->arg2 = (void*) decl;
    imop_list_append(l, op);

    if (treenode_numChildren(decl) > 2) {
        decl = treenode_childAt(decl, 2);
        struct ImopList *other;
        if (treenode_type(decl) == NODE_DECL_VSUFFIX) {
            other = imop_generate_vsuffix(decl, symbols);
        } else {
            struct ImopExprPair pair = imop_genExpr(decl, symbols);
            other = pair.code;
            pair.value->dest = op;
        }
        imop_list_merge(l, other);
        free(other);
    }

    return l;
}

struct ImopList *imop_generate(const struct TreeNode *program) {
    assert(program);

    struct TNSymbols *symbols = tnsymbols_init(program);
    struct ImopList *r = imop_generate_with_symbols(program, symbols);
    tnsymbols_free(symbols);
    return r;
}

struct ImopList *imop_generate_with_symbols(const struct TreeNode *node_p,
                                            const struct TNSymbols *symbols)
{
    /* Abort if we have unresolved symbols */
    assert(symbols);
    if (tnsymbols_unresolvedCount(symbols)) return 0;

    assert(node_p);
    assert(treenode_type(node_p) == NODE_PROGRAM);

    struct ImopList *program = imop_list_init();

    assert(treenode_numChildren(node_p) == 1
           || treenode_numChildren(node_p) == 2);

    if (treenode_numChildren(node_p) == 2) {
        /* Parse global declarations */
        const struct TreeNode *node_globals = treenode_childAt(node_p, 0);
        const struct TreeNode *child;
        unsigned i;
        treenode_for_each_child(node_globals, child, i) {
            struct ImopList *list = imop_genDecl(child, symbols);
            imop_list_merge(program, list);
            free(list);
        }

        node_p = treenode_childAt(node_p, 1);
    } else {
        node_p = treenode_childAt(node_p, 0);
    }

    /* Parse function definitions */
    /** \todo */

    return program;
}

struct ImopList *imop_list_init() {
    struct ImopList *list = malloc(sizeof(struct ImopList));
    list->head = list->tail = 0;
    return list;
}
