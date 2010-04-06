#include "secrec/types.h"

#include <cassert>
#include <sstream>


namespace {

inline const char *SecrecFundSecTypeToString(SecrecSecType secType) {
    switch (secType) {
        case SECTYPE_INVALID: return "invalid";
        case SECTYPE_PUBLIC:  return "public";
        case SECTYPE_PRIVATE: return "private";
    }
    return 0;
}

inline const char *SecrecFundDataTypeToString(SecrecVarType varType) {
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

/*******************************************************************************
  SecTypeBasic
*******************************************************************************/

std::string SecTypeBasic::toString() const {
    return SecrecFundSecTypeToString(m_secType);
}


/*******************************************************************************
  SecTypeFunctionVoid
*******************************************************************************/

std::string SecTypeFunctionVoid::toString() const {
    typedef std::vector<SecrecSecType>::const_iterator SVCI;

    std::ostringstream os;
    os << "(";
    if (m_params.size() > 0) {
        os << m_params.at(0);
        if (m_params.size() > 1) {
            for (SVCI it(++(m_params.begin())); it != m_params.end(); it++) {
                os << ", " << (*it);
            }
        }
    }
    os << ") -> void";
    return os.str();
}

inline bool SecTypeFunctionVoid::operator==(const SecType &other) {
    typedef std::vector<SecrecSecType>::const_iterator SVCI;

    if (!SecType::operator==(other)) return false;
    assert(dynamic_cast<const SecTypeFunctionVoid*>(&other) != 0);
    const SecTypeFunctionVoid &o(static_cast<const SecTypeFunctionVoid&>(other));

    if (m_params.size() != o.m_params.size()) return false;

    SVCI it(m_params.begin());
    SVCI jt(m_params.begin());
    for (; it != m_params.end(); it++, jt++) {
        if (*it != *jt) return false;
    }
    return true;
}


/*******************************************************************************
  SecTypeFunction
*******************************************************************************/

std::string SecTypeFunction::toString() const {
    typedef std::vector<SecrecSecType>::const_iterator SVCI;

    std::ostringstream os;
    os << "(";
    if (paramTypes().size() > 0) {
        os << paramTypes().at(0);
        if (paramTypes().size() > 1) {
            for (SVCI it(++(paramTypes().begin())); it != paramTypes().end(); it++) {
                os << ", " << (*it);
            }
        }
    }
    os << ") -> void";
    return os.str();
}

inline bool SecTypeFunction::operator==(const SecType &other) {
    typedef std::vector<SecrecSecType>::const_iterator SVCI;

    if (!SecType::operator==(other)) return false;
    assert(dynamic_cast<const SecTypeFunctionVoid*>(&other) != 0);
    const SecTypeFunctionVoid &o(static_cast<const SecTypeFunctionVoid&>(other));

    if (paramTypes().size() != o.paramTypes().size()) return false;

    SVCI it(paramTypes().begin());
    SVCI jt(paramTypes().begin());
    for (; it != paramTypes().end(); it++, jt++) {
        if (*it != *jt) return false;
    }
    return true;
}


/*******************************************************************************
  DataTypeBasic
*******************************************************************************/

DataTypeBasic::DataTypeBasic(const DataTypeVar &copy)
    : DataType(DataType::BASIC), m_varType(copy.varType())
{
    // Intentionally empty
}

std::string DataTypeBasic::toString() const {
    return SecrecFundDataTypeToString(m_varType);
}


/*******************************************************************************
  DataTypeVar
*******************************************************************************/

DataTypeVar::DataTypeVar(const DataTypeBasic &copy)
    : DataType(DataType::VAR), m_varType(copy.varType())
{
    // Intentionally empty
}

std::string DataTypeVar::toString() const {
    return std::string("VAR") + SecrecFundDataTypeToString(m_varType);
}


/*******************************************************************************
  DataTypeArray
*******************************************************************************/

std::string DataTypeArray::toString() const {
    assert(m_itemType != 0);

    std::ostringstream os;
    os << *m_itemType << "[";
    if (m_size > 0)
        os << m_size;
    os << "]";
    return os.str();
}


/*******************************************************************************
  DataTypeFunctionVoid
*******************************************************************************/

DataTypeFunctionVoid::DataTypeFunctionVoid(const DataTypeFunctionVoid &copy)
    : DataType(copy)
{
    typedef std::vector<DataType*>::const_iterator TVCI;
    for (TVCI it(copy.m_params.begin()); it != copy.m_params.end(); it++) {
        m_params.push_back((*it)->clone());
    }
}

DataTypeFunctionVoid::~DataTypeFunctionVoid() {
    typedef std::vector<DataType*>::const_iterator TVCI;
    for (TVCI it(m_params.begin()); it != m_params.end(); it++) {
        delete (*it);
    }
}

std::string DataTypeFunctionVoid::toString() const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    std::ostringstream os;
    os << "(";
    if (m_params.size() > 0) {
        os << *(m_params.at(0));
        if (m_params.size() > 1) {
            for (TVCI it(++(m_params.begin())); it != m_params.end(); it++) {
                os << ", " << (**it);
            }
        }
    }
    os << ") -> void";
    return os.str();
}

bool DataTypeFunctionVoid::operator==(const DataType &other) const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    if (!DataType::operator==(other)) return false;
    assert(dynamic_cast<const DataTypeFunctionVoid*>(&other) != 0);
    const DataTypeFunctionVoid &o(static_cast<const DataTypeFunctionVoid&>(other));

