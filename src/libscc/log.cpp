#include "log.h"

#include "treenode.h"


namespace SecreC {

#define DEFINE_LOG_IN_PROC(small,Caps) \
    CompileLogStream CompileLog::small ## InProc(const TreeNode * n) { \
        if (n->containingProcedure()) \
            small() << "In procedure '" << n->containingProcedure()->printableSignature() << "':"; \
        return CompileLogStream(*this, CompileLogMessage::Caps); \
    }

DEFINE_LOG_IN_PROC(fatal,Fatal)
DEFINE_LOG_IN_PROC(error,Error)
DEFINE_LOG_IN_PROC(warning,Warning)
DEFINE_LOG_IN_PROC(info,Info)
DEFINE_LOG_IN_PROC(debug,Debug)

} // namespace SecreC
