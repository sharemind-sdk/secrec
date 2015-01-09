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

#ifndef SECREC_TYPECONTEXT_H
#define SECREC_TYPECONTEXT_H

#include "ParserEnums.h"

#include <cassert>
#include <iosfwd>

namespace SecreC {

class Context;
class DataType;
class SecurityType;
class Type;
class TypeNonVoid;

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
                 DataType* dataType,
                 SecrecDimType dimType)
        : m_contextSecType (secType)
        , m_contextDataType (dataType)
        , m_contextDimType (dimType)
    { }

    TypeContext ()
        : m_contextSecType (nullptr)
        , m_contextDataType (nullptr)
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
        assert (cxt != nullptr);
        m_contextSecType = cxt->m_contextSecType;
        m_contextDataType = cxt->m_contextDataType;
        m_contextDimType = cxt->m_contextDimType;
    }

    void setContext (TypeNonVoid* ty);

    void setContextIndexType (Context& cxt);

    void setContextSecType (SecurityType* secTy) {
        m_contextSecType = secTy;
    }

    void setContextDataType (DataType* dataType) {
        m_contextDataType = dataType;
    }

    void setContextDimType (SecrecDimType dimType) {
        m_contextDimType = dimType;
    }

    SecurityType* contextSecType () const {
        return m_contextSecType;
    }

    DataType* contextDataType () const {
        return m_contextDataType;
    }

    SecrecDimType contextDimType () const {
        return m_contextDimType;
    }

    bool haveContextSecType () const {
        return m_contextSecType != nullptr;
    }

    bool haveContextDataType () const {
        return m_contextDataType != nullptr;
    }

    bool haveContextDimType () const {
        return m_contextDimType != ~ SecrecDimType (0);
    }

    bool matchType (TypeNonVoid* type) const;
    bool matchSecType (SecurityType* secType) const;
    bool matchDataType (DataType* dataType) const;

    bool matchDimType (SecrecDimType dimType) const {
        if (! haveContextDimType ()) {
            return true;
        }

        return dimType == m_contextDimType;
    }

    void prettyPrint (std::ostream& os) const;

protected: /* Fields: */
    SecurityType*    m_contextSecType;
    DataType*        m_contextDataType;
    SecrecDimType    m_contextDimType;
};

inline std::ostream& operator << (std::ostream& os, const TypeContext::PrettyPrint& pp) {
    pp (os);
    return os;
}

} // namespace SecreC

#endif // SECREC_TYPECONTEXT_H
