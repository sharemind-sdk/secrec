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

#include "Location.h"

#include "Parser.h"

#include <ostream>

namespace SecreC {

Location::FilenameCache Location::m_filenameCache;


std::ostream & operator<<(std::ostream & os, const Location & loc) {
    os << loc.filename()
       << ":(" << loc.firstLine()
       << ',' << loc.firstColumn()
       << ")(" << loc.lastLine()
       << ',' << loc.lastColumn() << ')';
    return os;
}

Location::Location(const YYLTYPE & loc)
    : m_firstLine(loc.first_line)
    , m_firstColumn(loc.first_column)
    , m_lastLine(loc.last_line)
    , m_lastColumn(loc.last_column)
{
    assert(loc.filename);
    init(loc.filename);
}

Location & Location::operator=(const YYLTYPE & loc) {
    assert(loc.filename);
    m_firstLine = loc.first_line;
    m_firstColumn = loc.first_column;
    m_lastLine = loc.last_line;
    m_lastColumn = loc.last_column;
    init(loc.filename);
    return *this;
}

YYLTYPE Location::toYYLTYPE() const {
    YYLTYPE r;
    r.first_line = m_firstLine;
    r.first_column = m_firstColumn;
    r.last_line = m_lastLine;
    r.last_column = m_lastColumn;
    r.filename = filename().c_str();
    return r;
}


}
