/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_TYPECONTEXT_H
#define SECREC_TYPECONTEXT_H

#include "parser.h"
#include "types.h"

namespace SecreC {

/*******************************************************************************
  TypeContext
*******************************************************************************/

/**
 * Type context may or may not define security, data, or dimensionality types.
 * We use NULL to represent undefined security type, DATATYPE_UNDEFINED to
 * represent undefined data type, and finally (~ SecrecDimType (0)) for undefined
 * dimensionality types.
 */
class TypeContext {
public: /* Types: */

    class PrettyPrint {
    public: /* Methods: */
        explicit PrettyPrint (const TypeContext& self) : m_self (self) { }
        inline void operator () (std::ostream& os) const { m_self.prettyPrint (os); }

    private: /* Fields: */
        const TypeContext& m_self;
    };

public: /* Methods: */

    TypeContext (SecurityType* secType,
                 SecrecDataType dataType,
                 SecrecDimType dimType)
        : m_contextSecType (secType)
        , m_contextDataType (dataType)
        , m_contextDimType (dimType)
    { }

    TypeContext ()
        : m_contextSecType (0)
        , m_contextDataType (DATATYPE_UNDEFINED)
        , m_contextDimType (~ SecrecDimType (0))
    { }

    TypeContext typeContext () const {
        return *this;
    }

    void setContext (const TypeContext& cxt) {
        m_contextSecType = cxt.m_contextSecType;
        m_contextDataType = cxt.m_contextDataType;
        m_contextDimType = cxt.m_contextDimType;
    }

    void setContext (const TypeContext* cxt) {
        assert (cxt != 0);
        m_contextSecType = cxt->m_contextSecType;
        m_contextDataType = cxt->m_contextDataType;
        m_contextDimType = cxt->m_contextDimType;
    }

    void setContext (TypeNonVoid* ty) {
        assert (ty != 0);
        setContextDataType (ty->secrecDataType ());
        setContextSecType (ty->secrecSecType ());
        setContextDimType (ty->secrecDimType ());
    }

    void setContextIndexType (Context& cxt) {
        setContextDataType (DATATYPE_UINT64);
        setContextDimType (0);
        setContextSecType (PublicSecType::get (cxt));
    }

    void setContextSecType (SecurityType* secTy) {
        m_contextSecType = secTy;
    }

    void setContextDataType (SecrecDataType dataType) {
        m_contextDataType = dataType;
    }

    void setContextDimType (SecrecDimType dimType) {
        m_contextDimType = dimType;
    }

    SecurityType* contextSecType () const {
        return m_contextSecType;
    }

    SecrecDataType contextDataType () const {
        return m_contextDataType;
    }

    SecrecDimType contextDimType () const {
        return m_contextDimType;
    }

    bool haveContextSecType () const {
        return m_contextSecType != 0;
    }

    bool haveContextDataType () const {
        return m_contextDataType != DATATYPE_UNDEFINED;
    }

    bool haveContextDimType () const {
        return m_contextDimType != ~ SecrecDimType (0);
    }

    bool matchType (TypeNonVoid* type) const {
        return matchSecType (type->secrecSecType ()) &&
            matchDataType (type->secrecDataType ()) &&
            matchDimType (type->secrecDimType ());
    }

    bool matchSecType (SecurityType* secType) const {
        assert (secType != 0);
        if (! haveContextSecType ()) {
            return true;
        }

        return secType == contextSecType ();
    }

    bool matchDataType (SecrecDataType dataType) const {
        assert (dataType != DATATYPE_UNDEFINED);
        if (! haveContextDataType ()) {
            return true;
        }

        return dataType == m_contextDataType;
    }

    bool matchDimType (SecrecDimType dimType) const {
        assert (dimType >= 0);
        if (! haveContextDimType ()) {
            return true;
        }

        return dimType == m_contextDimType;
    }

    void prettyPrint (std::ostream& os) const;

protected: /* Fields: */
    SecurityType*    m_contextSecType;
    SecrecDataType   m_contextDataType;
    SecrecDimType    m_contextDimType;
};

inline std::ostream& operator << (std::ostream& os, const TypeContext::PrettyPrint& pp) {
    pp (os);
    return os;
}

} // namespace SecreC

#endif // SECREC_TYPECONTEXT_H
