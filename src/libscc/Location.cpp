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

#include <boost/filesystem.hpp>
#include <ostream>
#include <set>


namespace /* anonymous */ {

// Pointers to set elements will are not invalidated!
using FilenameCache = std::set<std::string>;

static FilenameCache filenameCache;

std::string const * initFilename (const char* filename) {
    assert(filename);
    return &*filenameCache.insert (filename).first;
}

} // anonymous {

namespace SecreC {

Location::Location(std::size_t firstLine,
                   std::size_t firstColumn,
                   std::size_t lastColumn,
                   std::size_t lastLine,
                   char const * filename)
    : m_firstLine(firstLine)
    , m_firstColumn(firstColumn)
    , m_lastLine(lastLine)
    , m_lastColumn(lastColumn)
    , m_filenameItem(initFilename(filename))
{ }

Location::Location(const YYLTYPE & loc)
    : m_firstLine(loc.first_line)
    , m_firstColumn(loc.first_column)
    , m_lastLine(loc.last_line)
    , m_lastColumn(loc.last_column)
    , m_filenameItem(initFilename(loc.filename))
{  }

YYLTYPE Location::toYYLTYPE() const {
    YYLTYPE r;
    r.first_line = m_firstLine;
    r.first_column = m_firstColumn;
    r.last_line = m_lastLine;
    r.last_column = m_lastColumn;
    r.filename = filename().c_str();
    return r;
}

std::ostream & Location::print(std::ostream & os, Location::PathStyle style) const
{
    switch (style) {
    case PathStyle::FullPath:
        os << filename();
        break;
    case PathStyle::FileName:
        os << boost::filesystem::path(filename()).filename().c_str();
        break;
    }

    os << ":(" << m_firstLine
       << ',' << m_firstColumn
       << ")(" << m_lastLine
       << ',' << m_lastColumn
       << ')';

    return os;
}


} // namespace SecreC {
