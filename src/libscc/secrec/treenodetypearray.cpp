#include "secrec/treenodetypearray.h"


namespace SecreC {

const SecreC::Type &TreeNodeTypeArray::secrecType() const {
    typedef TreeNodeType TNT;

    assert(children().size() == 1);
    if (m_cachedType != 0) return *m_cachedType;

    assert(dynamic_cast<TNT*>(children().at(0).data()) != 0);
    TNT *t = static_cast<TNT*>(children().at(0).data());

    m_cachedType = new SecreC::ArrayType(t->secrecType(), m_value);
    return *m_cachedType;
}

std::string TreeNodeTypeArray::stringHelper() const {
    std::ostringstream os;
    os << m_value;
    return os.str();
}

std::string TreeNodeTypeArray::xmlHelper() const {
    std::ostringstream os;
    os << "value=\"array:" << m_value << "\"";
    return os.str();
}

} // namespace SecreC
