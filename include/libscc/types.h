#ifndef TYPES_H
#define TYPES_H

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
        enum Kind { BASIC, PROCEDURE, PROCEDUREVOID };

    public: /* Methods: */
        explicit SecType(Kind kind)
            : m_kind(kind) {}
        explicit SecType(const SecType &copy)
            : m_kind(copy.m_kind) {}
        virtual inline ~SecType() {}

        inline bool kind() const { return m_kind; }

        virtual SecType *clone() const = 0;
        virtual std::string toString() const = 0;
        virtual inline bool operator==(const SecType &other) const {
            return m_kind == other.kind();
        }
        virtual inline bool operator!=(const SecType &other) const {
            return !operator==(other);
        }
        virtual inline bool canAssign(const SecType &) const {
            return false;
        }

    private: /* Fields: */
        Kind m_kind;
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
        virtual inline bool operator==(const SecType &other) const {
            if (!SecType::operator==(other)) return false;
            return m_secType == static_cast<const SecTypeBasic &>(other).secType();
        }
        virtual inline bool canAssign(const SecType &other) const {
            if (other.kind() != SecType::BASIC) return false;
            if (m_secType == SECTYPE_PRIVATE) return true;
            assert(dynamic_cast<const SecTypeBasic*>(&other) != 0);
            return static_cast<const SecTypeBasic&>(other).m_secType == SECTYPE_PUBLIC;
        }

    private: /* Fields: */
        SecrecSecType m_secType;
};

class SecTypeProcedureVoid: public SecType {
    public: /* Methods: */
        SecTypeProcedureVoid(SecType::Kind kind = SecType::PROCEDUREVOID)
            : SecType(kind) {}
        explicit SecTypeProcedureVoid(const SecTypeProcedureVoid &copy);
        virtual ~SecTypeProcedureVoid();

        void addParamType(const SecType &paramType) {
            m_params.push_back(paramType.clone());
        }
        inline const std::vector<SecType*> &paramTypes() const {
            return m_params;
        }

        virtual SecType *clone() const { return new SecTypeProcedureVoid(*this); }
        virtual std::string toString() const;
        virtual bool operator==(const SecType &other) const;

    protected: /* Methods: */
        std::string mangle() const;

    private: /* Fields: */
        std::vector<SecType*> m_params;
};

class SecTypeProcedure: public SecTypeProcedureVoid {
    public: /* Methods: */
        SecTypeProcedure()
            : SecTypeProcedureVoid(SecType::PROCEDURE),
              m_returnSecType(SECTYPE_INVALID) {}
        explicit SecTypeProcedure(SecrecSecType secType)
            : SecTypeProcedureVoid(SecType::PROCEDURE),
              m_returnSecType(secType) {}
        explicit SecTypeProcedure(const SecTypeProcedure &copy)
            : SecTypeProcedureVoid(copy), m_returnSecType(copy.m_returnSecType) {}

        inline SecrecSecType returnSecType() const { return m_returnSecType; }

        virtual SecType *clone() const { return new SecTypeProcedure(*this); }
        virtual std::string toString() const;
        virtual bool operator==(const SecType &other) const;
        virtual inline bool canAssign(const SecType &other) const {
            return SecTypeBasic(m_returnSecType).canAssign(other);
        }

    private: /* Fields: */
        SecrecSecType              m_returnSecType;
};


/*******************************************************************************
  Data types
*******************************************************************************/

class DataType {
    public: /* Types: */
        enum Kind { BASIC, VAR, ARRAY, PROCEDURE, PROCEDUREVOID };

    public: /* Methods: */
        explicit DataType(Kind kind)
            : m_kind(kind) {}
        explicit DataType(const DataType &other)
            : m_kind(other.m_kind) {}
        virtual inline ~DataType() {}

        inline Kind kind() const { return m_kind; }
        inline SecrecDataType secrecDataType() const;

        virtual DataType *clone() const = 0;
        virtual std::string toString() const = 0;
        virtual inline bool operator==(const DataType &other) const {
            return m_kind == other.kind();
        }
        virtual inline bool operator!=(const DataType &other) const {
            return !operator==(other);
        }
        virtual inline bool canAssign(const DataType &) const {
            return false;
        }

    private: /* Fields: */
        Kind m_kind;
};

