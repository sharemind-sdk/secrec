#include "icodelist.h"

namespace SecreC {

ICodeList::~ICodeList() {
    clear_and_dispose (Imop::Disposer ());
}

void ICodeList::resetIndexes() {
    unsigned long i = 1;
    for (ImopList::iterator it (begin ()); it != end (); it++) {
        it->setIndex (i);
        i ++;
    }
}

} // namespace SecreC
