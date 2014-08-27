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
#include "StringRef.h"

#include <iosfwd>
#include <vector>

namespace SecreC {

class Context;
class TypeBasic;

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

/*******************************************************************************
  DataType
*******************************************************************************/

class DataType {
private:
    DataType (const DataType&); // DO NOT IMPLEMENT
    void operator = (const DataType&); // DO NOT IMPLEMENT

public: /* Methods: */

    virtual ~DataType () { }

    bool isComposite () const { return m_isComposite; }
    bool isPrimitive () const { return !m_isComposite; }

protected:

    virtual void print (std::ostream& os) const = 0;
    friend std::ostream& operator<<(std::ostream &out, const DataType& type);

    explicit DataType (bool isComposite)
        : m_isComposite (isComposite)
    { }

private: /* Fields: */
    const bool m_isComposite;
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
        : DataType (/* isComposite = */ false)
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

public: /* Methods: */

    StringRef name () const { return m_name; }
    static DataTypeStruct* get (Context& cxt, StringRef name, const std::vector<Field>& fields);
    const std::vector<DataTypeStruct::Field>& fields () const { return m_fields; }

protected:
    void print (std::ostream& os) const;

    explicit DataTypeStruct (StringRef name, const std::vector<Field>& fields)
        : DataType (/* isComposite = */ true)
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
