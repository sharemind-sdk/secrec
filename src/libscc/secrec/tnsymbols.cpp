#include "secrec/tnsymbols.h"

#include <cassert>
#include <stack>
#include "secrec/treenode.h"


namespace SecreC {

TNSymbols::TNSymbols(const TreeNodeProgram *program) {
    assert(program != 0);
    initSymbolMap(program);
}

const TreeNode *TNSymbols::symbol(const TreeNodeIdentifier *id) const {
    TNMAP::const_iterator it = m_symbolMap.find(id);
    return (it != m_symbolMap.end() ? (*it).second : 0);
}

void TNSymbols::initSymbolMap(const TreeNodeProgram *p) {
    typedef std::deque<SccPointer<TreeNode> >::const_iterator TDCI;

    DECLS globalScope;

    assert(p->type() == NODE_PROGRAM);
    assert(m_symbolMap.size() == 0);
    const TreeNodeFundefs *fs;

    assert(p->children().size() == 1 || p->children().size() == 2);

    // Check for global declarations:
    if (p->children().size() > 1) {
        assert(dynamic_cast<const TreeNodeGlobals*>(p->children().at(0).data()) != 0);
        const TreeNodeGlobals *gs = static_cast<const TreeNodeGlobals*>(p->children().at(0).data());

        // Handle global declarations:
        const std::deque<SccPointer<TreeNode> > &gds = gs->children();
        for (TDCI it = gds.begin(); it != gds.end(); it++) {
            assert(dynamic_cast<const TreeNodeDecl*>((*it).data()) != 0);
            const TreeNodeDecl *decl = static_cast<const TreeNodeDecl*>((*it).data());
            assert(decl->children().size() > 1 && decl->children().size() < 4);

            // First link the identifiers in the initializer/vector suffix:
            if (decl->children().size() == 3) {
                /*
                  1    0            2
                  type identifier = initializer;
                  type identifier   [vector][suffix];
                */
                initSymbolMap(globalScope, decl->children().at(2));
            }

            // Secondly add the global declaration:
            addDecl(globalScope, decl);
        }

        assert(dynamic_cast<const TreeNodeFundefs*>(p->children().at(1).data()) != 0);
        fs = static_cast<const TreeNodeFundefs*>(p->children().at(1).data());
    } else {
        assert(dynamic_cast<const TreeNodeFundefs*>(p->children().at(0).data()) != 0);
        fs = static_cast<const TreeNodeFundefs*>(p->children().at(0).data());
    }

    // Handle global functions
    for (TDCI it = fs->children().begin(); it != fs->children().end(); it++) {
        // First we register the names of all functions:
        /// \todo Warn, if a globals with identical names already exist
        assert(dynamic_cast<const TreeNodeDecl*>((*it).data()) != 0);
        addDecl(globalScope, static_cast<const TreeNodeDecl*>((*it).data()));
    }
    for (TDCI it = fs->children().begin(); it != fs->children().end(); it++) {
        // Handle symbols of each function individually:
        assert(dynamic_cast<const TreeNodeFundef*>((*it).data()));
        const TreeNodeFundef *f = static_cast<const TreeNodeFundef*>((*it).data());

        assert(f->children().size() >= 3);
        if (f->children().size() == 3) {
            // Function has no parameters
            initSymbolMap(globalScope, f->children().at(2));
        } else {
            // Function has parameters
            DECLS functionScope(globalScope);
            const std::deque<SccPointer<TreeNode> > &fc = f->children();
            for (TDCI it = fc.begin() + 3; it != fc.end(); it++) {
                assert(dynamic_cast<TreeNodeDecl*>((*it).data()) != 0);
                addDecl(functionScope, static_cast<TreeNodeDecl*>((*it).data()));
            }
            initSymbolMap(functionScope, f->children().at(2));
        }
    }
}

void TNSymbols::initSymbolMap(const DECLS &current, const TreeNode *node) {
    typedef std::deque<SccPointer<TreeNode> >::const_reverse_iterator TDCRI;

    DECLS localScope(current);

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
                initSymbolMap(localScope, n);
                break;
            case NODE_DECL:
                assert(n->children().size() > 1 && n->children().size() < 4);

                /*
                  Since we're in a while-loop of nodes in the program, this
                  declaration only has effect on the following nodes.
                */

                // First link the identifiers in the initializer/vector suffix:
                if (n->children().size() == 3) {
                    initSymbolMap(localScope, n->children().at(2));
                }

                // Secondly add the declaration to the local scope:
                assert(dynamic_cast<const TreeNodeDecl*>(n) != 0);
                addDecl(localScope, static_cast<const TreeNodeDecl*>(n));
                break;
            case NODE_IDENTIFIER:
            {
                assert(dynamic_cast<const TreeNodeIdentifier*>(n) != 0);
                const TreeNodeIdentifier* in = static_cast<const TreeNodeIdentifier*>(n);
                DECLS::const_iterator it = localScope.find(in->value());
                addRef(in, (it != localScope.end() ? (*it).second : 0));
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

void TNSymbols::addDecl(DECLS &ds, const TreeNodeDecl *decl) {
    assert(dynamic_cast<const TreeNodeIdentifier*>(decl->children().at(0).data()) != 0);
    const TreeNodeIdentifier *id = static_cast<const TreeNodeIdentifier*>(decl->children().at(0).data());
    ds[id->value()] = decl;
}

} // namespace SecreC