class DataTypeBasic: public DataType {
    public: /* Methods: */
        explicit DataTypeBasic(SecrecDataType dataType)
            : DataType(DataType::BASIC), m_dataType(dataType) {}
        explicit DataTypeBasic(const DataTypeBasic &copy)
            : DataType(copy), m_dataType(copy.m_dataType) {}

        inline SecrecDataType dataType() const { return m_dataType; }

        virtual inline DataType *clone() const { return new DataTypeBasic(*this); }
        virtual std::string toString() const;
        virtual bool operator==(const DataType &other) const {
            return DataType::operator==(other)
                   && m_dataType == static_cast<const DataTypeBasic &>(other).m_dataType;
        }

    private: /* Fields: */
        SecrecDataType m_dataType;
};

class DataTypeVar: public DataType {
    public: /* Methods: */
        explicit DataTypeVar(SecrecDataType dataType)
            : DataType(DataType::VAR), m_dataType(new DataTypeBasic(dataType)) {}
        explicit DataTypeVar(const DataType &dataType)
            : DataType(DataType::VAR), m_dataType(dataType.clone()) {}
        explicit DataTypeVar(const DataTypeVar &copy)
            : DataType(copy), m_dataType(copy.m_dataType->clone()) {}
        virtual inline ~DataTypeVar() { delete m_dataType; }

        const DataType &dataType() const { return *m_dataType; }

        virtual inline DataType *clone() const { return new DataTypeVar(*this); }
        virtual std::string toString() const;
        virtual bool operator==(const DataType &other) const {
            return DataType::operator==(other)
                   && m_dataType == static_cast<const DataTypeVar &>(other).m_dataType;
        }
        virtual inline bool canAssign(const DataType &other) const {
            return *m_dataType == other;
        }

    private: /* Fields: */
        DataType *m_dataType;
};

class DataTypeArray: public DataType {
    public: /* Methods: */
        DataTypeArray(const DataType &itemType, unsigned size)
            : DataType(DataType::ARRAY), m_itemType(itemType.clone()), m_size(size)
        {
            assert(itemType.kind() == DataType::BASIC || itemType.kind() == DataType::ARRAY);
        }
        explicit DataTypeArray(const DataTypeArray &copy)
            : DataType(copy), m_itemType(copy.m_itemType->clone()),
              m_size(copy.m_size) {}
        virtual inline ~DataTypeArray() { delete m_itemType; }

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

class DataTypeProcedureVoid: public DataType {
    public: /* Methods: */
        explicit DataTypeProcedureVoid(DataType::Kind kind = PROCEDUREVOID)
            : DataType(kind) {}
        explicit DataTypeProcedureVoid(const DataTypeProcedureVoid &copy);
        virtual ~DataTypeProcedureVoid();

        virtual inline DataType *clone() const { return new DataTypeProcedureVoid(*this); }
        virtual std::string toString() const;
        std::string mangle() const;

        inline void addParamType(const DataType &paramType) { m_params.push_back(paramType.clone()); }
        inline const std::vector<DataType*> &paramTypes() const { return m_params; }

        virtual bool operator==(const DataType &other) const;

    private: /* Fields: */
        std::vector<DataType*> m_params;
};

class DataTypeProcedure: public DataTypeProcedureVoid {
    public: /* Methods: */
        explicit DataTypeProcedure(const DataType &returnType)
            : DataTypeProcedureVoid(PROCEDURE), m_ret(returnType.clone()) {}
        explicit DataTypeProcedure(const DataTypeProcedure &copy)
            : DataTypeProcedureVoid(copy), m_ret(copy.m_ret->clone()) {}
        virtual inline ~DataTypeProcedure() { delete m_ret; }

        virtual inline DataType *clone() const { return new DataTypeProcedure(*this); }
        virtual std::string toString() const;

        inline const DataType &returnType() const { return *m_ret; }

        virtual bool operator==(const DataType &other) const;
        virtual inline bool canAssign(const DataType &other) const {
            return DataTypeVar(*m_ret).canAssign(other);
        }

