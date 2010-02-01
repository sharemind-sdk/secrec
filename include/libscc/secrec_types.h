#ifndef SECREC_H
#define SECREC_H


#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

struct TNSymbols;
struct TreeNode;

enum SecrecVarType {
    VARTYPE_VOID   = 0x10,
    VARTYPE_BOOL   = 0x20,
    VARTYPE_INT    = 0x40,
    VARTYPE_UINT   = 0x80,
    VARTYPE_STRING = 0x100
};

enum SecrecSecType {
    SECTYPE_PUBLIC  = 0x01,
    SECTYPE_PRIVATE = 0x02
};

enum SecrecBasicType {
    TYPE_PUBLIC_VOID    = SECTYPE_PUBLIC | VARTYPE_VOID,
    TYPE_PUBLIC_BOOL    = SECTYPE_PUBLIC | VARTYPE_BOOL,
    TYPE_PUBLIC_INT     = SECTYPE_PUBLIC | VARTYPE_INT,
    TYPE_PUBLIC_UINT    = SECTYPE_PUBLIC | VARTYPE_UINT,
    TYPE_PUBLIC_STRING  = SECTYPE_PUBLIC | VARTYPE_STRING,
    TYPE_PRIVATE_VOID   = SECTYPE_PRIVATE | VARTYPE_VOID,
    TYPE_PRIVATE_BOOL   = SECTYPE_PRIVATE | VARTYPE_BOOL,
    TYPE_PRIVATE_INT    = SECTYPE_PRIVATE | VARTYPE_INT,
    TYPE_PRIVATE_UINT   = SECTYPE_PRIVATE | VARTYPE_UINT,
    TYPE_PRIVATE_STRING = SECTYPE_PRIVATE | VARTYPE_STRING,
    TYPE_MASK_SECTYPE   = SECTYPE_PUBLIC | SECTYPE_PRIVATE,
    TYPE_MASK_VARTYPE   = VARTYPE_VOID | VARTYPE_BOOL | VARTYPE_INT
                          | VARTYPE_UINT | VARTYPE_STRING
};

inline enum SecrecBasicType make_basicType(const enum SecrecSecType st,
                                           const enum SecrecVarType vt)
{
    return (enum SecrecBasicType) (st | vt);
}

inline enum SecrecSecType secType_from_basicType(const enum SecrecBasicType t) {
    return (enum SecrecSecType) (t & 0x0f);
}

inline enum SecrecVarType varType_from_basicType(const enum SecrecBasicType t) {
    return (enum SecrecVarType) (t & ~0x0f);
}

const char *basicType_name(const enum SecrecBasicType type);
const char *varType_name(const enum SecrecVarType type);
const char *secType_name(const enum SecrecSecType type);


enum SecrecTypeType {
    TYPETYPE_BASIC,
    TYPETYPE_ARRAY,
    TYPETYPE_FUNCTION /* function reference */
};

struct SecrecType {
    enum SecrecTypeType type;
    union {
        enum SecrecBasicType basicType;
        struct {
            struct SecrecType *arrayItemType;
            unsigned long long arraySize; /* 0 for unspecified size */
        } a;
        struct {
            struct SecrecType *returnType;
            struct SecrecType *firstParamType;
        } f;
    } d;
    struct SecrecType *next;
};

struct SecrecType *secrecType_init(enum SecrecTypeType type);
struct SecrecType *secrecType_initFromType(const struct TreeNode *tn);
struct SecrecType *secrecType_initFromDecl(const struct TreeNode *decl);
struct SecrecType *secrecType_initFromExpr(const struct TreeNode *decl,
                                           const struct TNSymbols *sym);
struct SecrecType *secrecType_clone(const struct SecrecType *copy);
void secrecType_free(struct SecrecType *type);


#ifdef __cplusplus
} // extern "C"
#endif /* #ifdef __cplusplus */

#endif // SECREC_H
