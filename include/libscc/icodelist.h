#ifndef ICODELIST_H
#define ICODELIST_H

#include "imop.h"

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
