/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef CONTEXT_IMPL_H
#define CONTEXT_IMPL_H

#include <map>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include "parser.h"
#include "types.h"
#include "constant.h"
#include "StringTable.h"

namespace SecreC {

class ContextImpl {
private:

    ContextImpl (const ContextImpl&); // DO NOT IMPLEMENT
    void operator = (const ContextImpl&); // DO NOT IMPLEMENT

public: /* Types: */

    typedef std::map<StringRef, PrivateSecType*> PrivateSecTypeMap;
    typedef std::map<std::pair<Type*, std::vector<TypeBasic*> >, TypeProc*> TypeProcMap;
    typedef std::map<boost::tuple<SecurityType*, SecrecDataType, SecrecDimType>, TypeBasic*> TypeBasicMap;

    typedef std::map<StringRef, ConstantString*> ConstantStringMap;
    typedef std::map<APInt, ConstantInt*, APInt::BitwiseCmp> NumericConstantMap;
    typedef std::map<APFloat, ConstantFloat*, APFloat::BitwiseCmp> FloatConstantMap;

public: /* Methods: */

    ContextImpl () { }

    ~ContextImpl ();

    StringTable& stringTable () { return m_stringTable; }

    /* Security types: */
    PublicSecType* publicType ();
    PrivateSecType* privateType (StringRef domain, SymbolKind* kind);

public: /* Fields: */

    /* Strings: */
    StringTable           m_stringTable;

    /* All types: */
    TypeVoid              m_voidType;
    TypeProcMap           m_procTypes;
    TypeBasicMap          m_basicTypes;
    PublicSecType         m_pubSecType;
    PrivateSecTypeMap     m_privSecTypes;

    /* All constants: */
    ConstantStringMap     m_stringLiterals;
    NumericConstantMap    m_numericConstants[2]; ///< 0 - unsigned, 1 - signed
    FloatConstantMap      m_floatConstants;
};

} // namespace SecreC

#endif // CONTEXT_IMPL_H
