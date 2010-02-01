#include "tnsymbols.h"

#include <cassert>
#include <stack>
#include "treenode.h"


extern "C" struct TNSymbols *tnsymbols_init(const struct TreeNode *program) {
    return new TNSymbols(program);
}

extern "C" void tnsymbols_free(const struct TNSymbols *symbols) {
    delete symbols;
}

extern "C" const struct TreeNode *tnsymbols_symbol(const struct TNSymbols *s,
                                                   const struct TreeNode *id)
{
    return s->symbol(id);
}

extern "C" unsigned tnsymbols_unresolvedCount(const struct TNSymbols *symbols) {
    return symbols->unresolved().size();
}

extern "C" const struct TreeNode *tnsymbols_unresolvedAt(
        const struct TNSymbols *symbols,
        unsigned index)
{
    return symbols->unresolved().at(index);
}

TNSymbols::TNSymbols(const TreeNode *program) {
    assert(program != 0);
    initSymbolMap(program);
}

const TreeNode *TNSymbols::symbol(const TreeNode *id) const {
    assert(id->type() == NODE_IDENTIFIER);
    TNMAP::const_iterator it = m_symbolMap.find(id);
    return (it != m_symbolMap.end() ? (*it).second : 0);
}

void TNSymbols::initSymbolMap(const TreeNode *p) {
    typedef std::deque<TreeNode*>::const_iterator TDCI;

    DECLS ds;

    assert(p->type() == NODE_PROGRAM);
    assert(m_symbolMap.size() == 0);
    const TreeNode *fs;

    // Check for global declarations:
    assert(p->children().size() == 1 || p->children().size() == 2);
    if (p->children().size() > 1) {
        fs = p->children().at(0);

        // Handle global declarations:
        const std::deque<TreeNode*> &gds = fs->children();
        for (TDCI it = gds.begin(); it != gds.end(); it++) {
            const TreeNode *decl = (*it);
            assert(decl->children().size() > 1 && decl->children().size() < 4);

            /**
              \note Currently we allow stuff like "public int a = a;"
            */
            addDecl(ds, decl);
            if (decl->children().size() == 3) {
                /*
                  1    0            2
                  type identifier = initializer;
                  type identifier   [vector][suffix];
                */
                initSymbolMap(ds, decl->children().at(2));
            }
        }

        fs = p->children().at(1);
    } else {
        fs = p->children().at(0);
    }

    // Handle global functions:
    for (TDCI it = fs->children().begin(); it != fs->children().end(); it++) {
        // Register names of each function:
        addDecl(ds, *it);
    }
    for (TDCI it = fs->children().begin(); it != fs->children().end(); it++) {
        // Handle symbols of each function individually:
        const TreeNode *f = *it;

        assert(f->children().size() >= 3);
        if (f->children().size() == 3) {
            // Function has no parameters
            initSymbolMap(ds, f->children().at(2));
        } else {
            // Function has parameters
            DECLS nds(ds);
            const std::deque<TreeNode*> &fc = f->children();
            for (TDCI it = fc.begin() + 3; it != fc.end(); it++) {
                addDecl(nds, *it);
            }
            initSymbolMap(nds, f->children().at(2));
        }
    }
}

void TNSymbols::initSymbolMap(const DECLS &current, const TreeNode *node) {
    typedef std::deque<TreeNode*>::const_reverse_iterator TDCRI;

    DECLS ds(current);

    std::stack<const TreeNode*> s; // Tree nodes appearing in order


    if (node->type() == NODE_STMT_COMPOUND) {
        /*
          If the only node inside the new scope is a NODE_STMT_COMPOUND, the
          semantics of the scope are identical to a scope having all the child
          nodes of the NODE_STMT_COMPOUND.
        */
        const std::deque<TreeNode*> &nc = node->children();
        for (TDCRI it = nc.rbegin(); it != nc.rend(); it++) {
            s.push(*it);
        }
    } else {
        s.push(node);
    }

    while (!s.empty()) {
        const TreeNode *n = s.top();
        s.pop();

        switch (n->type()) {
            case NODE_STMT_COMPOUND:
                initSymbolMap(ds, n);
                break;
            case NODE_DECL:
                assert(n->children().size() > 1 && n->children().size() < 4);

                /*
                  Since we're in a while-loop of nodes in the program, this
                  declaration only has effect on the following nodes.
                */

                /**
                  \note Currently we allow stuff like "public int a = a;"
                */
                addDecl(ds, n);
                if (n->children().size() == 3) {
                    s.push(n->children().at(2));
                }
                break;
            case NODE_IDENTIFIER:
            {
                DECLS::const_iterator it = ds.find(n->valueString());
                addRef(n, (it != ds.end() ? (*it).second : 0));
                break;
            }
            default:
            {
                const std::deque<TreeNode*> &nc = n->children();
                for (TDCRI it = nc.rbegin(); it != nc.rend(); it++) {
                    s.push(*it);
                }
                break;
            }
        }
    }
}

void TNSymbols::addRef(const TreeNode *n, const TreeNode *decl) {
    assert(n != 0);
    if (decl == 0) {
        m_unresolved.push_back(n);
    }
    m_symbolMap[n] = decl;
}

void TNSymbols::addDecl(DECLS &ds, const TreeNode *decl) {
    const TreeNode *id = decl->children().at(0);
    assert(id->type() == NODE_IDENTIFIER);
    ds[id->valueString()] = decl;
}
