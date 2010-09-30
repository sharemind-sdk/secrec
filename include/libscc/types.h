#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <string>
#include <vector>
#include "parser.h"

/// \todo figure out how to merge Type/TypeVoid with DataType's
/// \todo add lub operator to DataType classes?

namespace SecreC {

inline SecrecSecType upperSecType(SecrecSecType a, SecrecSecType b) {
    return (a == SECTYPE_PUBLIC ? b : SECTYPE_PRIVATE);
}

inline SecrecDimType upperDimType(SecrecDimType n, SecrecDimType m) {
    assert (n == 0 || m == 0 || n == m);
    if (n == 0) return m;
    // if (m == 0) return n; // but we return n anyways
    return n;
}

inline bool latticeDimTypeLEQ (SecrecDimType n, SecrecDimType m) {
    return n == m || n == 0;
}

inline bool latticeSecTypeLEQ (SecrecSecType a, SecrecSecType b) {
    return a == b || a == SECTYPE_PUBLIC;
}

inline bool latticeDataTypeLEQ (SecrecDataType a, SecrecDataType b) {
    return a == b;
}


/*******************************************************************************
  Data types
*******************************************************************************/

class DataType {
    public: /* Types: */
        enum Kind { BASIC, VAR, PROCEDURE, PROCEDUREVOID };

    public: /* Methods: */
        explicit DataType(Kind kind)
            : m_kind(kind) {}
        explicit DataType(const DataType &other)
            : m_kind(other.m_kind) {}
        virtual inline ~DataType() {}

        inline Kind kind() const { return m_kind; }
        inline SecrecSecType secrecSecType() const;
        inline SecrecDataType secrecDataType() const;
        inline SecrecDimType secrecDimType() const;

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

        /**
         * We define less-than-equal relation on data types to check
         * if a kind of type can be converted into other. This is used to
         * convert public data to private and for interpreting scalar values
         * as arbitrary dimensional arrays.
         */
        virtual inline bool latticeLEQ(DataType const& other) const {
            return m_kind == other.m_kind;
        }

    private: /* Fields: */
        Kind m_kind;
};

class DataTypeBasic: public DataType {
    public: /* Methods: */
        explicit DataTypeBasic(SecrecSecType secType, SecrecDataType dataType, unsigned dim = 0)
            : DataType(DataType::BASIC), m_secType(secType), m_dataType(dataType), m_dimType(dim) {}
        explicit DataTypeBasic(const DataTypeBasic &copy)
            : DataType(copy), m_secType(copy.m_secType), m_dataType(copy.m_dataType), m_dimType(copy.m_dimType) {}

        inline SecrecSecType secType() const { return m_secType; }
        inline SecrecDataType dataType() const { return m_dataType; }
        inline SecrecDimType dimType() const { return m_dimType; }

        virtual inline DataType *clone() const { return new DataTypeBasic(*this); }
        virtual std::string toString() const;

        virtual bool operator==(const DataType &other) const {
            return DataType::operator==(other)
                   && m_secType == static_cast<const DataTypeBasic &>(other).m_secType
                   && m_dataType == static_cast<const DataTypeBasic &>(other).m_dataType
                   && m_dimType == static_cast<const DataTypeBasic &>(other).m_dimType;
        }

        virtual bool latticeLEQ(const DataType &other) const {
            return  DataType::latticeLEQ(other)
                    && latticeSecTypeLEQ(m_secType,static_cast<const DataTypeBasic&>(other).m_secType)
                    && latticeDataTypeLEQ(m_dataType,static_cast<const DataTypeBasic &>(other).m_dataType)
                    && latticeDimTypeLEQ(m_dimType,static_cast<const DataTypeBasic &>(other).m_dimType);
        }

    private: /* Fields: */
        SecrecSecType m_secType;
        SecrecDataType m_dataType;
        SecrecDimType m_dimType;
};

class DataTypeVar: public DataType {
    public: /* Methods: */
        explicit DataTypeVar(SecrecSecType secType, SecrecDataType dataType)
            : DataType(DataType::VAR), m_dataType(new DataTypeBasic(secType, dataType)) {}
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
            return other.latticeLEQ(*m_dataType);
        }

        virtual inline bool latticeLEQ(const DataType &other) const {
            return m_dataType->DataType::latticeLEQ(other)
                   && m_dataType->latticeLEQ(*static_cast<DataTypeVar const&>(other).m_dataType);
        }

    private: /* Fields: */
        DataType *m_dataType;
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

        virtual bool latticeLEQ(const DataType &) const {
            assert (false && "We don't define lattice structure on function types yet!");
        }

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

        virtual bool latticeLEQ(const DataType &) const {
            assert (false && "We don't define lattice structure on function types yet!");
        }

    private: /* Fields: */
        const DataType *m_ret;
};

