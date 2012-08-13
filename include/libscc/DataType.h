/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_DATATYPE_H
#define SECREC_DATATYPE_H

#include "parser.h"
#include "SecurityType.h"

#include <cassert>
#include <string>
#include <vector>

namespace SecreC {

class Context;

SecrecDataType upperDataType (SecrecDataType a, SecrecDataType b);
SecrecDimType upperDimType (SecrecDimType n, SecrecDimType m);

bool latticeDataTypeLEQ (SecrecDataType a, SecrecDataType b);
bool latticeDimTypeLEQ (SecrecDimType n, SecrecDimType m);
bool latticeExplicitLEQ (SecrecDataType a, SecrecDataType b);

bool isNumericDataType (SecrecDataType dType);
bool isXorDataType (SecrecDataType dType);
bool isSignedNumericDataType (SecrecDataType dType);
bool isUnsignedNumericDataType (SecrecDataType dType);

SecrecDataType dtypeDeclassify (SecrecDataType dtype);

/*******************************************************************************
  DataType
*******************************************************************************/

class DataType {
private:
    DataType (const DataType&); // DO NOT IMPLEMENT
    void operator = (const DataType&); // DO NOT IMPLEMENT

public: /* Types: */
    enum Kind { BASIC, VAR, PROCEDURE, PROCEDUREVOID };

public: /* Methods: */

    explicit DataType(Kind kind)
        : m_kind(kind) {}

    virtual inline ~DataType() {}

    inline Kind kind() const { return m_kind; }
    inline SecurityType* secrecSecType() const;
    inline SecrecDataType secrecDataType() const;
    inline SecrecDimType secrecDimType() const;

    virtual std::string toString() const = 0;

    virtual inline bool canAssign(const DataType*) const {
        return false;
    }

    /**
     * We define less-than-equal relation on data types to check
     * if a kind of type can be converted into other. This is used to
     * convert public data to private and for interpreting scalar values
     * as arbitrary dimensional arrays.
     */
    virtual inline bool latticeLEQ(const DataType* other) const {
        return m_kind == other->m_kind;
    }

private: /* Fields: */
    Kind const m_kind;
};

inline std::ostream &operator<<(std::ostream &out, const DataType &type) {
    out << type.toString();
    return out;
}

/*******************************************************************************
  DataTypeBasic
*******************************************************************************/

class DataTypeBasic: public DataType {
public: /* Methods: */

    explicit DataTypeBasic(SecurityType* secType,
                           SecrecDataType dataType,
                           SecrecDimType dim = 0)
        : DataType(DataType::BASIC)
        , m_secType(secType)
        , m_dataType(dataType)
        , m_dimType(dim)
    { }

    virtual ~DataTypeBasic () { }

    inline SecurityType* secType() const { return m_secType; }
    inline SecrecDataType dataType() const { return m_dataType; }
    inline SecrecDimType dimType() const { return m_dimType; }

    virtual std::string toString() const;

    virtual bool latticeLEQ(const DataType* _other) const {
        const DataTypeBasic* other = static_cast<const DataTypeBasic*>(_other);
        if (! DataType::latticeLEQ (other)) {
            return false;
        }

        SecrecDataType dataType = other->m_dataType;
        if (other->m_secType->isPrivate () && m_secType->isPublic ()) {
            dataType = dtypeDeclassify (dataType);
        }

        return     latticeSecTypeLEQ(m_secType, other->m_secType)
                && latticeDataTypeLEQ(m_dataType, dataType)
                && latticeDimTypeLEQ(m_dimType, other->m_dimType);
    }

    static DataTypeBasic* get (Context& cxt,
                               SecrecDataType dataType,
                               SecrecDimType dim = 0);

    static DataTypeBasic* get (Context& cxt,
                               SecurityType* secType,
                               SecrecDataType dataType,
                               SecrecDimType dim = 0);

private: /* Fields: */
    SecurityType*   const m_secType;
    SecrecDataType  const m_dataType;
    SecrecDimType   const m_dimType;
};

/*******************************************************************************
  DataTypeVar
*******************************************************************************/

class DataTypeVar: public DataType {
public: /* Methods: */

    explicit DataTypeVar(DataType* dataType)
        : DataType(DataType::VAR), m_dataType(dataType) {}

    virtual inline ~DataTypeVar() { }

    DataType* dataType() const { return m_dataType; }

    virtual std::string toString() const;
    virtual inline bool canAssign(const DataType* other) const {
        return other->latticeLEQ(m_dataType);
    }

