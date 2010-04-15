#ifndef ICODELIST_H
#define ICODELIST_H

#include <vector>
#include "imop.h"


namespace SecreC {

class ICodeList {
    public: /* Types: */
        typedef std::vector<Imop*> List;
        typedef List::const_iterator const_iterator;
        typedef List::iterator iterator;

    public: /* Methods: */
        ~ICodeList();

        void resetIndexes() const;

        inline const_iterator begin() const { return m_list.begin(); }
        inline iterator begin() { return m_list.begin(); }
        inline const_iterator end() const { return m_list.end(); }
        inline iterator end() { return m_list.end(); }
        inline void push_back(Imop *i) {
            m_list.push_back(i);
            i->setIndex(m_list.size());
        }
        inline Imop *push_comment(const std::string &comment) {
            Imop *c = new Imop(Imop::COMMENT, 0, (Symbol*) new std::string(comment));
            push_back(c);
            return c;
        }

    private: /* Fields: */
        List m_list;
};

} // namespace SecreC

#endif // ICODELIST_H
