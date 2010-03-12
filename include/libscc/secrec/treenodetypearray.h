#ifndef TREENODETYPEARRAY_H
#define TREENODETYPEARRAY_H

#include "treenodetype.h"


namespace SecreC {

class TreeNodeTypeArray: public TreeNodeType {
    public: /* Methods: */
        explicit TreeNodeTypeArray(unsigned value, const YYLTYPE &loc)
            : TreeNodeType(NODE_ARRAYTYPE, loc), m_value(value),
              m_cachedType(0) {}
        virtual inline ~TreeNodeTypeArray() { delete m_cachedType; }

        virtual const SecreC::Type &secrecType() const;

        inline void setValue(unsigned value) { m_value = value; }
        inline unsigned value() const { return m_value; }

        std::string stringHelper() const;
        std::string xmlHelper() const;

    private: /* Fields: */
        unsigned                   m_value;
        mutable SecreC::ArrayType *m_cachedType;
};

} // namespace SecreC

#endif // TREENODETYPEARRAY_H
