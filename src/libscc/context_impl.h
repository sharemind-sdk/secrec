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
    typedef std::map<DataType*, DataTypeVar*> DataTypeVarMap;
    typedef std::map<DataType*, TypeNonVoid*> TypeNonVoidMap;
    typedef std::map<std::vector<DataType*>, DataTypeProcedureVoid* > DataTypeProcedureVoidMap;
    typedef std::map<std::pair<DataTypeProcedureVoid*, DataType*>, DataTypeProcedure*> DataTypeProcedureMap;
    typedef std::map<boost::tuple<SecurityType*, SecrecDataType, SecrecDimType >, DataTypeBasic*> DataTypeBasicMap;

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

    /* Data types: */
    DataTypeVar* varType (DataType* dtype);
    DataTypeProcedureVoid* voidProcedureType (const std::vector<DataType*>& params);
    DataTypeProcedure* procedureType (const std::vector<DataType*>& params, DataType* ret);
    DataTypeBasic* basicDataType (SecurityType* secTy, SecrecDataType dataType, SecrecDimType dim = 0);

    /* Types: */
    TypeVoid* voidType ();
    TypeNonVoid* nonVoidType (DataType* dType);

public: /* Fields: */

    /* Strings: */
    StringTable               m_stringTable;

    /* All types: */
    TypeVoid                  m_voidType;
    PublicSecType             m_pubSecType;
    PrivateSecTypeMap         m_privSecTypes;
    DataTypeVarMap            m_varTypes;
    TypeNonVoidMap            m_nonVoidTypes;
    DataTypeProcedureVoidMap  m_voidProcTypes;
    DataTypeProcedureMap      m_procTypes;
    DataTypeBasicMap          m_basicTypes;

    /* All constants: */
    ConstantStringMap         m_stringLiterals;
    NumericConstantMap        m_numericConstants[2]; ///< 0 - unsigned, 1 - signed
    FloatConstantMap          m_floatConstants;
};

} // namespace SecreC

#endif // CONTEXT_IMPL_H
