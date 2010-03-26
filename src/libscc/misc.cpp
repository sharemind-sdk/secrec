#include "misc.h"

#include <sstream>


std::string xmlEncode(const std::string &input) {
    typedef std::string::const_iterator SCI;

    std::ostringstream os;

    const char *c = input.c_str();
    for (size_t i = 0; i < input.size(); i++, c++) {
        switch (*c) {
            case '&': os << "&amp;"; break;
            case '<': os << "&lt;"; break;
            case '>': os << "&gt;"; break;
            case '"': os << "&quot;"; break;
            case '\'': os << "&apos;"; break;
            default:
                if (*c < 32 || *c > 126) {
                    os << "&#" << static_cast<unsigned int>(*c) << ';';
                } else {
                    os << *c;
                }
                break;
        }
    }
    return os.str();
}