    virtual inline bool latticeLEQ(const DataType* other) const {
        return
            m_dataType->DataType::latticeLEQ(other) &&
            m_dataType->latticeLEQ(static_cast<const DataTypeVar*>(other)->m_dataType);
    }

    static DataTypeVar* get (Context& cxt, DataType* base);


private: /* Fields: */
    DataType* const m_dataType;
};

/*******************************************************************************
  DataTypeProcedureVoid
*******************************************************************************/

class DataTypeProcedureVoid: public DataType {
public: /* Methods: */
    explicit DataTypeProcedureVoid(const std::vector<DataType*>& params,
                                   DataType::Kind kind = PROCEDUREVOID)
        : DataType(kind)
        , m_params (params) {}
    explicit DataTypeProcedureVoid(DataType::Kind kind = PROCEDUREVOID)
        : DataType(kind) {}
    virtual ~DataTypeProcedureVoid() { }

    virtual std::string toString() const;
    std::string mangle() const;

    inline const std::vector<DataType*> &paramTypes() const { return m_params; }

    virtual bool latticeLEQ(const DataType*) const {
        assert (false && "We don't define lattice structure on function types yet!");
        return false;
    }

    static DataTypeProcedureVoid* get (Context& cxt, const std::vector<DataType*>& params);
    static DataTypeProcedureVoid* get (Context& cxt);

private: /* Fields: */
    std::vector<DataType*> const m_params;
};

/*******************************************************************************
  DataTypeProcedure
*******************************************************************************/

class DataTypeProcedure: public DataTypeProcedureVoid {
public: /* Methods: */

    explicit DataTypeProcedure(const std::vector<DataType*>& params, DataType* returnType)
        : DataTypeProcedureVoid(params, PROCEDURE)
        , m_ret(returnType)
    { }

    explicit DataTypeProcedure(DataType* returnType)
        : DataTypeProcedureVoid(PROCEDURE)
        , m_ret(returnType)
    { }

    virtual inline ~DataTypeProcedure() { }

    virtual std::string toString() const;

    inline DataType* returnType() const { return m_ret; }

    virtual inline bool canAssign(const DataType* other) const {
        return other->latticeLEQ(m_ret);
    }

    virtual bool latticeLEQ(const DataType*) const {
        assert (false && "We don't define lattice structure on function types yet!");
        return false;
    }

    static DataTypeProcedure* get (Context& cxt, const std::vector<DataType*>& params, DataType* returnType);
    static DataTypeProcedure* get (Context& cxt, DataTypeProcedureVoid* params, DataType* returnType);


private: /* Fields: */
    DataType* m_ret;
};

inline SecurityType* DataType::secrecSecType() const {
    switch (m_kind) {
    case BASIC:
        assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
        return static_cast<const DataTypeBasic*>(this)->secType();
    case VAR:
        assert(dynamic_cast<const DataTypeVar*>(this) != 0);
        return static_cast<const DataTypeVar*>(this)->dataType()->secrecSecType();
    case PROCEDURE:
        assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
        return static_cast<const DataTypeProcedure*>(this)->returnType()->secrecSecType();
    case PROCEDUREVOID:
    default:
        assert (false && "Invalid security type!");
        return 0;
    }
}

inline SecrecDataType DataType::secrecDataType() const {
    switch (m_kind) {
        case BASIC:
            assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
            return static_cast<const DataTypeBasic*>(this)->dataType();
        case VAR:
            assert(dynamic_cast<const DataTypeVar*>(this) != 0);
            return static_cast<const DataTypeVar*>(this)->dataType()->secrecDataType();
        case PROCEDURE:
            assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
            return static_cast<const DataTypeProcedure*>(this)->returnType()->secrecDataType();
        case PROCEDUREVOID:
        default:
            return DATATYPE_UNDEFINED;
    }
}

inline SecrecDimType DataType::secrecDimType() const {
    switch (m_kind) {
        case BASIC:
            assert(dynamic_cast<const DataTypeBasic*>(this) != 0);
            return static_cast<const DataTypeBasic*>(this)->dimType();
        case VAR:
            assert(dynamic_cast<const DataTypeVar*>(this) != 0);
            return static_cast<const DataTypeVar*>(this)->dataType()->secrecDimType();
        case PROCEDURE:
            assert(dynamic_cast<const DataTypeProcedure*>(this) != 0);
            return static_cast<const DataTypeProcedure*>(this)->returnType()->secrecDimType();
        case PROCEDUREVOID:
        default:
            return -1;
    }
}

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecrecDataType& type);

#endif // SECREC_DATATYPE_H
