#ifndef TREENODETYPEBASIC_H
#define TREENODETYPEBASIC_H

#include "treenodetype.h"


namespace SecreC {

class TreeNodeTypeBasic: public TreeNodeType {
    public: /* Methods: */
        explicit TreeNodeTypeBasic(SecrecSecType secType,
                                   SecrecVarType varType,
                                   const YYLTYPE &loc)
            : TreeNodeType(NODE_BASICTYPE, loc), m_type(secType, varType) {}

        virtual inline const SecreC::Type &secrecType() const { return m_type; }

        std::string stringHelper() const;
        std::string xmlHelper() const;

    private: /* Fields: */
        NonVoidType m_type;
};

} // namespace SecreC

#endif // TREENODETYPEBASIC_H
