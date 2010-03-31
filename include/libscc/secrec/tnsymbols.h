#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <map>
#include <string>
#include <vector>

namespace SecreC {

class TreeNode;
class TreeNodeDecl;
class TreeNodeIdentifier;
class TreeNodeProgram;

class TNSymbols {
    private: /* types*/
        typedef std::map<std::string,const TreeNodeDecl*> DECLS;
        typedef std::map<const TreeNodeIdentifier*,const TreeNode*> TNMAP;

    public: /* methods */
        explicit TNSymbols(const TreeNodeProgram *program);

        const TreeNode *symbol(const TreeNodeIdentifier *id) const;
        inline const std::vector<const TreeNodeIdentifier*> &unresolved() const {
            return m_unresolved;
        }

    private: /* methods */
        void initSymbolMap(const TreeNodeProgram *program);
        void initSymbolMap(const DECLS &current, const TreeNode *node);
        void addRef(const TreeNodeIdentifier *n, const TreeNode *decl);
        static void addDecl(DECLS &ds, const TreeNodeDecl *decl);

    private: /* fields */
        TNMAP                        m_symbolMap;
        std::vector<const TreeNodeIdentifier*> m_unresolved;
};

} // namespace SecreC

#endif /* #ifdef SYMBOLS_H */
