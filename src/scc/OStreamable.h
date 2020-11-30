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

#ifndef SHAREMIND_SCC_OSTREAMABLE_H
#define SHAREMIND_SCC_OSTREAMABLE_H

#include <ostream>
#include <string>
#include <utility>


namespace SecreCC {

struct OStreamable {
    virtual ~OStreamable() noexcept = default;
    virtual std::ostream & streamTo(std::ostream & os) const = 0;
};

template <typename T>
struct SimpleOStreamable: OStreamable {

    template <typename ... Args>
    SimpleOStreamable(Args && ... args)
        : m_value(std::forward<Args>(args)...)
    {}

    std::ostream & streamTo(std::ostream & os) const final override
    { return os << m_value; }

    T m_value;

};

using OStreamableString = SimpleOStreamable<std::string>;

} // namespace SecreCC

#endif /* SHAREMIND_SCC_OSTREAMABLE_H */
