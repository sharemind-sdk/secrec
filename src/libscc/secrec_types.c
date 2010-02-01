#include "secrec_types.h"

#include <assert.h>
#include <stdlib.h>
#include "tnsymbols.h"
#include "treenode.h"


const char *basicType_name(const enum SecrecBasicType type) {
    switch (type) {
        case TYPE_PUBLIC_VOID:    return "public void";
        case TYPE_PUBLIC_BOOL:    return "public bool";
        case TYPE_PUBLIC_INT:     return "public int";
        case TYPE_PUBLIC_UINT:    return "public unsigned int";
        case TYPE_PUBLIC_STRING:  return "public string";
        case TYPE_PRIVATE_VOID:   return "private void";
        case TYPE_PRIVATE_BOOL:   return "private bool";
        case TYPE_PRIVATE_INT:    return "private int";
        case TYPE_PRIVATE_UINT:   return "private unsigned int";
        case TYPE_PRIVATE_STRING: return "private string";
        default:                  return "<unknown>";
    }
}

const char *varType_name(const enum SecrecVarType type) {
    switch (type) {
        case VARTYPE_VOID:   return "void";
        case VARTYPE_BOOL:   return "bool";
        case VARTYPE_INT:    return "int";
        case VARTYPE_UINT:   return "unsigned int";
        case VARTYPE_STRING: return "string";
        default:             return "<unknown>";
    }
}

const char *secType_name(const enum SecrecSecType type) {
    switch (type) {
        case SECTYPE_PUBLIC:  return "public";
        case SECTYPE_PRIVATE: return "private";
        default:              return "<unknown>";
    }
}

struct SecrecType *secrecType_init(enum SecrecTypeType type) {
    struct SecrecType *t = malloc(sizeof(struct SecrecType *));
    if (!t) return 0;

    t->type = type;
    if (type == TYPETYPE_BASIC) {
        t->d.basicType = 0;
    } else if (type == TYPETYPE_ARRAY) {
        t->d.a.arrayItemType = 0;
        t->d.a.arraySize = 0;
    } else {
        assert(type == TYPETYPE_FUNCTION);
        t->d.f.returnType = 0;
        t->d.f.firstParamType = 0;
    }
    t->next = 0;
    return t;
}

struct SecrecType *secrecType_initFromType(const struct TreeNode *tn) {
    enum NodeType type = treenode_type(tn);
    struct SecrecType *t = 0;
    if (type == NODE_BASICTYPE) {
        t = secrecType_init(TYPETYPE_BASIC);
        if (!t) return 0;
        t->d.basicType = treenode_value_basicType(tn);
    } else {
        assert(type == NODE_ARRAYTYPE);
        t = secrecType_init(TYPETYPE_ARRAY);
        if (!t) return 0;
        t->d.a.arraySize = treenode_value_uint(tn);
        t->d.a.arrayItemType = secrecType_initFromType(treenode_childAt(tn, 0));
        if (!t->d.a.arrayItemType) {
            secrecType_free(t);
            return 0;
        }
    }
    return t;
}

struct SecrecType *secrecType_initFromDecl(const struct TreeNode *decl) {
    enum NodeType type = treenode_type(decl);
    struct SecrecType *t;

    if (type == NODE_FUNDEF) {
        t = secrecType_init(TYPETYPE_FUNCTION);
        if (t) {
            t->d.f.returnType = secrecType_initFromType(treenode_childAt(decl, 1));
            if (!t->d.f.returnType)
                goto secrecType_initFromDecl_nomem;

            if (treenode_numChildren(decl) > 3) {
                struct SecrecType *a = t;
                unsigned i = 3;
                do {
                    struct TreeNode *p = treenode_childAt(decl, i);
                    a->next = secrecType_initFromType(treenode_childAt(p, 1));
                    if (!a->next)
                        goto secrecType_initFromDecl_nomem;

                    a = a->next;
                    i++;
                } while (i < treenode_numChildren(decl));
            }
        }
    } else {
        assert(type == NODE_DECL);
        t = secrecType_initFromType(treenode_childAt(decl, 1));
    }
    return t;

secrecType_initFromDecl_nomem:
    secrecType_free(t);
    return 0;
}

