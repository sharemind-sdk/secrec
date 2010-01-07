#ifndef SYMBOLS_H
#define SYMBOLS_H

#ifdef __cplusplus
#include <map>
#include <string>
#include <vector>
#endif


#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/* Symbols for tree nodes: */
struct TNSymbols;
struct TreeNode;

struct TNSymbols *tnsymbols_init(const struct TreeNode *program);
void tnsymbols_free(const struct TNSymbols *s);
const struct TreeNode *tnsymbols_symbol(const struct TNSymbols *symbols,
                                        const struct TreeNode *id);
unsigned tnsymbols_unresolvedCount(const struct TNSymbols *symbols);
const struct TreeNode *tnsymbols_unresolvedAt(const struct TNSymbols *symbols,
                                              unsigned index);


#ifdef __cplusplus
} /* extern "C" */

class TreeNode;

class TNSymbols {
    private: /* types*/
        typedef std::map<std::string,const TreeNode*> DECLS;
        typedef std::map<const TreeNode*,const TreeNode*> TNMAP;

    public: /* methods */
        explicit TNSymbols(const TreeNode *program);

        const TreeNode *symbol(const TreeNode *id) const;
        inline const std::vector<const TreeNode*> &unresolved() const {
            return m_unresolved;
        }

    private: /* methods */
        void initSymbolMap(const TreeNode *program);
        void initSymbolMap(const DECLS &current, const TreeNode *node);
        void addRef(const TreeNode *n, const TreeNode *decl);
        static void addDecl(DECLS &ds, const TreeNode *decl);

    private: /* fields */
        TNMAP                        m_symbolMap;
        std::vector<const TreeNode*> m_unresolved;
};
#endif /* #ifdef __cplusplus */

#endif /* #ifdef SYMBOLS_H */
