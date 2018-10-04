/*
 * Copyright (C) Cybernetica
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

#ifndef SECREC_ANALYSIS_BOOSTINSERTWORKAROUND_H
#define SECREC_ANALYSIS_BOOSTINSERTWORKAROUND_H

#include <boost/container/container_fwd.hpp>

namespace SecreC {

template <typename Container, typename InputIterator>
inline void insertWorkaround(Container & c, InputIterator begin, InputIterator end) {
#if (BOOST_VERSION < 1068000)
    while (begin != end) {
        c.insert(*begin++);
    }
#else
    c.insert(boost::container::ordered_unique_range_t{}, begin, end);
#endif
}

} /* namespace SecreC */

#endif /* SECREC_ANALYSIS_BOOSTINSERTWORKAROUND_H */
