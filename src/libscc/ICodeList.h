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

#ifndef ICODELIST_H
#define ICODELIST_H

#include "Imop.h"

#include <boost/intrusive/list.hpp>

namespace SecreC {

class ICodeList : private ImopList {
public: /* Types: */

    using ImopList::const_iterator;
    using ImopList::iterator;
    using ImopList::const_reference;

public: /* Methods: */

    ICodeList () : m_counter (0) { }
    ~ICodeList();

    using ImopList::empty;
    using ImopList::begin;
    using ImopList::end;
    using ImopList::erase;
    using ImopList::front;
    using ImopList::back;
    using ImopList::iterator_to;

    inline iterator insert (const_iterator i, Imop* imop) {
        iterator j = ImopList::insert (i, *imop);
        imop->setIndex (m_counter ++);
        return j;
    }

    void resetIndexes();

private: /* Fields: */

    unsigned m_counter;
};

} // namespace SecreC

#endif // ICODELIST_H
