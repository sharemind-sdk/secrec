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

#include "ParserEnums.h"
#include "Types.h"
#include "Constant.h"
#include "StringTable.h"
#include "TypeArgument.h"
#include "SecurityType.h"

#include <map>
#include <tuple>

namespace SecreC {

class DataType;
class DataTypePrimitive;
class DataTypeStruct;
class PrivateSecType;
class PublicSecType;

class ContextImpl {
private:

    ContextImpl (const ContextImpl&) = delete;
    ContextImpl& operator = (const ContextImpl&) = delete;

public: /* Types: */

    using PrivateSecTypeMap = std::map<StringRef, PrivateSecType*>;
    using TypeProcMap = std::map<std::pair<Type*, std::vector<TypeBasic*> >, TypeProc*>;
    using TypeBasicMap = std::map<std::tuple<SecurityType*, DataType*, SecrecDimType>, TypeBasic*>;

    using ConstantStringMap = std::map<StringRef, ConstantString*>;
    using NumericConstantMap = std::map<APInt, ConstantInt*>;
    using FloatConstantMap = std::map<APFloat, ConstantFloat*, APFloat::BitwiseCmp>;

    using StructTypeMap = std::map<std::pair<StringRef, std::vector<TypeArgument> >, DataTypeStruct*>;
    using PrimitiveTypeMap = std::map<SecrecDataType, DataTypePrimitive*>;

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
    PrimitiveTypeMap      m_primitiveTypes;
    StructTypeMap         m_structTypes;

    /* All constants: */
    ConstantStringMap     m_stringLiterals;
    NumericConstantMap    m_numericConstants[2]; ///< 0 - unsigned, 1 - signed
    FloatConstantMap      m_floatConstants;
};

} // namespace SecreC

#endif // CONTEXT_IMPL_H
