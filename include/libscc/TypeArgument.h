/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_TYPE_ARGUMENT_H
#define SECREC_TYPE_ARGUMENT_H

#include "ParserEnums.h"

#include <cassert>

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
    TA_UNDEF,
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

    TypeArgument (SecurityType* secType)
        : m_kind (TA_SEC)
        , un_secType (secType)
    { }

    TypeArgument (DataType* dataType)
        : m_kind (TA_DATA)
        , un_dataType (dataType)
    { }

    SecrecDimType dimType () const {
        assert (m_kind == TA_DIM);
        return un_dimType;
    }

    SecurityType* secType () const {
        assert (m_kind == TA_SEC);
        return un_secType;
    }

    DataType* dataType () const {
        assert (m_kind == TA_DATA);
        return un_dataType;
    }

    TypeArgumentKind kind () const { return m_kind; }

    SymbolTypeVariable* bind (StringRef name) const;

    friend bool operator == (const TypeArgument& a, const TypeArgument& b);
    friend bool operator < (const TypeArgument& a, const TypeArgument& b);

private: /* Fields: */
    TypeArgumentKind m_kind;
    union {
        SecrecDimType  un_dimType;
        SecurityType*  un_secType;
        DataType*      un_dataType;
    };
};

inline bool operator == (const TypeArgument& a, const TypeArgument& b) {
    assert (a.m_kind != TA_UNDEF);
    assert (b.m_kind != TA_UNDEF);

    if (a.m_kind != b.m_kind)
        return false;

    switch (a.m_kind) {
    case TA_DIM:   return a.un_dimType  == b.un_dimType;
    case TA_SEC:   return a.un_secType  == b.un_secType;
    case TA_DATA:  return a.un_dataType == b.un_dataType;
    case TA_UNDEF: return false;
    }
}

inline bool operator != (const TypeArgument& a, const TypeArgument& b) {
    return !(a == b);
}

inline bool operator < (const TypeArgument& a, const TypeArgument& b) {
    assert (a.m_kind != TA_UNDEF);
    assert (b.m_kind != TA_UNDEF);

    if (a.m_kind < b.m_kind) return true;
    if (a.m_kind > b.m_kind) return false;
    switch (a.m_kind) {
    case TA_DIM:   return a.un_dimType  < b.un_dimType;
    case TA_SEC:   return a.un_secType  < b.un_secType;
    case TA_DATA:  return a.un_dataType < b.un_dataType;
    case TA_UNDEF: return false;
    }
}

TypeArgumentKind quantifierKind (const TreeNodeQuantifier& quant);

} // namespace SecreC;

#endif // SECREC_TYPE_ARGUMENT_H
