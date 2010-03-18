#ifndef TREENODETYPEVOID_H
#define TREENODETYPEVOID_H

#include "treenodetype.h"
#include "types.h"


namespace SecreC {

class TreeNodeTypeVoid: public TreeNodeType {
    public: /* Methods: */
        inline TreeNodeTypeVoid(const YYLTYPE &loc)
            : TreeNodeType(NODE_VOIDTYPE, loc) {}

        inline const SecreC::Type &secrecType() const { return m_voidType; }

        inline std::string stringHelper() const { return "\"void\""; }
        inline std::string xmlHelper() const { return "value=\"void\""; }

    private: /* Fields: */
        VoidType m_voidType;
};

} // namespace SecreC

#endif // TREENODETYPEVOID_H
