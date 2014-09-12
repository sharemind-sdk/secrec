#include "ICodeList.h"

namespace SecreC {

namespace /* anonymous */ {

struct imop_disposer {
    inline void operator () (SecreC::Imop* imop) const {
        delete imop;
    }
};

} // namespace anonymous

ICodeList::~ICodeList() {
    clear_and_dispose (imop_disposer ());
}

void ICodeList::resetIndexes() {
    unsigned long i = 1;
    for (auto & elem : *this) {
        elem.setIndex (i);
        i ++;
    }
}

} // namespace SecreC
