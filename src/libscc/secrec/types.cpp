#include "secrec/types.h"

#include <cassert>
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

bool BasicType::operator==(const Type &other) const {
    if (other.kind() != kind()) return false;
    const BasicType &o(static_cast<const BasicType&>(other));
    if (o.secType() != secType()) return false;
    if (o.varType() != varType()) return false;
    return true;
}

std::string ArrayType::toString() const {
    assert(m_itemType != 0);

    std::ostringstream os("(");
    os << m_itemType->toString() << ")[";
    if (m_size > 0)
        os << m_size;
    os << "]";
    return os.str();
}

bool ArrayType::operator==(const Type &other) const {
    if (other.kind() != kind()) return false;
    const ArrayType &o(static_cast<const ArrayType&>(other));
    if (o.size() != size()) return false;
    if (*o.itemType() != *itemType()) return false;
    return true;
}

FunctionType::FunctionType(const FunctionType &copy)
    : Type(copy), m_ret(copy.m_ret->clone())
{
    typedef std::vector<Type*>::const_iterator TVCI;
    for (TVCI it(copy.m_params.begin()); it != copy.m_params.end(); it++) {
        m_params.push_back((*it)->clone());
    }
}

FunctionType::~FunctionType() {
    typedef std::vector<Type*>::const_iterator TVCI;
    for (TVCI it(m_params.begin()); it != m_params.end(); it++) {
        delete (*it);
    }
}

std::string FunctionType::toString() const {
    typedef std::vector<Type*>::const_iterator TVCI;

    assert(m_ret != 0);

    std::ostringstream os(m_ret->toString());
    os << " (";
    if (m_params.size() > 0) {
        os << *(m_params.at(0));
        if (m_params.size() > 1) {
            for (TVCI it(++(m_params.begin())); it != m_params.end(); it++) {
                os << ", " << (**it);
            }
        }
    }
    os << ")";
    return os.str();
}

bool FunctionType::operator==(const Type &other) const {
    if (other.kind() != kind()) return false;
    const FunctionType &o(static_cast<const FunctionType&>(other));
    if (o.numParams() != numParams()) return false;
    if (*o.returnType() != *returnType()) return false;
    for (unsigned i = 0; i < numParams(); i++) {
        if (*o.paramAt(i) != *paramAt(i)) return false;
    }
    return true;
}

} // namespace SecreC
