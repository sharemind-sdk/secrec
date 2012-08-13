/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_SECURITYTYPE_H
#define SECREC_SECURITYTYPE_H

#include <string>

namespace SecreC {

class Context;
class SymbolKind;

/*******************************************************************************
  SecurityType
*******************************************************************************/

class SecurityType {
private:
    SecurityType (const SecurityType&); // DO NOT IMPLEMENT

public: /* Methods: */

    virtual ~SecurityType () { }
    virtual std::string toString () const = 0;
    inline bool isPrivate () const { return !m_isPublic; }
    inline bool isPublic () const { return m_isPublic; }

protected:

    explicit SecurityType (bool isPublic)
        : m_isPublic (isPublic)
    { }

private: /* Fields: */

    const bool m_isPublic;
};

inline std::ostream &operator<<(std::ostream &out, const SecurityType& type) {
    out << type.toString ();
    return out;
}

/*******************************************************************************
  PublicSecType
*******************************************************************************/

class PublicSecType : public SecurityType {
public: /* Methods: */

    PublicSecType ()
        : SecurityType (true)
    { }

    std::string toString () const;

    static PublicSecType* get (Context& cxt);
};

/*******************************************************************************
  PrivateSecType
*******************************************************************************/

class PrivateSecType : public SecurityType {
public: /* Methods: */

    PrivateSecType (const std::string& name,
                    SymbolKind* kind)
        : SecurityType (false)
        , m_name (name)
        , m_kind (kind)
    { }

    inline const std::string& name () const { return m_name; }
    inline SymbolKind* securityKind () const { return m_kind; }
    std::string toString () const;

    static PrivateSecType* get (Context& cxt, const std::string& name, SymbolKind* kind);

private: /* Fields: */

    std::string   const m_name;
    SymbolKind*   const m_kind;
};

inline bool latticeSecTypeLEQ (SecurityType* a, SecurityType* b) {
    if (a->isPublic ()) return true;
    if (b->isPublic ()) return false;
    return a == b;
}

/// \retval 0 if the upper security type is not defined
inline SecurityType* upperSecType (SecurityType* a, SecurityType* b) {
    if (a->isPublic ()) return b;
    if (b->isPublic ()) return a;
    if (a == b) return a;
    return 0;
}

} // namespace SecreC

#endif // SECREC_SECURITYTYPE_H