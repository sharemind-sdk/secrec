#include "secrec/treenodeexpr.h"


namespace {

void patchList(std::vector<SecreC::Imop*> &list,
               SecreC::Symbol *dest)
{
    typedef std::vector<SecreC::Imop*>::const_iterator IVCI;
    for (IVCI it(list.begin()); it != list.end(); it++) {
        (*it)->setDest(dest);
    }
    list.clear();
}

} // anonymous namespace

namespace SecreC {

void TreeNodeExpr::patchTrueList(Symbol *dest) {
    patchList(m_trueList, dest);
}

void TreeNodeExpr::patchFalseList(Symbol *dest) {
    patchList(m_falseList, dest);
}

void TreeNodeExpr::patchNextList(Symbol *dest) {
    patchList(m_nextList, dest);
}

} // namespace SecreC
