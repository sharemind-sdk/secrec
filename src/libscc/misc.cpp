#include "misc.h"

#include <sstream>


std::string xmlEncode(const std::string &input) {
    typedef std::string::const_iterator SCI;

    std::ostringstream os;

    for (SCI it = input.begin(); it != input.end(); it++) {
        unsigned char c = static_cast<unsigned char>(*it);
        switch (c) {
            case '&': os << "&amp;"; break;
            case '<': os << "&lt;"; break;
            case '>': os << "&gt;"; break;
            case '"': os << "&quot;"; break;
            case '\'': os << "&apos;"; break;
            default:
                if (c < 32 || c > 127) {
                    os << "&#" << static_cast<unsigned int>(c) << ';';
                } else {
                    os << c;
                }
                break;
        }
    }
    return os.str();
}

