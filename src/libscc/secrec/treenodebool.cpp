#include "secrec/treenodebool.h"


namespace SecreC {

std::string TreeNodeBool::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"bool:" << stringHelper() << "\"";
    return os.str();
}

} // namespace SecreC
