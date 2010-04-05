#ifndef SECREC_H
#define SECREC_H

#include <cassert>
#include <string>
#include <vector>
#include "parser.h"


namespace SecreC {

inline SecrecSecType upperSecType(SecrecSecType a, SecrecSecType b) {
    return (a == SECTYPE_PUBLIC ? b : SECTYPE_PRIVATE);
}


/*******************************************************************************
  Security types
*******************************************************************************/

class SecType {
    public: /* Types: */
        enum Kind { BASIC, FUNCTION, FUNCTIONVOID };

    public: /* Methods: */
        explicit SecType(Kind kind)
            : m_kind(kind) {}
        explicit SecType(const SecType &copy)
            : m_kind(copy.m_kind) {}
        virtual inline ~SecType() {}

        inline bool kind() const { return m_kind; }

        virtual SecType *clone() const = 0;
        virtual std::string toString() const = 0;
        virtual inline bool operator==(const SecType &other) {
            return m_kind == other.kind();
        }
        virtual inline bool operator!=(const SecType &other) {
            return !operator==(other);
        }

    private: /* Fields: */
        bool m_kind;
};

class SecTypeBasic: public SecType {
    public: /* Methods: */
        explicit SecTypeBasic(SecrecSecType secType)
            : SecType(SecType::BASIC), m_secType(secType) {}
        explicit SecTypeBasic(const SecTypeBasic &copy)
            : SecType(copy), m_secType(copy.secType()) {}

        inline SecrecSecType secType() const { return m_secType; }
        virtual SecType *clone() const { return new SecTypeBasic(*this); }
        virtual std::string toString() const;
        virtual inline bool operator==(const SecType &other) {
            if (!SecType::operator==(other)) return false;
            return m_secType == static_cast<const SecTypeBasic &>(other).secType();
        }

    private: /* Fields: */
        SecrecSecType m_secType;
};

class SecTypeFunctionVoid: public SecType {
    public: /* Methods: */
        SecTypeFunctionVoid(SecType::Kind kind = SecType::FUNCTIONVOID)
            : SecType(kind) {}
        explicit SecTypeFunctionVoid(const SecTypeFunctionVoid &copy)
            : SecType(copy), m_params(copy.m_params) {}
        virtual inline ~SecTypeFunctionVoid() {};

        void addParamType(SecrecSecType secType) {
            m_params.push_back(secType);
        }
        inline const std::vector<SecrecSecType> &paramTypes() const {
            return m_params;
        }

        virtual SecType *clone() const { return new SecTypeFunctionVoid(*this); }
        virtual std::string toString() const;
        virtual bool operator==(const SecType &other);

    private: /* Fields: */
        std::vector<SecrecSecType> m_params;
};

class SecTypeFunction: public SecTypeFunctionVoid {
    public: /* Methods: */
        SecTypeFunction()
            : SecTypeFunctionVoid(SecType::FUNCTION),
              m_returnSecType(SECTYPE_INVALID) {}
        explicit SecTypeFunction(SecrecSecType secType)
            : SecTypeFunctionVoid(SecType::FUNCTION),
              m_returnSecType(secType) {}
        explicit SecTypeFunction(const SecTypeFunction &copy)
            : SecTypeFunctionVoid(copy), m_returnSecType(copy.m_returnSecType) {}

        inline SecrecSecType returnSecType() const { return m_returnSecType; }

        virtual SecType *clone() const { return new SecTypeFunction(*this); }
        virtual std::string toString() const;
        virtual bool operator==(const SecType &other);

    private: /* Fields: */
        SecrecSecType              m_returnSecType;
};


/*******************************************************************************
  Data types
*******************************************************************************/

class DataType {
    public: /* Types: */
        enum Kind { BASIC, VAR, ARRAY, FUNCTION, FUNCTIONVOID };

    public: /* Methods: */
        explicit DataType(Kind kind)
            : m_kind(kind) {}
        explicit DataType(const DataType &other)
            : m_kind(other.m_kind) {}
        virtual inline ~DataType() {};

        inline Kind kind() const { return m_kind; }

        virtual DataType *clone() const = 0;
        virtual std::string toString() const = 0;
        virtual inline bool operator==(const DataType &other) const {
            return m_kind == other.kind();
        }
        virtual inline bool operator!=(const DataType &other) const {
            return !operator==(other);
        }

