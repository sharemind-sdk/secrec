#include "secrec/treenodestring.h"

#include "misc.h"


namespace SecreC {

std::string TreeNodeString::stringHelper() const {
    std::ostringstream os;
    os << "\"" << m_value << "\"";
    return os.str();
}

std::string TreeNodeString::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"string:" << xmlEncode(m_value) << "\"";
    return os.str();
}

} // namespace SecreC
