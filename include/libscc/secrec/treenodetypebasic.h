#ifndef TREENODETYPEBASIC_H
#define TREENODETYPEBASIC_H

#include "treenodetype.h"


namespace SecreC {

class TreeNodeTypeBasic: public TreeNodeType {
    public: /* Methods: */
        explicit TreeNodeTypeBasic(BasicType::SecType secType,
                                   BasicType::VarType varType,
                                   const YYLTYPE &loc)
            : TreeNodeType(NODE_BASICTYPE, loc), m_type(secType, varType) {}

        virtual inline const SecreC::Type &secrecType() const { return m_type; }
        inline void setSecrecBasicType(const SecreC::BasicType &type) {
            m_type = type;
        }

        inline BasicType::SecType secType() const { return m_type.secType(); }
        inline void setSecType(BasicType::SecType secType) {
            m_type.setSecType(secType);
        }
        inline BasicType::VarType varType() const { return m_type.varType(); }
        inline void setVarType(BasicType::VarType varType) {
            m_type.setVarType(varType);
        }

        std::string stringHelper() const;
        std::string xmlHelper() const;

    private: /* Fields: */
        SecreC::BasicType m_type;
};

} // namespace SecreC

#endif // TREENODETYPEBASIC_H
