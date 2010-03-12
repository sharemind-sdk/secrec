#include "secrec/treenodeint.h"


namespace SecreC {

std::string TreeNodeInt::stringHelper() const {
    std::ostringstream os;
    os << m_value;
    return os.str();
}

std::string TreeNodeInt::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"int:" << m_value << "\"";
    return os.str();
}

} // namespace SecreC
