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

#include "VMCode.h"

#include <boost/io/ios_state.hpp>
#include <cassert>
#include <ostream>
#include <iterator>

#include "VMValue.h"

namespace SecreCC {

/*******************************************************************************
  VMBlock
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMBlock& block) {
    if (block.name())
        os << block.name ()->name () << '\n';

    std::copy (block.begin (), block.end (),
               std::ostream_iterator<VMInstruction>(os, "\n"));
    return os;
}

/*******************************************************************************
  VMFunction
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMFunction& function) {
    assert(function.name());
    os << function.name ()->name () << '\n';
    if (function.numLocals () != 0) {
        assert (! function.isStart ()
                && "Must not have local registers in global scope");
        boost::io::ios_flags_saver saver(os);
        os << "resizestack 0x" << std::hex  << function.numLocals () << '\n';
    }

    std::copy (function.begin (), function.end (),
               std::ostream_iterator<VMBlock>(os, "\n"));
    return  os;
}

/*******************************************************************************
  VMBinding
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMBinding& binding) {
    os << binding.m_label->name () << " .bind \"" << binding.m_name << "\"";
    return  os;
}

/*******************************************************************************
  VMSection
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMSection& section) {
    os << ".section " << section.name () << '\n';
    return section.printBodyV (os);
}

/*******************************************************************************
  VMBindingSection
*******************************************************************************/

std::ostream& VMBindingSection::printBodyV (std::ostream& os) const {
    std::copy (begin (), end (), std::ostream_iterator<VMBinding> (os, "\n"));
    return os;
}

/*******************************************************************************
  VMDataSection
*******************************************************************************/

std::ostream& VMDataSection::Record::print (std::ostream& os) const {
    if (m_label) {
        os << m_label->name () << ' ';
    }

    os << ".data" << ' ';
    os << m_dataType << ' ';
    os << m_value;
    return os;
}

std::ostream&
operator << (std::ostream& os, const VMDataSection::Record& record) {
    return record.print (os);
}

std::ostream& VMDataSection::printBodyV (std::ostream& os) const {
    std::copy (m_records.begin (), m_records.end (),
        std::ostream_iterator<VMDataSection::Record>(os, "\n"));
    return os;
}

/*******************************************************************************
  VMCodeSection
*******************************************************************************/

std::ostream& VMCodeSection::printBodyV (std::ostream& os) const {
    if (numGlobals () > 0) {
        boost::io::ios_flags_saver saver(os);
        os << "resizestack 0x" << std::hex << numGlobals () << '\n';
    }

    std::copy (begin (), end (), std::ostream_iterator<VMFunction>(os, "\n"));
    return os;
}

/*******************************************************************************
  VMLinkingUnit
*******************************************************************************/

VMLinkingUnit::~VMLinkingUnit () {
    for (VMSection* section : m_sections) {
        delete section;
    }
}

void VMLinkingUnit::addSection (VMSection *section) {
    assert(section);
    m_sections.push_back (section);
}

std::ostream& operator << (std::ostream& os, const VMLinkingUnit& vmlu) {
    for (VMSection* section : vmlu.m_sections) {
        os << *section;
    }

    return os;
}




} // namespace SecreCC
