#ifndef SECREC_H
#define SECREC_H

#include <string>
#include <vector>
#include "parser.h"


namespace SecreC {

inline SecrecSecType upperSecType(SecrecSecType a, SecrecSecType b) {
    return (a == SECTYPE_PUBLIC ? b : SECTYPE_PRIVATE);
}

class Type {
    public: /* Types: */
        enum Kind { Basic, Array, Function };

    public: /* Methods: */
        Type(Kind kind)
            : m_kind(kind) {}

        inline Kind kind() const { return m_kind; }
        virtual Type *clone() const = 0;
        virtual std::string toString() const = 0;

        virtual bool operator==(const Type &other) const = 0;
        virtual bool operator!=(const Type &other) const {
            return !operator==(other);
        }

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
        BasicType(const BasicType &copy)
            : Type(copy), m_secType(copy.m_secType), m_varType(copy.m_varType) {}

        virtual inline Type *clone() const { return new BasicType(*this); }
        virtual inline std::string toString() const {
            return toString(m_secType, m_varType);
        }

        inline SecType secType() const { return m_secType; }
        inline void setSecType(BasicType::SecType secType) { m_secType = secType; }
        inline VarType varType() const { return m_varType; }
        inline void setVarType(BasicType::VarType varType) { m_varType = varType; }

        static const char *toString(SecType secType);
        static const char *toString(VarType varType);
        static std::string toString(SecType secType, VarType varType);

        virtual bool operator==(const Type &other) const;

    private: /* Fields: */
        SecType m_secType;
        VarType m_varType;
};

class ArrayType: public Type {
    public: /* Methods: */
        ArrayType(const Type &itemType, unsigned size = 0)
            : Type(Type::Array), m_itemType(itemType.clone()), m_size(size) {}
        ArrayType(const ArrayType &copy)
            : Type(copy), m_itemType(copy.m_itemType->clone()),
              m_size(copy.m_size) {}
        virtual inline ~ArrayType() {
            delete m_itemType;
        }

        virtual inline Type *clone() const { return new ArrayType(*this); }
        virtual std::string toString() const;

        inline Type *itemType() const { return m_itemType; }
        inline unsigned size() const { return m_size; }

        virtual bool operator==(const Type &other) const;

    private: /* Fields: */
        Type    *m_itemType;
        unsigned m_size;
};

class FunctionType: public Type {
    public: /* Methods: */
        FunctionType(Type *returnType)
            : Type(Type::Function), m_ret(returnType) {}
        FunctionType(const FunctionType &copy);
        virtual ~FunctionType();

        virtual inline Type *clone() const { return new FunctionType(*this); }
        virtual std::string toString() const;

        inline void addParamType(Type *paramType) { m_params.push_back(paramType); }
        inline unsigned numParams() const { return m_params.size(); }
        inline Type *paramAt(unsigned i) const { return m_params.at(i); }
        inline Type *returnType() const { return m_ret; }

        virtual bool operator==(const Type &other) const;

    private: /* Fields: */
        std::vector<Type*> m_params;
        Type              *m_ret;
};

} // namespace SecreC

inline std::ostream &operator<<(std::ostream &out, const SecreC::Type &type) {
    out << type.toString();
    return out;
}

#endif // SECREC_H
