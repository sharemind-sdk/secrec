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

        inline void push_imop (Imop *i) {
            ImopList::push_back (*i);
            i->setIndex (m_counter ++);
        }

        inline Imop* push_comment (const std::string &comment) {
            Imop *c = new Imop(0, Imop::COMMENT, 0, (Symbol*) new std::string(comment));
            ImopList::push_back(*c);
            return c;
        }

        void resetIndexes();

    private:

        unsigned m_counter;
};

} // namespace SecreC

#endif // ICODELIST_H
