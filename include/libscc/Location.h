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
#include <string>
#include <map>
#include <limits>

#include "parser.h"

namespace SecreC {

class Location {

    friend std::ostream & operator<<(std::ostream & os, const Location & loc);

private: /* Types: */

    struct FilenameItem {
        inline FilenameItem(const char * f) : filename(f), refCount(0u) {}
        const std::string filename;
        size_t refCount;
    };

    typedef std::map<std::string, FilenameItem> FilenameCache;

public: /* Methods: */

    inline Location(int firstLine, int firstColumn, int lastColumn, int lastLine, const char * filename)
        : m_firstLine(firstLine)
        , m_firstColumn(firstColumn)
        , m_lastLine(lastLine)
        , m_lastColumn(lastColumn)
    {
        assert(filename);
        init(filename);
    }

    inline Location(const YYLTYPE & loc)
        : m_firstLine(loc.first_line)
        , m_firstColumn(loc.first_column)
        , m_lastLine(loc.last_line)
        , m_lastColumn(loc.last_column)
    {
        assert(loc.filename);
        init(loc.filename);
    }

    inline Location(const Location & loc)
        : m_firstLine(loc.m_firstLine)
        , m_firstColumn(loc.m_firstColumn)
        , m_lastLine(loc.m_lastLine)
        , m_lastColumn(loc.m_lastColumn)
        , m_filenameItem(loc.m_filenameItem)
    {
        if (loc.m_filenameItem->refCount == std::numeric_limits<size_t>::max())
            throw std::bad_alloc();
        loc.m_filenameItem->refCount++;
    }

    inline ~Location() {
        deinit();
    }

    Location & operator=(const YYLTYPE & loc) {
        assert(loc.filename);
        deinit();
        m_firstLine = loc.first_line;
        m_firstColumn = loc.first_column;
        m_lastLine = loc.last_line;
        m_lastColumn = loc.last_column;
        init(loc.filename);
        return *this;
    }

    Location & operator=(const Location & loc) {
        if (m_filenameItem != loc.m_filenameItem)
            deinit();
        m_firstLine = loc.m_firstLine;
        m_firstColumn = loc.m_firstColumn;
        m_lastLine = loc.m_lastLine;
        m_lastColumn = loc.m_lastColumn;
        if (m_filenameItem != loc.m_filenameItem) {
            m_filenameItem = loc.m_filenameItem;
            if (loc.m_filenameItem->refCount == std::numeric_limits<size_t>::max())
                throw std::bad_alloc();
            loc.m_filenameItem->refCount++;
        }
        return *this;
    }

    bool operator==(const Location & rhs) const {
        return m_firstLine == rhs.m_firstLine
               && m_firstColumn == rhs.m_firstColumn
               && m_lastLine == rhs.m_lastLine
               && m_lastColumn == rhs.m_lastColumn
               && m_filenameItem == rhs.m_filenameItem;
    }

    bool operator!=(const Location & rhs) const {
        return m_firstLine != rhs.m_firstLine
               || m_firstColumn != rhs.m_firstColumn
               || m_lastLine != rhs.m_lastLine
               || m_lastColumn != rhs.m_lastColumn
               || m_filenameItem != rhs.m_filenameItem;
    }

    inline int firstLine() const { return m_firstLine; }
    inline int firstColumn() const { return m_firstColumn; }
    inline int lastLine() const { return m_lastLine; }
    inline int lastColumn() const { return m_lastColumn; }
    inline const std::string & filename() const {
        return m_filenameItem->filename;
    }

    inline YYLTYPE toYYLTYPE() const {
        YYLTYPE r;
        r.first_line = m_firstLine;
        r.first_column = m_firstColumn;
        r.last_line = m_lastLine;
        r.last_column = m_lastColumn;
        r.filename = filename().c_str();
        return r;
    }

private: /* Methods: */

    inline void init(const char * const filename) {
        const FilenameItem i(filename);
        std::pair<FilenameCache::iterator, bool> r = m_filenameCache.insert(std::make_pair(i.filename, i));

        if (!r.second) {
            if ((*(r.first)).second.refCount == std::numeric_limits<size_t>::max())
                throw std::bad_alloc();
            (*(r.first)).second.refCount++;
        }
        m_filenameItem = &((*(r.first)).second);
    }

    inline void deinit() {
        if (m_filenameItem->refCount <= 0u) {
            m_filenameCache.erase(m_filenameItem->filename);
        } else {
            m_filenameItem->refCount--;
        }
    }


private: /* Fields: */

    int m_firstLine;
    int m_firstColumn;
    int m_lastLine;
    int m_lastColumn;
    FilenameItem * m_filenameItem;

    static FilenameCache m_filenameCache;

}; /* class Location { */

std::ostream & operator<<(std::ostream & os, const Location & loc);

} // namespace SecreC

#endif /* SECREC_LOCATION_H */