inline SecrecSecType DataType::secrecSecType() const {
    switch (m_kind) {
        case BASIC:
            assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
            return static_cast<const DataTypeBasic*>(this)->secType();
        case VAR:
            assert(dynamic_cast<const DataTypeVar*>(this) != 0);
            return static_cast<const DataTypeVar*>(this)->dataType().secrecSecType();
        case PROCEDURE:
            assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
            return static_cast<const DataTypeProcedure*>(this)->returnType().secrecSecType();
        case PROCEDUREVOID:
        default:
            return SECTYPE_INVALID;
    }
}

inline SecrecDataType DataType::secrecDataType() const {
    switch (m_kind) {
        case BASIC:
            assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
            return static_cast<const DataTypeBasic*>(this)->dataType();
        case VAR:
            assert(dynamic_cast<const DataTypeVar*>(this) != 0);
            return static_cast<const DataTypeVar*>(this)->dataType().secrecDataType();
        case PROCEDURE:
            assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
            return static_cast<const DataTypeProcedure*>(this)->returnType().secrecDataType();
        case PROCEDUREVOID:
        default:
            return DATATYPE_INVALID;
    }
}

inline SecrecDimType DataType::secrecDimType() const {
    switch (m_kind) {
        case BASIC:
            assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
            return static_cast<const DataTypeBasic*>(this)->dimType();
        case VAR:
            assert(dynamic_cast<const DataTypeVar*>(this) != 0);
            return static_cast<const DataTypeVar*>(this)->dataType().secrecDimType();
        case PROCEDURE:
            assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
            return static_cast<const DataTypeProcedure*>(this)->returnType().secrecDimType();
        case PROCEDUREVOID:
        default:
            return 0; // or maybe -1 for invalid dimensionalities?
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

        // \todo i dislike that those 3 functions are also defined in DataType class
        inline SecrecSecType secrecSecType() const;
        inline SecrecDataType secrecDataType() const;
        inline SecrecDimType secrecDimType() const;

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
            BASIC,        /**< DataTypeBasic.         */
            VAR,          /**< DataTypeVar.           */
            PROCEDURE,    /**< DataTypeProcedure.     */
            PROCEDUREVOID /**< DataTypeProcedureVoid. */
        };

    public: /* Methods: */

        TypeNonVoid(SecrecSecType secType, SecrecDataType dataType, SecrecDimType dimType = 0)
            : Type(false), m_kind(BASIC),
            m_dataType(new DataTypeBasic(secType, dataType, dimType)) {}
        TypeNonVoid(const DataTypeBasic &dataType)
            : Type(false), m_kind(BASIC), m_dataType(dataType.clone()) {}
        TypeNonVoid(const DataTypeVar &dataType)
            : Type(false), m_kind(VAR), m_dataType(dataType.clone()) {}
        TypeNonVoid(const DataTypeProcedure &dataType)
            : Type(false), m_kind(PROCEDURE), m_dataType(dataType.clone()) {}
        TypeNonVoid(const DataTypeProcedureVoid &dataType)
            : Type(false), m_kind(PROCEDUREVOID), m_dataType(dataType.clone()) {}
        TypeNonVoid(const DataType &dataType);

        explicit inline TypeNonVoid(const TypeNonVoid &copy)
            : Type(copy), m_kind(copy.m_kind),
              m_dataType(copy.m_dataType->clone()) {}
        virtual ~TypeNonVoid();

        inline Kind kind() const { return m_kind; }
        inline const DataType &dataType() const { return *m_dataType; }

        virtual inline Type *clone() const { return new TypeNonVoid(*this); }
        virtual std::string toString() const;
        virtual inline bool operator==(const Type &other) const {
            return Type::operator==(other)
                   && m_kind == static_cast<const TypeNonVoid&>(other).kind()
                   && *m_dataType == *static_cast<const TypeNonVoid&>(other).m_dataType;
        }
        virtual inline bool canAssign(const Type &other) const {
            if (other.isVoid()) return false;
            assert(dynamic_cast<const TypeNonVoid*>(&other) != 0);
            const TypeNonVoid &o = static_cast<const TypeNonVoid&>(other);
            return m_dataType->canAssign(*(o.m_dataType));
        }

    private: /* Fields: */
        Kind      m_kind;
        DataType *m_dataType;
};

inline SecrecSecType Type::secrecSecType() const {
    if (isVoid()) return SECTYPE_INVALID;
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    return static_cast<const TypeNonVoid&>(*this).dataType().secrecSecType();
}

inline SecrecDataType Type::secrecDataType() const {
    if (isVoid()) return DATATYPE_INVALID;
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    return static_cast<const TypeNonVoid&>(*this).dataType().secrecDataType();
}

inline SecrecDimType Type::secrecDimType() const {
    assert(dynamic_cast<const TypeNonVoid*>(this) != 0);
    return static_cast<const TypeNonVoid&>(*this).dataType().secrecDimType();
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


#endif // SECREC_H
