/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "Misc.h"

#include <sstream>

std::string strEscape (const std::string& input) {
    std::ostringstream os;
    const char *c = input.c_str();
    for (size_t i = 0; i < input.size(); i++, c++) {
        switch (*c) {
        case '\n': os << "\\n"; break;
        case '\r': os << "\\r"; break;
        case '\t': os << "\\t"; break;
        case '$':  os << "\\$"; break;
        case '\\': os << "\\\\"; break;
        case '\'': os << "\\\'"; break;
        case '\"': os << "\\\""; break;
        default:   os << *c; break;
        }
    }

    return os.str ();
}

std::string xmlEncode(const std::string &input) {
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

const char * SecrecFundDataTypeToString(SecrecDataType dataType) {
    switch (dataType) {
        case DATATYPE_UNDEFINED:     return "undefined";
        case DATATYPE_UNIT:          return "unit";
        case DATATYPE_BOOL:          return "bool";
        case DATATYPE_NUMERIC:       return "numeric";
        case DATATYPE_INT8:          return "int8";
        case DATATYPE_INT16:         return "int16";
        case DATATYPE_INT32:         return "int32";
        case DATATYPE_INT64:         return "int64";
        case DATATYPE_UINT8:         return "uint8";
        case DATATYPE_UINT16:        return "uint16";
        case DATATYPE_UINT32:        return "uint32";
        case DATATYPE_UINT64:        return "uint64";
        case DATATYPE_NUMERIC_FLOAT: return "numeric_float";
        case DATATYPE_FLOAT32:       return "float32";
        case DATATYPE_FLOAT64:       return "float64";
        case DATATYPE_STRING:        return "string";
        case NUM_DATATYPES:
            assert(false && "ICE!");
            break;
    }

    return nullptr;
}

SecrecDataType stringToSecrecFundDataType(const char * in) {
    SecrecDataType ty = DATATYPE_UNDEFINED;
    std::string name = in;

    if (name == "bool")
        ty = DATATYPE_BOOL;
    else if (name == "uint8")
        ty = DATATYPE_UINT8;
    else if (name == "uint16")
        ty = DATATYPE_UINT16;
    else if (name == "uint32")
        ty = DATATYPE_UINT32;
    else if (name == "uint64")
        ty = DATATYPE_UINT64;
    else if (name == "int8")
        ty = DATATYPE_INT8;
    else if (name == "int16")
        ty = DATATYPE_INT16;
    else if (name == "int32")
        ty = DATATYPE_INT32;
    else if (name == "int64")
        ty = DATATYPE_INT64;
    else if (name == "float32")
        ty = DATATYPE_FLOAT32;
    else if (name == "float64")
        ty = DATATYPE_FLOAT64;

    return ty;
}
