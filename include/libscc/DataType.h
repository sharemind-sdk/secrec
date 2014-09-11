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

#include "ParserEnums.h"
#include "StringRef.h"
#include "TypeArgument.h"

#include <iosfwd>
#include <vector>

namespace SecreC {

class Context;
class TypeBasic;
class DataType;

/*******************************************************************************
 SecrecDataType related operations
*******************************************************************************/

SecrecDataType upperDataType (SecrecDataType a, SecrecDataType b);
SecrecDimType upperDimType (SecrecDimType n, SecrecDimType m);

bool latticeDataTypeLEQ (SecrecDataType a, SecrecDataType b);
bool latticeDimTypeLEQ (SecrecDimType n, SecrecDimType m);
bool latticeExplicitLEQ (SecrecDataType a, SecrecDataType b);

bool isFloatingDataType (SecrecDataType dType);
bool isNumericDataType (SecrecDataType dType);
bool isXorDataType (SecrecDataType dType);
bool isSignedNumericDataType (SecrecDataType dType);
bool isUnsignedNumericDataType (SecrecDataType dType);

SecrecDataType dtypeDeclassify (SecrecDataType dtype);

bool latticeDataTypeLEQ (const DataType* a, const DataType* b);
bool latticeExplicitLEQ (const DataType* a, const DataType* b);

bool isFloatingDataType (const DataType* dType);
bool isNumericDataType (const DataType* dType);
bool isXorDataType (const DataType* dType);
bool isSignedNumericDataType (const DataType* dType);
bool isUnsignedNumericDataType (const DataType* dType);

DataType* dtypeDeclassify (Context& cxt, DataType* dtype);
DataType* upperDataType (Context& cxt, DataType* a, DataType* b);


/*******************************************************************************
  DataType
*******************************************************************************/

class DataType {
private:
    DataType (const DataType&) = delete;
    DataType& operator = (const DataType&) = delete;

protected: /* Types: */

    enum Kind { COMPOSITE, PRIMITIVE };

public: /* Methods: */

    virtual ~DataType () { }

    bool isComposite () const { return m_kind == COMPOSITE; }
    bool isPrimitive () const { return m_kind == PRIMITIVE; }

    bool isString () const { return equals (DATATYPE_STRING); }
    bool isBool () const { return equals (DATATYPE_BOOL); }

    bool equals (const DataType* other) const { return this == other; }
    bool equals (SecrecDataType other) const;

protected:

    virtual void print (std::ostream& os) const = 0;
    friend std::ostream& operator<<(std::ostream &out, const DataType& type);

    explicit DataType (Kind kind)
        : m_kind (kind)
    { }

private: /* Fields: */
    const Kind m_kind;
};

inline std::ostream& operator<<(std::ostream &out, const DataType& type) {
    type.print (out);
    return out;
}

/*******************************************************************************
  DataTypePrimitive
*******************************************************************************/

class DataTypePrimitive : public DataType {
public: /* Methods: */

    explicit DataTypePrimitive (SecrecDataType dataType)
        : DataType (PRIMITIVE)
        , m_dataType (dataType)
    { }

    static DataTypePrimitive* get (Context& cxt, SecrecDataType dataType);
    SecrecDataType secrecDataType () const { return m_dataType; }

protected:
    void print (std::ostream& os) const;

private: /* Fields: */
    const SecrecDataType m_dataType;
};

/*******************************************************************************
  DataTypeStruct
*******************************************************************************/

class DataTypeStruct : public DataType {
public: /* Types: */

    struct Field {
        TypeBasic* type;
        StringRef  name;

        Field (TypeBasic* type, StringRef name)
            : type (type)
            , name (name)
        { }
    };

    typedef std::vector<Field> FieldList;
    typedef std::vector<TypeArgument> TypeArgumentList;

public: /* Methods: */

    StringRef name () const { return m_name; }
    static DataTypeStruct* find (Context& cxt, StringRef name,
        const TypeArgumentList& typeArgs = TypeArgumentList());
    static DataTypeStruct* get (Context& cxt, StringRef name,
        const FieldList& fields,
        const TypeArgumentList& typeArgs = TypeArgumentList());
    const std::vector<DataTypeStruct::Field>& fields () const { return m_fields; }

protected:
    void print (std::ostream& os) const;

    explicit DataTypeStruct (StringRef name, const std::vector<Field>& fields)
        : DataType (COMPOSITE)
        , m_name (name)
        , m_fields (fields)
    { }

private: /* Fields: */
    const StringRef          m_name;
    const std::vector<Field> m_fields;
};

inline DataTypeStruct::Field make_field (TypeBasic* type, StringRef name) {
    return DataTypeStruct::Field(type, name);
}

} // namespace SecreC

#endif // SECREC_DATATYPE_H
