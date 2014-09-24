#include "Optimizer.h"
#include "Intermediate.h"

namespace SecreC {

bool removeUnreachableBlocks (ICode& code) {
    Program& program = code.program ();
    std::vector<std::unique_ptr<const Block>> unreachableBlocks;
    FOREACH_BLOCK (bi, program) {
        const Block& block = *bi;
        if (! block.reachable()) {
            unreachableBlocks.emplace_back (&block);
        }
    }

    return unreachableBlocks.size () > 0;
}

} // namespace SecreCC
