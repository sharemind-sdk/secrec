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

#include "StringLiterals.h"

#include <sstream>

#include "VMSymbolTable.h"
#include "VMCode.h"
#include "VMValue.h"


namespace {

/// \todo probably broken
std::string escape (const std::string& str, bool asNullTerminated) {
    std::ostringstream os;

    os << '\"';
    for (char c : str) {
        switch (c) {
        case '\'': os << "\\\'"; break;
        case '\"': os << "\\\""; break;
        case '\?': os << "\\?";  break;
        case '\\': os << "\\\\"; break;
        case '\a': os << "\\a";  break;
        case '\b': os << "\\b";  break;
        case '\f': os << "\\f";  break;
        case '\n': os << "\\n";  break;
        case '\r': os << "\\r";  break;
        case '\t': os << "\\t";  break;
        case '\v': os << "\\v";  break;
        default:   os << c;      break;
        }
    }
    if (asNullTerminated)
        os << "\\0";

    os << '"';
    return os.str ();
}

}

namespace SecreCC {

/*******************************************************************************
  StringLiterals
*******************************************************************************/

StringLiterals::StringLiterals(VMSymbolTable & st)
    : m_st(st)
    , m_uniq (0)
{ }

StringLiterals::~StringLiterals () { }

void StringLiterals::init(std::shared_ptr<VMDataSection> section)
{ m_dataSection = std::move(section); }

StringLiterals::LiteralInfo StringLiterals::insert (const SecreC::ConstantString* str, bool asNullTerminated) {
    assert (str != nullptr);
    return insert (str->value ().str (), asNullTerminated);
}

StringLiterals::LiteralInfo StringLiterals::insert (const std::string& str, bool asNullTerminated) {
    LitMap::iterator i = m_literals.find (str);
    if (i == m_literals.end ()) {
        std::stringstream os;
        os << ":STR_" << m_uniq ++;
        VMLabel* label = m_st.getLabel (os.str ());
        const std::string& s = escape (str, asNullTerminated);
        m_dataSection->addStringRecord(label->nameStreamable(), s);
        auto size = str.size();
        if (asNullTerminated)
            ++size;
        i = m_literals.insert (i, std::make_pair (str, LiteralInfo (label, size)));
    }

    return i->second;
}


} // namespace SecreCC