    private: /* Fields: */
        Kind m_kind;
};

class DataTypeVar;

class DataTypeBasic: public DataType {
    public: /* Methods: */
        explicit DataTypeBasic(SecrecVarType varType)
            : DataType(DataType::BASIC), m_varType(varType) {}
        explicit DataTypeBasic(const DataTypeBasic &copy)
            : DataType(copy), m_varType(copy.m_varType) {}
        explicit DataTypeBasic(const DataTypeVar &copy);

        inline SecrecVarType varType() const { return m_varType; }

        virtual inline DataType *clone() const { return new DataTypeBasic(*this); }
        virtual std::string toString() const;
        virtual bool operator==(const DataType &other) const {
            return DataType::operator==(other)
                   && m_varType == static_cast<const DataTypeBasic &>(other).m_varType;
        }

    private: /* Fields: */
        SecrecVarType m_varType;
};

class DataTypeVar: public DataType {
    public: /* Methods: */
        explicit DataTypeVar(SecrecVarType varType)
            : DataType(DataType::BASIC), m_varType(varType) {}
        explicit DataTypeVar(const DataTypeVar &copy)
            : DataType(copy), m_varType(copy.m_varType) {}
        explicit DataTypeVar(const DataTypeBasic &copy);

        inline SecrecVarType varType() const { return m_varType; }

        virtual inline DataType *clone() const { return new DataTypeVar(*this); }
        virtual std::string toString() const;
        virtual bool operator==(const DataType &other) const {
            return DataType::operator==(other)
                   && m_varType == static_cast<const DataTypeVar &>(other).m_varType;
        }
        inline bool equivalentTo(const DataTypeBasic &basicType) const {
            return basicType.varType() == m_varType;
        }

    private: /* Fields: */
        SecrecVarType m_varType;
};

class DataTypeArray: public DataType {
    public: /* Methods: */
        DataTypeArray(const DataType &itemType, unsigned size)
            : DataType(DataType::ARRAY), m_itemType(itemType.clone()), m_size(size)
        {
            assert(itemType.kind() == DataType::BASIC || itemType.kind() == DataType::ARRAY);
        }
        explicit DataTypeArray(const DataTypeArray &copy)
            : DataType(copy), m_itemType(copy.m_itemType),
              m_size(copy.m_size) {}
        inline ~DataTypeArray() { delete m_itemType; }

        virtual inline DataType *clone() const { return new DataTypeArray(*this); }
        virtual std::string toString() const;

        inline const DataType &itemType() const { return *m_itemType; }
        inline unsigned size() const { return m_size; }

        virtual bool operator==(const DataType &other) const {
            return DataType::operator==(other)
                   && m_size == static_cast<const DataTypeArray &>(other).m_size
                   && m_itemType == static_cast<const DataTypeArray &>(other).m_itemType;
        }

    private: /* Fields: */
        DataType *m_itemType;
        unsigned m_size;
};

class DataTypeFunctionVoid: public DataType {
    public: /* Methods: */
        explicit DataTypeFunctionVoid(DataType::Kind kind = FUNCTIONVOID)
            : DataType(kind) {}
        explicit DataTypeFunctionVoid(const DataTypeFunctionVoid &copy);
        virtual ~DataTypeFunctionVoid();

        virtual inline DataType *clone() const { return new DataTypeFunctionVoid(*this); }
        virtual std::string toString() const;

        inline void addParamType(const DataType &paramType) { m_params.push_back(paramType.clone()); }
        inline const std::vector<DataType*> &paramTypes() const { return m_params; }

        virtual bool operator==(const DataType &other) const;

    private: /* Fields: */
        std::vector<DataType*> m_params;
};

class DataTypeFunction: public DataTypeFunctionVoid {
    public: /* Methods: */
        explicit DataTypeFunction(const DataType &returnType)
            : DataTypeFunctionVoid(FUNCTION), m_ret(returnType.clone()) {}
        explicit DataTypeFunction(const DataTypeFunction &copy)
            : DataTypeFunctionVoid(copy), m_ret(copy.m_ret->clone()) {}
        virtual inline ~DataTypeFunction() { delete m_ret; }

        virtual inline DataType *clone() const { return new DataTypeFunction(*this); }
        virtual std::string toString() const;

        inline const DataType &returnType() const { return *m_ret; }

        virtual bool operator==(const DataType &other) const;

