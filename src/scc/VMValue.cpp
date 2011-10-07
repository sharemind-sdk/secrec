#include "VMValue.h"

#include <sstream>

namespace SecreCC {

/******************************************************************
  VMValue
******************************************************************/

std::ostream& operator << (std::ostream& os, 
                           const VMValue& value) {
    os << value.toString ();
    return os;
}

/******************************************************************
  VMImm
******************************************************************/

std::string VMImm::toString () const {
    std::ostringstream ss;
    ss << "0x" << std::hex << value ();
    return ss.str ();
}

/******************************************************************
  VMStack
******************************************************************/

std::string VMStack::toString () const {
    std::ostringstream ss;
    ss << "stack 0x" << std::hex << number ();
    return ss.str ();
}

/******************************************************************
  VMReg
******************************************************************/

std::string VMReg::toString () const {
    std::ostringstream ss;
    ss << "reg 0x" << std::hex << number ();
    return ss.str ();
}

/******************************************************************
  VMLabel
******************************************************************/

std::string VMLabel::toString () const {
    std::ostringstream ss;
    ss << name ();
    return ss.str ();
}

/******************************************************************
  VMVreg
******************************************************************/

std::string VMVReg::toString () const {
    if (m_actualReg == 0) {
        std::ostringstream ss;
        ss << "vreg " << this;
        return ss.str ();
    }
    else {
        return m_actualReg->toString ();
    }
}

}
