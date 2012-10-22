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

std::ostream& operator << (std::ostream& os, const VMValue& value) {
    value.printV (os);
    return os;
}

/******************************************************************
  VMImm
******************************************************************/

void VMImm::printV (std::ostream& os) const {
    os << "imm 0x" << std::hex << value ();
}

/******************************************************************
  VMStack
******************************************************************/

void VMStack::printV (std::ostream& os) const {
    os << "stack 0x" << std::hex << number ();
}

/******************************************************************
  VMReg
******************************************************************/

void VMReg::printV (std::ostream& os) const {
    os << "reg 0x" << std::hex << number ();
}

/******************************************************************
  VMLabel
******************************************************************/

void VMLabel::printV (std::ostream& os) const {
    os << "imm " << name ();
}

/******************************************************************
  VMVreg
******************************************************************/

void VMVReg::printV (std::ostream& os) const {
    if (m_actualReg == 0) {
        os << "vreg " << this;
    }
    else {
        os << *m_actualReg;
    }
}

}
