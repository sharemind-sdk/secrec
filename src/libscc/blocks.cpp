#include "blocks.h"


typedef SecreC::ICode::CodeList::const_iterator CCI;


namespace {

CCI endBlock(SecreC::Block &b, CCI end) {
    assert(b.start == b.end);

    CCI next = b.end;

    for (; b.end != end; b.end++) {
        next++;

        // If *b.end is a jump instruction, *next must be a leader:
        if (((*b.end)->type() & SecreC::Imop::JUMP_MASK) != 0) {
            return next;
        }

        // If *next is the target of some jump, *next must be a leader:
        if (!(*next)->incoming().empty()) {
            return next;
        }

    }
    return next;
}

} // anonymous namespace


namespace SecreC {

/*******************************************************************************
  Blocks
*******************************************************************************/

Blocks::Blocks(const ICode::CodeList &code) {
    /// \todo Check for empty code
    CCI next;

    m_startBlock = new Block(code.begin());
    next = endBlock(*m_startBlock, code.end());
    m_blockList.push_back(m_startBlock);

    while (next != code.end()) {
        Block *b = new Block(next);
        next = endBlock(*b, code.end());
        m_blockList.push_back(b);
    }
}

} // namespace SecreC
