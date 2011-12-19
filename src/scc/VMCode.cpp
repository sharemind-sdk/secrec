/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "VMCode.h"

#include <cassert>
#include <ostream>
#include <iterator>
#include <boost/foreach.hpp>

#include "VMValue.h"

namespace SecreCC {

/*******************************************************************************
  VMBlock
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMBlock& block) {
    if (block.name () != 0) {
        os << block.name ()->name () << '\n';
    }

    std::copy (block.begin (), block.end (),
               std::ostream_iterator<VMInstruction>(os, "\n"));
    return os;
}

/*******************************************************************************
  VMFunction
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMFunction& function) {
    assert (function.name () != 0);
    os << function.name ()->name () << '\n';
    if (function.numLocals () != 0) {
        assert (! function.isStart ()
                && "Must not have local registers in global scope");
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
        os << "resizestack 0x" << std::hex << numGlobals () << '\n';
    }

    std::copy (begin (), end (), std::ostream_iterator<VMFunction>(os, "\n"));
    return os;
}

/*******************************************************************************
  VMLinkingUnit
*******************************************************************************/

VMLinkingUnit::~VMLinkingUnit () {
    BOOST_FOREACH (VMSection* section, m_sections) {
        delete section;
    }
}

void VMLinkingUnit::addSection (VMSection *section) {
    assert (section != 0);
    m_sections.push_back (section);
}

std::ostream& operator << (std::ostream& os, const VMLinkingUnit& vmlu) {
    BOOST_FOREACH (VMSection* section, vmlu.m_sections) {
        os << *section;
    }

    return os;
}




} // namespace SecreCC
