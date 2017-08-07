/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "../Intermediate.h"
#include "../Symbol.h"

#include <map>
#include <set>


namespace SecreC {

bool removeEmptyBlocksProc (SymbolTable& symbols, Procedure& proc) {
    Procedure::iterator it = proc.begin ();
    std::set<Block*> todo;
    std::map<Block*, Block*> blockMap;

    while (it != proc.end ()) {
        Block* block = &*it;
        ++it;

        if (block->empty ()) {
            auto successors = block->successors ();
            assert (successors.size () == 1);
            Block* nextBlock = successors.begin ()->first;
            blockMap.insert (std::make_pair (block, nextBlock));

            for (auto pair : blockMap) {
                if (pair.second == block) {
                    blockMap[pair.first] = nextBlock;
                }
            }

            todo.insert (block);
        }
    }

    // Fix jumps and CFG edges
    for (auto& block : proc) {
        for (auto& imop : block) {
            if (todo.count (&block) > 0)
                continue;

            if (! imop.isJump ())
                continue;

            SymbolLabel* l = imop.jumpDest ();
            assert (l->block () != nullptr);

            if (blockMap.count (l->block ()) > 0) {
                Imop* destImop = &(blockMap[l->block ()]->front ());
                SymbolLabel* newL = symbols.label (destImop);
                newL->setBlock (destImop->block ());
                imop.setDest (newL);

                #ifndef NDEBUG
                bool found = false;
                #endif
                for (auto succ : block.successors ()) {
                    if (succ.first == l->block ()) {
                        block.removeSucc (*l->block ());
                        Block::addEdge (block, succ.second, *destImop->block ());
                        #ifndef NDEBUG
                        found = true;
                        #endif
                        break;
                    }
                }

                assert (found);
            }
        }
    }

    for (auto block : todo) {
        block->unlink ();
        delete block;
    }

    return todo.size () > 0;
}

bool removeEmptyBlocks (ICode& code) {
    Program& program = code.program ();

    bool changes = false;
    for (auto& proc : program) {
        changes |= removeEmptyBlocksProc (code.symbols (), proc);
    }

    return changes;
}

} // namespace SecreC
