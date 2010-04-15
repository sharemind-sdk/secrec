#include "icodelist.h"


namespace SecreC {

ICodeList::~ICodeList() {
    for (const_iterator it(begin()); it != end(); it++) {
        delete *it;
    }
}

void ICodeList::resetIndexes() const {
    unsigned long i = 1;
    for (const_iterator it(begin()); it != end(); it++) {
        (*it)->setIndex(i);
        i++;
    }
}

} // namespace SecreC
