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

#ifndef SECREC_STRINGVIEWFASTCMP_H
#define SECREC_STRINGVIEWFASTCMP_H

#include <sharemind/StringView.h>
#include <tuple>


namespace SecreC {

struct StringViewFastCmp {
    bool operator()(sharemind::StringView a,
                    sharemind::StringView b) const noexcept
    {
        return std::make_tuple(a.data(), a.size())
                < std::make_tuple(b.data(), b.size());
    }
};

} /* namespace SecreC */

#endif /* SECREC_STRINGVIEWFASTCMP_H */
