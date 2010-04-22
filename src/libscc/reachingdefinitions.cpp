#include "reachingdefinitions.h"

#include "blocks.h"
#include "icodelist.h"
#include "imop.h"


namespace SecreC {
namespace {

inline bool isLeader(const Imop *i) {
    return *(i->block()->start) == i;
}

inline bool isEnd(const Imop *i) {
    return *(i->block()->end) == i;
}

inline void makeOuts(ReachingDefinitions::RMapping &ins, const Imop *i) {
    if ((i->type() & Imop::EXPR_MASK) == 0x0) return;
    ins[i->dest()] = i;
}

} // anonymous namespace



ReachingDefinitions::ReachingDefinitions(const ICodeList &code)
    : m_code(code)
{
    // Intentionally empty
}

void ReachingDefinitions::run() {
    std::vector<bool> changed(m_code.size(), false);

    /// \todo
}

bool step(const Imop *i) {

}

} // namespace SecreC
