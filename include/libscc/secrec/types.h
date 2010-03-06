#ifndef SECREC_H
#define SECREC_H

#include <string>
#include <vector>
#include "parser.h"

namespace SecreC {

class Type {
    public: /* Types: */
        enum Kind { Basic, Array, Function };

    public: /* Methods: */
        Type(Kind kind)
            : m_kind(kind) {};

        inline Kind kind() const { return m_kind; }

    private: /* Fields: */
        Kind m_kind;
};

class BasicType: public Type {
    public: /* Types: */
        typedef enum SecrecSecType SecType;
        typedef enum SecrecVarType VarType;

    public: /* Methods: */
        BasicType(SecType secType, VarType varType)
            : Type(Type::Basic), m_secType(secType), m_varType(varType) {}

        inline SecType secType() const { return m_secType; }
        inline VarType varType() const { return m_varType; }

        inline std::string toString() const { return toString(m_secType, m_varType); }
        static const char *toString(SecType secType);
        static const char *toString(VarType varType);
        static std::string toString(SecType secType, VarType varType);

    private: /* Fields: */
        SecType m_secType;
        VarType m_varType;
};

class ArrayType: public Type {
    public: /* Methods: */
        ArrayType(Type *itemType, unsigned size = 0)
            : Type(Type::Array), m_itemType(itemType), m_size(size) {}

        inline Type *itemType() const { return m_itemType; }
        inline unsigned size() const { return m_size; }

    private: /* Fields: */
        Type    *m_itemType;
        unsigned m_size;
};

class FunctionType: public Type {
    public: /* Methods: */
        FunctionType(Type *returnType)
            : Type(Type::Function), m_ret(returnType) {}

        inline void addParamType(Type *paramType) { m_params.push_back(paramType); }
        inline unsigned numParams() const { return m_params.size(); }
        inline Type *paramAt(unsigned i) const { return m_params.at(i); }
        inline Type *returnType() const { return m_ret; }

    private: /* Fields: */
        std::vector<Type*> m_params;
        Type              *m_ret;
};

} // namespace SecreC

#endif // SECREC_H
