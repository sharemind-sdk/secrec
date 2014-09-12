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
        case DATATYPE_XOR_UINT8:     return "xor_uint8";
        case DATATYPE_XOR_UINT16:    return "xor_uint16";
        case DATATYPE_XOR_UINT32:    return "xor_uint32";
        case DATATYPE_XOR_UINT64:    return "xor_uint64";
        case DATATYPE_FLOAT32:       return "float32";
        case DATATYPE_FLOAT64:       return "float64";
        case DATATYPE_STRING:        return "string";
        case NUM_DATATYPES:
            assert(false && "ICE!");
            break;
    }

    return nullptr;
}
