#include "RemoveUnreachableBlocks.h"

#include <boost/foreach.hpp>
#include <libscc/Intermediate.h>

namespace SecreCC {

using namespace SecreC;

void removeUnreachableBlocks (ICode& code) {
    Program& program = code.program ();
    std::vector<const Block*> unreachableBlocks;
    FOREACH_BLOCK (bi, program) {
        const Block& block = *bi;
        if (! block.reachable()) {
            unreachableBlocks.push_back(&block);
        }
    }

    BOOST_FOREACH (const Block* block, unreachableBlocks) {
        delete block;
    }
}

} // namespace SecreCC
