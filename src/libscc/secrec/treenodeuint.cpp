#include "secrec/treenodeuint.h"


namespace SecreC {

std::string TreeNodeUInt::stringHelper() const {
    std::ostringstream os;
    os << m_value;
    return os.str();
}

std::string TreeNodeUInt::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"uint:" << m_value << "\"";
    return os.str();
}

} // namespace SecreC
