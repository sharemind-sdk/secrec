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

#ifndef SECREC_DATATYPE_STRUCT_H
#define SECREC_DATATYPE_STRUCT_H

#include "TypeArgument.h"
#include "DataType.h"

namespace SecreC {

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

#endif // SECREC_DATATYPE_STRUCT_H
