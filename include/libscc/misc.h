#ifndef MISC_H
#define MISC_H

#include <string>
#include <ostream>
#include "parser.h"


std::string xmlEncode(const std::string &input);

inline std::ostream &operator<<(std::ostream &os, const YYLTYPE &v) {
    os << "("  << v.first_line << "," << v.first_column
       << ")(" << v.last_line  << "," << v.last_column << ")";
    return os;
}

#endif // MISC_H
