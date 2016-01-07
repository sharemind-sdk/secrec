/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#ifndef SECREC_DATATYPE_H
#define SECREC_DATATYPE_H

#include "ParserEnums.h"
#include "StringRef.h"
#include "TypeArgument.h"

#include <iosfwd>
#include <vector>
#include <utility>

namespace SecreC {

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
unsigned widthInBitsDataType (SecrecDataType dType);

SecrecDataType dtypeDeclassify (SecrecDataType dtype);

bool latticeDataTypeLEQ (const DataType* a, const DataType* b);
bool latticeExplicitLEQ (const DataType* a, const DataType* b);

bool isFloatingDataType (const DataType* dType);
bool isNumericDataType (const DataType* dType);
bool isXorDataType (const DataType* dType);
bool isSignedNumericDataType (const DataType* dType);
bool isUnsignedNumericDataType (const DataType* dType);

const DataType* dtypeDeclassify (const DataType* dtype);
const DataType* upperDataType (const DataType* a, const DataType* b);


/*******************************************************************************
  DataType
*******************************************************************************/

class DataType {
protected: /* Types: */

    enum Kind { COMPOSITE, PRIMITIVE };

public: /* Methods: */

    DataType (const DataType&) = delete;
    DataType& operator = (const DataType&) = delete;
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

    static const DataTypePrimitive* get (SecrecDataType dataType);
    SecrecDataType secrecDataType () const { return m_dataType; }

protected:
    void print (std::ostream& os) const override final;

private: /* Fields: */
    const SecrecDataType m_dataType;
};

/*******************************************************************************
  DataTypeStruct
*******************************************************************************/

class DataTypeStruct : public DataType {
public: /* Types: */

    struct Field {
        const TypeBasic* type;
        StringRef  name;

        Field (const TypeBasic* type, StringRef name)
            : type (type)
            , name (std::move(name))
        { }
    };

    using FieldList = std::vector<Field>;
    using TypeArgumentList = std::vector<TypeArgument>;

public: /* Methods: */

    StringRef name () const { return m_name; }

    static const DataTypeStruct* get (StringRef name,
        const FieldList& fields,
        const TypeArgumentList& typeArgs = TypeArgumentList());

    const FieldList& fields () const { return m_fields; }
    const TypeArgumentList& typeArgs () const { return m_typeArgs; }

protected:

    void print (std::ostream& os) const override final;

    explicit DataTypeStruct (StringRef name, TypeArgumentList typeArgs, FieldList fields)
        : DataType (COMPOSITE)
        , m_name (std::move(name))
        , m_typeArgs (std::move (typeArgs))
        , m_fields (std::move(fields))
    { }

private: /* Fields: */
    const StringRef        m_name;
    const TypeArgumentList m_typeArgs;
    const FieldList        m_fields;
};

inline DataTypeStruct::Field make_field (const TypeBasic* type, StringRef name) {
    return DataTypeStruct::Field(type, name);
}

} // namespace SecreC

#endif // SECREC_DATATYPE_H
