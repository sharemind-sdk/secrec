#include "types.h"

#include <cassert>
#include <sstream>

#include <iostream>


namespace {

inline const char *SecrecFundSecTypeToString(SecrecSecType secType) {
    switch (secType) {
        case SECTYPE_INVALID: return "invalid";
        case SECTYPE_PUBLIC:  return "public";
        case SECTYPE_PRIVATE: return "private";
    }
    return 0;
}

inline const char *SecrecFundDataTypeToString(SecrecDataType dataType) {
    switch (dataType) {
        case DATATYPE_INVALID: return "invalid";
        case DATATYPE_BOOL:    return "bool";
        case DATATYPE_INT:     return "int";
        case DATATYPE_UINT:    return "unsigned int";
        case DATATYPE_STRING:  return "string";
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
  SecTypeProcedureVoid
*******************************************************************************/

SecTypeProcedureVoid::SecTypeProcedureVoid(const SecTypeProcedureVoid &copy)
    : SecType(copy)
{
    typedef std::vector<SecType*>::const_iterator TVCI;
    for (TVCI it(copy.m_params.begin()); it != copy.m_params.end(); it++) {
        m_params.push_back((*it)->clone());
    }
}

SecTypeProcedureVoid::~SecTypeProcedureVoid() {
    typedef std::vector<SecType*>::const_iterator TVCI;
    for (TVCI it(m_params.begin()); it != m_params.end(); it++) {
        delete (*it);
    }
}

std::string SecTypeProcedureVoid::toString() const {
    return mangle() + " -> void";
}

std::string SecTypeProcedureVoid::mangle() const {
    typedef std::vector<SecType*>::const_iterator SVCI;

    std::ostringstream os;
    os << "(";
    if (m_params.size() > 0) {
        os << *m_params.at(0);
        if (m_params.size() > 1) {
            for (SVCI it(++(m_params.begin())); it != m_params.end(); it++) {
                os << ", " << (**it);
            }
        }
    }
    os << ")";
    return os.str();
}

inline bool SecTypeProcedureVoid::operator==(const SecType &other) const {
    typedef std::vector<SecType*>::const_iterator SVCI;

    if (!SecType::operator==(other)) return false;
    assert(dynamic_cast<const SecTypeProcedureVoid*>(&other) != 0);
    const SecTypeProcedureVoid &o(static_cast<const SecTypeProcedureVoid&>(other));

    if (m_params.size() != o.m_params.size()) return false;

    SVCI it(m_params.begin());
    SVCI jt(m_params.begin());
    for (; it != m_params.end(); it++, jt++) {
        if (*it != *jt) return false;
    }
    return true;
}


/*******************************************************************************
  SecTypeProcedure
*******************************************************************************/

std::string SecTypeProcedure::toString() const {
    std::ostringstream os;
    os << mangle() << " -> " << m_returnSecType;
    return os.str();
}

inline bool SecTypeProcedure::operator==(const SecType &other) const {
    typedef std::vector<SecType*>::const_iterator SVCI;

    if (!SecType::operator==(other)) return false;
    assert(dynamic_cast<const SecTypeProcedureVoid*>(&other) != 0);
    const SecTypeProcedureVoid &o(static_cast<const SecTypeProcedureVoid&>(other));

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

std::string DataTypeBasic::toString() const {
    return SecrecFundDataTypeToString(m_dataType);
}


/*******************************************************************************
  DataTypeVar
*******************************************************************************/

std::string DataTypeVar::toString() const {
    return std::string("VAR ") + m_dataType->toString();
}


/*******************************************************************************
  DataTypeProcedureVoid
*******************************************************************************/

DataTypeProcedureVoid::DataTypeProcedureVoid(const DataTypeProcedureVoid &copy)
    : DataType(copy)
{
    typedef std::vector<DataType*>::const_iterator TVCI;
    for (TVCI it(copy.m_params.begin()); it != copy.m_params.end(); it++) {
        m_params.push_back((*it)->clone());
    }
}

DataTypeProcedureVoid::~DataTypeProcedureVoid() {
    typedef std::vector<DataType*>::const_iterator TVCI;
    for (TVCI it(m_params.begin()); it != m_params.end(); it++) {
        delete (*it);
    }
}

std::string DataTypeProcedureVoid::toString() const {
    return mangle() + " -> void";
}

std::string DataTypeProcedureVoid::mangle() const {
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
    os << ")";
    return os.str();
}

bool DataTypeProcedureVoid::operator==(const DataType &other) const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    if (!DataType::operator==(other)) return false;
    assert(dynamic_cast<const DataTypeProcedureVoid*>(&other) != 0);
    const DataTypeProcedureVoid &o(static_cast<const DataTypeProcedureVoid&>(other));

    if (m_params.size() != o.m_params.size()) return false;

    TVCI it(m_params.begin());
    TVCI jt(m_params.begin());
    for (; it != m_params.end(); it++, jt++) {
        if (*it != *jt) return false;
    }
    return true;
}

/*******************************************************************************
  DataTypeProcedure
*******************************************************************************/

std::string DataTypeProcedure::toString() const {
    std::ostringstream os;
    os << mangle() << " -> " << *m_ret;
    return os.str();
}

bool DataTypeProcedure::operator==(const DataType &other) const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    if (!DataTypeProcedureVoid::operator==(other)) return false;
    assert(dynamic_cast<const DataTypeProcedure*>(&other) != 0);
    const DataTypeProcedure &o(static_cast<const DataTypeProcedure&>(other));

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
        case DataType::PROCEDURE:
            assert(secType.kind() == SecType::PROCEDURE);
            m_kind = TypeNonVoid::PROCEDURE;
            break;
        case DataType::PROCEDUREVOID:
            assert(secType.kind() == SecType::PROCEDUREVOID);
            m_kind = TypeNonVoid::PROCEDUREVOID;
            break;
    }
}

TypeNonVoid::TypeNonVoid(SecrecSecType secType,
                         const DataType &dataType)
     : Type(false), m_secType(new SecTypeBasic(secType)),
       m_dataType(dataType.clone())
{
    assert(dataType.kind() != DataType::PROCEDURE);

    switch (dataType.kind()) {
        case DataType::BASIC:
            m_kind = TypeNonVoid::BASIC;
            break;
        case DataType::VAR:
            m_kind = TypeNonVoid::VAR;
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

std::ostream &operator<<(std::ostream &out, const SecrecDataType &type) {
    out << SecrecFundDataTypeToString(type);
    return out;
}