    private: /* Fields: */
        const DataType *m_ret;
};


/*******************************************************************************
  General types
*******************************************************************************/

class Type {
    public: /* Methods: */
        explicit inline Type(bool isVoid)
            : m_isVoid(isVoid) {}
        explicit inline Type(const Type &copy)
            : m_isVoid(copy.m_isVoid) {}
        virtual inline ~Type() {}

        inline bool isVoid() const { return m_isVoid; }

        virtual Type *clone() const = 0;
        virtual std::string toString() const = 0;

        virtual inline bool operator==(const Type &other) const {
            return other.isVoid();
        }
        virtual inline bool operator!=(const Type &other) const {
            return !operator==(other);
        }

    private: /* Fields: */
        bool m_isVoid;
};

class TypeVoid: public Type {
    public: /* Methods: */
        inline TypeVoid()
            : Type(true) {}
        explicit inline TypeVoid(const TypeVoid &copy)
            : Type(copy) {}

        virtual inline Type *clone() const { return new TypeVoid(); }
        virtual inline std::string toString() const { return "void"; }
};

class TypeNonVoid: public Type {
    public: /* Types: */
        enum Kind {
            BASIC,       /**< SecTypeBasic        + SecTypeBasic.         */
            VAR,         /**< SecTypeBasic        + DataTypeVar.          */
            ARRAY,       /**< SecTypeBasic        + DataTypeArray.        */
            FUNCTION,    /**< SecTypeFunction     + DataTypeFunction.     */
            FUNCTIONVOID /**< SecTypeFunctionVoid + DataTypeFunctionVoid. */
        };

    public: /* Methods: */
        TypeNonVoid(SecrecSecType secType, SecrecVarType varType)
            : Type(false), m_kind(BASIC), m_secType(new SecTypeBasic(secType)),
              m_dataType(new DataTypeBasic(varType)) {}
        TypeNonVoid(const SecTypeBasic &secType, const DataTypeBasic &dataType)
            : Type(false), m_kind(BASIC), m_secType(secType.clone()),
              m_dataType(dataType.clone()) {}
        TypeNonVoid(const SecTypeBasic &secType, const DataTypeVar &dataType)
            : Type(false), m_kind(VAR), m_secType(secType.clone()),
              m_dataType(dataType.clone()) {}
        TypeNonVoid(const SecTypeBasic &secType,
                    const DataTypeArray &dataType)
            : Type(false), m_kind(ARRAY), m_secType(secType.clone()),
              m_dataType(dataType.clone()) {}
        TypeNonVoid(const SecTypeFunction &secType,
                    const DataTypeFunction &dataType)
            : Type(false), m_kind(FUNCTION), m_secType(secType.clone()),
              m_dataType(dataType.clone()) {}
        TypeNonVoid(const SecTypeFunctionVoid &secType,
                    const DataTypeFunctionVoid &dataType)
            : Type(false), m_kind(FUNCTIONVOID), m_secType(secType.clone()),
              m_dataType(dataType.clone()) {}
        TypeNonVoid(const SecType &secType,
                    const DataType &dataType);
        TypeNonVoid(SecrecSecType secType,
                    const DataType &dataType);
        explicit inline TypeNonVoid(const TypeNonVoid &copy)
            : Type(copy), m_kind(copy.m_kind),
              m_secType(copy.m_secType->clone()),
              m_dataType(copy.m_dataType->clone()) {}
        virtual ~TypeNonVoid();

        inline Kind kind() const { return m_kind; }
        inline const SecType &secType() const { return *m_secType; }
        inline const DataType &dataType() const { return *m_dataType; }

        virtual inline Type *clone() const { return new TypeNonVoid(*this); }
        virtual std::string toString() const;
        virtual inline bool operator==(const Type &other) const {
            return Type::operator==(other)
                   && m_kind == static_cast<const TypeNonVoid&>(other).kind();
        }

    private: /* Fields: */
        Kind      m_kind;
        SecType  *m_secType;
        DataType *m_dataType;
};


} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecrecSecType &type);
std::ostream &operator<<(std::ostream &out, const SecrecVarType &type);

inline std::ostream &operator<<(std::ostream &out, const SecreC::Type &type) {
    out << type.toString();
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const SecreC::DataType &type) {
    out << type.toString();
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const SecreC::SecType &type) {
    out << type.toString();
    return out;
}

#endif // SECREC_H
