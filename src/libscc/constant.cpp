#include "constant.h"

#include <string>
#include <sstream>

namespace SecreC {

/*******************************************************************************
  SymbolConstantBool
*******************************************************************************/

std::string ConstantBool::toString() const {
    std::ostringstream os;
    os << "CONSTANT bool " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantInt
*******************************************************************************/

std::string ConstantInt::toString() const {
    std::ostringstream os;
    os << "CONSTANT int " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantInt8
*******************************************************************************/

std::string ConstantInt8::toString() const {
    std::ostringstream os;
    os << "CONSTANT int8 " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantInt16
*******************************************************************************/

std::string ConstantInt16::toString() const {
    std::ostringstream os;
    os << "CONSTANT int16 " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantInt32
*******************************************************************************/

std::string ConstantInt32::toString() const {
    std::ostringstream os;
    os << "CONSTANT int32 " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantString
*******************************************************************************/

std::string ConstantString::toString() const {
    std::ostringstream os;
    os << "CONSTANT string " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantUInt
*******************************************************************************/

std::string ConstantUInt::toString() const {
    std::ostringstream os;
    os << "CONSTANT uint " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantUInt8
*******************************************************************************/

std::string ConstantUInt8::toString() const {
    std::ostringstream os;
    os << "CONSTANT uint8 " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantUInt16
*******************************************************************************/

std::string ConstantUInt16::toString() const {
    std::ostringstream os;
    os << "CONSTANT uint16 " << m_value;
    return os.str();
}

/*******************************************************************************
  SymbolConstantUInt32
*******************************************************************************/

std::string ConstantUInt32::toString() const {
    std::ostringstream os;
    os << "CONSTANT uint32 " << m_value;
    return os.str();
}

}
