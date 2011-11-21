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

namespace {
template <typename Key, typename T >
void eraseAll (const std::map<Key, T*>& m) {
    typedef std::pair<Key, T*> ElemTy;
    BOOST_FOREACH (const ElemTy& p, m) {
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

PrivateSecType* ContextImpl::privateType (SymbolDomain* domain) {
    std::map<SymbolDomain*, PrivateSecType*>::iterator
        i = m_privSecTypes.find (domain);
    if (i == m_privSecTypes.end ()) {
        i = m_privSecTypes.insert (i,
            std::make_pair (domain, new PrivateSecType (domain)));
    }

    return i->second;
}

/* Data types: */
DataTypeVar* ContextImpl::varType (DataType* dtype) {
    std::map<DataType*, DataTypeVar*>::iterator
        i = m_varTypes.find (dtype);
    if (i == m_varTypes.end ()) {
        i = m_varTypes.insert (i,
            std::make_pair (dtype, new DataTypeVar (dtype)));
    }

    return i->second;
}

DataTypeProcedureVoid* ContextImpl::voidProcedureType (const std::vector<DataType*>& params) {
    std::map<std::vector<DataType*>, DataTypeProcedureVoid* >::iterator
        i = m_voidProcTypes.find (params);
    if (i == m_voidProcTypes.end ()) {
        i = m_voidProcTypes.insert (i,
            std::make_pair (params, new DataTypeProcedureVoid (params)));
    }

    return i->second;
}

DataTypeProcedure* ContextImpl::procedureType (const std::vector<DataType*>& params, DataType* ret) {
    DataTypeProcedureVoid* voidProc = voidProcedureType (params);
    const std::pair<DataTypeProcedureVoid*, DataType*> index (voidProc, ret);
    std::map<std::pair<DataTypeProcedureVoid*, DataType*>, DataTypeProcedure*>::iterator
        i = m_procTypes.find (index);
    if (i == m_procTypes.end ()) {
        i = m_procTypes.insert (i,
            std::make_pair (index, new DataTypeProcedure (params, ret)));
    }

    return i->second;
}

DataTypeBasic* ContextImpl::basicDataType (SecurityType* secTy,
                                       SecrecDataType dataType,
                                       SecrecDimType dim)
{
    const boost::tuple<SecurityType*, SecrecDataType, SecrecDimType > index (secTy, dataType, dim);
    std::map<boost::tuple<SecurityType*, SecrecDataType, SecrecDimType >, DataTypeBasic*>::iterator
        i = m_basicTypes.find (index);
    if (i == m_basicTypes.end ()) {
        i = m_basicTypes.insert (i,
            std::make_pair (index, new DataTypeBasic (secTy, dataType, dim)));
    }

    return i->second;
}

/* Types: */
TypeVoid* ContextImpl::voidType () {
    return &m_voidType;
}

TypeNonVoid* ContextImpl::nonVoidType (DataType* dType) {
    std::map<DataType*, TypeNonVoid*>::iterator
        i = m_nonVoidTypes.find (dType);
    if (i == m_nonVoidTypes.end ()) {
        i = m_nonVoidTypes.insert (i,
            std::make_pair (dType, new TypeNonVoid (dType)));
    }

    return i->second;
}


}
