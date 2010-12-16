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
  DataTypeBasic
*******************************************************************************/

std::string DataTypeBasic::toString() const {
    std::ostringstream os;
    os << "(" << SecrecFundSecTypeToString(m_secType) << ","
       << SecrecFundDataTypeToString(m_dataType) << ","
       << m_dimType << ")";
    return os.str();
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

// \todo don't use mangle() here
std::string DataTypeProcedureVoid::toString() const {
    return mangle() + " -> void";
}

// right now functions can only be overloaded by data,
// it's also possible to overload by dimensionalities, but i'm
// not sure how wise this is
std::string DataTypeProcedureVoid::mangle() const {
    typedef std::vector<DataType*>::const_iterator TVCI;

    std::ostringstream os;
    os << "(";
    if (m_params.size() > 0) {
        os << SecrecFundDataTypeToString(m_params.at(0)->secrecDataType());
        os << m_params.at(0)->secrecDimType();
        if (m_params.size() > 1) {
            for (TVCI it(++(m_params.begin())); it != m_params.end(); it++) {
                os << ", " << SecrecFundDataTypeToString((*it)->secrecDataType());
                os << (*it)->secrecDimType();

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

TypeNonVoid::TypeNonVoid(const DataType &dataType)
     : Type(false), m_dataType(dataType.clone())
{
    switch (dataType.kind()) {
        case DataType::BASIC:
            m_kind = TypeNonVoid::BASIC;
            break;
        case DataType::VAR:
            m_kind = TypeNonVoid::VAR;
            break;
        case DataType::PROCEDURE:
            m_kind = TypeNonVoid::PROCEDURE;
            break;
        case DataType::PROCEDUREVOID:
            m_kind = TypeNonVoid::PROCEDUREVOID;
            break;
    }
}

TypeNonVoid::~TypeNonVoid() {
    delete m_dataType;
}

std::string TypeNonVoid::toString() const {
    assert(m_dataType != 0);
    std::ostringstream os;
    os << *m_dataType;
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
