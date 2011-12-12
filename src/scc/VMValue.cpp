/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

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
    ss << "imm 0x" << std::hex << value ();
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
    ss << "imm " << name ();
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
