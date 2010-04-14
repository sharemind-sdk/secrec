#ifndef BLOCKS_H
#define BLOCKS_H

#include <set>
#include <vector>
#include "intermediate.h"


namespace SecreC {

/*******************************************************************************
  Block
*******************************************************************************/

struct Block {
    inline Block(ICode::CodeList::const_iterator codestart)
        : start(codestart), end(codestart) {}

    ICode::CodeList::const_iterator start;
    ICode::CodeList::const_iterator end;
    std::set<Block*> predecessors;
    std::set<Block*> successors;
};


/*******************************************************************************
  Blocks
*******************************************************************************/

class Blocks {
    public:
        Blocks(const ICode::CodeList &code);

    std::vector<Block*> m_blockList;
    Block *m_startBlock;
};

} // namespace SecreC

#endif // BLOCKS_H
