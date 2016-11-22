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
#include <map>
#include <utility>
#include <vector>

namespace SecreC {

class Context;
class DataType;
class SymbolKind;
class TypeBasic;
class SecurityType;

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
bool isSignedNumericDataType (SecrecDataType dType);
bool isUnsignedNumericDataType (SecrecDataType dType);
unsigned widthInBitsDataType (SecrecDataType dType);

bool latticeDataTypeLEQ (const DataType* a, const DataType* b);
bool latticeExplicitLEQ (const DataType* a, const DataType* b);

bool isFloatingDataType (const DataType* dType);
bool isNumericDataType (const DataType* dType);
bool isSignedNumericDataType (const DataType* dType);
bool isUnsignedNumericDataType (const DataType* dType);

const DataType* dtypeDeclassify (const SecurityType* secType, const DataType* dType);
const DataType* upperDataType (const TypeBasic* a, const TypeBasic* b);


/*******************************************************************************
  DataType
*******************************************************************************/

class DataType {
protected: /* Types: */

    enum Kind { COMPOSITE, BUILTIN_PRIMITIVE, USER_PRIMITIVE };

public: /* Methods: */

    DataType (const DataType&) = delete;
    DataType& operator = (const DataType&) = delete;
    virtual ~DataType () { }

    bool isComposite () const { return m_kind == COMPOSITE; }
    bool isPrimitive () const {
        return m_kind == BUILTIN_PRIMITIVE || m_kind == USER_PRIMITIVE;
    }
    bool isBuiltinPrimitive () const { return m_kind == BUILTIN_PRIMITIVE; }
    bool isUserPrimitive () const { return m_kind == USER_PRIMITIVE; }

    bool isString () const { return equals (DATATYPE_STRING); }
    bool isBool () const { return equals (DATATYPE_BOOL); }

    virtual bool equals (const DataType* other) const { return this == other; };
    virtual bool equals (SecrecDataType other) const;

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
  DataTypeBuiltinPrimitive
*******************************************************************************/

class DataTypeBuiltinPrimitive : public DataType {
public: /* Methods: */

    explicit DataTypeBuiltinPrimitive (SecrecDataType dataType)
        : DataType (BUILTIN_PRIMITIVE)
        , m_dataType (dataType)
    { }

    static const DataTypeBuiltinPrimitive* get (SecrecDataType dataType);
    SecrecDataType secrecDataType () const { return m_dataType; }
    bool equals (const DataType* other) const override final;

protected:
    void print (std::ostream& os) const override final;

private: /* Fields: */
    const SecrecDataType m_dataType;
};

/*******************************************************************************
  DataTypeUserPrimitive
*******************************************************************************/

class DataTypeUserPrimitive : public DataType {

public: /* Methods: */

    explicit DataTypeUserPrimitive (StringRef name)
        : DataType (USER_PRIMITIVE)
        , m_name (name)
    { }

    static const DataTypeUserPrimitive* get (StringRef name);

    StringRef name () const { return m_name; }

    bool equals (SecrecDataType type) const override final;

    bool equals (const DataType* other) const override final;

protected:
    void print (std::ostream& os) const override final;

private: /* Fields: */
    const StringRef m_name;
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