    if (m_params.size() != o.m_params.size()) return false;

    TVCI it(m_params.begin());
    TVCI jt(m_params.begin());
    for (; it != m_params.end(); it++, jt++) {
        if (*it != *jt) return false;
    }
    return true;
}

/*******************************************************************************
  DataTypeFunction
*******************************************************************************/

std::string DataTypeFunction::toString() const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    std::ostringstream os;
    os << "(";
    if (paramTypes().size() > 0) {
        os << *(paramTypes().at(0));
        if (paramTypes().size() > 1) {
            for (TVCI it(++(paramTypes().begin())); it != paramTypes().end(); it++) {
                os << ", " << (**it);
            }
        }
    }
    os << ") -> " << *m_ret;
    return os.str();
}

bool DataTypeFunction::operator==(const DataType &other) const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    if (!DataTypeFunctionVoid::operator==(other)) return false;
    assert(dynamic_cast<const DataTypeFunction*>(&other) != 0);
    const DataTypeFunction &o(static_cast<const DataTypeFunction&>(other));

    return *m_ret == *o.m_ret;
}


/*******************************************************************************
  TypeNonVoid
*******************************************************************************/

TypeNonVoid::TypeNonVoid(const SecType &secType,
                         const DataType &dataType)
     : Type(false), m_secType(secType.clone()),
       m_dataType(dataType.clone())
{
    switch (dataType.kind()) {
        case DataType::BASIC:
            assert(secType.kind() == SecType::BASIC);
            m_kind = TypeNonVoid::BASIC;
            break;
        case DataType::VAR:
            assert(secType.kind() == SecType::BASIC);
            m_kind = TypeNonVoid::VAR;
            break;
        case DataType::ARRAY:
            assert(secType.kind() == SecType::BASIC);
            m_kind = TypeNonVoid::ARRAY;
            break;
        case DataType::FUNCTION:
            assert(secType.kind() == SecType::FUNCTION);
            m_kind = TypeNonVoid::FUNCTION;
            break;
        case DataType::FUNCTIONVOID:
            assert(secType.kind() == SecType::FUNCTIONVOID);
            m_kind = TypeNonVoid::FUNCTIONVOID;
            break;
    }
}

TypeNonVoid::TypeNonVoid(SecrecSecType secType,
                         const DataType &dataType)
     : Type(false), m_secType(new SecTypeBasic(secType)),
       m_dataType(dataType.clone())
{
    assert(dataType.kind() != DataType::FUNCTION);

    switch (dataType.kind()) {
        case DataType::BASIC:
            m_kind = TypeNonVoid::BASIC;
            break;
        case DataType::VAR:
            m_kind = TypeNonVoid::VAR;
            break;
        case DataType::ARRAY:
            m_kind = TypeNonVoid::ARRAY;
            break;
        default:
            assert(false); // Shouldn't happen
    }
}

TypeNonVoid::~TypeNonVoid() {
    delete m_secType;
    delete m_dataType;
}

std::string TypeNonVoid::toString() const {
    assert(m_secType != 0);
    assert(m_dataType != 0);
    std::ostringstream os;
    os << "(" << *m_secType << "," << *m_dataType << ")";
    return os.str();
}

} // namespace SecreC


/*******************************************************************************
  Global functions
*******************************************************************************/

std::ostream &operator<<(std::ostream &out, const SecrecSecType &type) {
    out << SecrecFundSecTypeToString(type);
    return out;
}

std::ostream &operator<<(std::ostream &out, const SecrecVarType &type) {
    out << SecrecFundDataTypeToString(type);
    return out;
}
