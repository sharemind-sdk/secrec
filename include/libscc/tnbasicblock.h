#ifndef TNBASICBLOCK_H
#define TNBASICBLOCK_H

#include <deque>

class TreeNode;

class TNBasicBlock {
    public:
        TNBasicBlock();
        ~TNBasicBlock();

        inline std::deque<TNBasicBlock*> &entryPoints() { return m_entryPoints; }
        inline std::deque<TreeNode*> &statements() { return m_statements; }
        inline TreeNode *&conditional() { return m_conditional; }
        inline TNBasicBlock *&target1() { return m_target1; }
        inline TNBasicBlock *&target2() { return m_target2; }

    private:
        std::deque<TNBasicBlock*> m_entryPoints;

        std::deque<TreeNode*> m_statements;

        TreeNode *m_conditional;
        TNBasicBlock *m_target1;
        TNBasicBlock *m_target2;
};

#endif // TNBASICBLOCK_H
