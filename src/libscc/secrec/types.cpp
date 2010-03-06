#include "secrec/types.h"

#include <sstream>

namespace SecreC {


const char *BasicType::toString(SecType secType) {
    switch (secType) {
        case SECTYPE_PUBLIC:  return "public";
        case SECTYPE_PRIVATE: return "private";
    }
    return 0;
}

const char *BasicType::toString(VarType varType) {
    switch (varType) {
        case VARTYPE_VOID:   return "void";
        case VARTYPE_BOOL:   return "bool";
        case VARTYPE_INT:    return "int";
        case VARTYPE_UINT:   return "unsigned int";
        case VARTYPE_STRING: return "string";
    }
    return 0;
}

std::string BasicType::toString(SecType secType, VarType varType) {
    std::ostringstream os;
    os << toString(secType) << ' ' << toString(varType);
    return os.str();
}

} // namespace SecreC
