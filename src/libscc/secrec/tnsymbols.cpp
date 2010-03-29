#include "secrec/tnsymbols.h"

#include <cassert>
#include <stack>
#include "secrec/treenode.h"


namespace SecreC {

TNSymbols::TNSymbols(const TreeNode *program) {
    assert(program != 0);
    initSymbolMap(program);
}

const TreeNode *TNSymbols::symbol(const TreeNodeIdentifier *id) const {
    TNMAP::const_iterator it = m_symbolMap.find(id);
    return (it != m_symbolMap.end() ? (*it).second : 0);
}

void TNSymbols::initSymbolMap(const TreeNode *p) {
    typedef std::deque<SccPointer<TreeNode> >::const_iterator TDCI;

    DECLS ds;

    assert(p->type() == NODE_PROGRAM);
    assert(m_symbolMap.size() == 0);
    const TreeNode *fs;

    // Check for global declarations:
    assert(p->children().size() == 1 || p->children().size() == 2);
    if (p->children().size() > 1) {
        fs = p->children().at(0);

        // Handle global declarations:
        const std::deque<SccPointer<TreeNode> > &gds = fs->children();
        for (TDCI it = gds.begin(); it != gds.end(); it++) {
            const TreeNode *decl = (*it);
            assert(decl->children().size() > 1 && decl->children().size() < 4);

            if (decl->children().size() == 3) {
                /*
                  1    0            2
                  type identifier = initializer;
                  type identifier   [vector][suffix];
                */
                initSymbolMap(ds, decl->children().at(2));
            }

            addDecl(ds, decl);
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
            const std::deque<SccPointer<TreeNode> > &fc = f->children();
            for (TDCI it = fc.begin() + 3; it != fc.end(); it++) {
                addDecl(nds, *it);
            }
            initSymbolMap(nds, f->children().at(2));
        }
    }
}

void TNSymbols::initSymbolMap(const DECLS &current, const TreeNode *node) {
    typedef std::deque<SccPointer<TreeNode> >::const_reverse_iterator TDCRI;

    DECLS ds(current);

    std::stack<const TreeNode*> s; // Tree nodes appearing in order


    if (node->type() == NODE_STMT_COMPOUND) {
        /*
          If the only node inside the new scope is a NODE_STMT_COMPOUND, the
          semantics of the scope are identical to a scope having all the child
          nodes of the NODE_STMT_COMPOUND.
        */
        const std::deque<SccPointer<TreeNode> > &nc = node->children();
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
                assert(dynamic_cast<const TreeNodeIdentifier*>(n) != 0);
                const TreeNodeIdentifier* in = static_cast<const TreeNodeIdentifier*>(n);
                DECLS::const_iterator it = ds.find(in->value());
                addRef(in, (it != ds.end() ? (*it).second : 0));
                break;
            }
            default:
            {
                const std::deque<SccPointer<TreeNode> > &nc = n->children();
                for (TDCRI it = nc.rbegin(); it != nc.rend(); it++) {
                    s.push(*it);
                }
                break;
            }
        }
    }
}

void TNSymbols::addRef(const TreeNodeIdentifier *n, const TreeNode *decl) {
    assert(n != 0);
    if (decl == 0) {
        m_unresolved.push_back(n);
    }
    m_symbolMap[n] = decl;
}

void TNSymbols::addDecl(DECLS &ds, const TreeNode *decl) {
    assert(dynamic_cast<const TreeNodeIdentifier*>(decl->children().at(0).data()) != 0);
    const TreeNodeIdentifier *id = static_cast<const TreeNodeIdentifier*>(decl->children().at(0).data());
    ds[id->value()] = decl;
}

} // namespace SecreC
