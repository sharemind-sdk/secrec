#include "secrec/types.h"

#include <cassert>
#include <sstream>


namespace {

const char *SecrecFundSecTypeToString(SecrecSecType secType) {
    switch (secType) {
        case SECTYPE_INVALID: return "invalid";
        case SECTYPE_PUBLIC:  return "public";
        case SECTYPE_PRIVATE: return "private";
    }
    return 0;
}

const char *SecrecFundDataTypeToString(SecrecVarType varType) {
    switch (varType) {
        case VARTYPE_INVALID: return "invalid";
        case VARTYPE_BOOL:    return "bool";
        case VARTYPE_INT:     return "int";
        case VARTYPE_UINT:    return "unsigned int";
        case VARTYPE_STRING:  return "string";
    }
    return 0;
}

}

namespace SecreC {

std::string BasicDataType::toString() const {
    return SecrecFundDataTypeToString(m_varType);
}

std::string ArrayDataType::toString() const {
    assert(m_itemType != 0);

    std::ostringstream os("(");
    os << m_itemType->toString() << ")[";
    if (m_size > 0)
        os << m_size;
    os << "]";
    return os.str();
}

FunctionDataType::FunctionDataType(const FunctionDataType &copy)
    : DataType(copy), m_ret(copy.m_ret->clone())
{
    typedef std::vector<DataType*>::const_iterator TVCI;
    for (TVCI it(copy.m_params.begin()); it != copy.m_params.end(); it++) {
        m_params.push_back((*it)->clone());
    }
}

FunctionDataType::~FunctionDataType() {
    typedef std::vector<DataType*>::const_iterator TVCI;
    for (TVCI it(m_params.begin()); it != m_params.end(); it++) {
        delete (*it);
    }
    delete m_ret;
}

std::string FunctionDataType::toString() const {
    typedef std::vector<DataType*>::const_iterator TVCI;

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

bool FunctionDataType::operator==(const DataType &other) const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    if (!DataType::operator==(other)) return false;
    assert(dynamic_cast<const FunctionDataType*>(&other) != 0);
    const FunctionDataType &o(static_cast<const FunctionDataType&>(other));

    if (m_params.size() != o.m_params.size()) return false;
    if (*m_ret != *o.m_ret) return false;

    TVCI it(m_params.begin());
    TVCI jt(m_params.begin());
    for (; it != m_params.end(); it++, jt++) {
        if (*it != *jt) return false;
    }
    return true;
}

} // namespace SecreC
