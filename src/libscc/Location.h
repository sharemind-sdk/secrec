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

#ifndef SECREC_LOCATION_H
#define SECREC_LOCATION_H

#include <cassert>
#include <iosfwd>
#include <string>

struct YYLTYPE;

namespace SecreC {

class Location {
public: /* Types: */

    enum class PathStyle {
        FullPath,
        FileName
    };

private: /* Types: */

    class Printer {
    public: /* Methods: */
        Printer(Location const * location, Location::PathStyle style) noexcept
            : m_location(location)
            , m_style(style)
        { }

        friend std::ostream & operator<<(std::ostream & os, Printer const & p) {
            return p.m_location->print(os, p.m_style);
        }

    private: /* Fields: */
        Location const * m_location;
        Location::PathStyle m_style;
    };

public: /* Methods: */

    Location(std::size_t firstLine,
             std::size_t firstColumn,
             std::size_t lastColumn,
             std::size_t lastLine,
             char const * filename);

    Location(const YYLTYPE & loc);

    Location(const Location & loc) noexcept = default;

    Location & operator = (Location const & loc) noexcept = default;

    inline const std::string & filename() const { return *m_filenameItem; }

    YYLTYPE toYYLTYPE() const;

    std::ostream & print(std::ostream & os, PathStyle style) const;

    Printer printer(PathStyle style) const { return Printer{this, style}; }

private: /* Fields: */

    std::size_t m_firstLine;
    std::size_t m_firstColumn;
    std::size_t m_lastLine;
    std::size_t m_lastColumn;
    const std::string * m_filenameItem;
}; /* class Location { */

inline std::ostream & operator<<(std::ostream & os, const Location & loc) {
    return loc.print(os, Location::PathStyle::FullPath);
}

} // namespace SecreC

#endif /* SECREC_LOCATION_H */