    private: /* Fields: */
        const DataType *m_ret;
};

inline SecrecDataType DataType::secrecDataType() const {
    switch (m_kind) {
        case BASIC:
            assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
            return static_cast<const DataTypeBasic*>(this)->dataType();
        case VAR:
            assert(dynamic_cast<const DataTypeVar*>(this) != 0);
            return static_cast<const DataTypeVar*>(this)->dataType().secrecDataType();
        case ARRAY:
            assert(dynamic_cast<const DataTypeArray*>(this) != 0);
            return static_cast<const DataTypeArray*>(this)->itemType().secrecDataType();
        case PROCEDURE:
            assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
            return static_cast<const DataTypeProcedure*>(this)->returnType().secrecDataType();
        case PROCEDUREVOID:
        default:
            return DATATYPE_INVALID;
    }
}


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
            return m_isVoid == other.m_isVoid;
        }
        virtual inline bool operator!=(const Type &other) const {
            return !operator==(other);
        }
        virtual inline bool canAssign(const Type &) const {
            return false;
        }
        inline const SecType &tnvSecType() const;
        inline const DataType &tnvDataType() const;
        inline SecrecSecType secrecSecType() const;
        inline SecrecDataType secrecDataType() const;

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
            BASIC,        /**< SecTypeBasic         + SecTypeBasic.          */
            VAR,          /**< SecTypeBasic         + DataTypeVar.           */
            ARRAY,        /**< SecTypeBasic         + DataTypeArray.         */
            PROCEDURE,    /**< SecTypeProcedure     + DataTypeProcedure.     */
            PROCEDUREVOID /**< SecTypeProcedureVoid + DataTypeProcedureVoid. */
        };

    public: /* Methods: */
        TypeNonVoid(SecrecSecType secType, SecrecDataType dataType)
            : Type(false), m_kind(BASIC), m_secType(new SecTypeBasic(secType)),
              m_dataType(new DataTypeBasic(dataType)) {}
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
        TypeNonVoid(const SecTypeProcedure &secType,
                    const DataTypeProcedure &dataType)
            : Type(false), m_kind(PROCEDURE), m_secType(secType.clone()),
              m_dataType(dataType.clone()) {}
        TypeNonVoid(const SecTypeProcedureVoid &secType,
                    const DataTypeProcedureVoid &dataType)
            : Type(false), m_kind(PROCEDUREVOID), m_secType(secType.clone()),
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
        virtual inline bool canAssign(const Type &other) const {
            if (other.isVoid()) return false;
            assert(dynamic_cast<const TypeNonVoid*>(&other) != 0);
            const TypeNonVoid &o = static_cast<const TypeNonVoid&>(other);
            return m_secType->canAssign(*(o.m_secType)) && m_dataType->canAssign(*(o.m_dataType));
        }

    private: /* Fields: */
        Kind      m_kind;
        SecType  *m_secType;
        DataType *m_dataType;
};

inline const SecType &Type::tnvSecType() const {
    assert(isVoid() == false);
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    const TypeNonVoid &t = static_cast<const TypeNonVoid&>(*this);

    return t.secType();
}

inline const DataType &Type::tnvDataType() const {
    assert(isVoid() == false);
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    const TypeNonVoid &t = static_cast<const TypeNonVoid&>(*this);

    return t.dataType();
}

inline SecrecSecType Type::secrecSecType() const {
    if (isVoid()) return SECTYPE_INVALID;
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    const TypeNonVoid &t = static_cast<const TypeNonVoid&>(*this);
    if (t.kind() == TypeNonVoid::BASIC
           || t.kind() == TypeNonVoid::VAR
           || t.kind() == TypeNonVoid::ARRAY)
    {
        assert(t.secType().kind() == SecType::BASIC);
        assert(dynamic_cast<const SecTypeBasic*>(&t.secType()) != 0);
        return static_cast<const SecTypeBasic &>(t.secType()).secType();
    } else {
        assert(t.kind() == TypeNonVoid::PROCEDURE);
        assert(t.secType().kind() == SecType::PROCEDURE);

        assert(dynamic_cast<const SecTypeProcedure*>(&t.secType()) != 0);
        return static_cast<const SecTypeProcedure &>(t.secType()).returnSecType();
    }
}

inline SecrecDataType Type::secrecDataType() const {
    if (isVoid()) return DATATYPE_INVALID;
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    return static_cast<const TypeNonVoid&>(*this).dataType().secrecDataType();
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecrecSecType &type);
std::ostream &operator<<(std::ostream &out, const SecrecDataType &type);

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
