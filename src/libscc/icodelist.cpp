#include "icodelist.h"

namespace {

struct imop_disposer {
    inline void operator () (SecreC::Imop* imop) const {
        delete imop;
    }
};

}

namespace SecreC {

ICodeList::~ICodeList() {
    clear_and_dispose (imop_disposer ());
}

void ICodeList::resetIndexes() {
    unsigned long i = 1;
    for (ImopList::iterator it (begin ()); it != end (); it++) {
        it->setIndex (i);
        i ++;
    }
}

} // namespace SecreC
