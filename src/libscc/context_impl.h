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

namespace SecreC {

class ContextImpl {
private:

    ContextImpl (const ContextImpl&); // DO NOT IMPLEMENT
    void operator = (const ContextImpl&); // DO NOT IMPLEMENT

public: /* Methods: */

    ContextImpl ()
        : m_trueConstant (0)
        , m_falseConstant (0)
    { }

    ~ContextImpl ();

    /* Security types: */
    PublicSecType* publicType ();
    PrivateSecType* privateType (const std::string& domain,
                                 SymbolKind* kind);

    /* Data types: */
    DataTypeVar* varType (DataType* dtype);
    DataTypeProcedureVoid* voidProcedureType (
            const std::vector<DataType*>& params);
    DataTypeProcedure* procedureType (
            const std::vector<DataType*>& params,
            DataType* ret);
    DataTypeBasic* basicDataType (SecurityType* secTy,
                                  SecrecDataType dataType,
                                  SecrecDimType dim = 0);

    /* Types: */
    TypeVoid* voidType ();
    TypeNonVoid* nonVoidType (DataType* dType);

public: /* Fields: */

    /* All types: */
    TypeVoid m_voidType;
    PublicSecType m_pubSecType;
    std::map<std::string, PrivateSecType*> m_privSecTypes;
    std::map<DataType*, DataTypeVar*> m_varTypes;
    std::map<DataType*, TypeNonVoid*> m_nonVoidTypes;
    std::map<std::vector<DataType*>, DataTypeProcedureVoid* > m_voidProcTypes;
    std::map<std::pair<DataTypeProcedureVoid*, DataType*>, DataTypeProcedure*> m_procTypes;
    std::map<boost::tuple<SecurityType*, SecrecDataType, SecrecDimType >, DataTypeBasic*> m_basicTypes;

    /* All constants: */
    ConstantBool* m_trueConstant;
    ConstantBool* m_falseConstant;
    std::map<std::string, ConstantString* > m_stringLiterals;
    std::map<std::pair<SecrecDataType, uint64_t>, Symbol*>  m_numericConstants;
};

} // namespace SecreC

#endif // CONTEXT_IMPL_H
