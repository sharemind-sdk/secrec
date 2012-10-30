/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "context_impl.h"

#include <map>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/foreach.hpp>

#include "types.h"

namespace /* anonymous */ {
template <typename Key, typename T >
void eraseAll (const std::map<Key, T*>& m) {
    typedef const typename std::map<Key, T*>::value_type value_type;
    BOOST_FOREACH (const value_type& p, m) {
        delete p.second;
    }
}
} // anonymous namespace


namespace SecreC {

ContextImpl::~ContextImpl () {
    eraseAll (m_privSecTypes);
    eraseAll (m_varTypes);
    eraseAll (m_nonVoidTypes);
    eraseAll (m_voidProcTypes);
    eraseAll (m_procTypes);
    eraseAll (m_basicTypes);
    delete m_trueConstant;
    delete m_falseConstant;
    eraseAll (m_stringLiterals);
    eraseAll (m_numericConstants);
}

/* Security types: */
PublicSecType* ContextImpl::publicType () {
    return &m_pubSecType;
}

PrivateSecType* ContextImpl::privateType (StringRef name,
                                          SymbolKind* kind)
{
    PrivateSecTypeMap::iterator i = m_privSecTypes.find (name);
    if (i == m_privSecTypes.end ()) {
        i = m_privSecTypes.insert (i,
            PrivateSecTypeMap::value_type (name, new PrivateSecType (name, kind)));
    }

    return i->second;
}

/* Data types: */
DataTypeVar* ContextImpl::varType (DataType* dtype) {
    DataTypeVarMap::iterator i = m_varTypes.find (dtype);
    if (i == m_varTypes.end ()) {
        i = m_varTypes.insert (i,
            DataTypeVarMap::value_type (dtype, new DataTypeVar (dtype)));
    }

    return i->second;
}

DataTypeProcedureVoid* ContextImpl::voidProcedureType (const std::vector<DataType*>& params) {
    DataTypeProcedureVoidMap::iterator i = m_voidProcTypes.find (params);
    if (i == m_voidProcTypes.end ()) {
        i = m_voidProcTypes.insert (i,
            DataTypeProcedureVoidMap::value_type (params, new DataTypeProcedureVoid (params)));
    }

    return i->second;
}

DataTypeProcedure* ContextImpl::procedureType (const std::vector<DataType*>& params, DataType* ret) {
    DataTypeProcedureVoid* voidProc = voidProcedureType (params);
    const DataTypeProcedureMap::key_type index (voidProc, ret);
    DataTypeProcedureMap::iterator i = m_procTypes.find (index);
    if (i == m_procTypes.end ()) {
        i = m_procTypes.insert (i,
            DataTypeProcedureMap::value_type (index, new DataTypeProcedure (params, ret)));
    }

    return i->second;
}

DataTypeBasic* ContextImpl::basicDataType (SecurityType* secTy,
                                           SecrecDataType dataType,
                                           SecrecDimType dim)
{
    const DataTypeBasicMap::key_type index (secTy, dataType, dim);
    DataTypeBasicMap::iterator i = m_basicTypes.find (index);
    if (i == m_basicTypes.end ()) {
        i = m_basicTypes.insert (i,
            DataTypeBasicMap::value_type (index, new DataTypeBasic (secTy, dataType, dim)));
    }

    return i->second;
}

/* Types: */
TypeVoid* ContextImpl::voidType () {
    return &m_voidType;
}

TypeNonVoid* ContextImpl::nonVoidType (DataType* dType) {
    TypeNonVoidMap::iterator i = m_nonVoidTypes.find (dType);
    if (i == m_nonVoidTypes.end ()) {
        i = m_nonVoidTypes.insert (i,
            TypeNonVoidMap::value_type (dType, new TypeNonVoid (dType)));
    }

    return i->second;
}


}
