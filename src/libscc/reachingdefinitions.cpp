#include "reachingdefinitions.h"

#include "icodelist.h"


namespace SecreC {

ReachingDefinitions::ReachingDefinitions(const ICodeList &code)
    : m_defs(code.size()), m_code(code)
{
    // Intentionally empty
}

void ReachingDefinitions::run() {
    /// \todo
}

} // namespace SecreC
