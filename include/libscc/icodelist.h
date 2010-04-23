#ifndef ICODELIST_H
#define ICODELIST_H

#include <vector>
#include "imop.h"


namespace SecreC {

class ICodeList: private std::vector<Imop*> {
    public: /* Types: */
        typedef std::vector<Imop*>::const_iterator const_iterator;
        typedef std::vector<Imop*>::const_reference const_reference;

    public: /* Methods: */
        ~ICodeList();

        inline const Imop *at(size_type n) const { return std::vector<Imop*>::at(n); }
        inline const_iterator begin() const { return std::vector<Imop*>::begin(); }
        inline const_iterator end() const { return std::vector<Imop*>::end(); }
        inline const_reference operator[] (size_type n) const { return std::vector<Imop*>::operator[](n); }
        inline size_t size() const { return std::vector<Imop*>::size(); }

        inline void push_imop(Imop *i) {
            std::vector<Imop*>::push_back(i);
            i->setIndex(size());
        }

        inline Imop *push_comment(const std::string &comment) {
            Imop *c = new Imop(0, Imop::COMMENT, 0, (Symbol*) new std::string(comment));
            std::vector<Imop*>::push_back(c);
            return c;
        }

        void resetIndexes() const;
};

} // namespace SecreC

#endif // ICODELIST_H
