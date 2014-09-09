/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Location.h"

#include "parser.h"

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
    deinit();
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
