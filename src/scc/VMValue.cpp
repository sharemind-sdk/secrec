/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "VMValue.h"

#include <boost/io/ios_state.hpp>
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
    boost::io::ios_flags_saver saver(os);
    os << "imm 0x" << std::hex << value ();
}

/******************************************************************
  VMStack
******************************************************************/

void VMStack::printV (std::ostream& os) const {
    boost::io::ios_flags_saver saver(os);
    os << "stack 0x" << std::hex << number ();
}

/******************************************************************
  VMReg
******************************************************************/

void VMReg::printV (std::ostream& os) const {
    boost::io::ios_flags_saver saver(os);
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
