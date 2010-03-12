#include "secrec/treenodetypebasic.h"


namespace SecreC {

std::string TreeNodeTypeBasic::stringHelper() const {
    std::ostringstream os;
    os << "\"" << m_type.toString() << "\"";
    return os.str();
}

std::string TreeNodeTypeBasic::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"basic:" << m_type.toString() << "\"";
    return os.str();
}

} // namespace SecreC