struct SecrecType *secrecType_initFromExpr(const struct TreeNode *expr,
                                           const struct TNSymbols *symbols)
{
    struct SecrecType *r = 0, *t1, *t2;

    switch (treenode_type(expr)) {
        case NODE_LITE_BOOL:
            r = secrecType_init(TYPETYPE_BASIC);
            r->d.basicType = TYPE_PUBLIC_BOOL;
            break;
        case NODE_LITE_INT:
            r = secrecType_init(TYPETYPE_BASIC);
            r->d.basicType = TYPE_PUBLIC_INT;
            break;
        case NODE_LITE_UINT:
            r = secrecType_init(TYPETYPE_BASIC);
            r->d.basicType = TYPE_PUBLIC_UINT;
            break;
        case NODE_LITE_STRING:
            r = secrecType_init(TYPETYPE_BASIC);
            r->d.basicType = TYPE_PUBLIC_STRING;
            break;
        case NODE_IDENTIFIER:
            r = secrecType_initFromDecl(tnsymbols_symbol(symbols, expr));
            break;
        case NODE_EXPR_EQ:
        case NODE_EXPR_NE:
        case NODE_EXPR_LE:
        case NODE_EXPR_LT:
        case NODE_EXPR_GE:
        case NODE_EXPR_GT:
        case NODE_EXPR_LAND:
        case NODE_EXPR_LOR:
            t1 = secrecType_initFromExpr(treenode_childAt(expr, 0), symbols);
            if (t1 == 0) break;
            t2 = secrecType_initFromExpr(treenode_childAt(expr, 1), symbols);
            if (t2 == 0) {
                secrecType_free(t1);
                break;
            }

            r = secrecType_init(TYPETYPE_BASIC);
            if ((t1->d.basicType | t2->d.basicType) & SECTYPE_PRIVATE) {
                r->d.basicType = TYPE_PRIVATE_BOOL;
            } else {
                r->d.basicType = TYPE_PUBLIC_BOOL;
            }
            secrecType_free(t1);
            secrecType_free(t2);
            break;
        case NODE_EXPR_MUL:
        case NODE_EXPR_DIV:
        case NODE_EXPR_MOD:
        case NODE_EXPR_ADD:
        case NODE_EXPR_SUB:
            t1 = secrecType_initFromExpr(treenode_childAt(expr, 0), symbols);
            if (t1 == 0) break;
            t2 = secrecType_initFromExpr(treenode_childAt(expr, 1), symbols);
            if (t2 == 0) {
                secrecType_free(t1);
                break;
            }
            if (t1->type == TYPETYPE_BASIC && t2->type == TYPETYPE_BASIC
                && ((t1->d.basicType | t2->d.basicType) & (VARTYPE_INT | VARTYPE_UINT)))
            {
                /** \todo check */
                r = secrecType_init(TYPETYPE_BASIC);
                if ((t1->d.basicType | t2->d.basicType) & VARTYPE_UINT) {
                    r->d.basicType = VARTYPE_UINT;
                } else {
                    r->d.basicType = VARTYPE_INT;
                }
                if ((t1->d.basicType | t2->d.basicType) & SECTYPE_PRIVATE) {
                    r->d.basicType |= SECTYPE_PRIVATE;
                } else {
                    r->d.basicType |= SECTYPE_PUBLIC;
                }
            }
            secrecType_free(t1);
            secrecType_free(t2);
            break;
        case NODE_EXPR_ASSIGN:
        case NODE_EXPR_ASSIGN_MUL:
        case NODE_EXPR_ASSIGN_DIV:
        case NODE_EXPR_ASSIGN_MOD:
        case NODE_EXPR_ASSIGN_ADD:
        case NODE_EXPR_ASSIGN_SUB:
            break;
        default:
            break;
    }
    return r;
}

struct SecrecType *secrecType_clone(const struct SecrecType *copy) {
    struct SecrecType *t = malloc(sizeof(struct SecrecType *));
    if (!t) return 0;

    t->type = copy->type;
    if (t->type == TYPETYPE_BASIC) {
        t->d.basicType = copy->d.basicType;
    } else if (t->type == TYPETYPE_ARRAY) {
        t->d.a.arrayItemType = copy->d.a.arrayItemType;
        if (t->d.a.arrayItemType) {
            t->d.a.arrayItemType = secrecType_clone(t->d.a.arrayItemType);
            if (!t->d.a.arrayItemType)
                goto secrecType_clone_nomem;
        }
        t->d.a.arraySize = copy->d.a.arraySize;
    } else {
        assert(t->type == TYPETYPE_FUNCTION);
        t->d.f.returnType = copy->d.f.returnType;
        if (t->d.f.returnType) {
            t->d.f.returnType = secrecType_clone(t->d.f.returnType);
            if (!t->d.f.returnType)
                goto secrecType_clone_nomem;
        }
        t->d.f.firstParamType = copy->d.f.firstParamType;
        if (t->d.f.firstParamType) {
            t->d.f.firstParamType = secrecType_clone(t->d.f.firstParamType);
            if (!t->d.f.firstParamType) {
                goto secrecType_clone_nomem;
            }
        }
    }
    return t;

secrecType_clone_nomem:
    secrecType_free(t);
    return 0;
}

void secrecType_free(struct SecrecType *t) {
    if (t->type == TYPETYPE_ARRAY) {
        if (t->d.a.arrayItemType != 0) {
            secrecType_free(t->d.a.arrayItemType);
        }
    } else if (t->type == TYPETYPE_FUNCTION) {
        if (t->d.f.returnType != 0) {
            secrecType_free(t->d.f.returnType);
        }
        if (t->d.f.firstParamType != 0) {
            secrecType_free(t->d.f.firstParamType);
        }
    }
    if (t->next != 0) secrecType_free(t->next);
    free(t);
}
