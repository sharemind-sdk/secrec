/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_LOCATION_H
#define SECREC_LOCATION_H

#include <cassert>
#include <iosfwd>
#include <set>
#include <string>

struct YYLTYPE;

namespace SecreC {

class Location {

    friend std::ostream & operator<<(std::ostream & os, const Location & loc);

private: /* Types: */

    // Pointers to set elements will are not invalidated!
    using FilenameCache = std::set<std::string>;

public: /* Methods: */

    inline Location(int firstLine, int firstColumn, int lastColumn, int lastLine, const char * filename)
        : m_firstLine(firstLine)
        , m_firstColumn(firstColumn)
        , m_lastLine(lastLine)
        , m_lastColumn(lastColumn)
    { init (filename); }

    Location(const YYLTYPE & loc);

    Location(const Location & loc) = default;

    Location & operator=(const YYLTYPE & loc);

    Location & operator=(const Location & loc) = default;

    bool operator==(const Location & rhs) const {
        return m_firstLine == rhs.m_firstLine
               && m_firstColumn == rhs.m_firstColumn
               && m_lastLine == rhs.m_lastLine
               && m_lastColumn == rhs.m_lastColumn
               && m_filenameItem == rhs.m_filenameItem;
    }

    bool operator!=(const Location & rhs) const {
        return !(*this == rhs);
    }

    inline int firstLine() const { return m_firstLine; }
    inline int firstColumn() const { return m_firstColumn; }
    inline int lastLine() const { return m_lastLine; }
    inline int lastColumn() const { return m_lastColumn; }
    inline const std::string & filename() const {
        return *m_filenameItem;
    }

    YYLTYPE toYYLTYPE() const;

private: /* Methods: */

    void init (const char* filename) {
        assert(filename);
        m_filenameItem= &*m_filenameCache.insert (filename).first;
    }

private: /* Fields: */

    int m_firstLine;
    int m_firstColumn;
    int m_lastLine;
    int m_lastColumn;
    const std::string * m_filenameItem;

    static FilenameCache m_filenameCache;

}; /* class Location { */

std::ostream & operator<<(std::ostream & os, const Location & loc);

} // namespace SecreC

#endif /* SECREC_LOCATION_H */
