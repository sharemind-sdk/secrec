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

#ifndef SECREC_TYPE_ARGUMENT_H
#define SECREC_TYPE_ARGUMENT_H

#include "ParserEnums.h"

#include <cassert>
#include <iosfwd>


namespace SecreC {

class DataType;
class SecurityType;
class StringRef;
class SymbolTypeVariable;
class TreeNodeQuantifier;

/*******************************************************************************
  TypeArgumentKind
*******************************************************************************/

enum TypeArgumentKind {
    TA_SEC,
    TA_DATA,
    TA_DIM
};

/*******************************************************************************
  TypeArgument
*******************************************************************************/

// Algebraic data types would be nice...
class TypeArgument {
public: /* Methods: */

    TypeArgument (SecrecDimType dimType)
        : m_kind (TA_DIM)
        , un_dimType (dimType)
    { }

    TypeArgument (const SecurityType* secType)
        : m_kind (TA_SEC)
        , un_secType (secType)
    { }

    TypeArgument (const DataType* dataType)
        : m_kind (TA_DATA)
        , un_dataType (dataType)
    { }

    SecrecDimType dimType () const {
        assert (m_kind == TA_DIM);
        return un_dimType;
    }

    const SecurityType* secType () const {
        assert (m_kind == TA_SEC);
        return un_secType;
    }

    const DataType* dataType () const {
        assert (m_kind == TA_DATA);
        return un_dataType;
    }

    // I am aware that we can actually just directly compare against the union
    // without bothering to check the kinds. For the sake of not horribly
    // violating the C++ standard I'm going for the current implementation.

    bool equals (SecrecDimType dimType) const {
        if (m_kind != TA_DIM)
            return false;
        return un_dimType == dimType;
    }

    bool equals (const SecurityType* secType) const {
        if (m_kind != TA_SEC)
            return false;
        return un_secType == secType;
    }

    bool equals (SecrecDataType dataType) const;

    bool equals (const DataType* dataType) const {
        if (m_kind != TA_DATA)
            return false;
        return un_dataType == dataType;
    }

    bool isDataType () const { return m_kind == TA_DATA; }
    bool isDimType () const { return m_kind == TA_DIM; }
    bool isSecType () const { return m_kind == TA_SEC; }

    TypeArgumentKind kind () const { return m_kind; }

    SymbolTypeVariable* bind (StringRef name) const;

    friend bool operator == (const TypeArgument& a, const TypeArgument& b);
    friend bool operator < (const TypeArgument& a, const TypeArgument& b);
    friend std::ostream& operator << (std::ostream& os, const TypeArgument& a);

private: /* Fields: */
    TypeArgumentKind m_kind;
    union {
        SecrecDimType       un_dimType;
        const SecurityType* un_secType;
        const DataType*     un_dataType;
    };
};

bool operator == (const TypeArgument& a, const TypeArgument& b);

inline bool operator != (const TypeArgument& a, const TypeArgument& b) {
    return !(a == b);
}

bool operator<(TypeArgument const & a, TypeArgument const & b);

std::ostream& operator << (std::ostream& os, const TypeArgument& a);

TypeArgumentKind quantifierKind (const TreeNodeQuantifier& quant);

} // namespace SecreC;

#endif // SECREC_TYPE_ARGUMENT_H
